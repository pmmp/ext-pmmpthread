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
#ifndef HAVE_PHP_PTHREADS
#define HAVE_PHP_PTHREADS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PHP_PTHREADS_H
#	include <php_pthreads.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

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

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

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

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

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
	zend_class_entry ce;

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

	INIT_CLASS_ENTRY(ce, "ThreadedBase", pthreads_threaded_base_methods);
	pthreads_threaded_base_entry = zend_register_internal_class(&ce);
	pthreads_threaded_base_entry->create_object = pthreads_threaded_base_ctor;
	pthreads_threaded_base_entry->serialize = pthreads_threaded_serialize;
	pthreads_threaded_base_entry->unserialize = pthreads_threaded_unserialize;
	pthreads_threaded_base_entry->get_iterator = pthreads_object_iterator_create;
	zend_class_implements(
		pthreads_threaded_base_entry,
		1,
		zend_ce_aggregate
	);
	INIT_CLASS_ENTRY(ce, "ThreadedArray", pthreads_threaded_array_methods);
	pthreads_threaded_array_entry=zend_register_internal_class_ex(&ce, pthreads_threaded_base_entry);
	pthreads_threaded_array_entry->create_object = pthreads_threaded_array_ctor;
	pthreads_threaded_array_entry->ce_flags |= ZEND_ACC_FINAL;
	pthreads_threaded_array_entry->ce_flags |= ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	INIT_CLASS_ENTRY(ce, "ThreadedConnectionException", NULL);
	pthreads_ce_ThreadedConnectionException = zend_register_internal_class_ex(&ce, spl_ce_RuntimeException);

	INIT_CLASS_ENTRY(ce, "ThreadedRunnable", pthreads_threaded_runnable_methods);
	pthreads_threaded_runnable_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_base_entry);
	pthreads_threaded_runnable_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, "Thread", pthreads_thread_methods);
	pthreads_thread_entry=zend_register_internal_class_ex(&ce, pthreads_threaded_runnable_entry);
	pthreads_thread_entry->create_object = pthreads_thread_ctor;
	pthreads_thread_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	INIT_CLASS_ENTRY(ce, "Worker", pthreads_worker_methods);
	pthreads_worker_entry=zend_register_internal_class_ex(&ce, pthreads_thread_entry);
	pthreads_worker_entry->create_object = pthreads_worker_ctor;

	INIT_CLASS_ENTRY(ce, "Pool", pthreads_pool_methods);
	pthreads_pool_entry=zend_register_internal_class(&ce);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("size"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("class"),   ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("workers"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("ctor"),    ZEND_ACC_PROTECTED);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("last"), 0, ZEND_ACC_PROTECTED);

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
	pthreads_threaded_array_handlers.read_property = pthreads_read_property_disallow;
	pthreads_threaded_array_handlers.write_property = pthreads_write_property_disallow;
	pthreads_threaded_array_handlers.has_property = pthreads_has_property_disallow;
	pthreads_threaded_array_handlers.unset_property = pthreads_unset_property_disallow;

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

	return SUCCESS;
}

PHP_RINIT_FUNCTION(pthreads) {
	ZEND_TSRMLS_CACHE_UPDATE();

	zend_hash_init(&PTHREADS_ZG(resolve), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(filenames), 15, NULL, NULL, 0);

	PTHREADS_ZG(hard_copy_interned_strings) = 0;

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
	zend_hash_destroy(&PTHREADS_ZG(resolve));
	zend_hash_destroy(&PTHREADS_ZG(filenames));

	return SUCCESS;
}

PHP_MINFO_FUNCTION(pthreads)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_PTHREADS_VERSION);
	php_info_print_table_end();
}

#include <classes/threaded_base.h>
#include <classes/threaded_runnable.h>

#ifndef HAVE_PTHREADS_CLASS_THREADED_ARRAY
#	include <classes/threaded_array.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_THREAD
#	include <classes/thread.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_WORKER
#	include <classes/worker.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_POOL
#	include <classes/pool.h>
#endif

#endif
