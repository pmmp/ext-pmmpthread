/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 4d940aff2680785e0149c2ffa4b4081e41f11d55 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Pool___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, class, IS_STRING, 0, "Worker::class")
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, ctor, IS_ARRAY, 0, "[]")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Pool_collect, 0, 0, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, collector, IS_CALLABLE, 0, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Pool_resize, 0, 1, IS_VOID, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Pool_shutdown, 0, 0, IS_VOID, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Pool_submit, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, task, ThreadedRunnable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_class_Pool_submitTo, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, worker, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, task, ThreadedRunnable, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Pool, __construct);
ZEND_METHOD(Pool, collect);
ZEND_METHOD(Pool, resize);
ZEND_METHOD(Pool, shutdown);
ZEND_METHOD(Pool, submit);
ZEND_METHOD(Pool, submitTo);


static const zend_function_entry class_Pool_methods[] = {
	ZEND_ME(Pool, __construct, arginfo_class_Pool___construct, ZEND_ACC_PUBLIC)
	ZEND_ME(Pool, collect, arginfo_class_Pool_collect, ZEND_ACC_PUBLIC)
	ZEND_ME(Pool, resize, arginfo_class_Pool_resize, ZEND_ACC_PUBLIC)
	ZEND_ME(Pool, shutdown, arginfo_class_Pool_shutdown, ZEND_ACC_PUBLIC)
	ZEND_ME(Pool, submit, arginfo_class_Pool_submit, ZEND_ACC_PUBLIC)
	ZEND_ME(Pool, submitTo, arginfo_class_Pool_submitTo, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Pool(void)
{
	zend_class_entry ce, *class_entry;

	INIT_CLASS_ENTRY(ce, "Pool", class_Pool_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);

	zval property_size_default_value;
	ZVAL_NULL(&property_size_default_value);
	zend_string *property_size_name = zend_string_init("size", sizeof("size") - 1, 1);
	zend_declare_property_ex(class_entry, property_size_name, &property_size_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_size_name);

	zval property_class_default_value;
	ZVAL_NULL(&property_class_default_value);
	zend_string *property_class_name = zend_string_init("class", sizeof("class") - 1, 1);
	zend_declare_property_ex(class_entry, property_class_name, &property_class_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_class_name);

	zval property_workers_default_value;
	ZVAL_NULL(&property_workers_default_value);
	zend_string *property_workers_name = zend_string_init("workers", sizeof("workers") - 1, 1);
	zend_declare_property_ex(class_entry, property_workers_name, &property_workers_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_workers_name);

	zval property_ctor_default_value;
	ZVAL_NULL(&property_ctor_default_value);
	zend_string *property_ctor_name = zend_string_init("ctor", sizeof("ctor") - 1, 1);
	zend_declare_property_ex(class_entry, property_ctor_name, &property_ctor_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_ctor_name);

	zval property_last_default_value;
	ZVAL_LONG(&property_last_default_value, 0);
	zend_string *property_last_name = zend_string_init("last", sizeof("last") - 1, 1);
	zend_declare_property_ex(class_entry, property_last_name, &property_last_default_value, ZEND_ACC_PROTECTED, NULL);
	zend_string_release(property_last_name);

	return class_entry;
}
