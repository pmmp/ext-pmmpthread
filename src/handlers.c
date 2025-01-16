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

#if PHP_VERSION_ID >= 80400
	zend_std_get_properties_ex(&threaded->std);
#else
	rebuild_object_properties(&threaded->std);
#endif

	pmmpthread_store_tohash(
		&threaded->std, threaded->std.properties);

	return threaded->std.properties;
} /* }}} */

/* {{{ */
zval *pmmpthread_get_property_ptr_ptr_stub(zend_object *object, zend_string *member, int type, void **cache_slot) { return NULL; }
/* }}} */

/* {{{ */
zval* pmmpthread_read_dimension(PMMPTHREAD_READ_DIMENSION_PASSTHRU_D) {
	if (pmmpthread_store_read(object, member, NULL, type, rv) == FAILURE) {
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
	zval result;

	zend_property_info* info = zend_get_property_info(object->ce, member, 0);
	if (info != NULL && info != ZEND_WRONG_PROPERTY_INFO) {
		ZVAL_STR(&zmember, info->name);
	} else {
		ZVAL_STR(&zmember, member);
	}
	//this moves the value to cache for zend_std_read_property() to work on
	pmmpthread_store_read_ex(object, &zmember, info, type, rv, 1);
	if (EG(exception)) {
		rv = &EG(uninitialized_zval);
	} else {
		//no cache for now - we don't want the VM bypassing this handler
		zend_std_read_property(object, member, type, NULL, rv);
		//tidy property cache so we don't read wrong values later
		if (!pmmpthread_store_retain_in_local_cache(rv)) {
			pmmpthread_store_clean_local_property(object, &zmember, info);
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
	if (pmmpthread_store_write(object, member, NULL, value, PMMPTHREAD_STORE_NO_COERCE_ARRAY) == FAILURE && !EG(exception)){
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
	zval tmp;
	zend_guard* guard;

	//no cache for now - cache would allow the VM to bypass this handler
	//std_write may coerce the var to a different type, so we need to use the result
	value = zend_std_write_property(object, member, value, NULL);

	if (value != &EG(error_zval)) {
		zval* real_value = NULL;
		zval zmember;
		ZVAL_UNDEF(&zmember);
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info != NULL) {
			if (info != ZEND_WRONG_PROPERTY_INFO) {
				ZVAL_STR(&zmember, info->name);
				real_value = OBJ_PROP(object, info->offset);
			}
		} else if (object->properties != NULL) {
			ZVAL_STR(&zmember, member);
			real_value = zend_hash_find(object->properties, member);
		}
		if (real_value != NULL) {
			zend_bool cached = 0;
			if (pmmpthread_store_write_ex(object, &zmember, info, real_value, PMMPTHREAD_STORE_NO_COERCE_ARRAY, &cached) == FAILURE && !EG(exception)) {
				zend_throw_error(
					pmmpthread_ce_nts_value_error,
					"Cannot assign non-thread-safe value of type %s to thread-safe class property %s::$%s",
					zend_zval_type_name(value),
					ZSTR_VAL(object->ce->name),
					ZSTR_VAL(member)
				);
				value = &EG(error_zval);
			}
			if (!cached) {
				pmmpthread_store_clean_local_property(object, &zmember, info);
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
			if (info != NULL && PMMPTHREAD_OBJECT_PROPERTY(info)) {
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
	pmmpthread_store_delete(object, member, NULL);
}

void pmmpthread_unset_property(PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D) {
	zval zmember;
	
	zend_std_unset_property(object, member, NULL);
	if (!EG(exception)) {
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info != NULL && info != ZEND_WRONG_PROPERTY_INFO) {
			ZVAL_STR(&zmember, info->name);
		} else {
			ZVAL_STR(&zmember, member);
		}
		pmmpthread_store_delete(object, &zmember, info);
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
