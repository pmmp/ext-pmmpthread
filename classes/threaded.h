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
#ifndef HAVE_PTHREADS_CLASS_THREADED_H
#define HAVE_PTHREADS_CLASS_THREADED_H

#include <src/compat.h>

PHP_METHOD(Threaded, merge);
PHP_METHOD(Threaded, shift);
PHP_METHOD(Threaded, chunk);
PHP_METHOD(Threaded, pop);
PHP_METHOD(Threaded, count);
PHP_METHOD(Threaded, fromArray);

ZEND_BEGIN_ARG_INFO_EX(Threaded_merge, 0, 0, 1)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_chunk, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, preserve)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_count, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(Threaded_fromArray, 0, 1, Threaded, 0)
	ZEND_ARG_ARRAY_INFO(0, array, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_threaded_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREADED
#	define HAVE_PTHREADS_CLASS_THREADED
zend_function_entry pthreads_threaded_methods[] = {
	PHP_ME(Threaded, merge, Threaded_merge, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, shift, Threaded_shift, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, chunk, Threaded_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, pop, Threaded_pop, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, count, Threaded_count, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, fromArray, Threaded_fromArray, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_FE_END
};

/* {{{ proto boolean Threaded::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Threaded */
PHP_METHOD(Threaded, merge)
{
	zval *from;
	zend_bool overwrite = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &from, &overwrite) != SUCCESS) {
		return;
	}

	RETURN_BOOL((pthreads_store_merge(Z_OBJ_P(getThis()), from, overwrite, PTHREADS_STORE_COERCE_ARRAY)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Threaded::shift()
	Will shift the first member from the object */
PHP_METHOD(Threaded, shift)
{
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_store_shift(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto mixed Threaded::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
PHP_METHOD(Threaded, chunk)
{
	zend_long size;
	zend_bool preserve = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|b", &size, &preserve) != SUCCESS) {
		return;
	}

	pthreads_store_chunk(Z_OBJ_P(getThis()), size, preserve, return_value);
} /* }}} */

/* {{{ proto mixed Threaded::pop()
	Will pop the last member from the object */
PHP_METHOD(Threaded, pop)
{
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_store_pop(Z_OBJ_P(getThis()), return_value);
} /* }}} */

/* {{{ proto boolean Threaded::count()
	Will return the size of the properties table */
PHP_METHOD(Threaded, count)
{
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	ZVAL_LONG(return_value, 0);

	pthreads_store_count(
		Z_OBJ_P(getThis()), &Z_LVAL_P(return_value));
} /* }}} */

/* {{{ proto Threaded Threaded::fromArray(array $array)
	Converts the given array to a Threaded object (recursively) */
PHP_METHOD(Threaded, fromArray)
{
	zval *input;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_ARRAY(input)
	ZEND_PARSE_PARAMETERS_END();

	object_init_ex(return_value, pthreads_threaded_entry);
	pthreads_store_merge(Z_OBJ_P(return_value), input, 1, PTHREADS_STORE_COERCE_ARRAY);
} /* }}} */

#	endif
#endif
