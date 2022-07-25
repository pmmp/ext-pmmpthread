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
#ifndef HAVE_PTHREADS_HANDLERS
#define HAVE_PTHREADS_HANDLERS

#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <src/handlers.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#define IN_GET      (1<<0)
#define IN_SET      (1<<1)
#define IN_UNSET    (1<<2)
#define IN_ISSET    (1<<3)

typedef uint32_t zend_guard;
#define pthreads_get_guard(o, m) \
	((Z_TYPE_P(m) == IS_STRING) ? zend_get_property_guard(o, Z_STR_P(m)) : NULL)
/* }}} */

#define PTHREADS_NO_PROPERTIES(object) \
	zend_throw_exception_ex(spl_ce_RuntimeException, 0, "%s objects are not allowed to have properties", ZSTR_VAL(object->ce->name))

/* {{{ */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D) {
	return pthreads_store_count(object, count);
} /* }}} */

/* {{{ */
int pthreads_count_properties_disallow(PTHREADS_COUNT_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return -1;
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
	zend_guard *guard = NULL;
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

	rebuild_object_properties(&threaded->std);

	if (threaded->std.ce->__get && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_GET)) {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;

		fci.size = sizeof(zend_fcall_info);
		fci.retval = rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
		fcc.function_handler = threaded->std.ce->__get;
		fcc.object = &threaded->std;

		(*guard) |= IN_GET;
		zend_call_function(&fci, &fcc);
		(*guard) &= ~IN_GET;

		zend_fcall_info_args_clear(&fci, 1);
	} else {
		pthreads_store_read(&threaded->std, member, type, rv);
	}

	return rv;
}

zval* pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zval zmember;
	ZVAL_STR(&zmember, member);
	return pthreads_read_dimension(object, &zmember, type, rv);
}
/* }}} */

/* {{{ */
zval * pthreads_read_property_disallow (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return &EG(uninitialized_zval);
}

zval* pthreads_read_dimension_disallow(PTHREADS_READ_DIMENSION_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return &EG(uninitialized_zval);
}
/* }}} */

/* {{{ */
static zend_bool pthreads_is_supported_type(zval *value) {
	switch(Z_TYPE_P(value)){
		case IS_UNDEF:
		case IS_STRING:
		case IS_LONG:
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_NULL:
		case IS_DOUBLE:
		case IS_RESOURCE:
		case IS_TRUE:
		case IS_FALSE:
			return 1;
		default:
			return 0;
	}
} /* }}} */

/* {{{ */
void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) {
	if (!pthreads_is_supported_type(value)) {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0,
			"pthreads detected an attempt to use unsupported data (%s) for %s::$%s",
			zend_get_type_by_const(Z_TYPE_P(value)),
			ZSTR_VAL(object->ce->name), Z_STRVAL_P(member));
	} else {
		pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

		rebuild_object_properties(&threaded->std);

		zend_guard *guard = NULL;
		if ((member && Z_TYPE_P(member) != IS_NULL) &&
			threaded->std.ce->__set &&
			(guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_SET)) {
			zend_fcall_info fci = empty_fcall_info;
			zend_fcall_info_cache fcc = empty_fcall_info_cache;
			zval rv;

			ZVAL_UNDEF(&rv);

			fci.size = sizeof(zend_fcall_info);
			fci.retval = &rv;
			fci.object = &threaded->std;
			zend_fcall_info_argn(&fci, 2, member, value);
			fcc.function_handler = threaded->std.ce->__set;
			fcc.object = &threaded->std;

			(*guard) |= IN_SET;
			zend_call_function(&fci, &fcc);
			(*guard) &= ~IN_SET;

			if (Z_TYPE(rv) != IS_UNDEF)
				zval_dtor(&rv);
			zend_fcall_info_args_clear(&fci, 1);
		} else {
			pthreads_store_write(&threaded->std, member, value);
		}
	}
}

zval* pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	zval zmember;
	ZVAL_STR(&zmember, member);
	pthreads_write_dimension(object, &zmember, value);
	return EG(exception) ? &EG(error_zval) : value;
}
/* }}} */

/* {{{ */
zval* pthreads_write_property_disallow(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return &EG(error_zval);
}

void pthreads_write_dimension_disallow(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) { PTHREADS_NO_PROPERTIES(object); }
/* }}} */

/* {{{ */
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D) {
	int isset = 0;
	zend_guard *guard = NULL;
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

	if (threaded->std.ce->__isset && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_ISSET)) {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		zval rv;

		ZVAL_UNDEF(&rv);

		fci.size = sizeof(zend_fcall_info);
		fci.retval = &rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
		fcc.function_handler = threaded->std.ce->__isset;
		fcc.object = &threaded->std;

		(*guard) |= IN_ISSET;
		zend_call_function(&fci, &fcc);
		(*guard) &= ~IN_ISSET;

		if (Z_TYPE(rv) != IS_UNDEF) {
			isset =
				zend_is_true(&rv);
			zval_dtor(&rv);
		}
		zend_fcall_info_args_clear(&fci, 1);
	} else {
		isset = pthreads_store_isset(&threaded->std, member, has_set_exists);
	}

	return isset;
}
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	cache = NULL;
	zval zmember;
	ZVAL_STR(&zmember, member);
	return pthreads_has_dimension(object, &zmember, has_set_exists);
}
/* }}} */

/* {{{ */
int pthreads_has_property_disallow(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return 0;
}

int pthreads_has_dimension_disallow(PTHREADS_HAS_DIMENSION_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
	return 0;
}
/* }}} */

/* {{{ */
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) {
	zend_guard *guard = NULL;
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

	rebuild_object_properties(&threaded->std);

	if (threaded->std.ce->__unset && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_UNSET)) {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		zval rv;

		ZVAL_UNDEF(&rv);

		fci.size = sizeof(zend_fcall_info);
		fci.retval = &rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
		fcc.function_handler = threaded->std.ce->__unset;
		fcc.object = &threaded->std;

		(*guard) |= IN_UNSET;
		zend_call_function(&fci, &fcc);
		(*guard) &= ~IN_UNSET;

		if (Z_TYPE(rv) != IS_UNDEF) {
			zval_dtor(&rv);
		}
		zend_fcall_info_args_clear(&fci, 1);
	} else {
		if (pthreads_store_delete(&threaded->std, member) == SUCCESS){

		}
	}
}
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	cache = NULL;
	zval zmember;
	ZVAL_STR(&zmember, member);
	pthreads_unset_dimension(object, &zmember);
}
/* }}} */

/* {{{ */
void pthreads_unset_property_disallow(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	PTHREADS_NO_PROPERTIES(object);
}
void pthreads_unset_dimension_disallow(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) { PTHREADS_NO_PROPERTIES(object); }
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
#endif
