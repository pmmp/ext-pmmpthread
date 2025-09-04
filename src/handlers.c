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

#if PHP_VERSION_ID >= 80400
/* {{{
* Copy pasta from zend_object_handlers.c because the equivalent over there had to be static...
*/
static bool zend_is_in_hook(const zend_property_info *prop_info)
{
	zend_execute_data *execute_data = EG(current_execute_data);
	if (!execute_data || !EX(func) || !EX(func)->common.prop_info) {
		return false;
	}

	const zend_property_info *parent_info = EX(func)->common.prop_info;
	ZEND_ASSERT(prop_info->prototype && parent_info->prototype);
	return prop_info->prototype == parent_info->prototype;
} /* }}} */

/* {{{
* Copy pasta from zend_object_handlers.c because the equivalent over there had to be static...
*/
static bool zend_should_call_hook(const zend_property_info *prop_info, const zend_object *obj)
{
	if (!zend_is_in_hook(prop_info)) {
		return true;
	}

	/* execute_data and This are guaranteed to be set if zend_is_in_hook() returns true. */
	zend_object *parent_obj = Z_OBJ(EG(current_execute_data)->This);
	if (parent_obj == obj) {
		return false;
	}

	/* pmmpthread objects can't be lazy */
	/*
	if (zend_object_is_lazy_proxy(parent_obj)
		&& zend_lazy_object_initialized(parent_obj)
		&& zend_lazy_object_get_instance(parent_obj) == obj) {
		return false;
	}
	*/

	return true;
} /* }}} */

/* {{{
* Copy pasta from zend_object_handlers.c because the equivalent over there had to be static...
*/
static void zend_throw_no_prop_backing_value_access(zend_string *class_name, zend_string *prop_name, bool is_read)
{
	zend_throw_error(NULL, "Must not %s virtual property %s::$%s",
		is_read ? "read from" : "write to",
		ZSTR_VAL(class_name), ZSTR_VAL(prop_name));
} /* }}} */

/* {{{
* Copy pasta from zend_object_handlers.c because the equivalent over there had to be static...
*/
static bool zend_call_get_hook(
	const zend_property_info *prop_info, zend_string *prop_name,
	zend_function *get, zend_object *zobj, zval *rv)
{
	if (!zend_should_call_hook(prop_info, zobj)) {
		if (UNEXPECTED(prop_info->flags & ZEND_ACC_VIRTUAL)) {
			zend_throw_no_prop_backing_value_access(zobj->ce->name, prop_name, /* is_read */ true);
		}
		return false;
	}

	GC_ADDREF(zobj);
	zend_call_known_instance_method_with_0_params(get, zobj, rv);
	OBJ_RELEASE(zobj);

	return true;
}
#endif //PHP_VERSION_ID >= 80400

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
			return &EG(uninitialized_zval);
		}

		if (info != NULL) {
			if (info->flags & ZEND_ACC_STATIC) {
				info = NULL;
#if PHP_VERSION_ID >= 80400
			} else if (info->hooks != NULL) {
				zend_function* get = info->hooks[ZEND_PROPERTY_HOOK_GET];
				if (!get) {
					if (info->flags & ZEND_ACC_VIRTUAL) {
						zend_throw_error(NULL, "Property %s::$%s is write-only",
							ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
						return &EG(uninitialized_zval);
					}
					//php-src has some checks for indirection here - we don't need these
					//since we don't allow indirect modification on ThreadSafe objects anyway

					//fallthru to pmmpthread_store_read() below
				} else {
					if (zend_call_get_hook(info, member, get, object, rv)) {
						return rv;
					}
					if (EG(exception)) {
						return &EG(uninitialized_zval);
					}
				}
#endif
			}
		}

		if (info == NULL) { //dynamic property
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
	zval tmp;
	zend_guard* guard;

	ZVAL_STR(&zmember, member);
	ZVAL_UNDEF(&tmp);

	if (object->ce->__set && (guard = zend_get_property_guard(object, member)) && !((*guard) & IN_SET)) {
		zval rv;
		ZVAL_UNDEF(&rv);

		(*guard) |= IN_SET;
		zend_call_known_instance_method_with_2_params(object->ce->__set, object, &rv, &zmember, value);
		(*guard) &= ~IN_SET;

		if (Z_TYPE(rv) != IS_UNDEF)
			zval_dtor(&rv);
	} else {
		bool write_store = true;
		zend_property_info* info = zend_get_property_info(object->ce, member, 0);
		if (info == ZEND_WRONG_PROPERTY_INFO) {
			return &EG(error_zval);
		}

		if (info != NULL && (info->flags & ZEND_ACC_STATIC) == 0) {
			ZVAL_STR(&zmember, info->name); //use mangled name to avoid private member shadowing issues

#if PHP_VERSION_ID >= 80400
			if (info->hooks != NULL) {
				zend_function* set = info->hooks[ZEND_PROPERTY_HOOK_SET];

				if (!set) {
					if (info->flags & ZEND_ACC_VIRTUAL) {
						zend_throw_error(NULL, "Property %s::$%s is read-only", ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
						value = &EG(error_zval);
						write_store = false;
					}
					//fallthru to normal write
				} else if (!zend_should_call_hook(info, object)) {
					if (info->flags & ZEND_ACC_VIRTUAL) {
						zend_throw_no_prop_backing_value_access(object->ce->name, member, /* is_read */ false);
						value = &EG(error_zval);
						write_store = false;
					}
					//fallthru to normal write
				} else {
					//call hook or die trying
					write_store = false;
					if (UNEXPECTED(info->flags & ZEND_ACC_PPP_SET_MASK
						&& !zend_asymmetric_property_has_set_access(info))) {
						zend_asymmetric_visibility_property_modification_error(info, "modify");
						value = &EG(error_zval);
					} else {
						GC_ADDREF(object);
						zend_call_known_instance_method_with_1_params(set, object, NULL, value);
						OBJ_RELEASE(object);
					}
				}
			}
#endif
			if (write_store) { //if hooked, the setter will do the type verification, so we can skip this
				zend_execute_data* execute_data = EG(current_execute_data);
				bool strict = execute_data
					&& execute_data->func
					&& ZEND_CALL_USES_STRICT_TYPES(EG(current_execute_data));

				//zend_verify_property_type() might modify the value
				//value is not copied before we receive it, so it might be
				//from opcache protected memory which we can't modify
				ZVAL_COPY(&tmp, value);
				value = &tmp;

				if (ZEND_TYPE_IS_SET(info->type) && !zend_verify_property_type(info, value, strict)) {
					write_store = false;
				}
			}
		}

		if (write_store && pmmpthread_store_write(object, &zmember, value, PMMPTHREAD_STORE_NO_COERCE_ARRAY) == FAILURE && !EG(exception)) {
			zend_throw_error(
				pmmpthread_ce_nts_value_error,
				"Cannot assign non-thread-safe value of type %s to thread-safe class property %s::$%s",
				zend_zval_type_name(value),
				ZSTR_VAL(object->ce->name),
				ZSTR_VAL(member)
			);
		}
	}

	zval_ptr_dtor(&tmp);

	return EG(exception) ? &EG(error_zval) : NULL;
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
			//TODO: this doesn't account for ZEND_PROPERTY_NOT_EMPTY
			isset = zend_is_true(&rv);
			zval_dtor(&rv);
		}
	} else {
		zend_property_info* info = zend_get_property_info(object->ce, member, 1);
		bool read_store = true;
		if (info != ZEND_WRONG_PROPERTY_INFO) {
			if (info != NULL && (info->flags & ZEND_ACC_STATIC) == 0) {
				ZVAL_STR(&zmember, info->name);
#if PHP_VERSION_ID >= 80400
				if (info->hooks != NULL) {
					zend_function* get = info->hooks[ZEND_PROPERTY_HOOK_GET];

					if (has_set_exists == ZEND_PROPERTY_EXISTS) {
						isset = 1;
						read_store = false;
					} else if (get == NULL) {
						if (info->flags & ZEND_ACC_VIRTUAL) {
							zend_throw_error(NULL, "Property %s::$%s is write-only",
								ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
							isset = 0;
							read_store = false;
						}

						//fallthru to store read
					} else {
						zval rv;
						if (zend_call_get_hook(info, member, get, object, &rv)) {
							read_store = false;
							if (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) {
								isset = zend_is_true(&rv);
							} else {
								ZEND_ASSERT(has_set_exists == ZEND_PROPERTY_ISSET);
								isset = Z_TYPE(rv) != IS_NULL
									&& (Z_TYPE(rv) != IS_REFERENCE || Z_TYPE_P(Z_REFVAL(rv)) != IS_NULL);
							}
							zval_ptr_dtor(&rv);
						} else if (EG(exception)) {
							read_store = false;
						}
					}
				}
#endif
			}

			if (read_store) {
				isset = pmmpthread_store_isset(object, &zmember, has_set_exists);
			}
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
#if PHP_VERSION_ID >= 80400
				if (info->hooks != NULL) {
					zend_throw_error(NULL, "Cannot unset hooked property %s::$%s",
						ZSTR_VAL(object->ce->name), ZSTR_VAL(member));
					return;
				}
#endif
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
