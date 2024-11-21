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
#ifndef HAVE_PMMPTHREAD_WORKER_H
#define HAVE_PMMPTHREAD_WORKER_H

#include "pmmpthread.h"

#include "queue.h"

#define PMMPTHREAD_WORKER_COLLECTOR_INIT(call, w) do { \
	memset(&call, 0, sizeof(pmmpthread_call_t)); \
	call.fci.size = sizeof(zend_fcall_info); \
	ZVAL_STR(&call.fci.function_name, zend_string_init(ZEND_STRL("collector"), 0)); \
	call.fcc.function_handler = (zend_function*) zend_hash_find_ptr(&(w)->ce->function_table, Z_STR(call.fci.function_name)); \
	call.fci.object = (w); \
	call.fcc.calling_scope = (w)->ce; \
	call.fcc.called_scope = (w)->ce; \
	call.fcc.object = (w); \
} while(0)

#define PMMPTHREAD_WORKER_COLLECTOR_DTOR(call) zval_ptr_dtor(&call.fci.function_name)

typedef struct _pmmpthread_worker_data_t pmmpthread_worker_data_t;
typedef zend_bool (*pmmpthread_worker_collect_function_t) (pmmpthread_call_t *call, zval *value);

pmmpthread_worker_data_t* pmmpthread_worker_data_alloc(pmmpthread_monitor_t *monitor);
zend_long pmmpthread_worker_task_queue_size(pmmpthread_worker_data_t *worker_data);
void pmmpthread_worker_data_free(pmmpthread_worker_data_t *worker_data);
zend_long pmmpthread_worker_add_task(pmmpthread_worker_data_t *worker_data, zval *value);
zend_long pmmpthread_worker_dequeue_task(pmmpthread_worker_data_t *worker_data, zval *value);
zend_long pmmpthread_worker_collect_tasks(pmmpthread_worker_data_t *worker_data, pmmpthread_call_t *call, pmmpthread_worker_collect_function_t collect);
/* {{{ Runs a pmmpthread_store_full_sync_local_properties() on every task in the GC queue, to ensure availability of properties */
zend_result pmmpthread_worker_sync_collectable_tasks(pmmpthread_worker_data_t * worker_data);
pmmpthread_monitor_state_t pmmpthread_worker_next_task(pmmpthread_worker_data_t *worker_data, pmmpthread_queue* done_tasks_cache, zval *value);
zend_get_gc_buffer* pmmpthread_worker_get_gc_extra(pmmpthread_worker_data_t * worker_data);
void pmmpthread_worker_add_garbage(pmmpthread_worker_data_t *worker_data, pmmpthread_queue* done_tasks_cache, zval* work_zval);

zend_bool pmmpthread_worker_collect_function(pmmpthread_call_t* call, zval* collectable);

#endif

