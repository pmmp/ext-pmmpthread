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
#ifndef HAVE_PTHREADS_CLASS_THREADED_RUNNABLE_H
#define HAVE_PTHREADS_CLASS_THREADED_RUNNABLE_H

#include <src/compat.h>

PHP_METHOD(ThreadedRunnable, run);
PHP_METHOD(ThreadedRunnable, isRunning);
PHP_METHOD(ThreadedRunnable, isTerminated);

ZEND_BEGIN_ARG_INFO_EX(ThreadedRunnable_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedRunnable_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(ThreadedRunnable_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_threaded_runnable_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREADED_RUNNABLE
#	define HAVE_PTHREADS_CLASS_THREADED_RUNNABLE
zend_function_entry pthreads_threaded_runnable_methods[] = {
	PHP_ABSTRACT_ME(ThreadedRunnable, run, ThreadedRunnable_run)
	PHP_ME(ThreadedRunnable, isRunning, ThreadedRunnable_isRunning, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedRunnable, isTerminated, ThreadedRunnable_isTerminated, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

/* {{{ proto boolean ThreadedRunnable::isRunning()
	Will return true while the referenced ThreadedRunnable is executing */
PHP_METHOD(ThreadedRunnable, isRunning)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_check(threaded->monitor, PTHREADS_MONITOR_RUNNING));
} /* }}} */

/* {{{ proto boolean ThreadedRunnable::isTerminated()
	Will return true if the referenced ThreadedRunnable suffered fatal errors or uncaught exceptions */
PHP_METHOD(ThreadedRunnable, isTerminated)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_check(threaded->monitor, PTHREADS_MONITOR_ERROR));
} /* }}} */

#	endif
#endif
