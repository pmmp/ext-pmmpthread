/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 0592dabf61bfcf1e3073647ba80e3b352afe5d6b */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedBase_notify, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ThreadedBase_notifyOne arginfo_class_ThreadedBase_notify

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedBase_synchronized, 0, 1, IS_MIXED, 0)
	ZEND_ARG_OBJ_INFO(0, function, Closure, 0)
	ZEND_ARG_VARIADIC_TYPE_INFO(0, args, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedBase_wait, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, timeout, IS_LONG, 0, "0")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_ThreadedBase_getIterator, 0, 0, Iterator, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(ThreadedBase, notify);
ZEND_METHOD(ThreadedBase, notifyOne);
ZEND_METHOD(ThreadedBase, synchronized);
ZEND_METHOD(ThreadedBase, wait);
ZEND_METHOD(ThreadedBase, getIterator);


static const zend_function_entry class_ThreadedBase_methods[] = {
	ZEND_ME(ThreadedBase, notify, arginfo_class_ThreadedBase_notify, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedBase, notifyOne, arginfo_class_ThreadedBase_notifyOne, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedBase, synchronized, arginfo_class_ThreadedBase_synchronized, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedBase, wait, arginfo_class_ThreadedBase_wait, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedBase, getIterator, arginfo_class_ThreadedBase_getIterator, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_ThreadedBase(zend_class_entry *class_entry_IteratorAggregate)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ThreadedBase", class_ThreadedBase_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	zend_class_implements(class_entry, 1, class_entry_IteratorAggregate);

	return class_entry;
}
