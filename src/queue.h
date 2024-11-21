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
#ifndef HAVE_PMMPTHREAD_QUEUE_H
#define HAVE_PMMPTHREAD_QUEUE_H

#include <zend.h>

#define PMMPTHREAD_STACK_FREE    1
#define PMMPTHREAD_STACK_NOTHING 0

typedef struct _pmmpthread_queue_item_t {
	struct _pmmpthread_queue_item_t* next;
	struct _pmmpthread_queue_item_t* prev;
	zval value;
} pmmpthread_queue_item_t;

typedef struct _pmmpthread_queue {
	zend_long 				size;
	pmmpthread_queue_item_t* head;
	pmmpthread_queue_item_t* tail;
} pmmpthread_queue;

void pmmpthread_queue_clean(pmmpthread_queue* queue);
void pmmpthread_queue_push(pmmpthread_queue* queue, pmmpthread_queue_item_t* item);
void pmmpthread_queue_push_new(pmmpthread_queue* queue, zval* value);
void pmmpthread_queue_unshift(pmmpthread_queue* queue, pmmpthread_queue_item_t* item);

zend_long pmmpthread_queue_remove(pmmpthread_queue* queue, pmmpthread_queue_item_t* item, zval* value, int garbage);

zend_long pmmpthread_queue_shift(pmmpthread_queue* queue, zval* value, int garbage);
zend_long pmmpthread_queue_pop(pmmpthread_queue* queue, zval* value, int garbage);

#endif
