/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
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

void pthreads_queue_clean(pthreads_queue* queue) {
	pthreads_queue_item_t* item = queue->head;
	while (item) {
		pthreads_queue_item_t* r = item;
		zval_ptr_dtor(&item->value);
		item = r->next;
		efree(r);
	}
}

void pthreads_queue_add_new(pthreads_queue* queue, zval* value) {
	pthreads_queue_item_t* item = emalloc(sizeof(pthreads_queue_item_t));
	ZVAL_COPY(&item->value, value);
	pthreads_queue_add(queue, item);
}

void pthreads_queue_add(pthreads_queue* queue, pthreads_queue_item_t* item) {
	if (!queue->tail) {
		queue->tail = item;
		queue->head = item;
		item->prev = NULL;
		item->next = NULL;
	} else {
		queue->tail->next = item;
		item->prev = queue->tail;
		queue->tail = item;
		item->next = NULL;
	}
	queue->size++;
}

zend_long pthreads_queue_remove(pthreads_queue* queue, pthreads_queue_item_t* item, zval* value, int garbage) {
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
		pthreads_queue_item_t* items[2] =
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
	case PTHREADS_STACK_FREE:
		efree(item);
		break;
	}

	return queue->size;
}
