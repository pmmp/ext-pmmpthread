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

#include <src/store_types.h>
#include <src/thread.h>

#define PTHREADS_STORE_COERCE_ARRAY 1
#define PTHREADS_STORE_NO_COERCE_ARRAY 0

#define TRY_PTHREADS_STORAGE_PTR_P(zval) ((zval) != NULL && Z_TYPE_P(zval) == IS_PTR ? (pthreads_storage *) Z_PTR_P(zval) : NULL)

pthreads_store_t* pthreads_store_alloc();
void pthreads_store_sync_local_properties(pthreads_zend_object_t *threaded);
int pthreads_store_merge(zend_object *destination, zval *from, zend_bool overwrite, zend_bool coerce_array_to_threaded);
int pthreads_store_delete(zend_object *object, zval *key);
int pthreads_store_read(zend_object *object, zval *key, int type, zval *read);
zend_bool pthreads_store_isset(zend_object *object, zval *key, int has_set_exists);
int pthreads_store_write(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded);
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

/* {{{ */
void pthreads_store_save_zval(zval *zstorage, zval *write);
void pthreads_store_restore_zval_ex(zval *unstore, zval *zstorage, zend_bool *was_pthreads_storage);
void pthreads_store_restore_zval(zval *unstore, zval *zstorage); /* }}} */
void pthreads_store_storage_dtor(zval *element);

#endif
