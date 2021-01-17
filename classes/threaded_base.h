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

#include <src/compat.h>

PHP_METHOD(ThreadedBase, run);
PHP_METHOD(ThreadedBase, wait);
PHP_METHOD(ThreadedBase, notify);
PHP_METHOD(ThreadedBase, notifyOne);
PHP_METHOD(ThreadedBase, isRunning);
PHP_METHOD(ThreadedBase, isTerminated);

PHP_METHOD(ThreadedBase, synchronized);
PHP_METHOD(ThreadedBase, extend);
PHP_METHOD(ThreadedBase, isGarbage);

PHP_METHOD(ThreadedBase, addRef);
PHP_METHOD(ThreadedBase, delRef);
PHP_METHOD(ThreadedBase, getRefCount);

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_run, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_wait, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_synchronized, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_VARIADIC_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_extend, 0, 0, 1)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_addRef, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_delRef, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedBase_getRefCount, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedBase_isGarbage, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_threaded_base_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREADED_BASE
#	define HAVE_PTHREADS_CLASS_THREADED_BASE
zend_function_entry pthreads_threaded_base_methods[] = {
	PHP_ME(ThreadedBase, run, ThreadedBase_run, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, wait, ThreadedBase_wait, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, notify, ThreadedBase_notify, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, notifyOne, ThreadedBase_notify, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, isRunning, ThreadedBase_isRunning, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, isTerminated, ThreadedBase_isTerminated, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, synchronized, ThreadedBase_synchronized, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, isGarbage, ThreadedBase_isGarbage, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, addRef, ThreadedBase_addRef, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, delRef, ThreadedBase_delRef, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, getRefCount, ThreadedBase_getRefCount, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedBase, extend, ThreadedBase_extend, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/* {{{ */
PHP_METHOD(ThreadedBase, run) {} /* }}} */

/* {{{{ */
PHP_METHOD(ThreadedBase, addRef) 		{ Z_ADDREF_P(getThis()); }
PHP_METHOD(ThreadedBase, delRef) 		{ zval_ptr_dtor(getThis()); }
PHP_METHOD(ThreadedBase, getRefCount) 	{ RETURN_LONG(Z_REFCOUNT_P(getThis())); } /* }}} */

/* {{{ proto boolean ThreadedBase::wait([long timeout])
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(ThreadedBase, wait)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;
	zend_long timeout = 0L;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &timeout)==SUCCESS) {
		RETURN_BOOL(pthreads_monitor_wait(threaded->monitor, timeout) == SUCCESS);
	}
} /* }}} */

/* {{{ proto boolean ThreadedBase::notify()
		Send notification to everyone waiting on the ThreadedBase
		Will return a boolean indication of success */
PHP_METHOD(ThreadedBase, notify)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	RETURN_BOOL(pthreads_monitor_notify(threaded->monitor) == SUCCESS);
} /* }}} */

/* {{{ proto boolean ThreadedBase::notifyOne()
		Send notification to one context waiting on the ThreadedBase
		Will return a boolean indication of success */
PHP_METHOD(ThreadedBase, notifyOne)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	RETURN_BOOL(pthreads_monitor_notify_one(threaded->monitor) == SUCCESS);
} /* }}} */

/* {{{ proto boolean ThreadedBase::isRunning()
	Will return true while the referenced ThreadedBase is being executed by a Worker */
PHP_METHOD(ThreadedBase, isRunning)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	RETURN_BOOL(pthreads_monitor_check(threaded->monitor, PTHREADS_MONITOR_RUNNING));
} /* }}} */

/* {{{ proto boolean ThreadedBase::isTerminated()
	Will return true if the referenced ThreadedBase suffered fatal errors or uncaught exceptions */
PHP_METHOD(ThreadedBase, isTerminated)
{
	pthreads_object_t* threaded = PTHREADS_FETCH_TS;

	RETURN_BOOL(pthreads_monitor_check(threaded->monitor, PTHREADS_MONITOR_ERROR));
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

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "f|+", &call.fci, &call.fcc, &argv, &argc) != SUCCESS) {
		return;
	}

	zend_fcall_info_argp(&call.fci, argc, argv);

	call.fci.retval = return_value;
#if PHP_VERSION_ID < 80000
	call.fci.no_separation = 1;
#endif

	if (pthreads_monitor_lock(threaded->monitor)) {
		/* synchronize property tables */
		pthreads_store_sync(Z_OBJ_P(getThis()));

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

/* {{{ proto ThreadedBase::isGarbage(void) : bool */
PHP_METHOD(ThreadedBase, isGarbage) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	RETURN_TRUE;
} /* }}} */

/* {{{ proto bool ThreadedBase::extend(string class) */
PHP_METHOD(ThreadedBase, extend) {
	zend_class_entry *ce = NULL;
	zend_bool is_final = 0;
	zend_class_entry *parent;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "C", &ce) != SUCCESS) {
		return;
	}

#ifdef ZEND_ACC_TRAIT
	if ((ce->ce_flags & ZEND_ACC_TRAIT) == ZEND_ACC_TRAIT) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"cannot extend trait %s", ce->name->val);
		return;
	}
#endif

	if (ce->ce_flags & ZEND_ACC_INTERFACE) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"cannot extend interface %s",
			ce->name->val);
		return;
	}

	if (ce->parent) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"cannot extend class %s, it already extends %s",
			ce->name->val,
			ce->parent->name->val);
		return;
	}

	is_final = ce->ce_flags & ZEND_ACC_FINAL;

	if (is_final)
		ce->ce_flags = ce->ce_flags &~ ZEND_ACC_FINAL;

	parent = zend_get_called_scope(EG(current_execute_data));

	zend_do_inheritance(ce, parent);

	if (is_final)
		ce->ce_flags |= ZEND_ACC_FINAL;

	RETURN_BOOL(instanceof_function(ce, parent));
} /* }}} */

#	endif
#endif
