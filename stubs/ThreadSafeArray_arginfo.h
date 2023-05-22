/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: dfd5b567a245e8ab9d2f05d71013202ec8c76bf3 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_chunk, 0, 1, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, preserve, _IS_BOOL, 0, "false")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_count, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_fromArray, 0, 1, pmmp\\thread\\ThreadSafeArray, 0)
	ZEND_ARG_TYPE_INFO(0, array, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_merge, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_MASK(0, from, MAY_BE_ARRAY|MAY_BE_OBJECT, NULL)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, overwrite, _IS_BOOL, 0, "true")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_pop, 0, 0, IS_MIXED, 0)
ZEND_END_ARG_INFO()

#define arginfo_class_pmmp_thread_ThreadSafeArray_shift arginfo_class_pmmp_thread_ThreadSafeArray_pop

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_offsetGet, 0, 1, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_offsetSet, 0, 2, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_offsetExists, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_pmmp_thread_ThreadSafeArray_offsetUnset, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_MIXED, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(pmmp_thread_ThreadSafeArray, chunk);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, count);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, fromArray);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, merge);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, pop);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, shift);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, offsetGet);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, offsetSet);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, offsetExists);
ZEND_METHOD(pmmp_thread_ThreadSafeArray, offsetUnset);


static const zend_function_entry class_pmmp_thread_ThreadSafeArray_methods[] = {
	ZEND_ME(pmmp_thread_ThreadSafeArray, chunk, arginfo_class_pmmp_thread_ThreadSafeArray_chunk, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, count, arginfo_class_pmmp_thread_ThreadSafeArray_count, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, fromArray, arginfo_class_pmmp_thread_ThreadSafeArray_fromArray, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, merge, arginfo_class_pmmp_thread_ThreadSafeArray_merge, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, pop, arginfo_class_pmmp_thread_ThreadSafeArray_pop, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, shift, arginfo_class_pmmp_thread_ThreadSafeArray_shift, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, offsetGet, arginfo_class_pmmp_thread_ThreadSafeArray_offsetGet, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, offsetSet, arginfo_class_pmmp_thread_ThreadSafeArray_offsetSet, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, offsetExists, arginfo_class_pmmp_thread_ThreadSafeArray_offsetExists, ZEND_ACC_PUBLIC)
	ZEND_ME(pmmp_thread_ThreadSafeArray, offsetUnset, arginfo_class_pmmp_thread_ThreadSafeArray_offsetUnset, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_pmmp_thread_ThreadSafeArray(zend_class_entry *class_entry_pmmp_thread_ThreadSafe, zend_class_entry *class_entry_Countable, zend_class_entry *class_entry_ArrayAccess)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "pmmp\\thread", "ThreadSafeArray", class_pmmp_thread_ThreadSafeArray_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_pmmp_thread_ThreadSafe);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 2, class_entry_Countable, class_entry_ArrayAccess);

	return class_entry;
}
