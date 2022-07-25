/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_CLASS_THREADED_ARRAY_H
#define HAVE_PTHREADS_CLASS_THREADED_ARRAY_H

PHP_METHOD(ThreadedArray, merge);
PHP_METHOD(ThreadedArray, shift);
PHP_METHOD(ThreadedArray, chunk);
PHP_METHOD(ThreadedArray, pop);
PHP_METHOD(ThreadedArray, count);
PHP_METHOD(ThreadedArray, fromArray);

ZEND_BEGIN_ARG_INFO_EX(ThreadedArray_merge, 0, 0, 1)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedArray_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedArray_chunk, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, preserve)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedArray_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedArray_count, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ThreadedArray_fromArray, 0, 1, ThreadedArray, 0)
	ZEND_ARG_ARRAY_INFO(0, array, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_threaded_array_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREADED_ARRAY
#	define HAVE_PTHREADS_CLASS_THREADED_ARRAY
zend_function_entry pthreads_threaded_array_methods[] = {
	PHP_ME(ThreadedArray, merge, ThreadedArray_merge, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedArray, shift, ThreadedArray_shift, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedArray, chunk, ThreadedArray_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedArray, pop, ThreadedArray_pop, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedArray, count, ThreadedArray_count, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedArray, fromArray, ThreadedArray_fromArray, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/* {{{ proto boolean ThreadedArray::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced ThreadedArray */
PHP_METHOD(ThreadedArray, merge)
{
	zval *from;
	zend_bool overwrite = 1;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ZVAL(from)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(overwrite)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL((pthreads_store_merge(Z_OBJ_P(getThis()), from, overwrite, PTHREADS_STORE_COERCE_ARRAY)==SUCCESS));
} /* }}} */

/* {{{ proto mixed ThreadedArray::shift()
	Will shift the first member from the object */
PHP_METHOD(ThreadedArray, shift)
{
	zend_parse_parameters_none_throw();

	pthreads_store_shift(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto mixed ThreadedArray::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
PHP_METHOD(ThreadedArray, chunk)
{
	zend_long size;
	zend_bool preserve = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(size)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(preserve)
	ZEND_PARSE_PARAMETERS_END();

	pthreads_store_chunk(Z_OBJ_P(getThis()), size, preserve, return_value);
} /* }}} */

/* {{{ proto mixed ThreadedArray::pop()
	Will pop the last member from the object */
PHP_METHOD(ThreadedArray, pop)
{
	zend_parse_parameters_none_throw();

	pthreads_store_pop(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto boolean ThreadedArray::count()
	Will return the size of the properties table */
PHP_METHOD(ThreadedArray, count)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, 0);

	pthreads_store_count(
		Z_OBJ_P(getThis()), &Z_LVAL_P(return_value));
} /* }}} */

/* {{{ proto ThreadedArray ThreadedArray::fromArray(array $array)
	Converts the given array to a ThreadedArray object (recursively) */
PHP_METHOD(ThreadedArray, fromArray)
{
	zval *input;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY(input)
	ZEND_PARSE_PARAMETERS_END();

	object_init_ex(return_value, pthreads_threaded_array_entry);
	pthreads_store_merge(Z_OBJ_P(return_value), input, 1, PTHREADS_STORE_COERCE_ARRAY);
} /* }}} */

#	endif
#endif
