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

#include <src/pthreads.h>
#include <src/store.h>

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

/* {{{ proto mixed ThreadedArray::offsetGet(mixed $offset)
	Gets an offset from the array */
PHP_METHOD(ThreadedArray, offsetGet)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	pthreads_store_read(Z_OBJ_P(getThis()), key, BP_VAR_R, return_value);
} /* }}} */

/* {{{ proto void ThreadedArray::offsetSet(mixed $offset, mixed $value)
	Sets an offset to the given value */
PHP_METHOD(ThreadedArray, offsetSet)
{
	zval* key;
	zval* value;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ZVAL(key)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	pthreads_store_write(Z_OBJ_P(getThis()), key, value, PTHREADS_STORE_NO_COERCE_ARRAY);
} /* }}} */

/* {{{ proto bool ThreadedArray::offsetExists(mixed $offset)
	Returns whether an offset exists in the array */
PHP_METHOD(ThreadedArray, offsetExists)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(pthreads_store_isset(Z_OBJ_P(getThis()), key, ZEND_PROPERTY_ISSET));
} /* }}} */

/* {{{ proto void ThreadedArray::offsetUnset(mixed $offset)
	Removes an offset from the array, if it exists */
PHP_METHOD(ThreadedArray, offsetUnset)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	pthreads_store_delete(Z_OBJ_P(getThis()), key);
} /* }}} */
