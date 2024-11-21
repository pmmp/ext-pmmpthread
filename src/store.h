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
#ifndef HAVE_PMMPTHREAD_STORE_H
#define HAVE_PMMPTHREAD_STORE_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include <src/pmmpthread.h>

#define PMMPTHREAD_STORE_COERCE_ARRAY 1
#define PMMPTHREAD_STORE_NO_COERCE_ARRAY 0

#define TRY_PMMPTHREAD_STORAGE_PTR_P(zval) ((zval) != NULL && Z_TYPE_P(zval) == IS_PTR ? (pmmpthread_storage *) Z_PTR_P(zval) : NULL)

typedef struct _pmmpthread_store_t {
	HashTable hash;
	zend_long modcount;
	HashPosition first;
	HashPosition last;
} pmmpthread_store_t;

void pmmpthread_store_init(pmmpthread_store_t* store);
void pmmpthread_store_destroy(pmmpthread_store_t* store);
void pmmpthread_store_sync_local_properties(zend_object* object);
void pmmpthread_store_full_sync_local_properties(zend_object *object);
int pmmpthread_store_merge(zend_object *destination, zval *from, zend_bool overwrite, zend_bool coerce_array_to_threaded);
int pmmpthread_store_delete(zend_object *object, zval *key);
int pmmpthread_store_read(zend_object *object, zval *key, int type, zval *read);
zend_bool pmmpthread_store_isset(zend_object *object, zval *key, int has_set_exists);
int pmmpthread_store_write(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded);
void pmmpthread_store_tohash(zend_object *object, HashTable *hash);
int pmmpthread_store_shift(zend_object *object, zval *member);
int pmmpthread_store_chunk(zend_object *object, zend_long size, zend_bool preserve, zval *chunk);
int pmmpthread_store_pop(zend_object *object, zval *member);
int pmmpthread_store_count(zend_object *object, zend_long *count);
/* {{{ Copies any thread-local data to permanent storage when an object ref is destroyed */
void pmmpthread_store_persist_local_properties(zend_object* object); /* }}} */

/* {{{ * iteration helpers */
void pmmpthread_store_reset(zend_object *object, HashPosition *position);
void pmmpthread_store_key(zend_object *object, zval *key, HashPosition *position);
void pmmpthread_store_data(zend_object *object, zval *value, HashPosition *position);
void pmmpthread_store_forward(zend_object *object, HashPosition *position); /* }}} */

#endif
