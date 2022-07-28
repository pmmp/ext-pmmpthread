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

#ifndef HAVE_PTHREADS_CLASS_THREADED_BASE_H
#define HAVE_PTHREADS_CLASS_THREADED_BASE_H

#include <stubs/ThreadedBase_arginfo.h>

/* {{{ proto boolean ThreadedBase::wait([long timeout])
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(ThreadedBase, wait)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;
	zend_long timeout = 0L;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(timeout)
	ZEND_PARSE_PARAMETERS_END();
	
	RETURN_BOOL(pthreads_monitor_wait(threaded->monitor, timeout) == SUCCESS);
} /* }}} */

/* {{{ proto boolean ThreadedBase::notify()
		Send notification to everyone waiting on the ThreadedBase
		Will return a boolean indication of success */
PHP_METHOD(ThreadedBase, notify)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_notify(threaded->monitor) == SUCCESS);
} /* }}} */

/* {{{ proto boolean ThreadedBase::notifyOne()
		Send notification to one context waiting on the ThreadedBase
		Will return a boolean indication of success */
PHP_METHOD(ThreadedBase, notifyOne)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_notify_one(threaded->monitor) == SUCCESS);
} /* }}} */

/* {{{ proto void ThreadedBase::synchronized(Callable function, ...)
	Will synchronize the object, call the function, passing anything after the function as parameters
	 */
PHP_METHOD(ThreadedBase, synchronized)
{
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	int argc = 0;
	zval *argv = NULL;
	pthreads_object_t* threaded= PTHREADS_FETCH_TS;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, -1)
		Z_PARAM_FUNC(call.fci, call.fcc)
		Z_PARAM_OPTIONAL
		Z_PARAM_VARIADIC('+', argv, argc)
	ZEND_PARSE_PARAMETERS_END();

	zend_fcall_info_argp(&call.fci, argc, argv);

	call.fci.retval = return_value;

	if (pthreads_monitor_lock(threaded->monitor)) {
		/* synchronize property tables */
		pthreads_store_sync_local_properties(PTHREADS_FETCH);

		zend_try {
			/* call the closure */
			zend_call_function(&call.fci, &call.fcc);
		} zend_catch {
			ZVAL_UNDEF(return_value);
		} zend_end_try ();

		pthreads_monitor_unlock(threaded->monitor);
	}

	zend_fcall_info_args_clear(&call.fci, 1);
} /* }}} */

/* {{{ proto Iterator ThreadedBase::getIterator() */
PHP_METHOD(ThreadedBase, getIterator)
{
	ZEND_PARSE_PARAMETERS_NONE();
	zend_create_internal_iterator_zval(return_value, getThis());
} /* }}} */

#endif
