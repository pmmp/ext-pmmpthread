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

#include <src/handlers.h>
#include <src/object.h>
#include <src/globals.h>

#define IN_GET      (1<<0)
#define IN_SET      (1<<1)
#define IN_UNSET    (1<<2)
#define IN_ISSET    (1<<3)

typedef uint32_t zend_guard;

/* {{{ */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D) {
	return pthreads_store_count(object, count);
} /* }}} */

/* {{{ */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D) {
	HashTable *table = emalloc(sizeof(HashTable));

	zend_hash_init(table, 8, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;

	pthreads_store_tohash(object, table);

	return table;
} /* }}} */

/* {{{ */
HashTable* pthreads_read_properties(PTHREADS_READ_PROPERTIES_PASSTHRU_D) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

	rebuild_object_properties(&threaded->std);

	pthreads_store_tohash(
		&threaded->std, threaded->std.properties);

	return threaded->std.properties;
} /* }}} */

/* {{{ */
zval *pthreads_get_property_ptr_ptr_stub(zend_object *object, zend_string *member, int type, void **cache_slot) { return NULL; }
/* }}} */

/* {{{ */
zval * pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D) {
	pthreads_store_read(object, member, type, rv);

	return rv;
}

zval* pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__get && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_GET)) {
		(*guard) |= IN_GET;
		zend_call_known_instance_method_with_1_params(object->ce->__get, object, rv, &zmember);
		(*guard) &= ~IN_GET;
	} else {
		pthreads_store_read(object, &zmember, type, rv);
	}
	return rv;
}
/* }}} */

/* {{{ */
void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) {
	if (pthreads_store_write(object, member, value, PTHREADS_STORE_NO_COERCE_ARRAY) == FAILURE){
		zend_throw_error(
			NULL,
			"Cannot assign non-thread-safe value of type %s to ThreadedArray",
			zend_get_type_by_const(Z_TYPE_P(value))
		);
	}
}

zval* pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__set && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_SET)) {
		zval rv;
		ZVAL_UNDEF(&rv);

		(*guard) |= IN_SET;
		zend_call_known_instance_method_with_2_params(object->ce->__set, object, &rv, &zmember, value);
		(*guard) &= ~IN_SET;

		if (Z_TYPE(rv) != IS_UNDEF)
			zval_dtor(&rv);
	} else if (pthreads_store_write(object, &zmember, value, PTHREADS_STORE_NO_COERCE_ARRAY) == FAILURE) {
		zend_throw_error(
			NULL,
			"Cannot assign non-thread-safe value of type %s to Threaded class property %s::$%s",
			zend_get_type_by_const(Z_TYPE_P(value)),
			ZSTR_VAL(object->ce->name),
			ZSTR_VAL(member)
		);
	}

	return EG(exception) ? &EG(error_zval) : value;
}
/* }}} */

/* {{{ */
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D) {
	return pthreads_store_isset(object, member, has_set_exists);
}

int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	int isset;
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__isset && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_ISSET)) {
		zval rv;
		ZVAL_UNDEF(&rv);

		(*guard) |= IN_ISSET;
		zend_call_known_instance_method_with_1_params(object->ce->__set, object, &rv, &zmember);
		(*guard) &= ~IN_ISSET;

		if (Z_TYPE(rv) != IS_UNDEF) {
			isset = zend_is_true(&rv);
			zval_dtor(&rv);
		}
	} else {
		isset = pthreads_store_isset(object, &zmember, has_set_exists);
	}
	return isset;
}
/* }}} */

/* {{{ */
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) {
	pthreads_store_delete(object, member);
}

void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__unset && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_UNSET)) {
		zval rv;
		ZVAL_UNDEF(&rv);

		(*guard) |= IN_UNSET;
		zend_call_known_instance_method_with_1_params(object->ce->__unset, object, &rv, &zmember);
		(*guard) &= ~IN_UNSET;

		if (Z_TYPE(rv) != IS_UNDEF) {
			zval_dtor(&rv);
		}
	} else {
		pthreads_store_delete(object, &zmember);
	}
}
/* }}} */

/* {{{ */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D) {
	switch (type) {
		case IS_ARRAY: {
			pthreads_store_tohash(from, Z_ARRVAL_P(to));
			return SUCCESS;
		} break;
	}

	return zend_handlers->cast_object(from, to, type);
} /* }}} */

/* {{{ */
int pthreads_compare_objects(PTHREADS_COMPARE_PASSTHRU_D) {
	pthreads_object_t *left = PTHREADS_FETCH_TS_FROM(Z_OBJ_P(op1));
	pthreads_object_t *right = PTHREADS_FETCH_TS_FROM(Z_OBJ_P(op2));

	/* comparing property tables is not useful or efficient for threaded objects */
	/* in addition, it might be useful to know if two variables are infact the same physical threaded object */
	if (left->monitor == right->monitor) {
		return 0;
	}

	return 1;
} /* }}} */
