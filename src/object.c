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

/* {{{ */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_zend_object_t* base, zend_class_entry *entry, unsigned int scope); /* }}} */

/* {{{ */
static void pthreads_ts_object_free(pthreads_zend_object_t* base); /* }}} */

/* {{{ */
static void * pthreads_routine(pthreads_routine_arg_t *arg); /* }}} */

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

static zend_object_iterator_funcs pthreads_object_iterator_funcs = {
	(void (*) (zend_object_iterator*))         pthreads_object_iterator_dtor,
	(int (*)(zend_object_iterator *))          pthreads_object_iterator_validate,
	(zval* (*)(zend_object_iterator *))        pthreads_object_iterator_current_data,
	(void (*)(zend_object_iterator *, zval *)) pthreads_object_iterator_current_key,
	(void (*)(zend_object_iterator *))         pthreads_object_iterator_move_forward,
	(void (*)(zend_object_iterator *))         pthreads_object_iterator_rewind,
	NULL
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
static void pthreads_routine_init(pthreads_routine_arg_t *r, pthreads_zend_object_t *thread, zend_ulong thread_options) {
	r->thread = thread;
	r->options = thread_options;
	r->ready  = pthreads_monitor_alloc();
	pthreads_monitor_add(
		r->thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED);
}

static void pthreads_routine_wait(pthreads_routine_arg_t *r) {
	pthreads_monitor_wait_until(
		r->ready, PTHREADS_MONITOR_READY);
	pthreads_monitor_free(r->ready);
}

static void pthreads_routine_free(pthreads_routine_arg_t *r) {
	pthreads_monitor_remove(
		r->thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED);
	pthreads_monitor_free(r->ready);
} /* }}} */

/* {{{ */
zend_object* pthreads_thread_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* thread = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(thread, entry, PTHREADS_SCOPE_THREAD);
	thread->std.handlers = &pthreads_threaded_base_handlers;

	return &thread->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_worker_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* worker = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(worker, entry, PTHREADS_SCOPE_WORKER);

	worker->worker_data = pthreads_worker_data_alloc(worker->ts_obj->monitor);

	worker->std.handlers = &pthreads_threaded_base_handlers;

	return &worker->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_threaded_base_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(threaded, entry, PTHREADS_SCOPE_THREADED);
	threaded->std.handlers = &pthreads_threaded_base_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_threaded_array_ctor(zend_class_entry *entry) {
	pthreads_zend_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_zend_object_t) + zend_object_properties_size(entry));

	pthreads_base_ctor(threaded, entry, PTHREADS_SCOPE_THREADED);
	threaded->std.handlers = &pthreads_threaded_array_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
int pthreads_threaded_serialize(zval *object, unsigned char **buffer, size_t *buflen, zend_serialize_data *data) {
	pthreads_zend_object_t *address = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	if (address->original_zobj != NULL) {
		address = address->original_zobj;
	}
	(*buflen) = snprintf(NULL, 0, ":%" PRIuPTR ":", (uintptr_t) address);
	(*buffer) = emalloc((*buflen) + 1);
	sprintf((char*) (*buffer), ":%" PRIuPTR ":", (uintptr_t) address);
	(*buffer)[(*buflen)] = 0;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_threaded_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t buflen, zend_unserialize_data *data) {
	pthreads_zend_object_t *address = NULL;

	if (!sscanf((const char*) buffer, ":%" PRIuPTR ":", (uintptr_t*)&address)) {
		zend_throw_exception_ex(pthreads_ce_ThreadedConnectionException, 0,
			"pthreads detected an attempt to connect to a corrupted object");
		return FAILURE;
	}

	if (!address) {
		zend_throw_exception_ex(pthreads_ce_ThreadedConnectionException, 0,
			"pthreads detected an attempt to connect to an invalid object");
		return FAILURE;
	}

	if (!pthreads_globals_object_connect(address, ce, object)) {
		zend_throw_exception_ex(pthreads_ce_ThreadedConnectionException, 0,
			"pthreads detected an attempt to connect to an object which has already been destroyed");
		return FAILURE;
	}

	return SUCCESS;
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
		//TODO: avoid these things being allocated to begin with...
		if (destination->worker_data) {
			pthreads_worker_data_free(destination->worker_data);
			destination->worker_data = NULL;
		}

		if (destination->ts_obj && --destination->ts_obj->refcount == 0) {
			pthreads_ts_object_free(destination);
		}

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
int pthreads_connect(pthreads_zend_object_t* source, pthreads_zend_object_t* destination) {
	int result = FAILURE;
	if(pthreads_globals_lock()){
		result = _pthreads_connect_nolock(source, destination);
		pthreads_globals_unlock();
	}
	return result;
} /* }}} */

/* {{{ */
//TODO: rename this
zend_bool pthreads_globals_object_connect(pthreads_zend_object_t* address, zend_class_entry *ce, zval *object) {
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
				/* no connection exists, create a new one */
				if (!ce) {
					/* we may not know the class, can't use ce directly
						from zend_object because it is from another context */
					PTHREADS_ZG(hard_copy_interned_strings) = 1;
					ce = pthreads_prepare_single_class(pthreads->ts_obj, pthreads->std.ce);
					PTHREADS_ZG(hard_copy_interned_strings) = 0;
				}
				object_init_ex(object, ce);
				connection = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
				_pthreads_connect_nolock(pthreads, connection);
				zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong)connection->ts_obj, connection);
			}
		}
	}

	pthreads_globals_unlock();
	return valid;
} /* }}} */

/* {{{ */
static inline void pthreads_base_init(pthreads_zend_object_t* base) {
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

			ZVAL_STR(&key, info->name);
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
						"Cannot use non-thread-safe default of type %s for Threaded class property %s::$%s",
						zend_get_type_by_const(Z_TYPE_P(value)),
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
	ts_obj->monitor = pthreads_monitor_alloc();
	ts_obj->creator.ls = TSRMLS_CACHE;
	ts_obj->creator.id = pthreads_self();
	ts_obj->props   = pthreads_store_alloc();
	return ts_obj;
} /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_zend_object_t* base, zend_class_entry *entry, unsigned int scope) {
	base->ts_obj = pthreads_ts_object_ctor(scope);
	base->owner.ls = TSRMLS_CACHE;
	base->owner.id = pthreads_self();
	base->original_zobj = NULL;
	base->worker_data = NULL;

	zend_object_std_init(&base->std, entry);
	object_properties_init(&base->std, entry);
	pthreads_base_init(base);
	base->local_props_modcount = base->ts_obj->props->modcount - 1;
} /* }}} */

/* {{{ */
void pthreads_base_dtor(zend_object *object) {
	//TODO: how does this play with __destruct() calls (e.g. adding a ref to self)?
	pthreads_zend_object_t* base = PTHREADS_FETCH_FROM(object);

	if (base->original_zobj == NULL && PTHREADS_IN_CREATOR(base) && (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) &&
		pthreads_monitor_check(base->ts_obj->monitor, PTHREADS_MONITOR_STARTED) &&
		!pthreads_monitor_check(base->ts_obj->monitor, PTHREADS_MONITOR_JOINED)) {
		pthreads_join(base);
	}

	zend_objects_destroy_object(object);
} /* }}} */

/* {{{ */
static void pthreads_ts_object_free(pthreads_zend_object_t* base) {
	pthreads_object_t *ts_obj = base->ts_obj;
	if (pthreads_monitor_lock(ts_obj->monitor)) {
		pthreads_store_free(ts_obj->props);
		pthreads_monitor_unlock(ts_obj->monitor);
	}

	pthreads_monitor_free(ts_obj->monitor);

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

/* {{{ */
zend_bool pthreads_start(pthreads_zend_object_t* thread, zend_ulong thread_options) {
	pthreads_routine_arg_t routine;
	pthreads_object_t *ts_obj = thread->ts_obj;

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may start it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_check(ts_obj->monitor, PTHREADS_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already started it", thread->std.ce->name->val);
		return 0;
	}

	pthreads_routine_init(&routine, thread, thread_options);

	switch (pthread_create(&ts_obj->thread, NULL, (void* (*) (void*)) pthreads_routine, (void*)&routine)) {
		case SUCCESS:
			pthreads_routine_wait(&routine);
			return 1;

		case EAGAIN:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, out of resources", thread->std.ce->name->val);
		break;

		default:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, unknown error", thread->std.ce->name->val);
	}

	pthreads_routine_free(&routine);

	return 0;
} /* }}} */

/* {{{ */
zend_bool pthreads_join(pthreads_zend_object_t* thread) {

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may join with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_check(thread->ts_obj->monitor, PTHREADS_MONITOR_JOINED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already joined with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (!pthreads_monitor_check(thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s has not been started",
			thread->std.ce->name->val);
		return 0;
	}

	pthreads_monitor_add(thread->ts_obj->monitor, PTHREADS_MONITOR_JOINED);

	//wait for the thread to signal that it's no longer running PHP code
	pthreads_monitor_wait_until(thread->ts_obj->monitor, PTHREADS_MONITOR_AWAIT_JOIN);

	//now, synchronize all object properties that may have been assigned by the thread
	if (pthreads_monitor_lock(thread->ts_obj->monitor)) {
		pthreads_store_full_sync_local_properties(thread);
		pthreads_monitor_unlock(thread->ts_obj->monitor);
	}
	if (thread->worker_data != NULL) {
		pthreads_worker_sync_collectable_tasks(thread->worker_data);
	}

	pthreads_monitor_add(thread->ts_obj->monitor, PTHREADS_MONITOR_EXIT);

	return (pthread_join(thread->ts_obj->thread, NULL) == SUCCESS);
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_routine_run_function(pthreads_zend_object_t* object, pthreads_zend_object_t* connection, zval *work) {
	zend_function *run;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	zval zresult;
	zend_execute_data execute_data;
	memset(&execute_data, 0, sizeof(execute_data));

	if (pthreads_connect(object, connection) != SUCCESS) {
		return 0;
	}

	if (pthreads_monitor_check(object->ts_obj->monitor, PTHREADS_MONITOR_ERROR)) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_monitor_add(object->ts_obj->monitor, PTHREADS_MONITOR_RUNNING);

	if (work)
		pthreads_store_write(Z_OBJ_P(work), &PTHREADS_G(strings).worker, &PTHREADS_ZG(this), PTHREADS_STORE_NO_COERCE_ARRAY);

	zend_try {
		if ((run = zend_hash_find_ptr(&connection->std.ce->function_table, PTHREADS_G(strings).run))) {
			if (run->type == ZEND_USER_FUNCTION) {
				EG(current_execute_data) = &execute_data;

				call.fci.size = sizeof(zend_fcall_info);
				call.fci.retval = &zresult;
				call.fci.object = &connection->std;
				call.fcc.object = &connection->std;
				call.fcc.calling_scope = connection->std.ce;
				call.fcc.called_scope = connection->std.ce;
				call.fcc.function_handler = run;

				zend_call_function(&call.fci, &call.fcc);

				EG(current_execute_data) = NULL;

				if (EG(exception)) {
					zend_try_exception_handler();
					if (EG(exception)) {
						zend_exception_error(EG(exception), E_ERROR);
						pthreads_monitor_add(object->ts_obj->monitor, PTHREADS_MONITOR_ERROR);
					}
				}
			}
		}
	} zend_catch {
		pthreads_monitor_add(object->ts_obj->monitor, PTHREADS_MONITOR_ERROR);
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pthreads_monitor_remove(object->ts_obj->monitor, PTHREADS_MONITOR_RUNNING);

	return 1;
} /* }}} */

/* {{{ */
static void * pthreads_routine(pthreads_routine_arg_t *routine) {
	pthreads_zend_object_t* thread = routine->thread;
	zend_ulong thread_options = routine->options;
	pthreads_object_t *ts_obj = thread->ts_obj;
	pthreads_monitor_t* ready = routine->ready;

	if (pthreads_prepared_startup(ts_obj, ready, thread->std.ce, thread_options) == SUCCESS) {
		pthreads_queue done_tasks_cache;

		zend_first_try {
			ZVAL_UNDEF(&PTHREADS_ZG(this));
			object_init_ex(&PTHREADS_ZG(this), pthreads_prepare_single_class(ts_obj, thread->std.ce));
			pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))), NULL);

			if (PTHREADS_IS_WORKER(thread)) {
				zval task;

				memset(&done_tasks_cache, 0, sizeof(pthreads_queue));

				while (pthreads_worker_next_task(thread->worker_data, &done_tasks_cache, &task) != PTHREADS_MONITOR_JOINED) {
					zval that;
					pthreads_zend_object_t* work = PTHREADS_FETCH_FROM(Z_OBJ(task));
					object_init_ex(&that, pthreads_prepare_single_class(ts_obj, work->std.ce));
					pthreads_routine_run_function(work, PTHREADS_FETCH_FROM(Z_OBJ(that)), &that);
					pthreads_worker_add_garbage(thread->worker_data, &done_tasks_cache, &that);
					zval_ptr_dtor(&that);
				}
			}


		} zend_end_try();

		pthreads_monitor_add(ts_obj->monitor, PTHREADS_MONITOR_AWAIT_JOIN);
		//wait for the parent to tell us it is done
		pthreads_monitor_wait_until(ts_obj->monitor, PTHREADS_MONITOR_EXIT);

		zend_first_try{
			//now we can safely get rid of our local objects
			zval_ptr_dtor(&PTHREADS_ZG(this));
			ZVAL_UNDEF(&PTHREADS_ZG(this));

			if (PTHREADS_IS_WORKER(thread)) {
				pthreads_queue_clean(&done_tasks_cache);
			}
		} zend_end_try();
	}

	pthreads_prepared_shutdown();

	pthread_exit(NULL);

#ifdef _WIN32
	return NULL;
#endif
} /* }}} */
