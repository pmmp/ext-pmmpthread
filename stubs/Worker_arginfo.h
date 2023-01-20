/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 5b9fa39f84d6aae5fc62ac5acf5400c5ad55a4fd */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_collect, 0, 0, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, function, IS_CALLABLE, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_collector, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, collectable, ThreadedRunnable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_getStacked, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_isShutdown, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_Worker_shutdown arginfo_class_Worker_isShutdown

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_stack, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, work, ThreadedRunnable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_Worker_unstack, 0, 0, ThreadedRunnable, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Worker_run, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Worker, collect);
ZEND_METHOD(Worker, collector);
ZEND_METHOD(Worker, getStacked);
ZEND_METHOD(Thread, isJoined);
ZEND_METHOD(Thread, join);
ZEND_METHOD(Worker, stack);
ZEND_METHOD(Worker, unstack);
ZEND_METHOD(Worker, run);


static const zend_function_entry class_Worker_methods[] = {
	ZEND_ME(Worker, collect, arginfo_class_Worker_collect, ZEND_ACC_PUBLIC)
	ZEND_ME(Worker, collector, arginfo_class_Worker_collector, ZEND_ACC_PUBLIC)
	ZEND_ME(Worker, getStacked, arginfo_class_Worker_getStacked, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(Thread, isShutdown, isJoined, arginfo_class_Worker_isShutdown, ZEND_ACC_PUBLIC)
	ZEND_MALIAS(Thread, shutdown, join, arginfo_class_Worker_shutdown, ZEND_ACC_PUBLIC)
	ZEND_ME(Worker, stack, arginfo_class_Worker_stack, ZEND_ACC_PUBLIC)
	ZEND_ME(Worker, unstack, arginfo_class_Worker_unstack, ZEND_ACC_PUBLIC)
	ZEND_ME(Worker, run, arginfo_class_Worker_run, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Worker(zend_class_entry *class_entry_Thread)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Worker", class_Worker_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Thread);

	return class_entry;
}
