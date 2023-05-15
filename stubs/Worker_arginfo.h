/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 3a5206f0788fb2c3f5c902dc55b866cd3fe4d2f9 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_collect, 0, 0, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, function, IS_CALLABLE, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_collector, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, collectable, pmmp\\thread\\Runnable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_getStacked, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_isShutdown, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_Worker_shutdown arginfo_class_pmmp_thread_Worker_isShutdown

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_stack, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, work, pmmp\\thread\\Runnable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pmmp_thread_Worker_unstack, 0, 0, pmmp\\thread\\Runnable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Worker_run, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pmmp_thread_Worker, collect);
ZEND_METHOD(pmmp_thread_Worker, collector);
ZEND_METHOD(pmmp_thread_Worker, getStacked);
ZEND_METHOD(pmmp_thread_Thread, isJoined);
ZEND_METHOD(pmmp_thread_Thread, join);
ZEND_METHOD(pmmp_thread_Worker, stack);
ZEND_METHOD(pmmp_thread_Worker, unstack);
ZEND_METHOD(pmmp_thread_Worker, run);


static const zend_function_entry class_pmmp_thread_Worker_methods[] = {
	ZEND_ME(pmmp_thread_Worker, collect, arginfo_class_pmmp_thread_Worker_collect, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Worker, collector, arginfo_class_pmmp_thread_Worker_collector, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Worker, getStacked, arginfo_class_pmmp_thread_Worker_getStacked, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(pmmp_thread_Thread, isShutdown, isJoined, arginfo_class_pmmp_thread_Worker_isShutdown, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(pmmp_thread_Thread, shutdown, join, arginfo_class_pmmp_thread_Worker_shutdown, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Worker, stack, arginfo_class_pmmp_thread_Worker_stack, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Worker, unstack, arginfo_class_pmmp_thread_Worker_unstack, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Worker, run, arginfo_class_pmmp_thread_Worker_run, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pmmp_thread_Worker(zend_class_entry *class_entry_pmmp_thread_Thread)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pmmp\\thread", "Worker", class_pmmp_thread_Worker_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_pmmp_thread_Thread);

	return class_entry;
}
