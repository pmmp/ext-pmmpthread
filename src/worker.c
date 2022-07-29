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
#ifndef HAVE_PTHREADS_STACK
#define HAVE_PTHREADS_STACK

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#include "worker.h"
#include "queue.h"

struct _pthreads_worker_data_t {
	pthreads_monitor_t   	*monitor;
	pthreads_queue queue;
	pthreads_queue gc;
};

pthreads_worker_data_t* pthreads_worker_data_alloc(pthreads_monitor_t *monitor) {
	pthreads_worker_data_t *stack =
		(pthreads_worker_data_t*) ecalloc(1, sizeof(pthreads_worker_data_t));

	stack->monitor = monitor;

	return stack;
}

zend_long pthreads_worker_task_queue_size(pthreads_worker_data_t *worker_data) {
	zend_long size = 0;

	if (pthreads_monitor_lock(worker_data->monitor)) {
		size = worker_data->queue.size;
		pthreads_monitor_unlock(worker_data->monitor);
	}
	return size;
}

void pthreads_worker_data_free(pthreads_worker_data_t *worker_data) {
	pthreads_monitor_t *monitor = worker_data->monitor;

	if (pthreads_monitor_lock(monitor)) {
		pthreads_queue_clean(&worker_data->queue);
		pthreads_queue_clean(&worker_data->gc);

		efree(worker_data);

		pthreads_monitor_unlock(monitor);
	}
}

zend_long pthreads_worker_add_task(pthreads_worker_data_t *worker_data, zval *value) {
	zend_long size = 0;

	if (pthreads_monitor_lock(worker_data->monitor)) {
		size = worker_data->queue.size;
		pthreads_queue_add_new(&worker_data->queue, value);
		if (!size) {
			pthreads_monitor_notify(worker_data->monitor);
		}
		size = worker_data->queue.size;
		pthreads_monitor_unlock(worker_data->monitor);
	} else {
		size = -1;
	}

	return size;
}

void pthreads_worker_add_garbage(pthreads_worker_data_t *worker_data, pthreads_queue_item_t *item) {
	if (pthreads_monitor_lock(worker_data->monitor)) {
		pthreads_queue_add(&worker_data->gc, item);
		pthreads_monitor_unlock(worker_data->monitor);
	} else {
		ZEND_ASSERT(0);
	}
}

zend_long pthreads_worker_dequeue_task(pthreads_worker_data_t *worker_data, zval *value) {
	zend_long size = 0;

	if (pthreads_monitor_lock(worker_data->monitor)) {
		size = pthreads_queue_remove(
			&worker_data->queue, worker_data->queue.head, value, PTHREADS_STACK_FREE);
		pthreads_monitor_unlock(worker_data->monitor);
	}

	return size;
}

zend_long pthreads_worker_collect_tasks(zend_object *std, pthreads_worker_data_t *worker_data, pthreads_call_t *call, pthreads_worker_collect_function_t collect) {
	zend_long size = 0, offset = 0;

	if (pthreads_monitor_lock(worker_data->monitor)) {
		pthreads_queue_item_t *item;

		item = worker_data->gc.head;
		while (item) {
			if (collect(call, &item->value)) {
				pthreads_queue_item_t *garbage = item;
				item = garbage->next;

				pthreads_queue_remove(
					&worker_data->gc, garbage, NULL, PTHREADS_STACK_FREE);
				continue;
			}

			item = item->next;
		}

		size = (worker_data->queue.size + worker_data->gc.size) + -offset;

		pthreads_monitor_unlock(worker_data->monitor);
	}

	return size;
}

pthreads_monitor_state_t pthreads_worker_next_task(pthreads_worker_data_t *worker_data, zval *value, pthreads_queue_item_t **item) {
	pthreads_monitor_state_t state = PTHREADS_MONITOR_RUNNING;
	if (pthreads_monitor_lock(worker_data->monitor)) {
		do {
			if (!worker_data->queue.head) {
				if (pthreads_monitor_check(worker_data->monitor, PTHREADS_MONITOR_JOINED)) {
					state = PTHREADS_MONITOR_JOINED;
					*item = NULL;
					break;
				}

				*item  = NULL;
				pthreads_monitor_wait(worker_data->monitor, 0);
			} else {
				*item = worker_data->queue.head; //this is allocated on the creator thread's ZMM, so we can't free it
				pthreads_queue_remove(&worker_data->queue, worker_data->queue.head, value, PTHREADS_STACK_NOTHING);
				break;
			}
		} while (state != PTHREADS_MONITOR_JOINED);
		pthreads_monitor_unlock(worker_data->monitor);
	}

	return state;
}

void pthreads_worker_data_tohash(pthreads_worker_data_t *worker_data, HashTable *hash) {
	zval stacked;
	zval waiting;
	zval gc;

	array_init(&stacked);
	array_init(&waiting);
	array_init(&gc);

	zend_hash_str_add(Z_ARRVAL(stacked), ":stacked:", sizeof(":stacked:")-1, &waiting);
	zend_hash_str_add(Z_ARRVAL(stacked), ":gc:", sizeof(":gc:")-1, &gc);

	if (pthreads_monitor_lock(worker_data->monitor)) {
		pthreads_queue_item_t *item = worker_data->queue.head;

		while (item) {
			if (add_next_index_zval(
					&waiting, &item->value)) {
				Z_ADDREF(item->value);
			}
			item = item->next;
		}

		item = worker_data->gc.head;
		while (item) {
			if (add_next_index_zval(
					&gc, &item->value)) {
				Z_ADDREF(item->value);
			}
			item = item->next;
		}
		pthreads_monitor_unlock(worker_data->monitor);
	}

	zend_hash_str_add(hash, ":stack:", sizeof(":stack:")-1, &stacked);
}
#endif

