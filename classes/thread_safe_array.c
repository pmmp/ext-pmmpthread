/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
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

#include <src/pmmpthread.h>
#include <src/store.h>
#include <src/handlers.h>

#define ThreadSafeArray_method(name) PHP_METHOD(pmmp_thread_ThreadSafeArray, name)

/* {{{ proto boolean ThreadSafeArray::merge(array|object $data, [boolean $overwrite = true])
	Will merge data with the referenced ThreadSafeArray */
ThreadSafeArray_method(merge)
{
	zval *from;
	zend_bool overwrite = 1;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_ARRAY_OR_OBJECT(from)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(overwrite)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL((pmmpthread_store_merge(Z_OBJ_P(getThis()), from, overwrite, PMMPTHREAD_STORE_COERCE_ARRAY)==SUCCESS));
} /* }}} */

/* {{{ proto mixed ThreadSafeArray::shift()
	Will shift the first member from the object */
ThreadSafeArray_method(shift)
{
	zend_parse_parameters_none_throw();

	pmmpthread_store_shift(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto mixed ThreadSafeArray::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
ThreadSafeArray_method(chunk)
{
	zend_long size;
	zend_bool preserve = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 2)
		Z_PARAM_LONG(size)
		Z_PARAM_OPTIONAL
		Z_PARAM_BOOL(preserve)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_store_chunk(Z_OBJ_P(getThis()), size, preserve, return_value);
} /* }}} */

/* {{{ proto mixed ThreadSafeArray::pop()
	Will pop the last member from the object */
ThreadSafeArray_method(pop)
{
	zend_parse_parameters_none_throw();

	pmmpthread_store_pop(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto boolean ThreadSafeArray::count()
	Will return the size of the properties table */
ThreadSafeArray_method(count)
{
	zend_parse_parameters_none_throw();

	ZVAL_LONG(return_value, 0);

	pmmpthread_store_count(
		Z_OBJ_P(getThis()), &Z_LVAL_P(return_value));
} /* }}} */

/* {{{ proto ThreadSafeArray ThreadSafeArray::fromArray(array $array)
	Converts the given array to a ThreadSafeArray object (recursively) */
ThreadSafeArray_method(fromArray)
{
	zval *input;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY(input)
	ZEND_PARSE_PARAMETERS_END();

	object_init_ex(return_value, pmmpthread_ce_array);
	pmmpthread_store_merge(Z_OBJ_P(return_value), input, 1, PMMPTHREAD_STORE_COERCE_ARRAY);
} /* }}} */

/* {{{ proto mixed ThreadSafeArray::offsetGet(mixed $offset)
	Gets an offset from the array */
ThreadSafeArray_method(offsetGet)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_read_dimension(Z_OBJ_P(getThis()), key, BP_VAR_R, return_value);
} /* }}} */

/* {{{ proto void ThreadSafeArray::offsetSet(mixed $offset, mixed $value)
	Sets an offset to the given value */
ThreadSafeArray_method(offsetSet)
{
	zval* key;
	zval* value;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_ZVAL(key)
		Z_PARAM_ZVAL(value)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_write_dimension(Z_OBJ_P(getThis()), key, value);
} /* }}} */

/* {{{ proto bool ThreadSafeArray::offsetExists(mixed $offset)
	Returns whether an offset exists in the array */
ThreadSafeArray_method(offsetExists)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	RETURN_BOOL(pmmpthread_has_dimension(Z_OBJ_P(getThis()), key, ZEND_PROPERTY_ISSET));
} /* }}} */

/* {{{ proto void ThreadSafeArray::offsetUnset(mixed $offset)
	Removes an offset from the array, if it exists */
ThreadSafeArray_method(offsetUnset)
{
	zval* key;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ZVAL(key)
	ZEND_PARSE_PARAMETERS_END();

	pmmpthread_unset_dimension(Z_OBJ_P(getThis()), key);
} /* }}} */
