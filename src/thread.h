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
#ifndef HAVE_PMMPTHREAD_THREAD_H
#define HAVE_PMMPTHREAD_THREAD_H

#include <src/pmmpthread.h>
#include <src/monitor.h>
#include <src/worker.h>
#include <src/store.h>

typedef struct _pmmpthread_ident_t {
	zend_ulong id;
	void*** ls;
} pmmpthread_ident_t;

/* {{{ */
typedef struct _pmmpthread_object_t {
	zend_ulong refcount;
	pthread_t thread;
	unsigned int scope;
	pmmpthread_monitor_t monitor;
	pmmpthread_store_t props;
	pmmpthread_ident_t creator;
	pmmpthread_ident_t local;
} pmmpthread_object_t; /* }}} */

/* {{{ */
struct _pmmpthread_zend_object_t;
typedef struct _pmmpthread_zend_object_t pmmpthread_zend_object_t;

struct _pmmpthread_zend_object_t {
	pmmpthread_object_t *ts_obj;
	pmmpthread_ident_t owner;
	pmmpthread_zend_object_t *original_zobj; //NULL if this is the original object
	zend_long local_props_modcount;
	pmmpthread_worker_data_t *worker_data;
	zend_object std;
}; /* }}} */

static inline pmmpthread_zend_object_t* _pmmpthread_fetch_object(zend_object *object) {
	return (pmmpthread_zend_object_t*) ((char*)object - XtOffsetOf(pmmpthread_zend_object_t, std));
}

/* {{{ fetches the pmmpthread_zend_object_t from a zend_object */
#define PMMPTHREAD_FETCH_FROM(object) _pmmpthread_fetch_object(object) /* }}} */

/* {{{ fetches the pmmpthread_zend_object_t from $this */
#define PMMPTHREAD_FETCH PMMPTHREAD_FETCH_FROM(Z_OBJ(EX(This))) /* }}} */

/* {{{ fetches the internal thread-safe object from the zend_object */
#define PMMPTHREAD_FETCH_TS_FROM(object) PMMPTHREAD_FETCH_FROM(object)->ts_obj /* }}} */

/* {{{ fetches the internal thread-safe object from $this */
#define PMMPTHREAD_FETCH_TS PMMPTHREAD_FETCH_TS_FROM(Z_OBJ(EX(This))) /* }}} */

/* {{{ option constants */
#define PMMPTHREAD_INHERIT_NONE      0x00000000
#define PMMPTHREAD_INHERIT_INI       0x00000001
#define PMMPTHREAD_INHERIT_CONSTANTS 0x00000010
#define PMMPTHREAD_INHERIT_FUNCTIONS 0x00000100
#define PMMPTHREAD_INHERIT_CLASSES   0x00001000
#define PMMPTHREAD_INHERIT_INCLUDES  0x00010000
#define PMMPTHREAD_INHERIT_COMMENTS  0x00100000
#define PMMPTHREAD_INHERIT_ALL       0x00111111
#define PMMPTHREAD_ALLOW_HEADERS	   0x10000000 /* }}} */

/* {{{ scope constants */
#define PMMPTHREAD_SCOPE_THREAD      (1<<2)
#define PMMPTHREAD_SCOPE_WORKER      (1<<3)
/* }}} */

/* {{{ scope macros */
#define PMMPTHREAD_IS_THREAD(t)           ((t)->ts_obj->scope & PMMPTHREAD_SCOPE_THREAD)
#define PMMPTHREAD_IS_WORKER(t)           ((t)->ts_obj->scope & PMMPTHREAD_SCOPE_WORKER)
/* }}} */

/* {{{ pthread_self wrapper */
static inline zend_ulong pmmpthread_self() {
#ifdef _WIN32
	return (zend_ulong) GetCurrentThreadId();
#else
	return (zend_ulong) pthread_self();
#endif
} /* }}} */

/* {{{ tell if the calling thread created referenced PTHREAD */
#define PMMPTHREAD_IN_CREATOR(t)	((t)->ts_obj->creator.ls == TSRMLS_CACHE) /* }}} */

/* {{{ tell if the calling thread owns this pmmpthread zend object */
#define PMMPTHREAD_THREAD_OWNS(t) ((t)->owner.ls == TSRMLS_CACHE) /* }}} */

/* {{{ tell if the referenced thread is the threading context */
#define PMMPTHREAD_IN_THREAD(t)	((t)->ts_obj->local.ls == TSRMLS_CACHE) /* }}} */

#endif /* HAVE_PMMPTHREAD_THREAD_H */

