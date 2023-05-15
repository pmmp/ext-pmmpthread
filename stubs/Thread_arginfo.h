/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a139cbddabbb2be1badbba15710c00c0c64a736b */

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
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, options, IS_LONG, 0, "pmmp\\thread\\Thread::INHERIT_ALL")
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

	zval const_INHERIT_NONE_value;
	ZVAL_LONG(&const_INHERIT_NONE_value, PTHREADS_INHERIT_NONE);
	zend_string *const_INHERIT_NONE_name = zend_string_init_interned("INHERIT_NONE", sizeof("INHERIT_NONE") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_NONE_name, &const_INHERIT_NONE_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_NONE_name);

	zval const_INHERIT_INI_value;
	ZVAL_LONG(&const_INHERIT_INI_value, PTHREADS_INHERIT_INI);
	zend_string *const_INHERIT_INI_name = zend_string_init_interned("INHERIT_INI", sizeof("INHERIT_INI") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_INI_name, &const_INHERIT_INI_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_INI_name);

	zval const_INHERIT_CONSTANTS_value;
	ZVAL_LONG(&const_INHERIT_CONSTANTS_value, PTHREADS_INHERIT_CONSTANTS);
	zend_string *const_INHERIT_CONSTANTS_name = zend_string_init_interned("INHERIT_CONSTANTS", sizeof("INHERIT_CONSTANTS") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_CONSTANTS_name, &const_INHERIT_CONSTANTS_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_CONSTANTS_name);

	zval const_INHERIT_FUNCTIONS_value;
	ZVAL_LONG(&const_INHERIT_FUNCTIONS_value, PTHREADS_INHERIT_FUNCTIONS);
	zend_string *const_INHERIT_FUNCTIONS_name = zend_string_init_interned("INHERIT_FUNCTIONS", sizeof("INHERIT_FUNCTIONS") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_FUNCTIONS_name, &const_INHERIT_FUNCTIONS_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_FUNCTIONS_name);

	zval const_INHERIT_CLASSES_value;
	ZVAL_LONG(&const_INHERIT_CLASSES_value, PTHREADS_INHERIT_CLASSES);
	zend_string *const_INHERIT_CLASSES_name = zend_string_init_interned("INHERIT_CLASSES", sizeof("INHERIT_CLASSES") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_CLASSES_name, &const_INHERIT_CLASSES_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_CLASSES_name);

	zval const_INHERIT_INCLUDES_value;
	ZVAL_LONG(&const_INHERIT_INCLUDES_value, PTHREADS_INHERIT_INCLUDES);
	zend_string *const_INHERIT_INCLUDES_name = zend_string_init_interned("INHERIT_INCLUDES", sizeof("INHERIT_INCLUDES") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_INCLUDES_name, &const_INHERIT_INCLUDES_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_INCLUDES_name);

	zval const_INHERIT_COMMENTS_value;
	ZVAL_LONG(&const_INHERIT_COMMENTS_value, PTHREADS_INHERIT_COMMENTS);
	zend_string *const_INHERIT_COMMENTS_name = zend_string_init_interned("INHERIT_COMMENTS", sizeof("INHERIT_COMMENTS") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_COMMENTS_name, &const_INHERIT_COMMENTS_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_COMMENTS_name);

	zval const_INHERIT_ALL_value;
	ZVAL_LONG(&const_INHERIT_ALL_value, PTHREADS_INHERIT_ALL);
	zend_string *const_INHERIT_ALL_name = zend_string_init_interned("INHERIT_ALL", sizeof("INHERIT_ALL") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_INHERIT_ALL_name, &const_INHERIT_ALL_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_INHERIT_ALL_name);

	zval const_ALLOW_HEADERS_value;
	ZVAL_LONG(&const_ALLOW_HEADERS_value, PTHREADS_ALLOW_HEADERS);
	zend_string *const_ALLOW_HEADERS_name = zend_string_init_interned("ALLOW_HEADERS", sizeof("ALLOW_HEADERS") - 1, 1);
	zend_declare_class_constant_ex(class_entry, const_ALLOW_HEADERS_name, &const_ALLOW_HEADERS_value, ZEND_ACC_PUBLIC, NULL);
	zend_string_release(const_ALLOW_HEADERS_name);

	return class_entry;
}
