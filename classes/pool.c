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

#define Pool_method(name) PHP_METHOD(pmmp_thread_Pool, name)

/* {{{ proto Pool Pool::__construct(integer size, [class worker, [array $ctor]])
	Construct a pool ready to create a maximum of $size workers of class $worker
	$ctor will be used as arguments to constructor when spawning workers */
Pool_method(__construct)
{
	zend_long size = 0;
	zend_class_entry *clazz = NULL;
	zval *ctor = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 3)
		Z_PARAM_LONG(size)
		Z_PARAM_OPTIONAL
		Z_PARAM_CLASS(clazz)
		Z_PARAM_ARRAY(ctor)
	ZEND_PARSE_PARAMETERS_END();

	if (clazz == NULL) clazz = pmmpthread_ce_worker;

	if (!instanceof_function(clazz, pmmpthread_ce_worker)) {
		zend_throw_exception_ex(NULL, 0,
			"The class provided (%s) does not extend Worker", clazz->name->val);
	}

	zend_update_property_long(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("size"), size);
	zend_update_property_stringl(
		Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("class"), clazz->name->val, clazz->name->len);
	if (ctor)
		zend_update_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("ctor"), ctor);
} /* }}} */

/* {{{ proto void Pool::resize(integer size)
	Resize the pool to the given number of workers, if the pool size is being reduced
	then the last workers started will be shutdown until the pool is the requested size */
Pool_method(resize) {
	zval tmp[2];
	zend_long newsize = 0;
	zval *workers = NULL;
	zval *size = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_LONG(newsize)
	ZEND_PARSE_PARAMETERS_END();

	workers = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("workers"), 1, &tmp[0]);
	size = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("size"), 1, &tmp[1]);

	if (Z_TYPE_P(workers) == IS_ARRAY &&
		newsize < zend_hash_num_elements(Z_ARRVAL_P(workers))) {
		do {
			zval *worker = NULL;
			zend_long top = zend_hash_num_elements(Z_ARRVAL_P(workers));

			if ((worker = zend_hash_index_find(
				Z_ARRVAL_P(workers), top-1))) {
				zend_call_method(
					Z_OBJ_P(worker), Z_OBJCE_P(worker), NULL, ZEND_STRL("shutdown"), NULL, 0, NULL, NULL);

			}

			zend_hash_index_del(Z_ARRVAL_P(workers), top-1);
		} while (zend_hash_num_elements(Z_ARRVAL_P(workers)) != newsize);
	}

	ZVAL_LONG(size, newsize);
} /* }}} */

/* {{{ proto integer Pool::submit(Runnable task)
	Will submit the given task to the next worker in the pool, by default workers are selected round robin */
Pool_method(submit) {
	zval tmp[5];
	zval *task = NULL;
	zval *last = NULL;
	zval *size = NULL;
	zval *workers = NULL;
	zval worker;
	zval *clazz = NULL;
	zval *ctor = NULL;
	zval *selected = NULL;

	zend_class_entry *ce = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 1, 1)
		Z_PARAM_OBJECT_OF_CLASS(task, pmmpthread_ce_runnable)
	ZEND_PARSE_PARAMETERS_END();

	last = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("last"), 1, &tmp[0]);
	size = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("size"), 1, &tmp[1]);
	workers = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("workers"), 1, &tmp[2]);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		array_init(workers);

	if (Z_LVAL_P(last) >= Z_LVAL_P(size))
		ZVAL_LONG(last, 0);

	if (!(selected = zend_hash_index_find(Z_ARRVAL_P(workers), Z_LVAL_P(last)))) {
		clazz = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("class"), 1, &tmp[3]);

		if (Z_TYPE_P(clazz) != IS_STRING) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"this Pool has not been initialized properly, Worker class not valid");
			return;
		}

		if (!(ce = zend_lookup_class(
			Z_STR_P(clazz)))) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"this Pool has not been initialized properly, the Worker class %s could not be found",
				Z_STRVAL_P(clazz));
			return;
		}

		ctor  = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("ctor"), 1, &tmp[4]);

		object_init_ex(&worker, ce);

		{
			zend_class_entry *scope = EG(fake_scope);
			zend_function *constructor = NULL;
			zval retval;

			ZVAL_UNDEF(&retval);

			EG(fake_scope) = ce;

			constructor = Z_OBJ_HT(worker)->get_constructor(Z_OBJ(worker));

			EG(fake_scope) = scope;

			if (constructor) {
				zend_fcall_info fci = empty_fcall_info;
				zend_fcall_info_cache fcc = empty_fcall_info_cache;

				fci.size = sizeof(zend_fcall_info);
				fci.object = Z_OBJ(worker);
				fci.retval = &retval;

				fcc.function_handler = constructor;
				fcc.calling_scope = zend_get_executed_scope();
				fcc.called_scope = Z_OBJCE(worker);
				fcc.object = Z_OBJ(worker);

				if (ctor)
					zend_fcall_info_args(&fci, ctor);

				zend_call_function(&fci, &fcc);

				if (ctor)
					zend_fcall_info_args_clear(&fci, 1);

				if (Z_TYPE(retval) != IS_UNDEF)
					zval_dtor(&retval);
			}

			zend_call_method(Z_OBJ_P(&worker), Z_OBJCE(worker), NULL, ZEND_STRL("start"), NULL, 0, NULL, NULL);
		}

		selected = zend_hash_index_update(
			Z_ARRVAL_P(workers), Z_LVAL_P(last), &worker);

	}

	zend_call_method(Z_OBJ_P(selected), Z_OBJCE_P(selected), NULL, ZEND_STRL("stack"), NULL, 1, task, NULL);
	ZVAL_LONG(return_value, Z_LVAL_P(last));
	Z_LVAL_P(last)++;
} /* }}} */

/* {{{ proto integer Pool::submitTo(integer $worker, Runnable task)
	Will submit the given task to the specified worker */
Pool_method(submitTo) {
	zval tmp;
	zval *task = NULL;
	zval *workers = NULL;
	zend_long worker = 0;
	zval *selected = NULL;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 2, 2)
		Z_PARAM_LONG(worker)
		Z_PARAM_OBJECT_OF_CLASS(task, pmmpthread_ce_runnable)
	ZEND_PARSE_PARAMETERS_END();

	workers = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("workers"), 1, &tmp);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		array_init(workers);

	if ((selected = zend_hash_index_find(Z_ARRVAL_P(workers), worker))) {
		zend_call_method(Z_OBJ_P(selected),
			Z_OBJCE_P(selected), NULL,
			ZEND_STRL("stack"), NULL, 1, task, NULL);
		ZVAL_LONG(return_value, worker);
	} else {
		zend_throw_exception_ex(NULL, 0,
			"The selected worker (%ld) does not exist", worker);
	}
} /* }}} */

/* {{{ proto void Pool::collect([callable collector])
	Shall execute the collector on each of the tasks in the working set
		removing the task if the collector returns positively
		the collector should be a function accepting a single task */
Pool_method(collect) {
	zval tmp;
	pmmpthread_call_t call;
	zval *workers = NULL,
	     *worker = NULL;
	zend_long collectable = 0;

	ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW, 0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_FUNC(call.fci, call.fcc)
	ZEND_PARSE_PARAMETERS_END();

	workers = zend_read_property(Z_OBJCE_P(getThis()), Z_OBJ_P(getThis()), ZEND_STRL("workers"), 1, &tmp);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		RETURN_LONG(0);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(workers), worker) {
		pmmpthread_zend_object_t *thread =
			PMMPTHREAD_FETCH_FROM(Z_OBJ_P(worker));
		if (!ZEND_NUM_ARGS())
			PMMPTHREAD_WORKER_COLLECTOR_INIT(call, Z_OBJ_P(worker));
		collectable += pmmpthread_worker_collect_tasks(
			thread->worker_data,
			&call,
			pmmpthread_worker_collect_function);
		if (!ZEND_NUM_ARGS())
			PMMPTHREAD_WORKER_COLLECTOR_DTOR(call);
	} ZEND_HASH_FOREACH_END();

	RETURN_LONG(collectable);
} /* }}} */

/* {{{ */
static inline int pmmpthread_pool_shutdown_worker(zval *worker) {
	zval retval;
	zend_execute_data *ex = EG(current_execute_data);
	ZVAL_UNDEF(&retval);
	EG(current_execute_data) = NULL;
	zend_call_method_with_0_params(
		Z_OBJ_P(worker), Z_OBJCE_P(worker), NULL, "shutdown", &retval);
	if (Z_TYPE(retval) != IS_UNDEF)
		zval_ptr_dtor(&retval);
	EG(current_execute_data) = ex;

	return ZEND_HASH_APPLY_REMOVE;
} /* }}} */

/* {{{ */
static inline void pmmpthread_pool_shutdown(zval *pool) {
	zval tmp;
	zval *workers = zend_read_property(
		Z_OBJCE_P(pool), Z_OBJ_P(pool), ZEND_STRL("workers"), 1, &tmp);

	if (Z_TYPE_P(workers) == IS_ARRAY) {
		if (zend_hash_num_elements(Z_ARRVAL_P(workers))) {
			zend_hash_apply(Z_ARRVAL_P(workers), pmmpthread_pool_shutdown_worker);
		}

		zend_hash_clean(Z_ARRVAL_P(workers));
	}
} /* }}} */

/* {{{ proto void Pool::shutdown(void)
	Will cause all the workers to finish executing their stacks and shutdown */
Pool_method(shutdown) {
	zend_parse_parameters_none_throw();

	pmmpthread_pool_shutdown(getThis());
} /* }}} */
