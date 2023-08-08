#include <src/globals.h>
#include <src/monitor.h>
#include <src/prepare.h>
#include <src/object.h>
#include <src/pmmpthread.h>
#include <src/store.h>
#include <src/thread.h>
#include <src/worker.h>

/* {{{ */
typedef struct _pmmpthread_routine_arg_t {
	pmmpthread_zend_object_t* thread;
	pmmpthread_monitor_t ready;
	zend_ulong options;
} pmmpthread_routine_arg_t; /* }}} */

/* {{{ */
static void pmmpthread_routine_init(pmmpthread_routine_arg_t* r, pmmpthread_zend_object_t* thread, zend_ulong thread_options) {
	r->thread = thread;
	r->options = thread_options;
	pmmpthread_monitor_init(&r->ready);
	pmmpthread_monitor_add(
		&r->thread->ts_obj->monitor, PMMPTHREAD_MONITOR_STARTED);
}

static void pmmpthread_routine_wait(pmmpthread_routine_arg_t* r) {
	pmmpthread_monitor_wait_until(
		&r->ready, PMMPTHREAD_MONITOR_READY);
	pmmpthread_monitor_destroy(&r->ready);
}

static void pmmpthread_routine_free(pmmpthread_routine_arg_t* r) {
	pmmpthread_monitor_remove(
		&r->thread->ts_obj->monitor, PMMPTHREAD_MONITOR_STARTED);
	pmmpthread_monitor_destroy(&r->ready);
} /* }}} */

/* {{{ */
static inline zend_result pmmpthread_routine_run_function(pmmpthread_zend_object_t* connection) {
	zend_function* run;
	pmmpthread_call_t call = PMMPTHREAD_CALL_EMPTY;
	zval zresult;
	zend_execute_data execute_data;
	memset(&execute_data, 0, sizeof(execute_data));
	zend_result result = FAILURE;

	if (pmmpthread_monitor_check(&connection->ts_obj->monitor, PMMPTHREAD_MONITOR_ERROR)) {
		return result;
	}

	ZVAL_UNDEF(&zresult);

	pmmpthread_monitor_add(&connection->ts_obj->monitor, PMMPTHREAD_MONITOR_RUNNING);

	zend_try{
		if ((run = zend_hash_find_ptr(&connection->std.ce->function_table, PMMPTHREAD_G(strings).run))) {
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
					}
					pmmpthread_monitor_add(&connection->ts_obj->monitor, PMMPTHREAD_MONITOR_ERROR);
				} else {
					result = SUCCESS;
				}
			} else {
				//pmmpthread internal run() stub for worker?
				result = SUCCESS;
			}
		}
	} zend_catch{
		pmmpthread_monitor_add(&connection->ts_obj->monitor, PMMPTHREAD_MONITOR_ERROR);
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pmmpthread_monitor_remove(&connection->ts_obj->monitor, PMMPTHREAD_MONITOR_RUNNING);

	return result;
} /* }}} */

/* {{{ */
static void* pmmpthread_routine(pmmpthread_routine_arg_t* routine) {
	pmmpthread_zend_object_t* thread = routine->thread;
	zend_ulong thread_options = routine->options;
	pmmpthread_object_t* ts_obj = thread->ts_obj;
	pmmpthread_monitor_t* ready = &routine->ready;

	if (pmmpthread_prepared_startup(ts_obj, ready, thread->std.ce, thread_options) == SUCCESS) {
		pmmpthread_queue done_tasks_cache;
		memset(&done_tasks_cache, 0, sizeof(pmmpthread_queue));

		zend_first_try{
			ZVAL_UNDEF(&PMMPTHREAD_ZG(this));
			pmmpthread_object_connect(thread, &PMMPTHREAD_ZG(this));
			if (pmmpthread_routine_run_function(PMMPTHREAD_FETCH_FROM(Z_OBJ_P(&PMMPTHREAD_ZG(this)))) == FAILURE) {
				zend_bailout();
			}

			if (PMMPTHREAD_IS_WORKER(thread)) {
				zval original;

				while (pmmpthread_worker_next_task(thread->worker_data, &done_tasks_cache, &original) != PMMPTHREAD_MONITOR_JOINED) {
					zval connection;
					pmmpthread_object_connect(PMMPTHREAD_FETCH_FROM(Z_OBJ(original)), &connection);
					zend_result task_result = pmmpthread_routine_run_function(PMMPTHREAD_FETCH_FROM(Z_OBJ(connection)));
					pmmpthread_worker_add_garbage(thread->worker_data, &done_tasks_cache, &connection);
					zval_ptr_dtor(&connection);

					if (task_result == FAILURE) {
						//we may have run out of memory or some error that left the interpreter in an unusable state
						pmmpthread_monitor_add(&ts_obj->monitor, PMMPTHREAD_MONITOR_ERROR);
						break;
					}
				}
			}
		} zend_end_try();

		pmmpthread_call_shutdown_functions();

		pmmpthread_monitor_add(&ts_obj->monitor, PMMPTHREAD_MONITOR_AWAIT_JOIN);
		//wait for the parent to tell us it is done
		pmmpthread_monitor_wait_until(&ts_obj->monitor, PMMPTHREAD_MONITOR_EXIT);

		zend_first_try{
			//now we can safely get rid of our local objects
			zval_ptr_dtor(&PMMPTHREAD_ZG(this));
			ZVAL_UNDEF(&PMMPTHREAD_ZG(this));

			if (PMMPTHREAD_IS_WORKER(thread)) {
				pmmpthread_queue_clean(&done_tasks_cache);
			}
		} zend_end_try();
	}

	pmmpthread_prepared_shutdown();

	pthread_exit(NULL);

#ifdef _WIN32
	return NULL;
#endif
} /* }}} */

/* {{{ */
zend_bool pmmpthread_start(pmmpthread_zend_object_t* thread, zend_ulong thread_options) {
	pmmpthread_routine_arg_t routine;
	pmmpthread_object_t* ts_obj = thread->ts_obj;

	if (!PMMPTHREAD_IN_CREATOR(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may start it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pmmpthread_monitor_check(&ts_obj->monitor, PMMPTHREAD_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already started it", thread->std.ce->name->val);
		return 0;
	}

	pmmpthread_routine_init(&routine, thread, thread_options);

	int create_result = pthread_create(&ts_obj->thread, NULL, (void* (*) (void*)) pmmpthread_routine, (void*)&routine);
	switch (create_result) {
	case SUCCESS:
		pmmpthread_routine_wait(&routine);
		return 1;

	case EAGAIN:
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "Cannot start thread: Out of resources or system thread limit reached");
		break;

	default:
		//this should never normally happen
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "Cannot start thread: %s", strerror(create_result));
		break;
	}

	pmmpthread_routine_free(&routine);

	return 0;
} /* }}} */

/* {{{ */
zend_bool pmmpthread_join(pmmpthread_zend_object_t* thread) {

	if (!PMMPTHREAD_IN_CREATOR(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may join with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pmmpthread_monitor_check(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_JOINED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already joined with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (!pmmpthread_monitor_check(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s has not been started",
			thread->std.ce->name->val);
		return 0;
	}

	pmmpthread_monitor_add(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_JOINED);

	//wait for the thread to signal that it's no longer running PHP code
	pmmpthread_monitor_wait_until(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_AWAIT_JOIN);

	//now, synchronize all object properties that may have been assigned by the thread
	if (pmmpthread_monitor_lock(&thread->ts_obj->monitor)) {
		pmmpthread_store_full_sync_local_properties(&thread->std);
		pmmpthread_monitor_unlock(&thread->ts_obj->monitor);
	}
	if (thread->worker_data != NULL) {
		pmmpthread_worker_sync_collectable_tasks(thread->worker_data);
	}
	pmmpthread_zend_object_t* user_globals = PMMPTHREAD_ZG(thread_shared_globals);
	if (pmmpthread_monitor_lock(&user_globals->ts_obj->monitor)) {
		pmmpthread_store_full_sync_local_properties(&user_globals->std);
		pmmpthread_monitor_unlock(&user_globals->ts_obj->monitor);
	}

	pmmpthread_monitor_add(&thread->ts_obj->monitor, PMMPTHREAD_MONITOR_EXIT);

	return (pthread_join(thread->ts_obj->thread, NULL) == SUCCESS);
} /* }}} */
