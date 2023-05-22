/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PMMPTHREAD_H
#define HAVE_PMMPTHREAD_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT HAVE_SOCKETS

#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#else
#define HAVE_STRUCT_TIMESPEC
#include <win32/time.h>
#include <pthread.h>
#include <signal.h>
#endif

#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_network.h>
#ifndef _WIN32
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#	ifdef HAVE_IF_NAMETOINDEX
#		include <net/if.h>
#	endif
#endif

#include <php_ticks.h>
#include <ext/standard/info.h>
#include <ext/standard/basic_functions.h>
#include <ext/standard/php_var.h>
#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
#include <ext/sockets/php_sockets.h>
#endif
#include <Zend/zend.h>
#include <Zend/zend_closures.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_inheritance.h>
#include <Zend/zend_list.h>
#include <Zend/zend_map_ptr.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_smart_str.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

extern zend_class_entry *pmmpthread_ce_thread_safe;
extern zend_class_entry *pmmpthread_ce_array;
extern zend_class_entry *pmmpthread_ce_runnable;
extern zend_class_entry *pmmpthread_ce_thread;
extern zend_class_entry *pmmpthread_ce_worker;
extern zend_class_entry *pmmpthread_ce_connection_exception;
extern zend_class_entry *pmmpthread_ce_nts_value_error;

#define IS_THREADSAFE_CLASS(c) \
	(instanceof_function(c, pmmpthread_ce_thread_safe))

#define IS_THREADSAFE_CLASS_INSTANCE(o)   \
        (Z_TYPE_P(o) == IS_OBJECT && IS_THREADSAFE_CLASS(Z_OBJCE_P(o)))

#define IS_CLOSURE_OBJECT(z) \
	(Z_TYPE_P(z) == IS_OBJECT && instanceof_function(Z_OBJCE_P(z), zend_ce_closure))

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
#define IS_EXT_SOCKETS_OBJECT(z) \
	(Z_TYPE_P(z) == IS_OBJECT && instanceof_function(Z_OBJCE_P(z), socket_ce))
#else
#define IS_EXT_SOCKETS_OBJECT(z) 0
#endif

extern zend_object_handlers pmmpthread_ts_ce_handlers;
extern zend_object_handlers pmmpthread_array_ce_handlers;
extern zend_object_handlers pmmpthread_socket_handlers;
extern zend_object_handlers *zend_handlers;

extern struct _pmmpthread_globals pmmpthread_globals;

typedef struct _pmmpthread_zend_object_t pmmpthread_zend_object_t;

ZEND_EXTERN_MODULE_GLOBALS(pmmpthread)

ZEND_BEGIN_MODULE_GLOBALS(pmmpthread)
	zval  this;
	zend_ulong options;
	HashTable resolve;
	HashTable filenames;
	HashTable closure_base_op_arrays;
	pmmpthread_zend_object_t* connecting_object;
	pmmpthread_zend_object_t* thread_shared_globals;
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
	zend_object_handlers *original_socket_object_handlers;
	zend_object_handlers custom_socket_object_handlers;
#endif
ZEND_END_MODULE_GLOBALS(pmmpthread)

#define PMMPTHREAD_ZG(v) TSRMG(pmmpthread_globals_id, zend_pmmpthread_globals *, v)

#define PMMPTHREAD_FETCH_ALL(ls, id, type) ((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])
#define PMMPTHREAD_FETCH_CTX(ls, id, type, element) (((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define PMMPTHREAD_CG(ls, v) PMMPTHREAD_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define PMMPTHREAD_CG_ALL(ls) PMMPTHREAD_FETCH_ALL(ls, compiler_globals_id, zend_compiler_globals*)
#define PMMPTHREAD_EG(ls, v) PMMPTHREAD_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#define PMMPTHREAD_SG(ls, v) PMMPTHREAD_FETCH_CTX(ls, sapi_globals_id, sapi_globals_struct*, v)
#define PMMPTHREAD_PG(ls, v) PMMPTHREAD_FETCH_CTX(ls, core_globals_id, php_core_globals*, v)
#define PMMPTHREAD_EG_ALL(ls) PMMPTHREAD_FETCH_ALL(ls, executor_globals_id, zend_executor_globals*)

#define PMMPTHREAD_MAP_PTR_OFFSET2PTR(ls, offset) \
	((void**)((char*)PMMPTHREAD_CG(ls, map_ptr_base) + offset))
#define PMMPTHREAD_MAP_PTR_PTR2OFFSET(ls, ptr) \
	((void*)(((char*)(ptr)) - ((char*)PMMPTHREAD_CG(ls, map_ptr_base))))

#if PHP_VERSION_ID >= 80200
#define PMMPTHREAD_MAP_PTR_GET(ls, ptr) \
	(ZEND_MAP_PTR_IS_OFFSET(ptr) ? \
		*PMMPTHREAD_MAP_PTR_OFFSET2PTR(ls, (uintptr_t)ZEND_MAP_PTR(ptr)) : \
		((void*)(ZEND_MAP_PTR(ptr))))
#else
#define PMMPTHREAD_MAP_PTR_GET(ls, ptr) \
	(*(ZEND_MAP_PTR_IS_OFFSET(ptr) ? \
		PMMPTHREAD_MAP_PTR_OFFSET2PTR(ls, (uintptr_t)ZEND_MAP_PTR(ptr)) : \
		((void**)(ZEND_MAP_PTR(ptr)))))
#endif

/* {{{ */
typedef struct _pmmpthread_call_t {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} pmmpthread_call_t; /* }}} */

#define PMMPTHREAD_CALL_EMPTY {empty_fcall_info, empty_fcall_info_cache}

/* this is a copy of the same struct in zend_closures.c, which unfortunately isn't exported */
typedef struct _zend_closure {
	zend_object       std;
	zend_function     func;
	zval              this_ptr;
	zend_class_entry* called_scope;
	zif_handler       orig_internal_handler;
} zend_closure;


#include <src/monitor.h>
#include <src/store.h>
#include <src/thread.h>
#include <src/worker.h>

#endif
