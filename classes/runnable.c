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

#define Runnable_method(name) PHP_METHOD(pmmp_thread_Runnable, name)

/* {{{ proto boolean Runnable::isRunning()
	Will return true while the referenced Runnable is executing */
Runnable_method(isRunning)
{
	pmmpthread_object_t* threaded = PMMPTHREAD_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pmmpthread_monitor_check(&threaded->monitor, PMMPTHREAD_MONITOR_RUNNING));
} /* }}} */

/* {{{ proto boolean Runnable::isTerminated()
	Will return true if the referenced Runnable suffered fatal errors or uncaught exceptions */
Runnable_method(isTerminated)
{
	pmmpthread_object_t* threaded = PMMPTHREAD_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pmmpthread_monitor_check(&threaded->monitor, PMMPTHREAD_MONITOR_ERROR));
} /* }}} */
