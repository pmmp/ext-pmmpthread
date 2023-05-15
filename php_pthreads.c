/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
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

#include <src/pthreads.h>
#include <src/object.h>

#include <stubs/Pool_arginfo.h>
#include <stubs/Thread_arginfo.h>
#include <stubs/ThreadSafeArray_arginfo.h>
#include <stubs/ThreadSafe_arginfo.h>
#include <stubs/Runnable_arginfo.h>
#include <stubs/ConnectionException_arginfo.h>
#include <stubs/Worker_arginfo.h>

#include <php_pthreads.h>


#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if PHP_VERSION_ID < 80100
#	error "pthreads requires PHP 8.1 or later"
#endif

#if COMPILE_DL_PTHREADS
	ZEND_TSRMLS_CACHE_DEFINE();
	ZEND_GET_MODULE(pthreads)
#endif

#include <src/globals.h>

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
#include <src/ext_sockets_hacks.h>
#endif

static const zend_module_dep pthreads_module_deps[] = {
	ZEND_MOD_REQUIRED("spl")
#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
	ZEND_MOD_REQUIRED("sockets")
#endif
	ZEND_MOD_END
};

zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER_EX,
  NULL,
  pthreads_module_deps,
  PHP_PTHREADS_EXTNAME,
  NULL,
  PHP_MINIT(pthreads),
  PHP_MSHUTDOWN(pthreads),
  PHP_RINIT(pthreads),
  PHP_RSHUTDOWN(pthreads),
  PHP_MINFO(pthreads),
  PHP_PTHREADS_VERSION,
  NO_MODULE_GLOBALS,
  ZEND_MODULE_POST_ZEND_DEACTIVATE_N(pthreads),
  STANDARD_MODULE_PROPERTIES_EX
};

zend_class_entry *pthreads_ce_thread_safe;
zend_class_entry *pthreads_ce_array;
zend_class_entry *pthreads_ce_runnable;
zend_class_entry *pthreads_ce_thread;
zend_class_entry *pthreads_ce_worker;
zend_class_entry *pthreads_ce_pool;
zend_class_entry *pthreads_ce_connection_exception;

zend_object_handlers pthreads_ts_ce_handlers;
zend_object_handlers pthreads_array_ce_handlers;
zend_object_handlers *zend_handlers;
void ***pthreads_instance = NULL;

ZEND_DECLARE_MODULE_GLOBALS(pthreads)

typedef struct _pthreads_supported_sapi_t {
	const char *name;
	size_t      nlen;
} pthreads_supported_sapi_t;

const static pthreads_supported_sapi_t whitelist[] = {
	{ZEND_STRL("cli")},
	{ZEND_STRL("phpdbg")}, /* not really supported, needs work */
	{ZEND_STRL("homegear")},
	{NULL, 0}
};

static inline zend_bool pthreads_is_supported_sapi(char *name) {
	const pthreads_supported_sapi_t *sapi = whitelist;
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

static inline void pthreads_globals_ctor(zend_pthreads_globals *pg) {
	ZVAL_UNDEF(&pg->this);
}

PHP_MINIT_FUNCTION(pthreads)
{
	if (!pthreads_is_supported_sapi(sapi_module.name)) {
		zend_error(E_ERROR, "The %s SAPI is not supported by pthreads",
			sapi_module.name);
		return FAILURE;
	}

	pthreads_ce_thread_safe = register_class_pmmp_thread_ThreadSafe(zend_ce_aggregate);
	pthreads_ce_thread_safe->create_object = pthreads_threaded_base_ctor;
	pthreads_ce_thread_safe->get_iterator = pthreads_object_iterator_create;
	pthreads_ce_thread_safe->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;

	pthreads_ce_array = register_class_pmmp_thread_ThreadSafeArray(pthreads_ce_thread_safe, zend_ce_countable, zend_ce_arrayaccess);
	pthreads_ce_array->create_object = pthreads_threaded_array_ctor;

	pthreads_ce_connection_exception = register_class_pmmp_thread_ConnectionException(spl_ce_RuntimeException);

	pthreads_ce_runnable = register_class_pmmp_thread_Runnable(pthreads_ce_thread_safe);

	pthreads_ce_thread = register_class_pmmp_thread_Thread(pthreads_ce_runnable);
	pthreads_ce_thread->create_object = pthreads_thread_ctor;

	pthreads_ce_worker = register_class_pmmp_thread_Worker(pthreads_ce_thread);
	pthreads_ce_worker->create_object = pthreads_worker_ctor;

	pthreads_ce_pool = register_class_pmmp_thread_Pool();

	/*
	* Setup object handlers
	*/
	zend_handlers = (zend_object_handlers*)zend_get_std_object_handlers();

	memcpy(&pthreads_ts_ce_handlers, zend_handlers, sizeof(zend_object_handlers));

	pthreads_ts_ce_handlers.offset = XtOffsetOf(pthreads_zend_object_t, std);

	pthreads_ts_ce_handlers.free_obj = pthreads_base_free;
	pthreads_ts_ce_handlers.dtor_obj = pthreads_base_dtor;
	pthreads_ts_ce_handlers.cast_object = pthreads_cast_object;

	pthreads_ts_ce_handlers.get_debug_info = pthreads_read_debug;
	pthreads_ts_ce_handlers.get_properties = pthreads_read_properties;

	pthreads_ts_ce_handlers.read_property = pthreads_read_property;
	pthreads_ts_ce_handlers.write_property = pthreads_write_property;
	pthreads_ts_ce_handlers.has_property = pthreads_has_property;
	pthreads_ts_ce_handlers.unset_property = pthreads_unset_property;


	pthreads_ts_ce_handlers.get_property_ptr_ptr = pthreads_get_property_ptr_ptr_stub;
	pthreads_ts_ce_handlers.get_gc = pthreads_base_gc;

	pthreads_ts_ce_handlers.clone_obj = NULL;
	pthreads_ts_ce_handlers.compare = pthreads_compare_objects;

	memcpy(&pthreads_array_ce_handlers, &pthreads_ts_ce_handlers, sizeof(zend_object_handlers));
	pthreads_array_ce_handlers.count_elements = pthreads_count_properties;
	pthreads_array_ce_handlers.read_dimension = pthreads_read_dimension;
	pthreads_array_ce_handlers.write_dimension = pthreads_write_dimension;
	pthreads_array_ce_handlers.has_dimension = pthreads_has_dimension;
	pthreads_array_ce_handlers.unset_dimension = pthreads_unset_dimension;
	pthreads_array_ce_handlers.read_property = pthreads_read_property_deny;
	pthreads_array_ce_handlers.write_property = pthreads_write_property_deny;
	pthreads_array_ce_handlers.has_property = pthreads_has_property_deny;
	pthreads_array_ce_handlers.unset_property = pthreads_unset_property_deny;

	ZEND_INIT_MODULE_GLOBALS(pthreads, pthreads_globals_ctor, NULL);

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
	pthreads_ext_sockets_hacks_init();
#endif

	if (pthreads_globals_init()) {
		TSRMLS_CACHE_UPDATE();

		/*
		* Global Init
		*/
		pthreads_instance = TSRMLS_CACHE;
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

PHP_MSHUTDOWN_FUNCTION(pthreads)
{
	if (pthreads_instance == TSRMLS_CACHE) {
		pthreads_globals_shutdown();

		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = sapi_cli_deactivate;
		}
	}

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pthreads)
{
	zend_hash_destroy(&PTHREADS_ZG(resolve));

	return SUCCESS;
}

PHP_RINIT_FUNCTION(pthreads) {
	ZEND_TSRMLS_CACHE_UPDATE();

	zend_hash_init(&PTHREADS_ZG(resolve), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(filenames), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(closure_base_op_arrays), 15, NULL, ZEND_FUNCTION_DTOR, 0);

	PTHREADS_ZG(options) = PTHREADS_INHERIT_ALL;
	PTHREADS_ZG(connecting_object) = NULL;

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
	PTHREADS_ZG(original_socket_object_handlers) = NULL;
#endif

	if (pthreads_instance != TSRMLS_CACHE) {
		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = NULL;
		}
	}

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pthreads) {
	zend_hash_destroy(&PTHREADS_ZG(filenames));
	zend_hash_destroy(&PTHREADS_ZG(closure_base_op_arrays));

	return SUCCESS;
}

PHP_MINFO_FUNCTION(pthreads)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_PTHREADS_VERSION);
	php_info_print_table_end();
}
