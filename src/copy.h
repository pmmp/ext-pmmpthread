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
#ifndef HAVE_PMMPTHREAD_COPY_H
#define HAVE_PMMPTHREAD_COPY_H

#include <src/pmmpthread.h>
#include <src/thread.h>
#include <Zend/zend_attributes.h>
#include <src/prepare.h>

/* {{{ */
zend_string* pmmpthread_copy_string(zend_string* s); /* }}} */

/* {{{ */
int pmmpthread_copy_zval(const pmmpthread_ident_t* owner, zval* dest, zval* source); /* }}} */

/* {{{ */
HashTable* pmmpthread_copy_attributes(const pmmpthread_ident_t* owner, HashTable *attributes, zend_string *filename); /* }}} */

/* {{{ */
zend_function* pmmpthread_copy_function(const pmmpthread_ident_t* owner, const zend_function *function); /* }}} */

/* {{{ */
zend_result pmmpthread_copy_closure(const pmmpthread_ident_t* owner, zend_closure* closure_obj, zend_bool silent, zval *pzval); /* }}} */

/* {{{ */
zend_result pmmpthread_resolve_enum_reference(zend_class_entry* enum_ce, zend_string* case_name, zval* pzval); /* }}} */
#endif

