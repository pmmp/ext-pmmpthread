/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2015                                       |
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
#include "queue.h"

void pmmpthread_queue_clean(pmmpthread_queue* queue) {
	pmmpthread_queue_item_t* item = queue->head;
	while (item) {
		pmmpthread_queue_item_t* r = item;
		zval_ptr_dtor(&item->value);
		item = r->next;
		efree(r);
	}
}

static inline pmmpthread_queue_item_t* pmmpthread_queue_item_new(zval* value) {
	pmmpthread_queue_item_t* item = emalloc(sizeof(pmmpthread_queue_item_t));
	ZVAL_COPY(&item->value, value);
	return item;
}

static inline void set_prev_and_next(pmmpthread_queue_item_t* item, pmmpthread_queue_item_t* prev, pmmpthread_queue_item_t* next) {
	item->prev = prev;
	item->next = next;
}

void pmmpthread_queue_push(pmmpthread_queue* queue, pmmpthread_queue_item_t* item) {
	if (!queue->tail) {
		queue->tail = item;
		queue->head = item;
		set_prev_and_next(item, NULL, NULL);
	} else {
		queue->tail->next = item;
		set_prev_and_next(item, queue->tail, NULL);
		queue->tail = item;
	}
	queue->size++;
}

void pmmpthread_queue_push_new(pmmpthread_queue* queue, zval* value) {
	pmmpthread_queue_push(queue, pmmpthread_queue_item_new(value));
}

void pmmpthread_queue_unshift(pmmpthread_queue* queue, pmmpthread_queue_item_t* item) {
	if (!queue->head) {
		queue->tail = item;
		queue->head = item;
		set_prev_and_next(item, NULL, NULL);
	} else {
		queue->head->prev = item;
		set_prev_and_next(item, NULL, queue->head);
		queue->head = item;
	}
}

void pmmpthread_queue_unshift_new(pmmpthread_queue* queue, zval* value) {
	pmmpthread_queue_unshift(queue, pmmpthread_queue_item_new(value));
}

zend_long pmmpthread_queue_remove(pmmpthread_queue* queue, pmmpthread_queue_item_t* item, zval* value, int garbage) {
	if (!item) {
		value = NULL;
		return 0;
	}

	if (queue->head == item && queue->tail == item) {
		queue->head = NULL;
		queue->tail = NULL;
	} else if (queue->head == item) {
		queue->head = item->next;
		queue->head->prev = NULL;
	} else if (queue->tail == item) {
		queue->tail = item->prev;
		queue->tail->next = NULL;
	} else {
		pmmpthread_queue_item_t* items[2] =
		{ item->next, item->prev };

		items[0]->prev = items[1];
		items[1]->next = items[0];
	}

	queue->size--;

	if (value) {
		memcpy(value, &item->value, sizeof(zval));
	} else {
		zval_ptr_dtor(&item->value);
	}

	switch (garbage) {
	case PMMPTHREAD_STACK_FREE:
		efree(item);
		break;
	}

	return queue->size;
}

zend_long pmmpthread_queue_shift(pmmpthread_queue* queue, zval* value, int garbage) {
	return pmmpthread_queue_remove(queue, queue->head, value, garbage);
}

zend_long pmmpthread_queue_pop(pmmpthread_queue* queue, zval* value, int garbage) {
	return pmmpthread_queue_remove(queue, queue->tail, value, garbage);
}
