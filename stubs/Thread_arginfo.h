/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 6ba6dee671385390ae93886fa2d3e6164deb1f36 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Thread_getCreatorId, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pmmp_thread_Thread_getCurrentThread, 0, 0, pmmp\\thread\\Thread, 1)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_Thread_getCurrentThreadId arginfo_class_pmmp_thread_Thread_getCreatorId

#define arginfo_class_pmmp_thread_Thread_getThreadId arginfo_class_pmmp_thread_Thread_getCreatorId

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Thread_isJoined, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_Thread_isStarted arginfo_class_pmmp_thread_Thread_isJoined

#define arginfo_class_pmmp_thread_Thread_join arginfo_class_pmmp_thread_Thread_isJoined

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Thread_start, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "PTHREADS_INHERIT_ALL")
ZEND_END_ARG_INFO()


ZEND_METHOD(pmmp_thread_Thread, getCreatorId);
ZEND_METHOD(pmmp_thread_Thread, getCurrentThread);
ZEND_METHOD(pmmp_thread_Thread, getCurrentThreadId);
ZEND_METHOD(pmmp_thread_Thread, getThreadId);
ZEND_METHOD(pmmp_thread_Thread, isJoined);
ZEND_METHOD(pmmp_thread_Thread, isStarted);
ZEND_METHOD(pmmp_thread_Thread, join);
ZEND_METHOD(pmmp_thread_Thread, start);


static const zend_function_entry class_pmmp_thread_Thread_methods[] = {
	ZEND_ME(pmmp_thread_Thread, getCreatorId, arginfo_class_pmmp_thread_Thread_getCreatorId, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Thread, getCurrentThread, arginfo_class_pmmp_thread_Thread_getCurrentThread, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(pmmp_thread_Thread, getCurrentThreadId, arginfo_class_pmmp_thread_Thread_getCurrentThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(pmmp_thread_Thread, getThreadId, arginfo_class_pmmp_thread_Thread_getThreadId, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Thread, isJoined, arginfo_class_pmmp_thread_Thread_isJoined, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Thread, isStarted, arginfo_class_pmmp_thread_Thread_isStarted, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Thread, join, arginfo_class_pmmp_thread_Thread_join, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Thread, start, arginfo_class_pmmp_thread_Thread_start, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pmmp_thread_Thread(zend_class_entry *class_entry_pmmp_thread_Runnable)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pmmp\\thread", "Thread", class_pmmp_thread_Thread_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_pmmp_thread_Runnable);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
