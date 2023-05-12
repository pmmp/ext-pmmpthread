/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 42556b1e46a4a648d6701c65e5347aa1d5ca9fdc */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Runnable_isRunning, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_Runnable_isTerminated arginfo_class_pmmp_thread_Runnable_isRunning

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_Runnable_run, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pmmp_thread_Runnable, isRunning);
ZEND_METHOD(pmmp_thread_Runnable, isTerminated);


static const zend_function_entry class_pmmp_thread_Runnable_methods[] = {
	ZEND_ME(pmmp_thread_Runnable, isRunning, arginfo_class_pmmp_thread_Runnable_isRunning, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_Runnable, isTerminated, arginfo_class_pmmp_thread_Runnable_isTerminated, ZEND_ACC_PUBLIC)
	ZEND_ABSTRACT_ME_WITH_FLAGS(pmmp_thread_Runnable, run, arginfo_class_pmmp_thread_Runnable_run, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
	ZEND_FE_END
};

static zend_class_entry *register_class_pmmp_thread_Runnable(zend_class_entry *class_entry_pmmp_thread_ThreadSafe)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pmmp\\thread", "Runnable", class_pmmp_thread_Runnable_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_pmmp_thread_ThreadSafe);
	class_entry->ce_flags |= ZEND_ACC_ABSTRACT;

	return class_entry;
}
