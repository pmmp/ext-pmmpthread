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
#ifndef HAVE_PTHREADS_WORKER_H
#define HAVE_PTHREADS_WORKER_H

#include "pthreads.h"

#include "queue.h"

#define PTHREADS_WORKER_COLLECTOR_INIT(call, w) do { \
	memset(&call, 0, sizeof(pthreads_call_t)); \
	call.fci.size = sizeof(zend_fcall_info); \
	ZVAL_STR(&call.fci.function_name, zend_string_init(ZEND_STRL("collector"), 0)); \
	call.fcc.function_handler = (zend_function*) zend_hash_find_ptr(&(w)->ce->function_table, Z_STR(call.fci.function_name)); \
	call.fci.object = (w); \
	call.fcc.calling_scope = (w)->ce; \
	call.fcc.called_scope = (w)->ce; \
	call.fcc.object = (w); \
} while(0)

#define PTHREADS_WORKER_COLLECTOR_DTOR(call) zval_ptr_dtor(&call.fci.function_name)

typedef struct _pthreads_worker_data_t pthreads_worker_data_t;
typedef zend_bool (*pthreads_worker_collect_function_t) (pthreads_call_t *call, zval *value);

pthreads_worker_data_t* pthreads_worker_data_alloc(pthreads_monitor_t *monitor);
zend_long pthreads_worker_task_queue_size(pthreads_worker_data_t *worker_data);
void pthreads_worker_data_free(pthreads_worker_data_t *worker_data);
zend_long pthreads_worker_add_task(pthreads_worker_data_t *worker_data, zval *value);
zend_long pthreads_worker_dequeue_task(pthreads_worker_data_t *worker_data, zval *value);
zend_long pthreads_worker_collect_tasks(pthreads_worker_data_t *worker_data, pthreads_call_t *call, pthreads_worker_collect_function_t collect);
pthreads_monitor_state_t pthreads_worker_next_task(pthreads_worker_data_t *worker_data, zval *value, pthreads_queue_item_t **item);
void pthreads_worker_add_garbage(pthreads_worker_data_t *worker_data, pthreads_queue_item_t *item);

zend_bool pthreads_worker_collect_function(pthreads_call_t* call, zval* collectable);

#endif

