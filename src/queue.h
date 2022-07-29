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
#ifndef HAVE_PTHREADS_QUEUE_H
#define HAVE_PTHREADS_QUEUE_H

#include <zend.h>

#define PTHREADS_STACK_FREE    1
#define PTHREADS_STACK_NOTHING 0

typedef struct _pthreads_queue_item_t {
	struct _pthreads_queue_item_t* next;
	struct _pthreads_queue_item_t* prev;
	zval value;
} pthreads_queue_item_t;

typedef struct _pthreads_queue {
	zend_long 				size;
	pthreads_queue_item_t* head;
	pthreads_queue_item_t* tail;
} pthreads_queue;

void pthreads_queue_clean(pthreads_queue* queue);
void pthreads_queue_push(pthreads_queue* queue, pthreads_queue_item_t* item);
void pthreads_queue_push_new(pthreads_queue* queue, zval* value);
void pthreads_queue_unshift(pthreads_queue* queue, pthreads_queue_item_t* item);

zend_long pthreads_queue_remove(pthreads_queue* queue, pthreads_queue_item_t* item, zval* value, int garbage);

zend_long pthreads_queue_shift(pthreads_queue* queue, zval* value, int garbage);
zend_long pthreads_queue_pop(pthreads_queue* queue, zval* value, int garbage);

#endif
