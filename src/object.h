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
#ifndef HAVE_PMMPTHREAD_OBJECT_H
#define HAVE_PMMPTHREAD_OBJECT_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <src/pmmpthread.h>

/* {{{ */
zend_object* pmmpthread_threaded_base_ctor(zend_class_entry *entry);
zend_object* pmmpthread_threaded_array_ctor(zend_class_entry *entry);
zend_object* pmmpthread_worker_ctor(zend_class_entry *entry);
zend_object* pmmpthread_thread_ctor(zend_class_entry *entry);
void         pmmpthread_base_dtor(zend_object *object);
void         pmmpthread_base_free(zend_object *object);
HashTable*   pmmpthread_base_gc(zend_object *object, zval **table, int *n);
/* }}} */

/* {{{ */
void pmmpthread_current_thread(zval *return_value); /* }}} */

/* {{{ */
zend_bool pmmpthread_object_connect(pmmpthread_zend_object_t* address, zval *object); /* }}} */

/* {{{ */
zend_object_iterator* pmmpthread_object_iterator_create(zend_class_entry *ce, zval *object, int by_ref); /* }}} */

/* {{{ */
#ifndef HAVE_PMMPTHREAD_HANDLERS_H
#	include <src/handlers.h>
#endif /* }}} */

#endif /* HAVE_PMMPTHREAD_OBJECT_H */
