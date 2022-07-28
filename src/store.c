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
#ifndef HAVE_PTHREADS_STORE
#define HAVE_PTHREADS_STORE

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

#ifndef HAVE_PTHREADS_COPY_H
#	include <src/copy.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#include <src/globals.h>
#include <Zend/zend_ast.h>

#define PTHREADS_STORAGE_EMPTY {0, 0, 0, 0, NULL}

/* {{{ */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength);
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength);
/* }}} */

/* {{{ */
static inline void pthreads_store_storage_table_dtor (zval *element) {
	pthreads_store_storage_dtor(Z_PTR_P(element));
} /* }}} */

/* {{{ */
pthreads_store_t* pthreads_store_alloc() {
	pthreads_store_t *store = (pthreads_store_t*) calloc(1, sizeof(pthreads_store_t));

	if (store) {
		zend_hash_init(
			store, 8, NULL,
			(dtor_func_t) pthreads_store_storage_table_dtor, 1);
	}

	return store;
} /* }}} */

static inline zend_bool pthreads_store_retain_in_local_cache(zval *val) {
	return IS_PTHREADS_OBJECT(val) || IS_PTHREADS_CLOSURE_OBJECT(val) || IS_EXT_SOCKETS_OBJECT(val);
}

void pthreads_store_sync(zend_object *object) { /* {{{ */
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_ulong idx;
	zend_string *name;
	zval *val;
	pthreads_storage *ts_val;
	zend_bool remove;

	rebuild_object_properties(&threaded->std);

	ZEND_HASH_FOREACH_KEY_VAL(threaded->std.properties, idx, name, val) {
		if (!name) {
			ts_val = zend_hash_index_find_ptr(ts_obj->store.props, idx);
		} else {
			ts_val = zend_hash_find_ptr(ts_obj->store.props, name);
		}

		remove = 1;
		if (ts_val) {
			ZVAL_DEINDIRECT(val);
			if (ts_val->type == IS_PTHREADS && IS_PTHREADS_OBJECT(val)) {
				pthreads_object_t* threadedStorage = ((pthreads_zend_object_t *) ts_val->data)->ts_obj;
				pthreads_object_t *threadedProperty = PTHREADS_FETCH_TS_FROM(Z_OBJ_P(val));

				if (threadedStorage->monitor == threadedProperty->monitor) {
					remove = 0;
				}
			} else if (ts_val->type == IS_CLOSURE && IS_PTHREADS_CLOSURE_OBJECT(val)) {
				zend_closure *shared = (zend_closure *) ts_val->data;
				zend_closure *local = (zend_closure *) Z_OBJ_P(val);
				if (shared == local) {
					remove = 0;
				}
#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
			} else if (ts_val->type == IS_SOCKET && IS_EXT_SOCKETS_OBJECT(val)) {
				pthreads_storage_socket *shared = (pthreads_storage_socket *) ts_val->data;
				php_socket *local = Z_SOCKET_P(val);
				if (shared->bsd_socket == local->bsd_socket) {
					remove = 0;
				}
#endif
			}
		}

		if (remove) {
			if (!name) {
				zend_hash_index_del(threaded->std.properties, idx);
			} else {
				zend_hash_del(threaded->std.properties, name);
			}
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

static inline zend_bool pthreads_store_coerce(zval *key, zval *member) {
	zend_ulong hval;

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
			if (ZEND_HANDLE_NUMERIC(Z_STR_P(key), hval)) {
				ZVAL_LONG(member, hval);
				return 0;
			}
		case IS_LONG:
			ZVAL_ZVAL(member, key, 0, 0);
			return 0;

		default:
			ZVAL_STR(member, zval_get_string(key));
			return 1;
	}
}

/* {{{ */
static inline zend_bool pthreads_store_is_immutable(zend_object *object, zval *key) {
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_storage *storage;

	if (IS_PTHREADS_VOLATILE_CLASS(object->ce)) {
		return 0;
	}

	if (Z_TYPE_P(key) == IS_LONG) {
		storage = zend_hash_index_find_ptr(threaded->ts_obj->store.props, Z_LVAL_P(key));
	} else storage = zend_hash_find_ptr(threaded->ts_obj->store.props, Z_STR_P(key));

	if ((storage) && (storage->type == IS_PTHREADS)) {
		if (Z_TYPE_P(key) == IS_LONG) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Threaded members previously set to Threaded objects are immutable, cannot overwrite %ld",
			Z_LVAL_P(key));
		} else zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Threaded members previously set to Threaded objects are immutable, cannot overwrite %s",
			Z_STRVAL_P(key));
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ */
int pthreads_store_delete(zend_object *object, zval *key) {
	int result = FAILURE;
	zval member;
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pthreads_store_coerce(key, &member);

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		if (!pthreads_store_is_immutable(object, &member)) {
			if (Z_TYPE(member) == IS_LONG) {
				result = zend_hash_index_del(ts_obj->store.props, Z_LVAL(member));
			} else result = zend_hash_del(ts_obj->store.props, Z_STR(member));
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	} else result = FAILURE;

	if (result == SUCCESS) {
		if (Z_TYPE(member) == IS_LONG) {
			zend_hash_index_del(threaded->std.properties, Z_LVAL(member));
		} else zend_hash_del(threaded->std.properties, Z_STR(member));
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
}
/* }}} */

/* {{{ */
zend_bool pthreads_store_isset(zend_object *object, zval *key, int has_set_exists) {
	zend_bool isset = 0;
	zval member;
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pthreads_store_coerce(key, &member);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		pthreads_storage *storage;

		if (Z_TYPE(member) == IS_LONG) {
			storage = zend_hash_index_find_ptr(ts_obj->store.props, Z_LVAL(member));
		} else storage = zend_hash_find_ptr(ts_obj->store.props, Z_STR(member));

		if (storage) {
			isset = 1;
			if (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) {
				switch (storage->type) {
					case IS_LONG:
					case IS_TRUE:
					case IS_FALSE:
						if (storage->simple.lval == 0)
							isset = 0;
						break;

					case IS_ARRAY:
						if (storage->exists == 0)
							isset = 0;
						break;

					case IS_STRING:
						switch (storage->length) {
							case 0:
								isset = 0;
								break;

							case 1:
								if (memcmp(storage->data, "0", 1) == SUCCESS)
									isset = 0;
								break;
						} break;

					case IS_DOUBLE:
						if (storage->simple.dval == 0.0)
							isset = 0;
						break;

					case IS_NULL:
						isset = 0;
						break;
				}
			} else if (has_set_exists == ZEND_PROPERTY_ISSET) {
				switch (storage->type) {
					case IS_NULL:
						isset = 0;
						break;
				}
			} else if (has_set_exists != ZEND_PROPERTY_EXISTS) {
				ZEND_ASSERT(0);
			}
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return isset;
} /* }}} */

/* {{{ */
int pthreads_store_read(zend_object *object, zval *key, int type, zval *read) {
	int result = FAILURE;
	zval member, *property = NULL;
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pthreads_store_coerce(key, &member);

	rebuild_object_properties(&threaded->std);

	if (!IS_PTHREADS_VOLATILE_CLASS(object->ce)) {
		//fast path for non-Volatile object - their Threaded members are immutable
		if (Z_TYPE(member) == IS_LONG) {
			property = zend_hash_index_find(threaded->std.properties, Z_LVAL(member));
		} else property = zend_hash_find(threaded->std.properties, Z_STR(member));

		if (property && IS_PTHREADS_OBJECT(property)) {
			ZVAL_COPY(read, property);
			if (coerced) {
				zval_ptr_dtor(&member);
			}
			return SUCCESS;
		}
	}

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		pthreads_store_sync(object);

		/* check if there's still a ref in local cache after sync - this ensures ref reuse for Threaded and Closure objects */
		if (Z_TYPE(member) == IS_LONG) {
			property = zend_hash_index_find(threaded->std.properties, Z_LVAL(member));
		} else property = zend_hash_find(threaded->std.properties, Z_STR(member));

		if (property) {
			pthreads_monitor_unlock(ts_obj->monitor);
			ZVAL_DEINDIRECT(property);
			ZVAL_COPY(read, property);
			if (coerced) {
				zval_ptr_dtor(&member);
			}
			return SUCCESS;
		}

		pthreads_storage *storage;

		if (Z_TYPE(member) == IS_LONG) {
			storage = zend_hash_index_find_ptr(ts_obj->store.props, Z_LVAL(member));
		} else storage = zend_hash_find_ptr(ts_obj->store.props, Z_STR(member));

		if (storage) {
			/* strictly only reads are supported */
			if (storage->type != IS_PTHREADS && type != BP_VAR_R && type != BP_VAR_IS){
				zend_throw_error(zend_ce_error, "Indirect modification of non-Threaded members of %s is not supported", ZSTR_VAL(object->ce->name));
				result = FAILURE;
			} else result = pthreads_store_convert(storage, read);
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}

	if (result != SUCCESS) {
		ZVAL_NULL(read);
	} else {
		if (pthreads_store_retain_in_local_cache(read)) {
			rebuild_object_properties(&threaded->std);
			if (Z_TYPE(member) == IS_LONG) {
				zend_hash_index_update(threaded->std.properties, Z_LVAL(member), read);
			} else zend_hash_update(threaded->std.properties, Z_STR(member), read);
			Z_ADDREF_P(read);
		}
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ */
int pthreads_store_write(zend_object *object, zval *key, zval *write) {
	int result = FAILURE;
	pthreads_storage *storage;
	zval vol, member;
	pthreads_zend_object_t *threaded =
		PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = 0;

	if (Z_TYPE_P(write) == IS_ARRAY) {
		if (!pthreads_check_opline_ex(EG(current_execute_data), -1, ZEND_CAST, IS_ARRAY) &&
			!pthreads_check_opline_ex(EG(current_execute_data), -2, ZEND_CAST, IS_ARRAY)) {
			/* coerce arrays into volatile objects unless explicitly cast as array */
			object_init_ex(
				&vol, pthreads_volatile_entry);
			pthreads_store_merge(Z_OBJ(vol), write, 1);
			/* this will be addref'd when caching the object */
			Z_SET_REFCOUNT(vol, 0);
			write = &vol;
		}
	}

	if (Z_TYPE_P(write) == IS_OBJECT) {
		/* when we copy out in another context, we want properties table
			to be initialized */
		rebuild_object_properties(Z_OBJ_P(write));
	}

	storage = pthreads_store_create(write);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		if (!key) {
			zend_ulong next = zend_hash_next_free_element(ts_obj->store.props);
			if (next == ZEND_LONG_MIN) next = 0;
			ZVAL_LONG(&member, next);
		} else {
			coerced = pthreads_store_coerce(key, &member);
		}
		if (!pthreads_store_is_immutable(object, &member)) {
			if (Z_TYPE(member) == IS_LONG) {
				if (zend_hash_index_update_ptr(ts_obj->store.props, Z_LVAL(member), storage))
					result = SUCCESS;
			} else {
				/* anything provided by this context might not live long enough to be used by another context,
				 * so we have to hard copy, even if the string is interned. */
				zend_string *orig_key = Z_STR(member);
				zend_string *keyed = zend_string_init(ZSTR_VAL(orig_key), ZSTR_LEN(orig_key), 1);

				if (zend_hash_update_ptr(ts_obj->store.props, keyed, storage)) {
					result = SUCCESS;
				}
				zend_string_release(keyed);
			}
			//this isn't necessary for any specific property write, but since we don't have any other way to clean up local
			//cached Threaded and Closure references that are dead, we have to take the opportunity
			pthreads_store_sync(object);
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}

	if (result != SUCCESS) {
		pthreads_store_storage_dtor(storage);
	} else {
		if (pthreads_store_retain_in_local_cache(write)) {
			/*
				This could be a volatile object, but, we don't want to break
				normal refcounting, we'll read the reference only at volatile objects
			*/
			rebuild_object_properties(&threaded->std);

			if (Z_TYPE(member) == IS_LONG) {
				zend_hash_index_update(threaded->std.properties, Z_LVAL(member), write);
			} else {
				zend_string *keyed = zend_string_dup(Z_STR(member), 0);
				if (zend_hash_update(
					threaded->std.properties, keyed, write)) {
					result = SUCCESS;
				}
				zend_string_release(keyed);
			}
			Z_ADDREF_P(write);
		}
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ */
int pthreads_store_count(zend_object *object, zend_long *count) {
	pthreads_object_t* ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		(*count) = zend_hash_num_elements(ts_obj->store.props);
		pthreads_monitor_unlock(ts_obj->monitor);
	} else (*count) = 0L;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_shift(zend_object *object, zval *member) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval key;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_reset_ex(ts_obj->store.props, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(ts_obj->store.props, &position))) {
			zend_hash_get_current_key_zval_ex(ts_obj->store.props, &key, &position);
			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, member);
				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_del(ts_obj->store.props, Z_LVAL(key));
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				} else {
					zend_hash_del(ts_obj->store.props, Z_STR(key));
					zend_hash_del(threaded->std.properties, Z_STR(key));
					zval_dtor(&key);
				}
			}
		} else ZVAL_NULL(member);
		pthreads_monitor_unlock(ts_obj->monitor);

		return SUCCESS;
	}

	return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_chunk(zend_object *object, zend_long size, zend_bool preserve, zval *chunk) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		HashPosition position;
		pthreads_storage *storage;

		array_init(chunk);
		zend_hash_internal_pointer_reset_ex(ts_obj->store.props, &position);
		while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
			(storage = zend_hash_get_current_data_ptr_ex(ts_obj->store.props, &position))) {
			zval key, zv;

			zend_hash_get_current_key_zval_ex(ts_obj->store.props, &key, &position);

			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, &zv);
				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_update(
						Z_ARRVAL_P(chunk), Z_LVAL(key), &zv);
					zend_hash_index_del(ts_obj->store.props, Z_LVAL(key));
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				} else {
					zend_hash_update(
						Z_ARRVAL_P(chunk), Z_STR(key), &zv);
					zend_hash_del(ts_obj->store.props, Z_STR(key));
					zend_hash_del(threaded->std.properties, Z_STR(key));
					zval_dtor(&key);
				}
			} else break;

			zend_hash_internal_pointer_reset_ex(ts_obj->store.props, &position);
		}
		pthreads_monitor_unlock(ts_obj->monitor);

		return SUCCESS;
	}

	return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_pop(zend_object *object, zval *member) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval key;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_end_ex(ts_obj->store.props, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(ts_obj->store.props, &position))) {
			zend_hash_get_current_key_zval_ex(ts_obj->store.props, &key, &position);

			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, member);

				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_del(
						ts_obj->store.props, Z_LVAL(key));
					zend_hash_index_del(
						threaded->std.properties, Z_LVAL(key));
				} else {
					zend_hash_del(
						ts_obj->store.props, Z_STR(key));
					zend_hash_del(
						threaded->std.properties, Z_STR(key));
					zval_dtor(&key);
				}
			}
		} else ZVAL_NULL(member);

		pthreads_monitor_unlock(ts_obj->monitor);

		return SUCCESS;
	}

   return FAILURE;
} /* }}} */

/* {{{ */
void pthreads_store_tohash(zend_object *object, HashTable *hash) {
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_string *name = NULL;
		zend_ulong idx;
		pthreads_storage *storage;

		pthreads_store_sync(object);


		ZEND_HASH_FOREACH_KEY_PTR(ts_obj->store.props, idx, name, storage) {
			zval *cached;
			zval pzval;
			zend_string *rename;

			//we just synced local cache, so if something is already here, it doesn't need to be modified
			if (hash == threaded->std.properties) {
				if (!name) {
					if (zend_hash_index_exists(hash, idx))
						continue;
				} else {
					if (zend_hash_exists(hash, name))
						continue;
				}
			} else {
				if (!name) {
					cached = zend_hash_index_find(threaded->std.properties, idx);
					if (cached) {
						zend_hash_index_update(hash, idx, cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				} else {
					cached = zend_hash_find(threaded->std.properties, name);
					if (cached) {
						zend_hash_update(hash, name, cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				}
			}

			if (pthreads_store_convert(storage, &pzval)!=SUCCESS) {
				continue;
			}

			if (!name) {
				if (!zend_hash_index_update(hash, idx, &pzval)) {
					zval_ptr_dtor(&pzval);
				}
			} else {
				rename = zend_string_init(name->val, name->len, 0);
				if (!zend_hash_update(hash, rename, &pzval))
					zval_ptr_dtor(&pzval);
				zend_string_release(rename);
			}
		} ZEND_HASH_FOREACH_END();

		pthreads_monitor_unlock(ts_obj->monitor);
	}
} /* }}} */

/* {{{ */
void pthreads_store_free(pthreads_store_t *store){
	zend_hash_destroy(store);
	free(store);
} /* }}} */

/* {{{ */
pthreads_storage* pthreads_store_create(zval *unstore){
	pthreads_storage *storage = NULL;

	if (Z_TYPE_P(unstore) == IS_INDIRECT)
		return pthreads_store_create(Z_INDIRECT_P(unstore));
	if (Z_TYPE_P(unstore) == IS_REFERENCE)
		return pthreads_store_create(&Z_REF_P(unstore)->val);

	storage = (pthreads_storage*) calloc(1, sizeof(pthreads_storage));

	switch((storage->type = Z_TYPE_P(unstore))){
		case IS_NULL: /* do nothing */ break;
		case IS_TRUE: storage->simple.lval = 1; break;
		case IS_FALSE: storage->simple.lval = 0; break;
		case IS_DOUBLE: storage->simple.dval = Z_DVAL_P(unstore); break;
		case IS_LONG: storage->simple.lval = Z_LVAL_P(unstore); break;

		case IS_STRING: if ((storage->length = Z_STRLEN_P(unstore))) {
			storage->data =
				(char*) malloc(storage->length+1);
			memcpy(storage->data, Z_STRVAL_P(unstore), storage->length);
			((char *)storage->data)[storage->length] = 0;
		} break;

		case IS_RESOURCE: {
			pthreads_resource resource = malloc(sizeof(*resource));
			if (resource) {
				resource->original = Z_RES_P(unstore);
				resource->ls = TSRMLS_CACHE;

				storage->data = resource;
				Z_ADDREF_P(unstore);
			}
		} break;

		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(unstore), zend_ce_closure)) {
				const zend_closure *closure = (const zend_closure *) Z_OBJ_P(unstore);
				storage->type = IS_CLOSURE;
				//TODO: this might result in faults because it's not copied properly
				//since we aren't copying this to persistent memory, a fault is going to
				//happen if it's dereferenced after the original closure is destroyed
				//(for what it's worth, this was always a problem.)
				storage->data = closure;
				break;
			}

			if (instanceof_function(Z_OBJCE_P(unstore), pthreads_threaded_entry)) {
				pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(unstore));
				if (threaded->original_zobj != NULL) {
					threaded = threaded->original_zobj;
				}
				storage->type = IS_PTHREADS;
				storage->data = threaded;
				break;
			}

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
			if (instanceof_function(Z_OBJCE_P(unstore), socket_ce)) {
				php_socket *socket = Z_SOCKET_P(unstore);
				if (!Z_ISUNDEF(socket->zstream)) {
					break; // we can't handle these; resources can't be shared safely
				}
				if (!IS_INVALID_SOCKET(socket)) {
					//we still copy invalid sockets, for consistency with normal objects
					pthreads_globals_shared_socket_track(socket->bsd_socket);
				}

				pthreads_storage_socket *stored_socket = malloc(sizeof(pthreads_storage_socket));
				stored_socket->bsd_socket = socket->bsd_socket;
				stored_socket->type = socket->type;
				stored_socket->error = socket->error;
				stored_socket->blocking = socket->blocking;

				storage->type = IS_SOCKET;
				storage->data = stored_socket;
				break;
			}
#endif

		/* break intentionally omitted */
		case IS_ARRAY: if (pthreads_store_tostring(unstore, (char**) &storage->data, &storage->length)==SUCCESS) {
			if (Z_TYPE_P(unstore) == IS_ARRAY)
				storage->exists = zend_hash_num_elements(Z_ARRVAL_P(unstore));
		} break;

	}
	return storage;
}
/* }}} */

/* {{{ */
int pthreads_store_convert(pthreads_storage *storage, zval *pzval){
	int result = SUCCESS;

	switch(storage->type) {
		case IS_NULL: ZVAL_NULL(pzval); break;

		case IS_STRING:
			if (storage->data && storage->length) {
				ZVAL_STRINGL(pzval, (char*)storage->data, storage->length);
			} else ZVAL_EMPTY_STRING(pzval);
		break;

		case IS_FALSE:
		case IS_TRUE: ZVAL_BOOL(pzval, storage->simple.lval); break;

		case IS_LONG: ZVAL_LONG(pzval, storage->simple.lval); break;
		case IS_DOUBLE: ZVAL_DOUBLE(pzval, storage->simple.dval); break;
		case IS_RESOURCE: {
			pthreads_resource stored = (pthreads_resource) storage->data;

			if (stored->ls != TSRMLS_CACHE) {
				zval *search = NULL;
				zend_resource *resource, *found = NULL;

				ZEND_HASH_FOREACH_VAL(&EG(regular_list), search) {
					resource = Z_RES_P(search);
					if (resource->ptr == stored->original->ptr) {
						found = resource;
						break;
					}
				} ZEND_HASH_FOREACH_END();

				if (!found) {
					ZVAL_RES(pzval, stored->original);
					if (zend_hash_next_index_insert(&EG(regular_list), pzval)) {
						pthreads_resources_keep(stored);
					} else ZVAL_NULL(pzval);
					Z_ADDREF_P(pzval);
				} else ZVAL_COPY(pzval, search);
			} else {
				ZVAL_RES(pzval, stored->original);
				Z_ADDREF_P(pzval);
			}
		} break;

		case IS_CLOSURE: {
			char *name;
			size_t name_len;
			zend_string *zname;
			const zend_closure *closure_obj = (const zend_closure *) storage->data;
			zend_function *closure = pthreads_copy_function(&closure_obj->func);

			//TODO: scopes aren't copied here - this will lead to faults if this is being copied from child -> parent
			zend_create_closure(pzval, closure, closure->common.scope, closure_obj->called_scope, NULL);

			name_len = spprintf(&name, 0, "Closure@%p", zend_get_closure_method_def(Z_OBJ_P(pzval)));
			zname = zend_string_init(name, name_len, 0);

			if (!zend_hash_update_ptr(EG(function_table), zname, closure)) {
				result = FAILURE;
				zval_dtor(pzval);
			} else result = SUCCESS;
			efree(name);
			zend_string_release(zname);
		} break;

		case IS_PTHREADS: {
			pthreads_zend_object_t* threaded = storage->data;

			if (!pthreads_globals_object_connect(threaded, NULL, pzval)) {
				zend_throw_exception_ex(
					spl_ce_RuntimeException, 0,
					"pthreads detected an attempt to connect to an object which has already been destroyed");
				result = FAILURE;
			}
		} break;

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
		case IS_SOCKET: {
			pthreads_storage_socket *stored_socket = storage->data;

			object_init_ex(pzval, socket_ce);
			php_socket *socket = Z_SOCKET_P(pzval);
			socket->bsd_socket = stored_socket->bsd_socket;
			socket->type = stored_socket->type;
			socket->error = stored_socket->error;
			socket->blocking = stored_socket->blocking;
			result = SUCCESS;
		} break;
#endif

		case IS_OBJECT:
		case IS_ARRAY:
			result = pthreads_store_tozval(pzval, (char*) storage->data, storage->length);
		break;

		default: ZVAL_NULL(pzval);
	}

	if (result == FAILURE) {
		ZVAL_NULL(pzval);
	}

	return result;
}
/* }}} */

static HashTable *pthreads_store_copy_hash(HashTable *source);
static zend_ast_ref* pthreads_store_copy_ast(zend_ast* ast);
static void* pthreads_store_copy_ast_tree(zend_ast* ast, void* buf);

static int pthreads_store_copy_zval(zval *dest, zval *source) {
	if (Z_TYPE_P(source) == IS_INDIRECT)
		return pthreads_store_copy_zval(dest, Z_INDIRECT_P(source));
	if (Z_TYPE_P(source) == IS_REFERENCE)
		return pthreads_store_copy_zval(dest, &Z_REF_P(source)->val);

	int result = FAILURE;
	switch (Z_TYPE_P(source)) {
		case IS_NULL:
		case IS_TRUE:
		case IS_FALSE:
		case IS_LONG:
		case IS_DOUBLE:
			ZVAL_DUP(dest, source);
			result = SUCCESS;
		break;

		case IS_STRING:
			ZVAL_STR(dest, zend_string_new(Z_STR_P(source)));
			result = SUCCESS;
		break;

		case IS_ARRAY:
			ZVAL_ARR(dest, pthreads_store_copy_hash(Z_ARRVAL_P(source)));
			result = SUCCESS;
		break;

		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(source), pthreads_threaded_entry)) {
				pthreads_globals_object_connect(PTHREADS_FETCH_FROM(Z_OBJ_P(source)), NULL, dest);
				result = SUCCESS;
			} else if (instanceof_function(Z_OBJCE_P(source), zend_ce_closure)) {
				const zend_closure *closure_obj = (const zend_closure *) Z_OBJ_P(source);

				char *name;
				size_t name_len;
				zend_string *zname;
				zend_function *closure = pthreads_copy_function(&closure_obj->func);

				//TODO: scopes aren't being copied here - this will lead to faults if we're copying from child -> parent
				zend_create_closure(dest, closure, closure->common.scope, closure_obj->called_scope, NULL);

				name_len = spprintf(&name, 0, "Closure@%p", zend_get_closure_method_def(Z_OBJ_P(dest)));
				zname = zend_string_init(name, name_len, 0);

				if (!zend_hash_update_ptr(EG(function_table), zname, closure)) {
					result = FAILURE;
					zval_dtor(dest);
				} else result = SUCCESS;
				efree(name);
				zend_string_release(zname);

				result = SUCCESS;
			}
		break;

		case IS_CONSTANT_AST:
			ZVAL_AST(dest, pthreads_store_copy_ast(GC_AST(Z_AST_P(source))));
			result = SUCCESS;
		break;
		default:
			result = FAILURE;
	}
	return result;
}

static HashTable *pthreads_store_copy_hash(HashTable *source) {
	Bucket *p;
	zval newzval;

	//TODO: what about IS_ARRAY_IMMUTABLE?
	HashTable *ht = (HashTable*) pemalloc(sizeof(HashTable), GC_FLAGS(source) & IS_ARRAY_PERSISTENT);
	zend_hash_init(ht, source->nNumUsed, NULL, source->pDestructor, GC_FLAGS(source) & IS_ARRAY_PERSISTENT);

	ZEND_HASH_FOREACH_BUCKET(source, p){
		if(pthreads_store_copy_zval(&newzval, &p->val) == FAILURE){
			continue;
		}

		if (p->key) {
			zend_hash_update(ht, zend_string_new(p->key), &newzval);
		} else {
			zend_hash_index_update(ht, p->h, &newzval);
		}
	} ZEND_HASH_FOREACH_END();

	return ht;
}

#if PHP_VERSION_ID < 80100
static inline size_t zend_ast_size(uint32_t children) {
	//this is an exact copy of zend_ast_size() in zend_ast.c, which we can't use because it's static :(
	//this is in the header in 8.1, so it's only needed for 8.0
	return sizeof(zend_ast) - sizeof(zend_ast*) + sizeof(zend_ast*) * children;
}
#endif

static inline size_t zend_ast_list_size(uint32_t children) {
	//this is an exact copy of zend_ast_list_size() in zend_ast.c, which we can't use because it's static :(
	return sizeof(zend_ast_list) - sizeof(zend_ast*) + sizeof(zend_ast*) * children;
}

static size_t zend_ast_tree_size(zend_ast* ast) {
	//this is an exact copy of zend_ast_tree_size() in zend_ast.c, which we can't use because it's static :(
	size_t size;

	if (ast->kind == ZEND_AST_ZVAL || ast->kind == ZEND_AST_CONSTANT) {
		size = sizeof(zend_ast_zval);
	}
	else if (zend_ast_is_list(ast)) {
		uint32_t i;
		zend_ast_list* list = zend_ast_get_list(ast);

		size = zend_ast_list_size(list->children);
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				size += zend_ast_tree_size(list->child[i]);
			}
		}
	}
	else {
		uint32_t i, children = zend_ast_get_num_children(ast);

		size = zend_ast_size(children);
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				size += zend_ast_tree_size(ast->child[i]);
			}
		}
	}
	return size;
}

static void* pthreads_store_copy_ast_tree(zend_ast* ast, void* buf)
{
	//this code is adapted from zend_ast_tree_copy() in zend_ast.c
	//sadly we have to duplicate all of this even though we only need to change a couple of lines

	if (ast->kind == ZEND_AST_ZVAL) {
		zend_ast_zval* new = (zend_ast_zval*)buf;
		new->kind = ZEND_AST_ZVAL;
		new->attr = ast->attr;
		pthreads_store_copy_zval(&new->val, zend_ast_get_zval(ast)); //changed
		buf = (void*)((char*)buf + sizeof(zend_ast_zval));
	} else if (ast->kind == ZEND_AST_CONSTANT) {
		zend_ast_zval* new = (zend_ast_zval*)buf;
		new->kind = ZEND_AST_CONSTANT;
		new->attr = ast->attr;
		ZVAL_STR(&new->val, zend_string_new(zend_ast_get_constant_name(ast))); //changed
		buf = (void*)((char*)buf + sizeof(zend_ast_zval));
	} else if (zend_ast_is_list(ast)) {
		zend_ast_list* list = zend_ast_get_list(ast);
		zend_ast_list* new = (zend_ast_list*)buf;
		uint32_t i;
		new->kind = list->kind;
		new->attr = list->attr;
		new->children = list->children;
		buf = (void*)((char*)buf + zend_ast_list_size(list->children));
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				new->child[i] = (zend_ast*)buf;
				buf = pthreads_store_copy_ast_tree(list->child[i], buf); //changed
			}
			else {
				new->child[i] = NULL;
			}
		}
	} else {
		uint32_t i, children = zend_ast_get_num_children(ast);
		zend_ast* new = (zend_ast*)buf;
		new->kind = ast->kind;
		new->attr = ast->attr;
		buf = (void*)((char*)buf + zend_ast_size(children));
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				new->child[i] = (zend_ast*)buf;
				buf = pthreads_store_copy_ast_tree(ast->child[i], buf); //changed
			}
			else {
				new->child[i] = NULL;
			}
		}
	}
	return buf;
}

static zend_ast_ref* pthreads_store_copy_ast(zend_ast* ast) {
	//this code is adapted from zend_ast_copy() in zend_ast.c
	//sadly we have to duplicate all of this even though we only need to change a couple of lines

	size_t tree_size;
	zend_ast_ref* ref;

	ZEND_ASSERT(ast != NULL);
	tree_size = zend_ast_tree_size(ast) + sizeof(zend_ast_ref);
	ref = emalloc(tree_size);
	pthreads_store_copy_ast_tree(ast, GC_AST(ref));
	GC_SET_REFCOUNT(ref, 1);
	GC_TYPE_INFO(ref) = GC_CONSTANT_AST;
	return ref;
}

/* {{{ */
int pthreads_store_separate(zval * pzval, zval *separated) {
	if(pthreads_store_copy_zval(separated, pzval) != SUCCESS){
		ZVAL_NULL(separated);
		return FAILURE;
	}
	return SUCCESS;
} /* }}} */

/* {{{ */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength) {
	int result = FAILURE;
	if (pzval && (Z_TYPE_P(pzval) != IS_NULL)) {
		smart_str smart;
		memset(&smart, 0, sizeof(smart_str));

		php_serialize_data_t vars;

		PHP_VAR_SERIALIZE_INIT(vars);
		php_var_serialize(&smart, pzval, &vars);
		PHP_VAR_SERIALIZE_DESTROY(vars);

		if (EG(exception)) {
			smart_str_free(&smart);

			*pstring = NULL;
			*slength = 0;
			return FAILURE;
		}

		if (smart.s) {
			*slength = smart.s->len;
			if (*slength) {
				*pstring = malloc(*slength+1);
				if (*pstring) {
					memcpy(
						(char*) *pstring, (const void*) smart.s->val, smart.s->len
					);
					(*pstring)[*slength] = 0;
					result = SUCCESS;
				}
			} else *pstring = NULL;
		}

		smart_str_free(&smart);
	} else {
		*slength = 0;
		*pstring = NULL;
	}
	return result;
} /* }}} */

/* {{{ */
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength) {
	int result = SUCCESS;

	if (pstring) {
		const unsigned char* pointer = (const unsigned char*) pstring;

		if (pointer) {
			php_unserialize_data_t vars;

			PHP_VAR_UNSERIALIZE_INIT(vars);
			if (!php_var_unserialize(pzval, &pointer, pointer+slength, &vars)) {
				result = FAILURE;
			} else if (Z_REFCOUNTED_P(pzval)) {
				gc_check_possible_root(Z_COUNTED_P(pzval));
			}
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		} else result = FAILURE;
	} else result = FAILURE;

	return result;
} /* }}} */

/* {{{ */
int pthreads_store_merge(zend_object *destination, zval *from, zend_bool overwrite) {
	if (Z_TYPE_P(from) != IS_ARRAY &&
		Z_TYPE_P(from) != IS_OBJECT) {
		return FAILURE;
	}

	switch (Z_TYPE_P(from)) {
		case IS_OBJECT: {
			if (IS_PTHREADS_OBJECT(from)) {
				pthreads_object_t* threaded[2] = {PTHREADS_FETCH_TS_FROM(destination), PTHREADS_FETCH_TS_FROM(Z_OBJ_P(from))};

				if (pthreads_monitor_lock(threaded[0]->monitor)) {
					if (pthreads_monitor_lock(threaded[1]->monitor)) {
						HashPosition position;
						pthreads_storage *storage;
						HashTable *tables[2] = {threaded[0]->store.props, threaded[1]->store.props};
						zval key;

						for (zend_hash_internal_pointer_reset_ex(tables[1], &position);
							 (storage = zend_hash_get_current_data_ptr_ex(tables[1], &position));
							 zend_hash_move_forward_ex(tables[1], &position)) {
							zend_hash_get_current_key_zval_ex(tables[1], &key, &position);
							if (Z_TYPE(key) == IS_STRING)
								zend_string_delref(Z_STR(key));

							if (!overwrite) {
								if (Z_TYPE(key) == IS_LONG) {
									if (zend_hash_index_exists(tables[0], Z_LVAL(key)))
										continue;
								} else {
									if (zend_hash_exists(tables[0], Z_STR(key)))
										continue;
								}
							}

							if (pthreads_store_is_immutable(destination, &key)) {
								break;
							}

							if (storage->type != IS_RESOURCE) {
								pthreads_storage *copy = malloc(sizeof(pthreads_storage));

								memcpy(copy, storage, sizeof(pthreads_storage));

								switch (copy->type) {
									case IS_STRING:
									case IS_OBJECT:
									case IS_ARRAY: if (storage->length) {
										copy->data = (char*) malloc(copy->length+1);
										if (!copy->data) {
											break;
										}
										memcpy(copy->data, (const void*) storage->data, copy->length);
										((char *)copy->data)[copy->length] = 0;
									} break;
								}

								if (Z_TYPE(key) == IS_LONG) {
									zend_hash_index_update_ptr(tables[0], Z_LVAL(key), copy);
								} else {
									/* anything provided by this context might not live long enough to be used by another context,
									 * so we have to hard copy, even if the string is interned. */
									zend_string *orig_key = Z_STR(key);
									zend_string *keyed = zend_string_init(ZSTR_VAL(orig_key), ZSTR_LEN(orig_key), 1);

									zend_hash_update_ptr(tables[0], keyed, copy);
									zend_string_release(keyed);
								}
							}
						}

						pthreads_monitor_unlock(threaded[1]->monitor);
					}

					pthreads_monitor_unlock(threaded[0]->monitor);

					return SUCCESS;

				} else return FAILURE;
			}
		}

		/* fall through on purpose to handle normal objects and arrays */

		default: {
			pthreads_object_t* ts_obj = PTHREADS_FETCH_TS_FROM(destination);

			if (pthreads_monitor_lock(ts_obj->monitor)) {
				HashPosition position;
				zval *pzval;
				int32_t index = 0;
				HashTable *table = (Z_TYPE_P(from) == IS_ARRAY) ? Z_ARRVAL_P(from) : Z_OBJPROP_P(from);

				for (zend_hash_internal_pointer_reset_ex(table, &position);
					(pzval = zend_hash_get_current_data_ex(table, &position));
					zend_hash_move_forward_ex(table, &position)) {
					zval key;

					zend_hash_get_current_key_zval_ex(table, &key, &position);
					if (Z_TYPE(key) == IS_STRING)
						zend_string_delref(Z_STR(key));

					switch (Z_TYPE(key)) {
						case IS_LONG:
							if (!overwrite && zend_hash_index_exists(ts_obj->store.props, Z_LVAL(key))) {
								goto next;
							}
							pthreads_store_write(destination, &key, pzval);
						break;

						case IS_STRING:
							if (!overwrite && zend_hash_exists(ts_obj->store.props, Z_STR(key))) {
								goto next;
							}
							pthreads_store_write(destination, &key, pzval);
						break;
					}

next:
					index++;
				}

				pthreads_monitor_unlock(ts_obj->monitor);
			}
		} break;
	}

	return SUCCESS;
} /* }}} */


/* {{{ Will free store element */
void pthreads_store_storage_dtor (pthreads_storage *storage){
	if (storage) {
		switch (storage->type) {
			case IS_OBJECT:
			case IS_STRING:
			case IS_ARRAY:
			case IS_RESOURCE:
			case IS_SOCKET:
				if (storage->data) {
					free(storage->data);
				}
			break;
		}
		free(storage);
	}
} /* }}} */

/* {{{ iteration helpers */
void pthreads_store_reset(zend_object *object, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_hash_internal_pointer_reset_ex(ts_obj->store.props, position);
		if (zend_hash_has_more_elements_ex(ts_obj->store.props, position) == FAILURE) { //empty
			*position = HT_INVALID_IDX;
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}
}

void pthreads_store_key(zend_object *object, zval *key, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);
	zend_string *str_key;
	zend_ulong num_key;

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		switch (zend_hash_get_current_key_ex(ts_obj->store.props, &str_key, &num_key, position)) {
			case HASH_KEY_NON_EXISTENT:
				ZVAL_NULL(key);
			break;
			case HASH_KEY_IS_LONG:
				ZVAL_LONG(key, num_key);
			break;
			case HASH_KEY_IS_STRING:
				ZVAL_STR(key, zend_string_dup(str_key, 0));
			break;
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}
}

void pthreads_store_data(zend_object *object, zval *value, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval key;
		zend_hash_get_current_key_zval_ex(ts_obj->store.props, &key, position);

		if (pthreads_store_read(object, &key, BP_VAR_R, value) == FAILURE) {
			ZVAL_UNDEF(value);
		}
		zval_dtor(&key);

		pthreads_monitor_unlock(ts_obj->monitor);
	}
}

void pthreads_store_forward(zend_object *object, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_hash_move_forward_ex(
			ts_obj->store.props, position);
		if (zend_hash_has_more_elements_ex(ts_obj->store.props, position) == FAILURE) {
			*position = HT_INVALID_IDX;
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}
} /* }}} */

#endif
