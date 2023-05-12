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
#include <src/object.h>
#include <src/globals.h>
#include <src/prepare.h>
#include <src/routine.h>

/* {{{ */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_zend_object_t* base, zend_class_entry *entry, unsigned int scope); /* }}} */

/* {{{ */
static void pthreads_ts_object_free(pthreads_zend_object_t* base); /* }}} */


/* {{{ object iterator structure */
typedef struct _pthreads_iterator_t {
	zend_object_iterator zit;
	zval object;
	HashPosition position;
} pthreads_iterator_t; /* }}} */

static inline void pthreads_object_iterator_dtor(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF)
		zval_ptr_dtor(&iterator->zit.data);
	zval_ptr_dtor(&iterator->object);
}

static inline int pthreads_object_iterator_validate(pthreads_iterator_t* iterator) {
	return (iterator->position != HT_INVALID_IDX) ? SUCCESS : FAILURE;
}

static inline zval* pthreads_object_iterator_current_data(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		zval_ptr_dtor(&iterator->zit.data);
	}

	pthreads_store_data(Z_OBJ(iterator->object), &iterator->zit.data, &iterator->position);

	if (Z_ISUNDEF(iterator->zit.data)) {
		return &EG(uninitialized_zval);
	}

	return &iterator->zit.data;
}

static inline void pthreads_object_iterator_current_key(pthreads_iterator_t* iterator, zval* result) {
	pthreads_store_key(Z_OBJ(iterator->object), result, &iterator->position);
}

static inline void pthreads_object_iterator_move_forward(pthreads_iterator_t* iterator) {
	pthreads_store_forward(Z_OBJ(iterator->object), &iterator->position);
}

static inline void pthreads_object_iterator_rewind(pthreads_iterator_t* iterator) {
	pthreads_store_reset(Z_OBJ(iterator->object), &iterator->position);
}

static HashTable* pthreads_object_iterator_get_gc(pthreads_iterator_t* iterator, zval** table, int* n) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		*n = 1;
		*table = &iterator->zit.data;
	} else {
		*n = 0;
		*table = NULL;
	}

	return NULL;
}

static zend_object_iterator_funcs pthreads_object_iterator_funcs = {
	(void (*) (zend_object_iterator*))                    pthreads_object_iterator_dtor,
	(int (*)(zend_object_iterator *))                     pthreads_object_iterator_validate,
	(zval* (*)(zend_object_iterator *))                   pthreads_object_iterator_current_data,
	(void (*)(zend_object_iterator *, zval *))            pthreads_object_iterator_current_key,
	(void (*)(zend_object_iterator *))                    pthreads_object_iterator_move_forward,
	(void (*)(zend_object_iterator *))                    pthreads_object_iterator_rewind,
	NULL,
	(HashTable* (*)(zend_object_iterator*, zval**, int*)) pthreads_object_iterator_get_gc,
};

zend_object_iterator* pthreads_object_iterator_create(zend_class_entry *ce, zval *object, int by_ref) {
	pthreads_iterator_t *iterator;

	if (by_ref) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"iteration by reference is not allowed on %s objects", ZSTR_VAL(ce->name));
		return NULL;
	}

	iterator = (pthreads_iterator_t*)
		ecalloc(1, sizeof(pthreads_iterator_t));

	zend_iterator_init((zend_object_iterator*)iterator);

	ZVAL_COPY(&iterator->object, object);
	ZVAL_UNDEF(&iterator->zit.data);

	pthreads_store_reset(Z_OBJ(iterator->object), &iterator->position);

	iterator->zit.funcs = &pthreads_object_iterator_funcs;

	return (zend_object_iterator*) iterator;
}

/* {{{ */
zend_object* pthreads_thread_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* thread = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(thread, entry, PTHREADS_SCOPE_THREAD);
	thread->std.handlers = &pthreads_ts_ce_handlers;

	return &thread->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_worker_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* worker = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(worker, entry, PTHREADS_SCOPE_WORKER);
	if (worker->original_zobj == NULL) {
		//this may be a connection and not the original object
		worker->worker_data = pthreads_worker_data_alloc(&worker->ts_obj->monitor);
	}

	worker->std.handlers = &pthreads_ts_ce_handlers;

	return &worker->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_threaded_base_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(threaded, entry, 0);
	threaded->std.handlers = &pthreads_ts_ce_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_threaded_array_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(threaded, entry, 0);
	threaded->std.handlers = &pthreads_array_ce_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
void pthreads_current_thread(zval *return_value) {
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		ZVAL_COPY(return_value, &PTHREADS_ZG(this));
	}
} /* }}} */

/* {{{ */
static inline int _pthreads_connect_nolock(pthreads_zend_object_t* source, pthreads_zend_object_t* destination) {
	if (source && destination) {
		destination->ts_obj = source->ts_obj;
		++destination->ts_obj->refcount;
		if (source->original_zobj != NULL) {
			destination->original_zobj = source->original_zobj;
		} else {
			destination->original_zobj = source;
		}

		if (destination->std.properties)
			zend_hash_clean(destination->std.properties);

		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ */
static int pthreads_connect(pthreads_zend_object_t* source, pthreads_zend_object_t* destination) {
	int result = FAILURE;
	if(pthreads_globals_lock()){
		result = _pthreads_connect_nolock(source, destination);
		pthreads_globals_unlock();
	}
	return result;
} /* }}} */

/* {{{ */
zend_bool pthreads_object_connect(pthreads_zend_object_t* address, zval *object) {
	zend_bool valid = 0;
	if (!pthreads_globals_lock()) {
		return valid;
	}
	if (pthreads_globals_object_valid(address)) {
		valid = 1;
		pthreads_zend_object_t *pthreads = address;

		if (PTHREADS_THREAD_OWNS(pthreads)) {
			/* we own the object in this context */
			ZVAL_OBJ(object, &pthreads->std);
			Z_ADDREF_P(object);
		} else {
			/* we do not own the object, create or find a connection */
			pthreads_zend_object_t* connection = (pthreads_zend_object_t*) zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong)pthreads->ts_obj);
			if (connection) {
				/* a connection already exists on this thread, reuse it */
				ZVAL_OBJ_COPY(object, &connection->std);
			} else {
				zend_class_entry* ce = NULL;
				/* no connection exists, create a new one */
				ce = pthreads_prepare_single_class(&pthreads->owner, pthreads->std.ce);
				PTHREADS_ZG(connecting_object) = pthreads;
				object_init_ex(object, ce);
				PTHREADS_ZG(connecting_object) = NULL;
				connection = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
				zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong)connection->ts_obj, connection);
			}
		}
	}

	pthreads_globals_unlock();
	return valid;
} /* }}} */

/* {{{ */
static inline void pthreads_base_write_property_defaults(pthreads_zend_object_t* base) {
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

			zend_string* interned_name = pthreads_globals_add_interned_string(info->name);
			ZVAL_INTERNED_STR(&key, interned_name);

			value = OBJ_PROP(&base->std, info->offset);
			if (!Z_ISUNDEF_P(value)) {
				result = pthreads_store_write(
					&base->std, &key,
					value,
					PTHREADS_STORE_NO_COERCE_ARRAY
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
static pthreads_object_t* pthreads_ts_object_ctor(unsigned int scope) {
	pthreads_object_t* ts_obj = calloc(1, sizeof(pthreads_object_t));
	ts_obj->scope = scope;
	ts_obj->refcount = 1;
	pthreads_monitor_init(&ts_obj->monitor);
	ts_obj->creator.ls = TSRMLS_CACHE;
	ts_obj->creator.id = pthreads_self();
	pthreads_store_init(&ts_obj->props);
	return ts_obj;
} /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_zend_object_t* base, zend_class_entry *entry, unsigned int scope) {
	base->owner.ls = TSRMLS_CACHE;
	base->owner.id = pthreads_self();
	base->original_zobj = NULL;
	base->worker_data = NULL;

	zend_object_std_init(&base->std, entry);
	object_properties_init(&base->std, entry);

	if (PTHREADS_ZG(connecting_object) != NULL) {
		pthreads_connect(PTHREADS_ZG(connecting_object), base);
	} else {
		base->ts_obj = pthreads_ts_object_ctor(scope);
		pthreads_base_write_property_defaults(base);
	}

	base->local_props_modcount = base->ts_obj->props.modcount - 1;
} /* }}} */

/* {{{ */
void pthreads_base_dtor(zend_object *object) {
	//TODO: how does this play with __destruct() calls (e.g. adding a ref to self)?
	pthreads_zend_object_t* base = PTHREADS_FETCH_FROM(object);

	if (base->original_zobj == NULL && PTHREADS_IN_CREATOR(base) && (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) &&
		pthreads_monitor_check(&base->ts_obj->monitor, PTHREADS_MONITOR_STARTED) &&
		!pthreads_monitor_check(&base->ts_obj->monitor, PTHREADS_MONITOR_JOINED)) {
		zend_call_method_with_0_params(object, object->ce, NULL, "join", NULL);

		//in case the user join didn't call the parent - make sure we join, otherwise bad things may happen
		if (!pthreads_monitor_check(&base->ts_obj->monitor, PTHREADS_MONITOR_JOINED)) {
			pthreads_join(base);
		}
	}

	zend_objects_destroy_object(object);
} /* }}} */

/* {{{ */
static void pthreads_ts_object_free(pthreads_zend_object_t* base) {
	pthreads_object_t *ts_obj = base->ts_obj;
	if (pthreads_monitor_lock(&ts_obj->monitor)) {
		pthreads_store_destroy(&ts_obj->props);
		pthreads_monitor_unlock(&ts_obj->monitor);
	}

	pthreads_monitor_destroy(&ts_obj->monitor);

	free(ts_obj);
} /* }}} */

/* {{{ */
void pthreads_base_free(zend_object *object) {
	pthreads_zend_object_t* base = PTHREADS_FETCH_FROM(object);

	if (base->worker_data) {
		pthreads_worker_data_free(base->worker_data);
	}

	if (zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong)base->ts_obj) == base) {
		/* this is the primary connection to the TS object on the current thread - destroy it */
		zend_hash_index_del(&PTHREADS_ZG(resolve), (zend_ulong)base->ts_obj);
	}

	if (pthreads_globals_lock()) {
		if (--base->ts_obj->refcount == 0) {
			pthreads_ts_object_free(base);
		} else {
			pthreads_store_persist_local_properties(object);
		}
		pthreads_globals_object_delete(base);
		pthreads_globals_unlock();
	}

	zend_object_std_dtor(object);
} /* }}} */

/* {{{ */
HashTable* pthreads_base_gc(zend_object *object, zval **table, int *n) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	if (threaded->worker_data != NULL) {
		zend_get_gc_buffer* buffer = pthreads_worker_get_gc_extra(threaded->worker_data);
		zend_get_gc_buffer_use(buffer, table, n);
	} else {
		*table = NULL;
		*n = 0;
	}
	return object->properties;
} /* }}} */
