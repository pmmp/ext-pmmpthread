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

#include <src/pmmpthread.h>

#define Worker_method(name) PHP_METHOD(pmmp_thread_Worker, name)

/* {{{ */
Worker_method(run) {} /* }}} */

/* {{{ proto int Worker::stack(Runnable $work)
	Pushes an item onto the stack, returns the size of stack */
Worker_method(stack)
{
	pmmpthread_zend_object_t* thread = PMMPTHREAD_FETCH;
	zval *work;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(work, pmmpthread_ce_runnable)
	ZEND_PARSE_PARAMETERS_END();

	if (!PMMPTHREAD_IN_CREATOR(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may call stack",
			thread->std.ce->name->val);
		return;
	}
	if (pmmpthread_monitor_check(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_ERROR | PMMPTHREAD_MONITOR_AWAIT_JOIN | PMMPTHREAD_MONITOR_JOINED)) {
		//allow submitting tasks before the worker starts, but not after it exits
		zend_throw_exception_ex(
			spl_ce_RuntimeException,
			0,
			"this %s is no longer running and cannot accept tasks",
			ZSTR_VAL(thread->std.ce->name)
		);
		return;
	}

	RETURN_LONG(pmmpthread_worker_add_task(thread->worker_data, work));
} /* }}} */

/* {{{ proto Runnable Worker::unstack()
	Removes the first item from the stack */
Worker_method(unstack)
{
	pmmpthread_zend_object_t* thread = PMMPTHREAD_FETCH;

	zend_parse_parameters_none_throw();

	if (!PMMPTHREAD_IN_CREATOR(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may call unstack",
			thread->std.ce->name->val);
		return;
	}

	pmmpthread_worker_dequeue_task(thread->worker_data, return_value);
}

/* {{{ proto int Worker::getStacked()
	Returns the current size of the stack */
Worker_method(getStacked)
{
	pmmpthread_zend_object_t* thread = PMMPTHREAD_FETCH;

	zend_parse_parameters_none_throw();

	RETURN_LONG(pmmpthread_worker_task_queue_size(thread->worker_data));
}

/* {{{ proto bool Worker::collector(Runnable collectable) */
Worker_method(collector) {
	zval *collectable;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(collectable, pmmpthread_ce_runnable)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_TRUE;
} /* }}} */

/* {{{ proto int Worker::collect([callable collector]) */
Worker_method(collect)
{
	pmmpthread_zend_object_t *thread = PMMPTHREAD_FETCH;
	pmmpthread_call_t call = PMMPTHREAD_CALL_EMPTY;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC(call.fci, call.fcc)
	ZEND_PARSE_PARAMETERS_END();

	if (!ZEND_NUM_ARGS()) {
		PMMPTHREAD_WORKER_COLLECTOR_INIT(call, Z_OBJ_P(getThis()));
	}

	if (!PMMPTHREAD_IN_CREATOR(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"only the creator of this %s may call collect",
			thread->std.ce->name->val);
		return;
	}

	RETVAL_LONG(pmmpthread_worker_collect_tasks(thread->worker_data, &call, pmmpthread_worker_collect_function));

	if (!ZEND_NUM_ARGS()) {
		PMMPTHREAD_WORKER_COLLECTOR_DTOR(call);
	}
}


