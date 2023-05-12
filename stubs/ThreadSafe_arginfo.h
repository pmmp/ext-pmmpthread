/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8658914d704f8d643e3c4a57f2dcf5db1ad02f99 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafe_notify, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_ThreadSafe_notifyOne arginfo_class_pmmp_thread_ThreadSafe_notify

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafe_synchronized, 0, 1, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO(0, function, Closure, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafe_wait, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pmmp_thread_ThreadSafe_getIterator, 0, 0, Iterator, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pmmp_thread_ThreadSafe, notify);
ZEND_METHOD(pmmp_thread_ThreadSafe, notifyOne);
ZEND_METHOD(pmmp_thread_ThreadSafe, synchronized);
ZEND_METHOD(pmmp_thread_ThreadSafe, wait);
ZEND_METHOD(pmmp_thread_ThreadSafe, getIterator);


static const zend_function_entry class_pmmp_thread_ThreadSafe_methods[] = {
	ZEND_ME(pmmp_thread_ThreadSafe, notify, arginfo_class_pmmp_thread_ThreadSafe_notify, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafe, notifyOne, arginfo_class_pmmp_thread_ThreadSafe_notifyOne, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafe, synchronized, arginfo_class_pmmp_thread_ThreadSafe_synchronized, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafe, wait, arginfo_class_pmmp_thread_ThreadSafe_wait, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafe, getIterator, arginfo_class_pmmp_thread_ThreadSafe_getIterator, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pmmp_thread_ThreadSafe(zend_class_entry *class_entry_IteratorAggregate)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pmmp\\thread", "ThreadSafe", class_pmmp_thread_ThreadSafe_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	zend_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	return class_entry;
}
