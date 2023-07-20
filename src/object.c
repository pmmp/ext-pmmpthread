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
#include <src/object.h>
#include <src/globals.h>
#include <src/prepare.h>
#include <src/routine.h>

/* {{{ */
extern zend_module_entry pmmpthread_module_entry; /* }}} */

/* {{{ */
static void pmmpthread_base_ctor(pmmpthread_zend_object_t* base, zend_class_entry *entry, unsigned int scope); /* }}} */

/* {{{ */
static void pmmpthread_ts_object_free(pmmpthread_zend_object_t* base); /* }}} */


/* {{{ object iterator structure */
typedef struct _pmmpthread_iterator_t {
	zend_object_iterator zit;
	zval object;
	HashPosition position;
} pmmpthread_iterator_t; /* }}} */

static inline void pmmpthread_object_iterator_dtor(pmmpthread_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF)
		zval_ptr_dtor(&iterator->zit.data);
	zval_ptr_dtor(&iterator->object);
}

static inline int pmmpthread_object_iterator_validate(pmmpthread_iterator_t* iterator) {
	return (iterator->position != HT_INVALID_IDX) ? SUCCESS : FAILURE;
}

static inline zval* pmmpthread_object_iterator_current_data(pmmpthread_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		zval_ptr_dtor(&iterator->zit.data);
	}

	pmmpthread_store_data(Z_OBJ(iterator->object), &iterator->zit.data, &iterator->position);

	if (Z_ISUNDEF(iterator->zit.data)) {
		return &EG(uninitialized_zval);
	}

	return &iterator->zit.data;
}

static inline void pmmpthread_object_iterator_current_key(pmmpthread_iterator_t* iterator, zval* result) {
	pmmpthread_store_key(Z_OBJ(iterator->object), result, &iterator->position);
}

static inline void pmmpthread_object_iterator_move_forward(pmmpthread_iterator_t* iterator) {
	pmmpthread_store_forward(Z_OBJ(iterator->object), &iterator->position);
}

static inline void pmmpthread_object_iterator_rewind(pmmpthread_iterator_t* iterator) {
	pmmpthread_store_reset(Z_OBJ(iterator->object), &iterator->position);
}

static HashTable* pmmpthread_object_iterator_get_gc(pmmpthread_iterator_t* iterator, zval** table, int* n) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		*n = 1;
		*table = &iterator->zit.data;
	} else {
		*n = 0;
		*table = NULL;
	}

	return NULL;
}

static zend_object_iterator_funcs pmmpthread_object_iterator_funcs = {
	(void (*) (zend_object_iterator*))                    pmmpthread_object_iterator_dtor,
	(int (*)(zend_object_iterator *))                     pmmpthread_object_iterator_validate,
	(zval* (*)(zend_object_iterator *))                   pmmpthread_object_iterator_current_data,
	(void (*)(zend_object_iterator *, zval *))            pmmpthread_object_iterator_current_key,
	(void (*)(zend_object_iterator *))                    pmmpthread_object_iterator_move_forward,
	(void (*)(zend_object_iterator *))                    pmmpthread_object_iterator_rewind,
	NULL,
	(HashTable* (*)(zend_object_iterator*, zval**, int*)) pmmpthread_object_iterator_get_gc,
};

zend_object_iterator* pmmpthread_object_iterator_create(zend_class_entry *ce, zval *object, int by_ref) {
	pmmpthread_iterator_t *iterator;

	if (by_ref) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"iteration by reference is not allowed on %s objects", ZSTR_VAL(ce->name));
		return NULL;
	}

	iterator = (pmmpthread_iterator_t*)
		ecalloc(1, sizeof(pmmpthread_iterator_t));

	zend_iterator_init((zend_object_iterator*)iterator);

	ZVAL_COPY(&iterator->object, object);
	ZVAL_UNDEF(&iterator->zit.data);

	pmmpthread_store_reset(Z_OBJ(iterator->object), &iterator->position);

	iterator->zit.funcs = &pmmpthread_object_iterator_funcs;

	return (zend_object_iterator*) iterator;
}

/* {{{ */
zend_object* pmmpthread_thread_ctor(zend_class_entry *entry) {
	pmmpthread_zend_object_t* thread = pmmpthread_globals_object_alloc(
		sizeof(pmmpthread_zend_object_t) + zend_object_properties_size(entry));

	pmmpthread_base_ctor(thread, entry, PMMPTHREAD_SCOPE_THREAD);
	thread->std.handlers = &pmmpthread_ts_ce_handlers;

	return &thread->std;
} /* }}} */

/* {{{ */
zend_object* pmmpthread_worker_ctor(zend_class_entry *entry) {
	pmmpthread_zend_object_t* worker = pmmpthread_globals_object_alloc(
		sizeof(pmmpthread_zend_object_t) + zend_object_properties_size(entry));

	pmmpthread_base_ctor(worker, entry, PMMPTHREAD_SCOPE_WORKER);
	if (PMMPTHREAD_IN_CREATOR(worker)) {
		//this may be a connection and not the original object
		worker->worker_data = pmmpthread_worker_data_alloc(&worker->ts_obj->monitor);
	}

	worker->std.handlers = &pmmpthread_ts_ce_handlers;

	return &worker->std;
} /* }}} */

/* {{{ */
zend_object* pmmpthread_threaded_base_ctor(zend_class_entry *entry) {
	pmmpthread_zend_object_t* threaded = pmmpthread_globals_object_alloc(
		sizeof(pmmpthread_zend_object_t) + zend_object_properties_size(entry));

	pmmpthread_base_ctor(threaded, entry, 0);
	threaded->std.handlers = &pmmpthread_ts_ce_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pmmpthread_threaded_array_ctor(zend_class_entry *entry) {
	pmmpthread_zend_object_t* threaded = pmmpthread_globals_object_alloc(
		sizeof(pmmpthread_zend_object_t) + zend_object_properties_size(entry));

	pmmpthread_base_ctor(threaded, entry, 0);
	threaded->std.handlers = &pmmpthread_array_ce_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
void pmmpthread_current_thread(zval *return_value) {
	if (Z_TYPE(PMMPTHREAD_ZG(this)) != IS_UNDEF) {
		ZVAL_COPY(return_value, &PMMPTHREAD_ZG(this));
	}
} /* }}} */

/* {{{ */
static inline int _pmmpthread_connect_nolock(pmmpthread_zend_object_t* source, pmmpthread_zend_object_t* destination) {
	if (source && destination) {
		destination->ts_obj = source->ts_obj;
		++destination->ts_obj->refcount;

		if (destination->std.properties)
			zend_hash_clean(destination->std.properties);

		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ */
static int pmmpthread_connect(pmmpthread_zend_object_t* source, pmmpthread_zend_object_t* destination) {
	int result = FAILURE;
	if(pmmpthread_globals_lock()){
		result = _pmmpthread_connect_nolock(source, destination);
		pmmpthread_globals_unlock();
	}
	return result;
} /* }}} */

/* {{{ */
zend_bool pmmpthread_object_connect(pmmpthread_zend_object_t* address, zval *object) {
	zend_bool valid = 0;
	if (!pmmpthread_globals_lock()) {
		return valid;
	}
	if (pmmpthread_globals_object_valid(address)) {
		valid = 1;
		pmmpthread_zend_object_t *original = address;

		if (PMMPTHREAD_THREAD_OWNS(original)) {
			/* we own the object in this context */
			ZVAL_OBJ(object, &original->std);
			Z_ADDREF_P(object);
		} else {
			/* we do not own the object, create or find a connection */
			pmmpthread_zend_object_t* connection = (pmmpthread_zend_object_t*) zend_hash_index_find_ptr(&PMMPTHREAD_ZG(resolve), (zend_ulong)original->ts_obj);
			if (connection) {
				/* a connection already exists on this thread, reuse it */
				ZVAL_OBJ_COPY(object, &connection->std);
			} else {
				zend_class_entry* ce = NULL;
				/* no connection exists, create a new one */
				ce = pmmpthread_prepare_single_class(&original->owner, original->std.ce);
				PMMPTHREAD_ZG(connecting_object) = original;
				object_init_ex(object, ce);
				PMMPTHREAD_ZG(connecting_object) = NULL;
			}
		}
	}

	pmmpthread_globals_unlock();
	return valid;
} /* }}} */

/* {{{ */
static inline void pmmpthread_base_write_property_defaults(pmmpthread_zend_object_t* base) {
	zend_property_info *info;
	zval key;

	zend_class_entry* ce = base->std.ce;

	while (ce != NULL) {
		ZEND_HASH_FOREACH_PTR(&ce->properties_info, info) {
			zval* value;
			int result;

			if (info->flags & ZEND_ACC_STATIC) {
				continue;
			}

			zend_string* interned_name = pmmpthread_globals_add_interned_string(info->name);
			ZVAL_INTERNED_STR(&key, interned_name);

			value = OBJ_PROP(&base->std, info->offset);
			if (!Z_ISUNDEF_P(value)) {
				result = pmmpthread_store_write(
					&base->std, &key,
					value,
					PMMPTHREAD_STORE_NO_COERCE_ARRAY
				);
				if (result == FAILURE) {
					zend_throw_error(
						NULL,
						"Cannot use non-thread-safe default of type %s for thread-safe class property %s::$%s",
						zend_zval_type_name(value),
						ZSTR_VAL(ce->name),
						ZSTR_VAL(Z_STR(key))
					);
					break;
				}
				ZVAL_UNDEF(value);
			}
		} ZEND_HASH_FOREACH_END();

		ce = ce->parent;
	}
} /* }}} */

/* {{{ */
static pmmpthread_object_t* pmmpthread_ts_object_ctor(unsigned int scope) {
	pmmpthread_object_t* ts_obj = calloc(1, sizeof(pmmpthread_object_t));
	ts_obj->scope = scope;
	ts_obj->refcount = 1;
	pmmpthread_monitor_init(&ts_obj->monitor);
	ts_obj->creator.ls = TSRMLS_CACHE;
	ts_obj->creator.id = pmmpthread_self();
	pmmpthread_store_init(&ts_obj->props);
	return ts_obj;
} /* }}} */

/* {{{ */
static void pmmpthread_base_ctor(pmmpthread_zend_object_t* base, zend_class_entry *entry, unsigned int scope) {
	base->owner.ls = TSRMLS_CACHE;
	base->owner.id = pmmpthread_self();
	base->worker_data = NULL;

	zend_object_std_init(&base->std, entry);
	object_properties_init(&base->std, entry);

	if (PMMPTHREAD_ZG(connecting_object) != NULL) {
		pmmpthread_connect(PMMPTHREAD_ZG(connecting_object), base);
	} else {
		base->ts_obj = pmmpthread_ts_object_ctor(scope);
		pmmpthread_base_write_property_defaults(base);
	}
	zend_hash_index_add_ptr(&PMMPTHREAD_ZG(resolve), (zend_ulong)base->ts_obj, base);

	base->local_props_modcount = base->ts_obj->props.modcount - 1;
} /* }}} */

/* {{{ */
void pmmpthread_base_dtor(zend_object *object) {
	//TODO: how does this play with __destruct() calls (e.g. adding a ref to self)?
	pmmpthread_zend_object_t* base = PMMPTHREAD_FETCH_FROM(object);

	if (PMMPTHREAD_IN_CREATOR(base) && (PMMPTHREAD_IS_THREAD(base)||PMMPTHREAD_IS_WORKER(base)) &&
		pmmpthread_monitor_check(&base->ts_obj->monitor, PMMPTHREAD_MONITOR_STARTED) &&
		!pmmpthread_monitor_check(&base->ts_obj->monitor, PMMPTHREAD_MONITOR_JOINED)) {
		zend_call_method_with_0_params(object, object->ce, NULL, "join", NULL);

		//in case the user join didn't call the parent - make sure we join, otherwise bad things may happen
		if (!pmmpthread_monitor_check(&base->ts_obj->monitor, PMMPTHREAD_MONITOR_JOINED)) {
			pmmpthread_join(base);
		}
	}

	zend_objects_destroy_object(object);
} /* }}} */

/* {{{ */
static void pmmpthread_ts_object_free(pmmpthread_zend_object_t* base) {
	pmmpthread_object_t *ts_obj = base->ts_obj;
	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		pmmpthread_store_destroy(&ts_obj->props);
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}

	pmmpthread_monitor_destroy(&ts_obj->monitor);

	free(ts_obj);
} /* }}} */

/* {{{ */
void pmmpthread_base_free(zend_object *object) {
	pmmpthread_zend_object_t* base = PMMPTHREAD_FETCH_FROM(object);

	if (base->worker_data) {
		pmmpthread_worker_data_free(base->worker_data);
	}

	if (zend_hash_index_find_ptr(&PMMPTHREAD_ZG(resolve), (zend_ulong)base->ts_obj) == base) {
		/* this is the primary connection to the TS object on the current thread - destroy it */
		zend_hash_index_del(&PMMPTHREAD_ZG(resolve), (zend_ulong)base->ts_obj);
	}

	if (PMMPTHREAD_ZG(thread_shared_globals) == base) {
		//clean up our local connection to the shared globals
		//opcache preload creates a fake request, so we need to ensure that
		//globals are cleaned up properly for the real main thread
		PMMPTHREAD_ZG(thread_shared_globals) = NULL;
	}
	if (pmmpthread_globals_lock()) {
		if (--base->ts_obj->refcount == 0) {
			pmmpthread_ts_object_free(base);
		} else {
			pmmpthread_store_persist_local_properties(object);
		}
		pmmpthread_globals_object_delete(base);
		if (PMMPTHREAD_G(thread_shared_globals) == base) {
			//if this is the original shared globals object, clean up the ref
			//opcache preload creates a fake request, so we need to ensure that
			//globals are cleaned up properly for the real main thread
			PMMPTHREAD_G(thread_shared_globals) = NULL;
		}
		pmmpthread_globals_unlock();
	}

	zend_object_std_dtor(object);
} /* }}} */

/* {{{ */
HashTable* pmmpthread_base_gc(zend_object *object, zval **table, int *n) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);
	if (threaded->worker_data != NULL) {
		zend_get_gc_buffer* buffer = pmmpthread_worker_get_gc_extra(threaded->worker_data);
		zend_get_gc_buffer_use(buffer, table, n);
	} else {
		*table = NULL;
		*n = 0;
	}
	return object->properties;
} /* }}} */
