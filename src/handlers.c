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

#include <src/handlers.h>
#include <src/object.h>
#include <src/globals.h>

#define IN_GET      (1<<0)
#define IN_SET      (1<<1)
#define IN_UNSET    (1<<2)
#define IN_ISSET    (1<<3)

typedef uint32_t zend_guard;

/* {{{ */
int pmmpthread_count_properties(PMMPTHREAD_COUNT_PASSTHRU_D) {
	return pmmpthread_store_count(object, count);
} /* }}} */

/* {{{ */
HashTable* pmmpthread_read_debug(PMMPTHREAD_READ_DEBUG_PASSTHRU_D) {
	HashTable *table = emalloc(sizeof(HashTable));

	zend_hash_init(table, 8, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;

	pmmpthread_store_tohash(object, table);

	return table;
} /* }}} */

/* {{{ */
HashTable* pmmpthread_read_properties(PMMPTHREAD_READ_PROPERTIES_PASSTHRU_D) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);

	rebuild_object_properties(&threaded->std);

	pmmpthread_store_tohash(
		&threaded->std, threaded->std.properties);

	return threaded->std.properties;
} /* }}} */

/* {{{ */
zval *pmmpthread_get_property_ptr_ptr_stub(zend_object *object, zend_string *member, int type, void **cache_slot) { return NULL; }
/* }}} */

/* {{{ */
zval* pmmpthread_read_dimension(PMMPTHREAD_READ_DIMENSION_PASSTHRU_D) {
	if (pmmpthread_store_read(object, member, type, rv) == FAILURE) {
		//TODO: this ought to generate warnings, but this is a pain right now due to key type juggling
		//for now this maintains the v4 behaviour of silently generating NULL, which is better than segfaulting
		if (!EG(exception)) {
			//returning uninitialized_zval may cause indirect modification errors to be generated
			//we don't want this if an exception was thrown
			rv = &EG(uninitialized_zval);
		}
	}

	return rv;
}

zval* pmmpthread_read_property(PMMPTHREAD_READ_PROPERTY_PASSTHRU_D) {
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__get && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_GET)) {
		(*guard) |= IN_GET;
		zend_call_known_instance_method_with_1_params(object->ce->__get, object, rv, &zmember);
		(*guard) &= ~IN_GET;
	} else {
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info == ZEND_WRONG_PROPERTY_INFO) {
			rv = &EG(uninitialized_zval);
		} else if (info == NULL || (info->flags & ZEND_ACC_STATIC) != 0) { //dynamic property
			if (pmmpthread_store_read(object, &zmember, type, rv) == FAILURE) {
				if (type != BP_VAR_IS) {
					zend_error(E_WARNING, "Undefined property: %s::$%s", ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
				}
				rv = &EG(uninitialized_zval);
			}
		} else {
			//defined property, use mangled name
			ZVAL_STR(&zmember, info->name);

			if (pmmpthread_store_read(object, &zmember, type, rv) == FAILURE) {
				if (type != BP_VAR_IS && !EG(exception)) {
					zend_throw_error(NULL, "Typed property %s::$%s must not be accessed before initialization",
						ZSTR_VAL(info->ce->name),
						ZSTR_VAL(member));
				}
				rv = &EG(uninitialized_zval);
			}
		}
	}
	return rv;
}
/* }}} */

/* {{{ */
zval* pmmpthread_read_property_deny(PMMPTHREAD_READ_PROPERTY_PASSTHRU_D) {
	if (type != BP_VAR_IS) {
		zend_error(E_WARNING, "Undefined property: %s::$%s", ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
	}
	rv = &EG(uninitialized_zval);
	return rv;
} /* }}} */

/* {{{ */
void pmmpthread_write_dimension(PMMPTHREAD_WRITE_DIMENSION_PASSTHRU_D) {
	if (pmmpthread_store_write(object, member, value, PMMPTHREAD_STORE_NO_COERCE_ARRAY) == FAILURE && !EG(exception)){
		zend_throw_error(
			pmmpthread_ce_nts_value_error,
			"Cannot assign non-thread-safe value of type %s to %s",
			zend_zval_type_name(value),
			ZSTR_VAL(object->ce->name)
		);
	}
}

zval* pmmpthread_write_property(PMMPTHREAD_WRITE_PROPERTY_PASSTHRU_D) {
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
	} else {
		bool ok = true;
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info != ZEND_WRONG_PROPERTY_INFO) {
			if (info != NULL && (info->flags & ZEND_ACC_STATIC) == 0) {
				ZVAL_STR(&zmember, info->name); //use mangled name to avoid private member shadowing issues

				zend_execute_data* execute_data = EG(current_execute_data);
				bool strict = execute_data
					&& execute_data->func
					&& ZEND_CALL_USES_STRICT_TYPES(EG(current_execute_data));

				//zend_verify_property_type() might modify the value
				//value is not copied before we receive it, so it might be
				//from opcache protected memory which we can't modify
				zval tmp;
				ZVAL_COPY(&tmp, value);
				value = &tmp;

				if (ZEND_TYPE_IS_SET(info->type) && !zend_verify_property_type(info, value, strict)) {
					ok = false;
					zval_ptr_dtor(&tmp);
				}
			}

			if (ok && pmmpthread_store_write(object, &zmember, value, PMMPTHREAD_STORE_NO_COERCE_ARRAY) == FAILURE && !EG(exception)) {
				zend_throw_error(
					pmmpthread_ce_nts_value_error,
					"Cannot assign non-thread-safe value of type %s to thread-safe class property %s::$%s",
					zend_zval_type_name(value),
					ZSTR_VAL(object->ce->name),
					ZSTR_VAL(member)
				);
			}
		}
	}

	return EG(exception) ? &EG(error_zval) : value;
}
/* }}} */

/* {{{ */
zval* pmmpthread_write_property_deny(PMMPTHREAD_WRITE_PROPERTY_PASSTHRU_D) {
	zend_throw_error(NULL, "Cannot create dynamic property %s::$%s",
		ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
	return &EG(uninitialized_zval);
} /* }}} */

/* {{{ */
int pmmpthread_has_dimension(PMMPTHREAD_HAS_DIMENSION_PASSTHRU_D) {
	return pmmpthread_store_isset(object, member, has_set_exists);
}

int pmmpthread_has_property(PMMPTHREAD_HAS_PROPERTY_PASSTHRU_D) {
	int isset = 0;
	zval zmember;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);

	if (object->ce->__isset && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_ISSET)) {
		zval rv;
		ZVAL_UNDEF(&rv);

		(*guard) |= IN_ISSET;
		zend_call_known_instance_method_with_1_params(object->ce->__isset, object, &rv, &zmember);
		(*guard) &= ~IN_ISSET;

		if (Z_TYPE(rv) != IS_UNDEF) {
			isset = zend_is_true(&rv);
			zval_dtor(&rv);
		}
	} else {
		zend_property_info* info = zend_get_property_info(object->ce, member, 1);
		if (info != ZEND_WRONG_PROPERTY_INFO) {
			if (info != NULL && (info->flags & ZEND_ACC_STATIC) == 0) {
				ZVAL_STR(&zmember, info->name); //defined property, use mangled name
			}
			isset = pmmpthread_store_isset(object, &zmember, has_set_exists);
		} else isset = 0;
	}
	return isset;
}
/* }}} */

/* {{{ */
int pmmpthread_has_property_deny(PMMPTHREAD_HAS_PROPERTY_PASSTHRU_D) {
	return 0;
} /* }}} */

/* {{{ */
void pmmpthread_unset_dimension(PMMPTHREAD_UNSET_DIMENSION_PASSTHRU_D) {
	pmmpthread_store_delete(object, member);
}

void pmmpthread_unset_property(PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D) {
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
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info != ZEND_WRONG_PROPERTY_INFO) {
			if (info != NULL && (info->flags & ZEND_ACC_STATIC) == 0) {
				ZVAL_STR(&zmember, info->name); //defined property, use mangled name
			}
			pmmpthread_store_delete(object, &zmember);
		}
	}
}
/* }}} */

/* {{{ */
void pmmpthread_unset_property_deny(PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D) {
	//NOOP
} /* }}} */

/* {{{ */
int pmmpthread_compare_objects(PMMPTHREAD_COMPARE_PASSTHRU_D) {
	pmmpthread_object_t *left = PMMPTHREAD_FETCH_TS_FROM(Z_OBJ_P(op1));
	pmmpthread_object_t *right = PMMPTHREAD_FETCH_TS_FROM(Z_OBJ_P(op2));

	/* comparing property tables is not useful or efficient for threaded objects */
	/* in addition, it might be useful to know if two variables are infact the same physical threaded object */
	if (left == right) {
		return 0;
	}

	return 1;
} /* }}} */
