/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
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
#ifndef HAVE_PTHREADS_COPY_H
#define HAVE_PTHREADS_COPY_H

#include <src/pthreads.h>
#include <src/thread.h>
#include <Zend/zend_attributes.h>

/* {{{ */
int pthreads_copy_zval(const pthreads_ident_t* owner, zval* dest, zval* source); /* }}} */

/* {{{ */
HashTable* pthreads_copy_attributes(const pthreads_ident_t* owner, HashTable *attributes, zend_string *filename); /* }}} */

/* {{{ */
zend_function* pthreads_copy_function(const pthreads_ident_t* owner, const zend_function *function); /* }}} */

#endif

