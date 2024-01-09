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

#include <src/globals.h>
#include <src/pmmpthread.h>
#include <src/object.h>
#include <src/routine.h>

#define Thread_method(name) PHP_METHOD(pmmp_thread_Thread, name)

/* {{{ proto boolean Thread::start([long $options])
		Starts executing the implementations run method in a thread, will return a boolean indication of success
		$options should be a mask of inheritance constants */
Thread_method(start)
{
	pmmpthread_zend_object_t* thread = PMMPTHREAD_FETCH;
	zend_long options;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(pmmpthread_start(thread, options));
} /* }}} */

/* {{{ proto Thread::isStarted()
	Will return true if a Thread has been started */
Thread_method(isStarted)
{
	pmmpthread_object_t* thread = PMMPTHREAD_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pmmpthread_monitor_check(&thread->monitor, PMMPTHREAD_MONITOR_STARTED));
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
Thread_method(isJoined)
{
	pmmpthread_object_t* thread = PMMPTHREAD_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pmmpthread_monitor_check(&thread->monitor, PMMPTHREAD_MONITOR_JOINED));
} /* }}} */

/* {{{ proto boolean Thread::join()
		Will return a boolean indication of success */
Thread_method(join)
{
	pmmpthread_zend_object_t* thread = PMMPTHREAD_FETCH;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pmmpthread_join(thread));
} /* }}} */

/* {{{ proto long Thread::getThreadId()
	Will return the identifier of the referenced Thread */
Thread_method(getThreadId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, (PMMPTHREAD_FETCH_TS_FROM(Z_OBJ_P(getThis())))->local.id);
} /* }}} */

/* {{{ proto long Thread::getCurrentThreadId()
	Will return the identifier of the current Thread */
Thread_method(getCurrentThreadId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, pmmpthread_self());
} /* }}} */

/* {{{ proto Thread Thread::getCurrentThread()
	Will return the currently executing Thread */
Thread_method(getCurrentThread)
{
	zend_parse_parameters_none_throw();

	pmmpthread_current_thread(return_value);
} /* }}} */

/* {{{ proto long Thread::getCreatorId()
	Will return the identifier of the thread ( or process ) that created the referenced Thread */
Thread_method(getCreatorId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, (PMMPTHREAD_FETCH_TS_FROM(Z_OBJ_P(getThis())))->creator.id);
} /* }}} */

/* {{{ proto ThreadSafeArray Thread::getSharedGlobals()
	Returns a ThreadSafeArray of globals accessible to all threads */
Thread_method(getSharedGlobals)
{
	zend_parse_parameters_none_throw();

	RETURN_OBJ_COPY(&PMMPTHREAD_ZG(thread_shared_globals)->std);
} /* }}} */

Thread_method(getRunningCount)
{
	zend_parse_parameters_none_throw();

	zend_long count = 0;

	if (pmmpthread_globals_lock()) {
		count = PMMPTHREAD_G(thread_count);
		pmmpthread_globals_unlock();
	}

	RETURN_LONG(count);
}
