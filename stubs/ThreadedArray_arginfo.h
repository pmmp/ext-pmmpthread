/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8380bb99a8aace877672dd60e8435679e046d22d */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_chunk, 0, 1, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_ThreadedArray_fromArray, 0, 1, ThreadedArray, 0)
	ZEND_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_merge, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, from, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, overwrite, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_pop, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_ThreadedArray_shift arginfo_class_ThreadedArray_pop

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_offsetGet, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_offsetSet, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_offsetExists, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_ThreadedArray_offsetUnset, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(ThreadedArray, chunk);
ZEND_METHOD(ThreadedArray, count);
ZEND_METHOD(ThreadedArray, fromArray);
ZEND_METHOD(ThreadedArray, merge);
ZEND_METHOD(ThreadedArray, pop);
ZEND_METHOD(ThreadedArray, shift);
ZEND_METHOD(ThreadedArray, offsetGet);
ZEND_METHOD(ThreadedArray, offsetSet);
ZEND_METHOD(ThreadedArray, offsetExists);
ZEND_METHOD(ThreadedArray, offsetUnset);


static const zend_function_entry class_ThreadedArray_methods[] = {
	ZEND_ME(ThreadedArray, chunk, arginfo_class_ThreadedArray_chunk, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, count, arginfo_class_ThreadedArray_count, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, fromArray, arginfo_class_ThreadedArray_fromArray, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(ThreadedArray, merge, arginfo_class_ThreadedArray_merge, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, pop, arginfo_class_ThreadedArray_pop, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, shift, arginfo_class_ThreadedArray_shift, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, offsetGet, arginfo_class_ThreadedArray_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, offsetSet, arginfo_class_ThreadedArray_offsetSet, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, offsetExists, arginfo_class_ThreadedArray_offsetExists, ZEND_ACC_PUBLIC)
	ZEND_ME(ThreadedArray, offsetUnset, arginfo_class_ThreadedArray_offsetUnset, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_ThreadedArray(zend_class_entry *class_entry_ThreadedBase, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_ArrayAccess)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "ThreadedArray", class_ThreadedArray_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_ThreadedBase);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Countable, class_entry_ArrayAccess);

	return class_entry;
}
