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
#include <src/object.h>

/* {{{ proto boolean Thread::start([long $options = PTHREADS_INHERIT_ALL])
		Starts executing the implementations run method in a thread, will return a boolean indication of success
		$options should be a mask of inheritance constants */
PHP_METHOD(Thread, start)
{
	pthreads_zend_object_t* thread = PTHREADS_FETCH;
	zend_long options = PTHREADS_INHERIT_ALL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(options)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(pthreads_start(thread, options));
} /* }}} */

/* {{{ proto Thread::isStarted()
	Will return true if a Thread has been started */
PHP_METHOD(Thread, isStarted)
{
	pthreads_object_t* thread = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_STARTED));
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
PHP_METHOD(Thread, isJoined)
{
	pthreads_object_t* thread = PTHREADS_FETCH_TS;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED));
} /* }}} */

/* {{{ proto boolean Thread::join()
		Will return a boolean indication of success */
PHP_METHOD(Thread, join)
{
	pthreads_zend_object_t* thread = PTHREADS_FETCH;

	zend_parse_parameters_none_throw();

	RETURN_BOOL(pthreads_join(thread));
} /* }}} */

/* {{{ proto long Thread::getThreadId()
	Will return the identifier of the referenced Thread */
PHP_METHOD(Thread, getThreadId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, (PTHREADS_FETCH_TS_FROM(Z_OBJ_P(getThis())))->local.id);
} /* }}} */

/* {{{ proto long Thread::getCurrentThreadId()
	Will return the identifier of the current Thread */
PHP_METHOD(Thread, getCurrentThreadId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, pthreads_self());
} /* }}} */

/* {{{ proto Thread Thread::getCurrentThread()
	Will return the currently executing Thread */
PHP_METHOD(Thread, getCurrentThread)
{
	zend_parse_parameters_none_throw();

	pthreads_current_thread(return_value);
} /* }}} */

/* {{{ proto long Thread::getCreatorId()
	Will return the identifier of the thread ( or process ) that created the referenced Thread */
PHP_METHOD(Thread, getCreatorId)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, (PTHREADS_FETCH_TS_FROM(Z_OBJ_P(getThis())))->creator.id);
} /* }}} */
