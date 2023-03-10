#include <src/globals.h>
#include <src/monitor.h>
#include <src/prepare.h>
#include <src/object.h>
#include <src/pthreads.h>
#include <src/store.h>
#include <src/thread.h>
#include <src/worker.h>

/* {{{ */
static void pthreads_routine_init(pthreads_routine_arg_t* r, pthreads_zend_object_t* thread, zend_ulong thread_options) {
	r->thread = thread;
	r->options = thread_options;
	pthreads_monitor_init(&r->ready);
	pthreads_monitor_add(
		&r->thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED);
}

static void pthreads_routine_wait(pthreads_routine_arg_t* r) {
	pthreads_monitor_wait_until(
		&r->ready, PTHREADS_MONITOR_READY);
	pthreads_monitor_destroy(&r->ready);
}

static void pthreads_routine_free(pthreads_routine_arg_t* r) {
	pthreads_monitor_remove(
		&r->thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED);
	pthreads_monitor_destroy(&r->ready);
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_routine_run_function(pthreads_zend_object_t* object, pthreads_zend_object_t* connection, zval* work) {
	zend_function* run;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	zval zresult;
	zend_execute_data execute_data;
	memset(&execute_data, 0, sizeof(execute_data));

	if (pthreads_connect(object, connection) != SUCCESS) {
		return 0;
	}

	if (pthreads_monitor_check(&object->ts_obj->monitor, PTHREADS_MONITOR_ERROR)) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_monitor_add(&object->ts_obj->monitor, PTHREADS_MONITOR_RUNNING);

	if (work)
		pthreads_store_write(Z_OBJ_P(work), &PTHREADS_G(strings).worker, &PTHREADS_ZG(this), PTHREADS_STORE_NO_COERCE_ARRAY);

	zend_try{
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
						pthreads_monitor_add(&object->ts_obj->monitor, PTHREADS_MONITOR_ERROR);
					}
				}
			}
		}
	} zend_catch{
		pthreads_monitor_add(&object->ts_obj->monitor, PTHREADS_MONITOR_ERROR);
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pthreads_monitor_remove(&object->ts_obj->monitor, PTHREADS_MONITOR_RUNNING);

	return 1;
} /* }}} */

/* {{{ */
static void* pthreads_routine(pthreads_routine_arg_t* routine) {
	pthreads_zend_object_t* thread = routine->thread;
	zend_ulong thread_options = routine->options;
	pthreads_object_t* ts_obj = thread->ts_obj;
	pthreads_monitor_t* ready = &routine->ready;

	if (pthreads_prepared_startup(ts_obj, ready, thread->std.ce, thread_options) == SUCCESS) {
		pthreads_queue done_tasks_cache;

		zend_first_try{
			ZVAL_UNDEF(&PTHREADS_ZG(this));
			object_init_ex(&PTHREADS_ZG(this), pthreads_prepare_single_class(&thread->owner, thread->std.ce));
			pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))), NULL);

			if (PTHREADS_IS_WORKER(thread)) {
				zval task;

				memset(&done_tasks_cache, 0, sizeof(pthreads_queue));

				while (pthreads_worker_next_task(thread->worker_data, &done_tasks_cache, &task) != PTHREADS_MONITOR_JOINED) {
					zval that;
					pthreads_zend_object_t* work = PTHREADS_FETCH_FROM(Z_OBJ(task));
					object_init_ex(&that, pthreads_prepare_single_class(&work->owner, work->std.ce));
					pthreads_routine_run_function(work, PTHREADS_FETCH_FROM(Z_OBJ(that)), &that);
					pthreads_worker_add_garbage(thread->worker_data, &done_tasks_cache, &that);
					zval_ptr_dtor(&that);
				}
			}


		} zend_end_try();

		pthreads_monitor_add(&ts_obj->monitor, PTHREADS_MONITOR_AWAIT_JOIN);
		//wait for the parent to tell us it is done
		pthreads_monitor_wait_until(&ts_obj->monitor, PTHREADS_MONITOR_EXIT);

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

/* {{{ */
zend_bool pthreads_start(pthreads_zend_object_t* thread, zend_ulong thread_options) {
	pthreads_routine_arg_t routine;
	pthreads_object_t* ts_obj = thread->ts_obj;

	if (!PTHREADS_IN_CREATOR(thread) || thread->original_zobj != NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may start it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_check(&ts_obj->monitor, PTHREADS_MONITOR_STARTED)) {
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

	if (pthreads_monitor_check(&thread->ts_obj->monitor, PTHREADS_MONITOR_JOINED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already joined with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (!pthreads_monitor_check(&thread->ts_obj->monitor, PTHREADS_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s has not been started",
			thread->std.ce->name->val);
		return 0;
	}

	pthreads_monitor_add(&thread->ts_obj->monitor, PTHREADS_MONITOR_JOINED);

	//wait for the thread to signal that it's no longer running PHP code
	pthreads_monitor_wait_until(&thread->ts_obj->monitor, PTHREADS_MONITOR_AWAIT_JOIN);

	//now, synchronize all object properties that may have been assigned by the thread
	if (pthreads_monitor_lock(&thread->ts_obj->monitor)) {
		pthreads_store_full_sync_local_properties(&thread->std);
		pthreads_monitor_unlock(&thread->ts_obj->monitor);
	}
	if (thread->worker_data != NULL) {
		pthreads_worker_sync_collectable_tasks(thread->worker_data);
	}

	pthreads_monitor_add(&thread->ts_obj->monitor, PTHREADS_MONITOR_EXIT);

	return (pthread_join(thread->ts_obj->thread, NULL) == SUCCESS);
} /* }}} */
