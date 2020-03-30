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
#ifndef HAVE_PTHREADS_STORE_TYPES_H
#define HAVE_PTHREADS_STORE_TYPES_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

typedef struct _pthreads_store_t {
	HashTable hash;
	zend_long modcount;
} pthreads_store_t;

typedef struct _pthreads_storage {
	zend_uchar 	type;
	size_t 	length;
	zend_bool 	exists;
	union {
		zend_long   lval;
		double     dval;
	} simple;
	void    	*data;
} pthreads_storage;

#endif
