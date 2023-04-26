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

#include <src/pthreads.h>

/* {{{ */
PHP_METHOD(Worker, run) {} /* }}} */

/* {{{ proto int Worker::stack(ThreadedRunnable $work)
	Pushes an item onto the stack, returns the size of stack */
PHP_METHOD(Worker, stack)
{
	pthreads_zend_object_t* thread = PTHREADS_FETCH;
	zval *work;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(work, pthreads_threaded_runnable_entry)
	ZEND_PARSE_PARAMETERS_END();

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may call stack",
			thread->std.ce->name->val);
		return;
	}
	if (pthreads_monitor_check(&thread->ts_obj->monitor, PTHREADS_MONITOR_ERROR | PTHREADS_MONITOR_AWAIT_JOIN | PTHREADS_MONITOR_JOINED)) {
		//allow submitting tasks before the worker starts, but not after it exits
		zend_throw_exception_ex(
			spl_ce_RuntimeException,
			0,
			"this %s is no longer running and cannot accept tasks",
			ZSTR_VAL(thread->std.ce->name)
		);
		return;
	}

	RETURN_LONG(pthreads_worker_add_task(thread->worker_data, work));
} /* }}} */

/* {{{ proto ThreadedRunnable Worker::unstack()
	Removes the first item from the stack */
PHP_METHOD(Worker, unstack)
{
	pthreads_zend_object_t* thread = PTHREADS_FETCH;

	zend_parse_parameters_none_throw();

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may call unstack",
			thread->std.ce->name->val);
		return;
	}

	pthreads_worker_dequeue_task(thread->worker_data, return_value);
}

/* {{{ proto int Worker::getStacked()
	Returns the current size of the stack */
PHP_METHOD(Worker, getStacked)
{
	pthreads_zend_object_t* thread = PTHREADS_FETCH;

	zend_parse_parameters_none_throw();

	RETURN_LONG(pthreads_worker_task_queue_size(thread->worker_data));
}

/* {{{ proto bool Worker::collector(ThreadedRunnable collectable) */
PHP_METHOD(Worker, collector) {
	zval *collectable;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(collectable, pthreads_threaded_runnable_entry)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_TRUE;
} /* }}} */

/* {{{ proto int Worker::collect([callable collector]) */
PHP_METHOD(Worker, collect)
{
	pthreads_zend_object_t *thread = PTHREADS_FETCH;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC(call.fci, call.fcc)
	ZEND_PARSE_PARAMETERS_END();

	if (!ZEND_NUM_ARGS()) {
		PTHREADS_WORKER_COLLECTOR_INIT(call, Z_OBJ_P(getThis()));
	}

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"only the creator of this %s may call collect",
			thread->std.ce->name->val);
		return;
	}

	RETVAL_LONG(pthreads_worker_collect_tasks(thread->worker_data, &call, pthreads_worker_collect_function));

	if (!ZEND_NUM_ARGS()) {
		PTHREADS_WORKER_COLLECTOR_DTOR(call);
	}
}


