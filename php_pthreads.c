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
#include <stubs/ThreadedArray_arginfo.h>
#include <stubs/ThreadedBase_arginfo.h>
#include <stubs/ThreadedRunnable_arginfo.h>
#include <stubs/ThreadedConnectionException_arginfo.h>
#include <stubs/Worker_arginfo.h>

#include <php_pthreads.h>


#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if PHP_VERSION_ID < 80000
#	error "pthreads requires PHP 8.0 or later"
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

zend_class_entry *pthreads_threaded_base_entry;
zend_class_entry *pthreads_threaded_array_entry;
zend_class_entry *pthreads_threaded_runnable_entry;
zend_class_entry *pthreads_thread_entry;
zend_class_entry *pthreads_worker_entry;
zend_class_entry *pthreads_pool_entry;
zend_class_entry *pthreads_ce_ThreadedConnectionException;

zend_object_handlers pthreads_threaded_base_handlers;
zend_object_handlers pthreads_threaded_array_handlers;
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
	pg->pid = 0L;
	pg->signal = 0;
	pg->resources = NULL;
}

PHP_MINIT_FUNCTION(pthreads)
{
	if (!pthreads_is_supported_sapi(sapi_module.name)) {
		zend_error(E_ERROR, "The %s SAPI is not supported by pthreads",
			sapi_module.name);
		return FAILURE;
	}

	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_ALL", PTHREADS_INHERIT_ALL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_NONE", PTHREADS_INHERIT_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INI", PTHREADS_INHERIT_INI, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CONSTANTS", PTHREADS_INHERIT_CONSTANTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CLASSES", PTHREADS_INHERIT_CLASSES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_FUNCTIONS", PTHREADS_INHERIT_FUNCTIONS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INCLUDES", PTHREADS_INHERIT_INCLUDES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_COMMENTS", PTHREADS_INHERIT_COMMENTS, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("PTHREADS_ALLOW_HEADERS", PTHREADS_ALLOW_HEADERS, CONST_CS | CONST_PERSISTENT);

	pthreads_threaded_base_entry = register_class_ThreadedBase(zend_ce_aggregate);
	pthreads_threaded_base_entry->create_object = pthreads_threaded_base_ctor;
	pthreads_threaded_base_entry->get_iterator = pthreads_object_iterator_create;
#if PHP_VERSION_ID < 80100
	pthreads_threaded_base_entry->serialize = zend_class_serialize_deny;
	pthreads_threaded_base_entry->unserialize = zend_class_unserialize_deny;
#else
	pthreads_threaded_base_entry->ce_flags |= ZEND_ACC_NOT_SERIALIZABLE;
#endif

	pthreads_threaded_array_entry = register_class_ThreadedArray(pthreads_threaded_base_entry, zend_ce_countable, zend_ce_arrayaccess);
	pthreads_threaded_array_entry->create_object = pthreads_threaded_array_ctor;

	pthreads_ce_ThreadedConnectionException = register_class_ThreadedConnectionException(spl_ce_RuntimeException);

	pthreads_threaded_runnable_entry = register_class_ThreadedRunnable(pthreads_threaded_base_entry);

	pthreads_thread_entry = register_class_Thread(pthreads_threaded_runnable_entry);
	pthreads_thread_entry->create_object = pthreads_thread_ctor;

	pthreads_worker_entry = register_class_Worker(pthreads_thread_entry);
	pthreads_worker_entry->create_object = pthreads_worker_ctor;

	pthreads_pool_entry = register_class_Pool();

	/*
	* Setup object handlers
	*/
	zend_handlers = (zend_object_handlers*)zend_get_std_object_handlers();

	memcpy(&pthreads_threaded_base_handlers, zend_handlers, sizeof(zend_object_handlers));

	pthreads_threaded_base_handlers.offset = XtOffsetOf(pthreads_zend_object_t, std);

	pthreads_threaded_base_handlers.free_obj = pthreads_base_free;
	pthreads_threaded_base_handlers.dtor_obj = pthreads_base_dtor;
	pthreads_threaded_base_handlers.cast_object = pthreads_cast_object;

	pthreads_threaded_base_handlers.get_debug_info = pthreads_read_debug;
	pthreads_threaded_base_handlers.get_properties = pthreads_read_properties;

	pthreads_threaded_base_handlers.read_property = pthreads_read_property;
	pthreads_threaded_base_handlers.write_property = pthreads_write_property;
	pthreads_threaded_base_handlers.has_property = pthreads_has_property;
	pthreads_threaded_base_handlers.unset_property = pthreads_unset_property;


	pthreads_threaded_base_handlers.get_property_ptr_ptr = pthreads_get_property_ptr_ptr_stub;
	pthreads_threaded_base_handlers.get_gc = pthreads_base_gc;

	pthreads_threaded_base_handlers.clone_obj = NULL;
	pthreads_threaded_base_handlers.compare = pthreads_compare_objects;

	memcpy(&pthreads_threaded_array_handlers, &pthreads_threaded_base_handlers, sizeof(zend_object_handlers));
	pthreads_threaded_array_handlers.count_elements = pthreads_count_properties;
	pthreads_threaded_array_handlers.read_dimension = pthreads_read_dimension;
	pthreads_threaded_array_handlers.write_dimension = pthreads_write_dimension;
	pthreads_threaded_array_handlers.has_dimension = pthreads_has_dimension;
	pthreads_threaded_array_handlers.unset_dimension = pthreads_unset_dimension;

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
	if (PTHREADS_ZG(resources)) {
		zend_hash_destroy(PTHREADS_ZG(resources));
		FREE_HASHTABLE(PTHREADS_ZG(resources));
		PTHREADS_ZG(resources) = NULL;
	}
	zend_hash_destroy(&PTHREADS_ZG(resolve));

	return SUCCESS;
}

PHP_RINIT_FUNCTION(pthreads) {
	ZEND_TSRMLS_CACHE_UPDATE();

	zend_hash_init(&PTHREADS_ZG(resolve), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(filenames), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(closure_base_op_arrays), 15, NULL, ZEND_FUNCTION_DTOR, 0);

	PTHREADS_ZG(hard_copy_interned_strings) = 0;
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
