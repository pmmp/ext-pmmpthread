/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a2b65be0718b120ca3b8d84a62c95407958cf7d5 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Thread_getCreatorId, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Thread_getCurrentThread, 0, 0, Thread, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_Thread_getCurrentThreadId arginfo_class_Thread_getCreatorId

#define arginfo_class_Thread_getThreadId arginfo_class_Thread_getCreatorId

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Thread_isJoined, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Thread_isStarted arginfo_class_Thread_isJoined

#define arginfo_class_Thread_join arginfo_class_Thread_isJoined

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Thread_start, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "PTHREADS_INHERIT_ALL")
ZEND_END_ARG_INFO()


ZEND_METHOD(Thread, getCreatorId);
ZEND_METHOD(Thread, getCurrentThread);
ZEND_METHOD(Thread, getCurrentThreadId);
ZEND_METHOD(Thread, getThreadId);
ZEND_METHOD(Thread, isJoined);
ZEND_METHOD(Thread, isStarted);
ZEND_METHOD(Thread, join);
ZEND_METHOD(Thread, start);


static const zend_function_entry class_Thread_methods[] = {
	ZEND_ME(Thread, getCreatorId, arginfo_class_Thread_getCreatorId, ZEND_ACC_PUBLIC)
	ZEND_ME(Thread, getCurrentThread, arginfo_class_Thread_getCurrentThread, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Thread, getCurrentThreadId, arginfo_class_Thread_getCurrentThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(Thread, getThreadId, arginfo_class_Thread_getThreadId, ZEND_ACC_PUBLIC)
	ZEND_ME(Thread, isJoined, arginfo_class_Thread_isJoined, ZEND_ACC_PUBLIC)
	ZEND_ME(Thread, isStarted, arginfo_class_Thread_isStarted, ZEND_ACC_PUBLIC)
	ZEND_ME(Thread, join, arginfo_class_Thread_join, ZEND_ACC_PUBLIC)
	ZEND_ME(Thread, start, arginfo_class_Thread_start, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Thread(zend_class_entry *class_entry_ThreadedRunnable)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Thread", class_Thread_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_ThreadedRunnable);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
