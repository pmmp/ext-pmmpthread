/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                         |
  +----------------------------------------------------------------------+
 */

#include <src/pmmpthread.h>
#include <src/object.h>
#include <src/globals.h>

#include <stubs/Pool_arginfo.h>
#include <stubs/Thread_arginfo.h>
#include <stubs/ThreadSafeArray_arginfo.h>
#include <stubs/ThreadSafe_arginfo.h>
#include <stubs/Runnable_arginfo.h>
#include <stubs/ConnectionException_arginfo.h>
#include <stubs/Worker_arginfo.h>

#include <php_pmmpthread.h>


#ifndef ZTS
#	error "pmmpthread requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if PHP_VERSION_ID < 80100
#	error "pmmpthread requires PHP 8.1 or later"
#endif

#if COMPILE_DL_PMMPTHREAD
	ZEND_TSRMLS_CACHE_DEFINE();
	ZEND_GET_MODULE(pmmpthread)
#endif

#include <src/globals.h>

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
#include <src/ext_sockets_hacks.h>
#endif

static const zend_module_dep pmmpthread_module_deps[] = {
	ZEND_MOD_REQUIRED("spl")
	ZEND_MOD_CONFLICTS("pthreads")
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
	ZEND_MOD_REQUIRED("sockets")
#endif
	ZEND_MOD_END
};

zend_module_entry pmmpthread_module_entry = {
  STANDARD_MODULE_HEADER_EX,
  NULL,
  pmmpthread_module_deps,
  PHP_PMMPTHREAD_EXTNAME,
  NULL,
  PHP_MINIT(pmmpthread),
  PHP_MSHUTDOWN(pmmpthread),
  PHP_RINIT(pmmpthread),
  PHP_RSHUTDOWN(pmmpthread),
  PHP_MINFO(pmmpthread),
  PHP_PMMPTHREAD_VERSION,
  NO_MODULE_GLOBALS,
  ZEND_MODULE_POST_ZEND_DEACTIVATE_N(pmmpthread),
  STANDARD_MODULE_PROPERTIES_EX
};

zend_class_entry *pmmpthread_ce_thread_safe;
zend_class_entry *pmmpthread_ce_array;
zend_class_entry *pmmpthread_ce_runnable;
zend_class_entry *pmmpthread_ce_thread;
zend_class_entry *pmmpthread_ce_worker;
zend_class_entry *pmmpthread_ce_pool;
zend_class_entry *pmmpthread_ce_connection_exception;

zend_object_handlers pmmpthread_ts_ce_handlers;
zend_object_handlers pmmpthread_array_ce_handlers;
zend_object_handlers *zend_handlers;
void ***pmmpthread_instance = NULL;

ZEND_DECLARE_MODULE_GLOBALS(pmmpthread)

typedef struct _pmmpthread_supported_sapi_t {
	const char *name;
	size_t      nlen;
} pmmpthread_supported_sapi_t;

const static pmmpthread_supported_sapi_t whitelist[] = {
	{ZEND_STRL("cli")},
	{ZEND_STRL("phpdbg")}, /* not really supported, needs work */
	{ZEND_STRL("homegear")},
	{NULL, 0}
};

static inline zend_bool pmmpthread_is_supported_sapi(char *name) {
	const pmmpthread_supported_sapi_t *sapi = whitelist;
	zend_long nlen = strlen(name);

	while (sapi->name) {
		if (nlen == sapi->nlen &&
			memcmp(sapi->name, name, nlen) == SUCCESS) {
			return 1;
		}
		sapi++;
	}

	return 0;
}

static inline void pmmpthread_globals_ctor(zend_pmmpthread_globals *pg) {
	ZVAL_UNDEF(&pg->this);
}

PHP_MINIT_FUNCTION(pmmpthread)
{
	if (!pmmpthread_is_supported_sapi(sapi_module.name)) {
		zend_error(E_ERROR, "The %s SAPI is not supported by pmmpthread",
			sapi_module.name);
		return FAILURE;
	}

	pmmpthread_ce_thread_safe = register_class_pmmp_thread_ThreadSafe(zend_ce_aggregate);
	pmmpthread_ce_thread_safe->create_object = pmmpthread_threaded_base_ctor;
	pmmpthread_ce_thread_safe->get_iterator = pmmpthread_object_iterator_create;
	pmmpthread_ce_thread_safe->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;

	pmmpthread_ce_array = register_class_pmmp_thread_ThreadSafeArray(pmmpthread_ce_thread_safe, zend_ce_countable, zend_ce_arrayaccess);
	pmmpthread_ce_array->create_object = pmmpthread_threaded_array_ctor;

	pmmpthread_ce_connection_exception = register_class_pmmp_thread_ConnectionException(spl_ce_RuntimeException);

	pmmpthread_ce_runnable = register_class_pmmp_thread_Runnable(pmmpthread_ce_thread_safe);

	pmmpthread_ce_thread = register_class_pmmp_thread_Thread(pmmpthread_ce_runnable);
	pmmpthread_ce_thread->create_object = pmmpthread_thread_ctor;

	pmmpthread_ce_worker = register_class_pmmp_thread_Worker(pmmpthread_ce_thread);
	pmmpthread_ce_worker->create_object = pmmpthread_worker_ctor;

	pmmpthread_ce_pool = register_class_pmmp_thread_Pool();

	/*
	* Setup object handlers
	*/
	zend_handlers = (zend_object_handlers*)zend_get_std_object_handlers();

	memcpy(&pmmpthread_ts_ce_handlers, zend_handlers, sizeof(zend_object_handlers));

	pmmpthread_ts_ce_handlers.offset = XtOffsetOf(pmmpthread_zend_object_t, std);

	pmmpthread_ts_ce_handlers.free_obj = pmmpthread_base_free;
	pmmpthread_ts_ce_handlers.dtor_obj = pmmpthread_base_dtor;
	pmmpthread_ts_ce_handlers.cast_object = pmmpthread_cast_object;

	pmmpthread_ts_ce_handlers.get_debug_info = pmmpthread_read_debug;
	pmmpthread_ts_ce_handlers.get_properties = pmmpthread_read_properties;

	pmmpthread_ts_ce_handlers.read_property = pmmpthread_read_property;
	pmmpthread_ts_ce_handlers.write_property = pmmpthread_write_property;
	pmmpthread_ts_ce_handlers.has_property = pmmpthread_has_property;
	pmmpthread_ts_ce_handlers.unset_property = pmmpthread_unset_property;


	pmmpthread_ts_ce_handlers.get_property_ptr_ptr = pmmpthread_get_property_ptr_ptr_stub;
	pmmpthread_ts_ce_handlers.get_gc = pmmpthread_base_gc;

	pmmpthread_ts_ce_handlers.clone_obj = NULL;
	pmmpthread_ts_ce_handlers.compare = pmmpthread_compare_objects;

	memcpy(&pmmpthread_array_ce_handlers, &pmmpthread_ts_ce_handlers, sizeof(zend_object_handlers));
	pmmpthread_array_ce_handlers.count_elements = pmmpthread_count_properties;
	pmmpthread_array_ce_handlers.read_dimension = pmmpthread_read_dimension;
	pmmpthread_array_ce_handlers.write_dimension = pmmpthread_write_dimension;
	pmmpthread_array_ce_handlers.has_dimension = pmmpthread_has_dimension;
	pmmpthread_array_ce_handlers.unset_dimension = pmmpthread_unset_dimension;
	pmmpthread_array_ce_handlers.read_property = pmmpthread_read_property_deny;
	pmmpthread_array_ce_handlers.write_property = pmmpthread_write_property_deny;
	pmmpthread_array_ce_handlers.has_property = pmmpthread_has_property_deny;
	pmmpthread_array_ce_handlers.unset_property = pmmpthread_unset_property_deny;

	ZEND_INIT_MODULE_GLOBALS(pmmpthread, pmmpthread_globals_ctor, NULL);

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
	pmmpthread_ext_sockets_hacks_init();
#endif

	if (pmmpthread_globals_init()) {
		TSRMLS_CACHE_UPDATE();

		/*
		* Global Init
		*/
		pmmpthread_instance = TSRMLS_CACHE;
	}

	return SUCCESS;
}

static inline int sapi_cli_deactivate(void)
{
	fflush(stdout);
	if (SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pmmpthread)
{
	if (pmmpthread_instance == TSRMLS_CACHE) {
		pmmpthread_globals_shutdown();

		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = sapi_cli_deactivate;
		}
	}

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pmmpthread)
{
	zend_hash_destroy(&PMMPTHREAD_ZG(resolve));

	return SUCCESS;
}


static zend_result pmmpthread_init_thread_shared_globals() {
	zend_result result = FAILURE;

	if (pmmpthread_globals_lock()) {
		zval globals_array;

		if (PMMPTHREAD_G(thread_shared_globals)) { //this is not the main thread - connect to existing globals
			if (pmmpthread_object_connect(PMMPTHREAD_G(thread_shared_globals), &globals_array) == 0) {
				ZEND_ASSERT(0);
				result = FAILURE;
			} else {
				result = SUCCESS;
			}
		} else { //this is the main thread - create new globals
			if (object_init_ex(&globals_array, pmmpthread_ce_array) == FAILURE) {
				ZEND_ASSERT(0);
				result = FAILURE;
			} else {
				PMMPTHREAD_G(thread_shared_globals) = PMMPTHREAD_FETCH_FROM(Z_OBJ(globals_array));
				result = SUCCESS;
			}
		}

		pmmpthread_globals_unlock();

		if (result == SUCCESS) {
			PMMPTHREAD_ZG(thread_shared_globals) = PMMPTHREAD_FETCH_FROM(Z_OBJ(globals_array));
			Z_ADDREF(globals_array);
		}
	}

	return result;
}

PHP_RINIT_FUNCTION(pmmpthread) {
	ZEND_TSRMLS_CACHE_UPDATE();

	zend_hash_init(&PMMPTHREAD_ZG(resolve), 15, NULL, NULL, 0);
	zend_hash_init(&PMMPTHREAD_ZG(filenames), 15, NULL, NULL, 0);
	zend_hash_init(&PMMPTHREAD_ZG(closure_base_op_arrays), 15, NULL, ZEND_FUNCTION_DTOR, 0);

	PMMPTHREAD_ZG(options) = PMMPTHREAD_INHERIT_ALL;
	PMMPTHREAD_ZG(connecting_object) = NULL;

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
	PMMPTHREAD_ZG(original_socket_object_handlers) = NULL;
#endif

	if (pmmpthread_instance != TSRMLS_CACHE) {
		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = NULL;
		}
	}

	if (pmmpthread_init_thread_shared_globals() == FAILURE) {
		return FAILURE;
	}

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pmmpthread) {
	zend_hash_destroy(&PMMPTHREAD_ZG(filenames));
	zend_hash_destroy(&PMMPTHREAD_ZG(closure_base_op_arrays));

	pmmpthread_zend_object_t* ts_globals = PMMPTHREAD_ZG(thread_shared_globals);
	if (PMMPTHREAD_IN_CREATOR(ts_globals)) {
		//this is the main thread (we created these globals), but we may be in a special opcache preload "request"
		//clean up globals so they don't break the context that follows
		if (pmmpthread_globals_lock()) {
			PMMPTHREAD_G(thread_shared_globals) = NULL;
			pmmpthread_globals_unlock();
		}
	}
	zend_object_release(&ts_globals->std);

	return SUCCESS;
}

PHP_MINFO_FUNCTION(pmmpthread)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_PMMPTHREAD_VERSION);
	php_info_print_table_end();
}
