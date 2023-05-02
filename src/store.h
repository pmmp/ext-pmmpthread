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

#include <src/pthreads.h>

#define PTHREADS_STORE_COERCE_ARRAY 1
#define PTHREADS_STORE_NO_COERCE_ARRAY 0

#define TRY_PTHREADS_STORAGE_PTR_P(zval) ((zval) != NULL && Z_TYPE_P(zval) == IS_PTR ? (pthreads_storage *) Z_PTR_P(zval) : NULL)

typedef struct _pthreads_store_t {
	HashTable hash;
	zend_long modcount;
} pthreads_store_t;

typedef enum {
	WRITE_SUCCESS = 0,
	WRITE_FAIL_UNKNOWN = -1,
	WRITE_FAIL_NOT_THREAD_SAFE = -2,
	WRITE_FAIL_WOULD_OVERWRITE = -3
} pthreads_store_write_result;

void pthreads_store_init(pthreads_store_t* store);
void pthreads_store_destroy(pthreads_store_t* store);
void pthreads_store_sync_local_properties(zend_object* object);
void pthreads_store_full_sync_local_properties(zend_object *object);
int pthreads_store_merge(zend_object *destination, zval *from, zend_bool overwrite, zend_bool coerce_array_to_threaded);
int pthreads_store_delete(zend_object *object, zval *key);
int pthreads_store_read(zend_object *object, zval *key, int type, zval *read);
int pthreads_store_read_local_property(zend_object* object, zend_string* key, int type, zval* read);
zend_bool pthreads_store_isset(zend_object *object, zval *key, int has_set_exists);
int pthreads_store_write(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded);
pthreads_store_write_result pthreads_store_write_ex(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded, zend_bool overwrite);
void pthreads_store_tohash(zend_object *object, HashTable *hash);
int pthreads_store_shift(zend_object *object, zval *member);
int pthreads_store_chunk(zend_object *object, zend_long size, zend_bool preserve, zval *chunk);
int pthreads_store_pop(zend_object *object, zval *member);
int pthreads_store_count(zend_object *object, zend_long *count);
/* {{{ Copies any thread-local data to permanent storage when an object ref is destroyed */
void pthreads_store_persist_local_properties(zend_object* object); /* }}} */

/* {{{ * iteration helpers */
void pthreads_store_reset(zend_object *object, HashPosition *position);
void pthreads_store_key(zend_object *object, zval *key, HashPosition *position);
void pthreads_store_data(zend_object *object, zval *value, HashPosition *position);
void pthreads_store_forward(zend_object *object, HashPosition *position); /* }}} */

#endif
