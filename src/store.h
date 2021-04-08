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
#ifndef HAVE_PTHREADS_STORE_H
#define HAVE_PTHREADS_STORE_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#define IS_CLOSURE  (IS_PTR + 1)
#define IS_PTHREADS (IS_PTR + 2)

typedef HashTable pthreads_store_t;

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

/* this is a copy of the same struct in zend_closures.c, which unfortunately isn't exported */
typedef struct _zend_closure {
	zend_object       std;
	zend_function     func;
	zval              this_ptr;
	zend_class_entry *called_scope;
	zif_handler       orig_internal_handler;
} zend_closure;

pthreads_store_t* pthreads_store_alloc();
void pthreads_store_sync(zend_object *object);
int pthreads_store_merge(zend_object *destination, zval *from, zend_bool overwrite);
int pthreads_store_delete(zend_object *object, zval *key);
int pthreads_store_read(zend_object *object, zval *key, int type, zval *read);
zend_bool pthreads_store_isset(zend_object *object, zval *key, int has_set_exists);
int pthreads_store_write(zend_object *object, zval *key, zval *write);
int pthreads_store_separate(zval *pzval, zval *seperated);
void pthreads_store_tohash(zend_object *object, HashTable *hash);
int pthreads_store_shift(zend_object *object, zval *member);
int pthreads_store_chunk(zend_object *object, zend_long size, zend_bool preserve, zval *chunk);
int pthreads_store_pop(zend_object *object, zval *member);
int pthreads_store_count(zend_object *object, zend_long *count);
void pthreads_store_free(pthreads_store_t *store);

/* {{{ * iteration helpers */
void pthreads_store_reset(zend_object *object, HashPosition *position);
void pthreads_store_key(zend_object *object, zval *key, HashPosition *position);
void pthreads_store_data(zend_object *object, zval *value, HashPosition *position);
void pthreads_store_forward(zend_object *object, HashPosition *position); /* }}} */

pthreads_storage* pthreads_store_create(zval *pzval);
int pthreads_store_convert(pthreads_storage *storage, zval *pzval);
void pthreads_store_storage_dtor(pthreads_storage *element);

#endif
