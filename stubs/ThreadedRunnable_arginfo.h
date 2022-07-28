/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a95496e9a16f332a82d590834bbb71496554472c */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedRunnable_isRunning, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ThreadedRunnable_isTerminated arginfo_class_ThreadedRunnable_isRunning

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedRunnable_run, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(ThreadedRunnable, isRunning);
ZEND_METHOD(ThreadedRunnable, isTerminated);


static const zend_function_entry class_ThreadedRunnable_methods[] = {
	ZEND_ME(ThreadedRunnable, isRunning, arginfo_class_ThreadedRunnable_isRunning, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedRunnable, isTerminated, arginfo_class_ThreadedRunnable_isTerminated, ZEND_ACC_PUBLIC)
	ZEND_ABSTRACT_ME_WITH_FLAGS(ThreadedRunnable, run, arginfo_class_ThreadedRunnable_run, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_ThreadedRunnable(zend_class_entry *class_entry_ThreadedBase)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ThreadedRunnable", class_ThreadedRunnable_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_ThreadedBase);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
