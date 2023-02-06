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
#include <src/resources.h>
#include <src/copy.h>
#include <src/store.h>
#include <src/globals.h>
#include <src/prepare.h>
#include <src/store_types.h>
#include <src/thread.h>
#include <Zend/zend_ast.h>
#if PHP_VERSION_ID >= 80100
#include <Zend/zend_enum.h>
#endif

#define PTHREADS_STORAGE_EMPTY {0, 0, 0, 0, NULL}

/* {{{ */
static zend_result pthreads_store_save_zval(pthreads_ident_t* source, zval* zstorage, zval* write);
static void pthreads_store_restore_zval_ex(zval* unstore, zval* zstorage, zend_bool* was_pthreads_storage);
static void pthreads_store_restore_zval(zval* unstore, zval* zstorage); /* }}} */
static void pthreads_store_storage_dtor(zval* element);

/* {{{ */
pthreads_store_t* pthreads_store_alloc() {
	pthreads_store_t *store = (pthreads_store_t*) calloc(1, sizeof(pthreads_store_t));

	if (store) {
		store->modcount = 0;
		zend_hash_init(
			&store->hash, 8, NULL,
			(dtor_func_t) pthreads_store_storage_dtor, 1);
	}

	return store;
} /* }}} */

void pthreads_store_sync_local_properties(zend_object* object) { /* {{{ */
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_ulong idx;
	zend_string *name;
	zval *val;
	pthreads_storage *ts_val;
	zend_bool remove;

	if (threaded->local_props_modcount == ts_obj->props->modcount) {
		return;
	}

	if (threaded->std.properties) {
		ZEND_HASH_FOREACH_KEY_VAL(threaded->std.properties, idx, name, val) {
			if (!name) {
				ts_val = TRY_PTHREADS_STORAGE_PTR_P(zend_hash_index_find(&ts_obj->props->hash, idx));
			} else {
				ts_val = TRY_PTHREADS_STORAGE_PTR_P(zend_hash_find(&ts_obj->props->hash, name));
			}

			remove = 1;
			if (ts_val) {
				ZVAL_DEINDIRECT(val);
				if (ts_val->type == STORE_TYPE_PTHREADS && IS_PTHREADS_OBJECT(val)) {
					pthreads_zend_object_t* shared = (pthreads_zend_object_t*)ts_val->data;
					pthreads_zend_object_t* local = PTHREADS_FETCH_FROM(Z_OBJ_P(val));

					if (
						shared == local || //same object
						(pthreads_globals_object_valid(shared) && shared->ts_obj == local->ts_obj) //connection to a valid foreign object
					) {
						remove = 0;
					}
				} else if (ts_val->type == STORE_TYPE_CLOSURE && IS_PTHREADS_CLOSURE_OBJECT(val)) {
					zend_closure* shared = ((pthreads_closure_storage_t*)ts_val->data)->closure;
					zend_closure* local = (zend_closure*)Z_OBJ_P(val);
					if (shared == local) {
						remove = 0;
					}
#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
				} else if (ts_val->type == STORE_TYPE_SOCKET && IS_EXT_SOCKETS_OBJECT(val)) {
					pthreads_storage_socket* shared = (pthreads_storage_socket*)ts_val->data;
					php_socket* local = Z_SOCKET_P(val);
					if (shared->bsd_socket == local->bsd_socket) {
						remove = 0;
					}
#endif
				} else if (ts_val->type == STORE_TYPE_STRING_PTR && Z_TYPE_P(val) == IS_STRING) {
					pthreads_string_storage_t* string = (pthreads_string_storage_t*)ts_val->data;
					if (string->owner.ls == TSRMLS_CACHE && string->string == Z_STR_P(val)) {
						//local caching of this by other threads is probably fine too, but fully caching it would probably
						//require bytewise comparison, which isn't gonna be very performant
						//it should be sufficient to only persist the owner thread's ref, since that's where copies will be
						//made from anyway.
						remove = 0;
					}
				}
			}

			if (remove) {
				if (!name) {
					zend_hash_index_del(threaded->std.properties, idx);
				}
				else {
					zend_hash_del(threaded->std.properties, name);
				}
			}
		} ZEND_HASH_FOREACH_END();
		HT_FLAGS(threaded->std.properties) &= ~HASH_FLAG_HAS_EMPTY_IND;
	}
	threaded->local_props_modcount = ts_obj->props->modcount;
} /* }}} */

static inline zend_bool pthreads_store_retain_in_local_cache(zval* val) {
	return IS_PTHREADS_OBJECT(val) || IS_PTHREADS_CLOSURE_OBJECT(val) || IS_EXT_SOCKETS_OBJECT(val) || Z_TYPE_P(val) == IS_STRING;
}

static inline zend_bool pthreads_store_valid_local_cache_item(zval* val) {
	//rebuild_object_properties() may add IS_INDIRECT zvals to point to the linear property table
	//we don't want that, because they aren't used by pthreads and are always uninitialized
	return Z_TYPE_P(val) != IS_INDIRECT;
}

/* {{{ */
static inline zend_bool pthreads_store_storage_is_cacheable(zval* zstorage) {
	pthreads_storage* storage = TRY_PTHREADS_STORAGE_PTR_P(zstorage);
	return storage && (storage->type == STORE_TYPE_PTHREADS || storage->type == STORE_TYPE_CLOSURE || storage->type == STORE_TYPE_SOCKET || storage->type == STORE_TYPE_STRING_PTR);
} /* }}} */

/* {{{ Syncs all the cacheable properties from TS storage into local cache */
void pthreads_store_full_sync_local_properties(zend_object *object) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);

	if (GC_IS_RECURSIVE(&threaded->std)) {
		return;
	}

	if (!pthreads_monitor_lock(threaded->ts_obj->monitor)) {
		return;
	}
	GC_PROTECT_RECURSION(&threaded->std);

	pthreads_store_sync_local_properties(object); //remove any outdated cache elements

	pthreads_object_t* ts_obj = threaded->ts_obj;

	zend_long idx;
	zend_string* name;
	zval* zstorage;

	rebuild_object_properties(&threaded->std);

	ZEND_HASH_FOREACH_KEY_VAL(&ts_obj->props->hash, idx, name, zstorage) {
		zval* cached;
		zval pzval;

		//we just synced local cache, so if something is already here, it doesn't need to be modified
		if (!name) {
			cached = zend_hash_index_find(threaded->std.properties, idx);
		} else {
			cached = zend_hash_find(threaded->std.properties, name);
		}
		if (cached && pthreads_store_valid_local_cache_item(cached)) {
			continue;
		}
		if (pthreads_store_storage_is_cacheable(zstorage)) {
			pthreads_store_restore_zval(&pzval, zstorage);

			if (IS_PTHREADS_OBJECT(&pzval)) {
				pthreads_store_full_sync_local_properties(Z_OBJ(pzval));
			}

			//TODO: we need to recursively sync here if this is a closure or thread-safe object
			if (!name) {
				if (!zend_hash_index_update(threaded->std.properties, idx, &pzval)) {
					zval_ptr_dtor(&pzval);
				}
			} else {
				/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
				if (!zend_hash_str_update(threaded->std.properties, ZSTR_VAL(name), ZSTR_LEN(name), &pzval))
					zval_ptr_dtor(&pzval);
			}
		}
	} ZEND_HASH_FOREACH_END();

	if (threaded->worker_data != NULL) {
		pthreads_worker_sync_collectable_tasks(threaded->worker_data);
	}

	GC_UNPROTECT_RECURSION(&threaded->std);
	pthreads_monitor_unlock(threaded->ts_obj->monitor);
} /* }}} */

/* {{{ */
static inline void _pthreads_store_bump_modcount_nolock(pthreads_zend_object_t *threaded) {
	if (threaded->local_props_modcount == threaded->ts_obj->props->modcount) {
		/* It's possible we may have caused a modification via a connection whose property table was not in sync. This is OK
		 * for writes, because these cases usually cause destruction of outdated caches anyway, but we have to avoid
		 * incorrectly marking the local table as in-sync.
		 */
		threaded->local_props_modcount++;
	}
	threaded->ts_obj->props->modcount++;
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
static inline zend_bool pthreads_store_member_is_cacheable(zend_object *object, zval *key) {
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	zval *zstorage;

	if (Z_TYPE_P(key) == IS_LONG) {
		zstorage = zend_hash_index_find(&threaded->ts_obj->props->hash, Z_LVAL_P(key));
	} else zstorage = zend_hash_find(&threaded->ts_obj->props->hash, Z_STR_P(key));

	return pthreads_store_storage_is_cacheable(zstorage);
} /* }}} */

/* {{{ */
int pthreads_store_delete(zend_object *object, zval *key) {
	int result = FAILURE;
	zval member;
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pthreads_store_coerce(key, &member);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_bool was_pthreads_object = pthreads_store_member_is_cacheable(object, &member);
		if (Z_TYPE(member) == IS_LONG) {
			result = zend_hash_index_del(&ts_obj->props->hash, Z_LVAL(member));
		} else result = zend_hash_del(&ts_obj->props->hash, Z_STR(member));

		if (result == SUCCESS && was_pthreads_object) {
			_pthreads_store_bump_modcount_nolock(threaded);
		}
		//TODO: sync local properties?
		pthreads_monitor_unlock(ts_obj->monitor);
	} else result = FAILURE;

	if (result == SUCCESS && threaded->std.properties) {
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
		zval *zstorage;

		if (Z_TYPE(member) == IS_LONG) {
			zstorage = zend_hash_index_find(&ts_obj->props->hash, Z_LVAL(member));
		} else zstorage = zend_hash_find(&ts_obj->props->hash, Z_STR(member));

		if (zstorage) {
			isset = 1;
			if (has_set_exists == ZEND_PROPERTY_NOT_EMPTY) {
				switch (Z_TYPE_P(zstorage)) {
					case IS_NULL:
					case IS_FALSE:
						isset = 0;
						break;
					case IS_LONG:
						if (Z_LVAL_P(zstorage) == 0) {
							isset = 0;
						}
						break;
					case IS_DOUBLE:
						if (Z_DVAL_P(zstorage) == 0.0) {
							isset = 0;
						}
						break;
					case IS_STRING:
						if (Z_STRLEN_P(zstorage) == 0 || Z_STRVAL_P(zstorage)[0] == '0') {
							isset = 0;
						}
						break;
					case IS_ARRAY:
						if (zend_hash_num_elements(Z_ARRVAL_P(zstorage)) == 0) {
							isset = 0;
						}
						break;
					case IS_PTR: {
						pthreads_storage* storage = TRY_PTHREADS_STORAGE_PTR_P(zstorage);
						if (storage->type == STORE_TYPE_STRING_PTR) {
							pthreads_string_storage_t* string = (pthreads_string_storage_t*)storage->data;
							if (ZSTR_LEN(string->string) == 0 || ZSTR_VAL(string->string)[0] == '0') {
								isset = 0;
							}
						}
					} break;
					default:
						break;
				}
			} else if (has_set_exists == ZEND_PROPERTY_ISSET) {
				if (Z_TYPE_P(zstorage) == IS_NULL) {
					isset = 0;
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

static inline void pthreads_store_update_local_property(zend_object* object, zval* key, zval* value) {
	if (pthreads_store_retain_in_local_cache(value)) {
		rebuild_object_properties(object);
		if (Z_TYPE_P(key) == IS_LONG) {
			zend_hash_index_update(object->properties, Z_LVAL_P(key), value);
		} else {
			zend_string* str_key = Z_STR_P(key);
			if ((GC_FLAGS(str_key) & (IS_STR_PERSISTENT|IS_STR_INTERNED)) == IS_STR_PERSISTENT) {
				//refcounted persistent string from pthreads_store - we can't use it directly
				//if a bucket with this key already exists, it'll be reused
				zend_hash_str_update(object->properties, Z_STRVAL_P(key), Z_STRLEN_P(key), value);
			} else {
				//any other interned or emalloc'd strings should be safe to use directly here
				zend_hash_update(object->properties, str_key, value);
			}
		}
		Z_TRY_ADDREF_P(value);
	}
}

static inline zend_bool pthreads_store_update_shared_property(pthreads_object_t* ts_obj, zval* key, zval* zstorage) {
	zend_bool result = FAILURE;
	if (Z_TYPE_P(key) == IS_LONG) {
		if (zend_hash_index_update(&ts_obj->props->hash, Z_LVAL_P(key), zstorage))
			result = SUCCESS;
	} else {
		zend_string* str_key = Z_STR_P(key);
		if (GC_FLAGS(str_key) & IS_STR_PERMANENT) {
			//only permanent strings can be used directly
			if (zend_hash_update(&ts_obj->props->hash, str_key, zstorage)) {
				result = SUCCESS;
			}
		} else {
			//refcounted or request-local interned string - this must be hard-copied, regardless of where it came from
			if (zend_hash_str_update(&ts_obj->props->hash, ZSTR_VAL(str_key), ZSTR_LEN(str_key), zstorage)) {
				result = SUCCESS;
			}
		}
	}

	return result;
}

/* {{{ */
int pthreads_store_read(zend_object *object, zval *key, int type, zval *read) {
	int result = FAILURE;
	zval member, *property = NULL;
	pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pthreads_store_coerce(key, &member);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		if (threaded->std.properties) {
			pthreads_store_sync_local_properties(object);

			/* check if there's still a ref in local cache after sync - this ensures ref reuse for Threaded and Closure objects */

			if (Z_TYPE(member) == IS_LONG) {
				property = zend_hash_index_find(threaded->std.properties, Z_LVAL(member));
			} else property = zend_hash_find(threaded->std.properties, Z_STR(member));

			if (property && pthreads_store_valid_local_cache_item(property)) {
				pthreads_monitor_unlock(ts_obj->monitor);
				ZVAL_DEINDIRECT(property);
				ZVAL_COPY(read, property);
				if (coerced) {
					zval_ptr_dtor(&member);
				}
				return SUCCESS;
			}
		}

		zval *zstorage;

		if (Z_TYPE(member) == IS_LONG) {
			zstorage = zend_hash_index_find(&ts_obj->props->hash, Z_LVAL(member));
		} else zstorage = zend_hash_find(&ts_obj->props->hash, Z_STR(member));

		if (zstorage) {
			pthreads_storage *serialized = TRY_PTHREADS_STORAGE_PTR_P(zstorage);
			/* strictly only reads are supported */
			if ((serialized == NULL || serialized->type != STORE_TYPE_PTHREADS) && type != BP_VAR_R && type != BP_VAR_IS){
				zend_throw_error(zend_ce_error, "Indirect modification of non-Threaded members of %s is not supported", ZSTR_VAL(object->ce->name));
				result = FAILURE;
			} else {
				pthreads_store_restore_zval(read, zstorage);
				result = SUCCESS;
			}
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}

	if (result != SUCCESS) {
		ZVAL_UNDEF(read);
	} else {
		pthreads_store_update_local_property(&threaded->std, &member, read);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ Copies strings (as needed) for use in thread-safe object tables */
static zend_string* pthreads_store_save_string(zend_string* string) {
	zend_string* result;
	if (GC_FLAGS(string) & IS_STR_PERMANENT) { //interned by OPcache, or provided by builtin class
		result = string;
	}
	else {
		result = zend_string_init(ZSTR_VAL(string), ZSTR_LEN(string), 1);
	}

	return result;
} /* }}} */

static zend_string* pthreads_store_restore_string(zend_string* string) {
	zend_string* result;
	if (GC_FLAGS(string) & IS_STR_PERMANENT) {
		/* string from OPcache or internal class */
		result = string;
	} else {
		result = zend_string_init(ZSTR_VAL(string), ZSTR_LEN(string), 0);
	}
	return result;
}

/* {{{ */
int pthreads_store_write(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded) {
	int result = FAILURE;
	zval vol, member, zstorage;
	pthreads_zend_object_t *threaded =
		PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = 0;

	if (Z_TYPE_P(write) == IS_ARRAY && coerce_array_to_threaded == PTHREADS_STORE_COERCE_ARRAY) {
		/* coerce arrays into threaded objects */
		object_init_ex(
			&vol, pthreads_threaded_array_entry);
		pthreads_store_merge(Z_OBJ(vol), write, 1, PTHREADS_STORE_COERCE_ARRAY);
		/* this will be addref'd when caching the object */
		Z_SET_REFCOUNT(vol, 0);
		write = &vol;
	}

	if (Z_TYPE_P(write) == IS_OBJECT) {
		/* when we copy out in another context, we want properties table
			to be initialized */
		rebuild_object_properties(Z_OBJ_P(write));
	}

	if (pthreads_store_save_zval(&threaded->owner, &zstorage, write) != SUCCESS) {
		zend_throw_error(zend_ce_error, "Unsupported data type %s", zend_get_type_by_const(Z_TYPE_P(write)));
		return FAILURE;
	}

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		if (!key) {
			zend_ulong next = zend_hash_next_free_element(&ts_obj->props->hash);
			if (next == ZEND_LONG_MIN) next = 0;
			ZVAL_LONG(&member, next);
		} else {
			coerced = pthreads_store_coerce(key, &member);
		}

		zend_bool was_pthreads_object = pthreads_store_member_is_cacheable(object, &member);
		result = pthreads_store_update_shared_property(ts_obj, &member, &zstorage);
		if (result == SUCCESS && was_pthreads_object) {
			_pthreads_store_bump_modcount_nolock(threaded);
		}
		//this isn't necessary for any specific property write, but since we don't have any other way to clean up local
		//cached Threaded references that are dead, we have to take the opportunity
		pthreads_store_sync_local_properties(object);

		pthreads_monitor_unlock(ts_obj->monitor);
	}

	if (result != SUCCESS) {
		pthreads_store_storage_dtor(&zstorage);
	} else {
		pthreads_store_update_local_property(&threaded->std, &member, write);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ */
int pthreads_store_count(zend_object *object, zend_long *count) {
	pthreads_object_t* ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		(*count) = zend_hash_num_elements(&ts_obj->props->hash);
		pthreads_monitor_unlock(ts_obj->monitor);
	} else (*count) = 0L;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_shift(zend_object *object, zval *member) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval key;
		HashPosition position;
		zval *zstorage;

		zend_hash_internal_pointer_reset_ex(&ts_obj->props->hash, &position);
		if ((zstorage = zend_hash_get_current_data_ex(&ts_obj->props->hash, &position))) {
			zend_hash_get_current_key_zval_ex(&ts_obj->props->hash, &key, &position);
			zend_bool was_pthreads_object;

			pthreads_store_restore_zval_ex(member, zstorage, &was_pthreads_object);
			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_del(&ts_obj->props->hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				zend_hash_del(&ts_obj->props->hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}

			if (was_pthreads_object) {
				_pthreads_store_bump_modcount_nolock(threaded);
			}
			//TODO: maybe we should be syncing local properties here?
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

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		HashPosition position;
		zval *zstorage;

		array_init(chunk);
		zend_hash_internal_pointer_reset_ex(&ts_obj->props->hash, &position);
		zend_bool removed_pthreads_object = 0;
		while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
			(zstorage = zend_hash_get_current_data_ex(&ts_obj->props->hash, &position))) {
			zval key, zv;

			zend_hash_get_current_key_zval_ex(&ts_obj->props->hash, &key, &position);

			zend_bool was_pthreads_object;
			pthreads_store_restore_zval_ex(&zv, zstorage, &was_pthreads_object);
			if (!removed_pthreads_object) {
				removed_pthreads_object = was_pthreads_object;
			}
			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_update(
					Z_ARRVAL_P(chunk), Z_LVAL(key), &zv);
				zend_hash_index_del(&ts_obj->props->hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
				zend_hash_str_update(
					Z_ARRVAL_P(chunk), Z_STRVAL(key), Z_STRLEN(key), &zv);
				zend_hash_del(&ts_obj->props->hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}

			zend_hash_internal_pointer_reset_ex(&ts_obj->props->hash, &position);
		}
		if (removed_pthreads_object) {
			_pthreads_store_bump_modcount_nolock(threaded);
		}
		//TODO: sync local properties?
		pthreads_monitor_unlock(ts_obj->monitor);

		return SUCCESS;
	}

	return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_pop(zend_object *object, zval *member) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t *ts_obj = threaded->ts_obj;

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval key;
		HashPosition position;
		zval *zstorage;

		zend_hash_internal_pointer_end_ex(&ts_obj->props->hash, &position);
		if ((zstorage = zend_hash_get_current_data_ex(&ts_obj->props->hash, &position))) {
			zend_hash_get_current_key_zval_ex(&ts_obj->props->hash, &key, &position);

			zend_bool was_pthreads_object;
			pthreads_store_restore_zval_ex(member, zstorage, &was_pthreads_object);

			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_del(
					&ts_obj->props->hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				zend_hash_del(
					&ts_obj->props->hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}
			if (was_pthreads_object) {
				_pthreads_store_bump_modcount_nolock(threaded);
			}
			//TODO: sync local properties?
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
		zval *zstorage;
		zend_bool changed = 0;

		pthreads_store_sync_local_properties(object);


		ZEND_HASH_FOREACH_KEY_VAL(&ts_obj->props->hash, idx, name, zstorage) {
			zval *cached;
			zval pzval;

			//we just synced local cache, so if something is already here, it doesn't need to be modified
			if (hash == threaded->std.properties) {
				if (!name) {
					cached = zend_hash_index_find(threaded->std.properties, idx);
				} else {
					cached = zend_hash_find(threaded->std.properties, name);
				}
				if (cached && pthreads_store_valid_local_cache_item(cached)) {
					continue;
				}
			} else {
				if (!name) {
					cached = zend_hash_index_find(threaded->std.properties, idx);
					if (cached && pthreads_store_valid_local_cache_item(cached)) {
						zend_hash_index_update(hash, idx, cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				} else {
					cached = zend_hash_find(threaded->std.properties, name);
					if (cached && pthreads_store_valid_local_cache_item(cached)) {
						/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
						zend_hash_str_update(hash, ZSTR_VAL(name), ZSTR_LEN(name), cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				}
			}

			pthreads_store_restore_zval(&pzval, zstorage);

			if (!name) {
				if (!zend_hash_index_update(hash, idx, &pzval)) {
					zval_ptr_dtor(&pzval);
				}
			} else {
				/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
				if (!zend_hash_str_update(hash, ZSTR_VAL(name), ZSTR_LEN(name), &pzval))
					zval_ptr_dtor(&pzval);
			}
			changed = 1;
		} ZEND_HASH_FOREACH_END();

		for (int i = 0; i < object->ce->default_properties_count; i++) {
			zend_property_info* info = object->ce->properties_info_table[i];
			if (info == NULL || (info->flags & ZEND_ACC_STATIC) != 0) {
				continue;
			}

			zval pzval;
			if (zend_hash_find(hash, info->name) == NULL) {
				//uninitialized typed property
				ZVAL_INDIRECT(&pzval, &object->properties_table[OBJ_PROP_TO_NUM(info->offset)]);
				zend_hash_update(hash, info->name, &pzval);
				HT_FLAGS(hash) |= HASH_FLAG_HAS_EMPTY_IND;
				changed = 1;
			}
		}

		if (changed && hash == threaded->std.properties) {
			//if this is the object's own properties table, we need to ensure that junk added here
			//doesn't get incorrectly treated as gospel
			threaded->local_props_modcount = ts_obj->props->modcount - 1;
		}

		pthreads_monitor_unlock(ts_obj->monitor);
	}
} /* }}} */

/* {{{ */
void pthreads_store_persist_local_properties(zend_object* object) {
	pthreads_zend_object_t* threaded = PTHREADS_FETCH_FROM(object);
	pthreads_object_t* ts_obj = threaded->ts_obj;

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zval *zstorage;
		ZEND_HASH_FOREACH_VAL(&ts_obj->props->hash, zstorage) {
			pthreads_storage* storage = TRY_PTHREADS_STORAGE_PTR_P(zstorage);

			if (storage != NULL && storage->type == STORE_TYPE_STRING_PTR) {
				pthreads_string_storage_t* string = (pthreads_string_storage_t*)storage->data;
				if (string->owner.ls == TSRMLS_CACHE) {
					//we can't guarantee this string will continue to be available once we stop referencing it on this thread,
					//so we must create a persistent copy now

					zend_string* persistent_string = pthreads_store_save_string(string->string);
					pthreads_store_storage_dtor(zstorage);

					ZVAL_STR(zstorage, persistent_string);
				}
			}
		} ZEND_HASH_FOREACH_END();

		pthreads_monitor_unlock(ts_obj->monitor);
	}
} /* }}} */

/* {{{ */
void pthreads_store_free(pthreads_store_t *store){
	zend_hash_destroy(&store->hash);
	free(store);
} /* }}} */

/* {{{ */
static pthreads_storage* pthreads_store_create(pthreads_ident_t* source, zval *unstore){
	pthreads_storage *storage = NULL;

	if (Z_TYPE_P(unstore) == IS_INDIRECT)
		return pthreads_store_create(source, Z_INDIRECT_P(unstore));
	if (Z_TYPE_P(unstore) == IS_REFERENCE)
		return pthreads_store_create(source, &Z_REF_P(unstore)->val);

	storage = (pthreads_storage*) calloc(1, sizeof(pthreads_storage));


	switch(Z_TYPE_P(unstore)){
		case IS_STRING: {
			storage->type = STORE_TYPE_STRING_PTR;
			pthreads_string_storage_t* string = malloc(sizeof(pthreads_string_storage_t));
			string->owner = *source;
			string->string = Z_STR_P(unstore);
			storage->data = string;
		} break;

		case IS_RESOURCE: {
			pthreads_resource resource = malloc(sizeof(*resource));
			storage->type = STORE_TYPE_RESOURCE;
			if (resource) {
				resource->original = Z_RES_P(unstore);
				resource->ls = TSRMLS_CACHE;

				storage->data = resource;
				Z_ADDREF_P(unstore);
			}
		} break;

		case IS_OBJECT:
#if PHP_VERSION_ID >= 80100
			if (Z_OBJCE_P(unstore)->ce_flags & ZEND_ACC_ENUM) {
				storage->type = STORE_TYPE_ENUM;
				zval* zname = zend_enum_fetch_case_name(Z_OBJ_P(unstore));

				pthreads_enum_storage_t* enum_info = malloc(sizeof(pthreads_enum_storage_t));

				enum_info->class_name = pthreads_store_save_string(Z_OBJCE_P(unstore)->name);
				enum_info->member_name = pthreads_store_save_string(Z_STR_P(zname));

				storage->data = enum_info;
				break;
			}
#endif

			if (instanceof_function(Z_OBJCE_P(unstore), zend_ce_closure)) {
				const zend_closure *closure = (const zend_closure *) Z_OBJ_P(unstore);
				storage->type = STORE_TYPE_CLOSURE;
				//TODO: this might result in faults because it's not copied properly
				//since we aren't copying this to persistent memory, a fault is going to
				//happen if it's dereferenced after the original closure is destroyed
				//(for what it's worth, this was always a problem.)
				pthreads_closure_storage_t* closure_info = malloc(sizeof(pthreads_closure_storage_t));
				closure_info->closure = closure;
				closure_info->owner = *source;
				storage->data = closure_info;
				break;
			}

			if (instanceof_function(Z_OBJCE_P(unstore), pthreads_threaded_base_entry)) {
				pthreads_zend_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(unstore));
				if (threaded->original_zobj != NULL) {
					threaded = threaded->original_zobj;
				}
				storage->type = STORE_TYPE_PTHREADS;
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

				storage->type = STORE_TYPE_SOCKET;
				storage->data = stored_socket;
				break;
			}
#endif

			/* fallthru to default, non-threaded, non-closure objects cannot be stored */
		default:
			free(storage);
			storage = NULL;
			break;
	}
	return storage;
}
/* }}} */

/* {{{ */
static zend_result pthreads_store_save_zval(pthreads_ident_t* source, zval *zstorage, zval *write) {
	zend_result result = FAILURE;
	switch (Z_TYPE_P(write)) {
		case IS_NULL:
		case IS_FALSE:
		case IS_TRUE:
		case IS_LONG:
		case IS_DOUBLE:
			ZVAL_COPY(zstorage, write);
			result = SUCCESS;
			break;
		case IS_STRING:
			//permanent strings can be used directly
			//non-permanent strings are handled differently
			if (GC_FLAGS(Z_STR_P(write)) & IS_STR_PERMANENT) {
				ZVAL_STR(zstorage, Z_STR_P(write));

				result = SUCCESS;
				break;
			}
		default: {
			pthreads_storage *storage = pthreads_store_create(source, write);
			if (storage != NULL) {
				ZVAL_PTR(zstorage, storage);
				result = SUCCESS;
			} else {
				result = FAILURE;
			}
		} break;
	}
	return result;
} /* }}} */

/* {{{ */
static int pthreads_store_convert(pthreads_storage *storage, zval *pzval){
	int result = SUCCESS;

	switch(storage->type) {
		case STORE_TYPE_STRING_PTR: {
			pthreads_string_storage_t* string = (pthreads_string_storage_t*)storage->data;

			if (string->owner.ls == TSRMLS_CACHE) {
				//this thread owns the string - we can use it directly
				ZVAL_STR_COPY(pzval, string->string);
			} else {
				//this thread does not own the string - create a copy
				ZVAL_STR(pzval, pthreads_store_restore_string(string->string));
			}
		} break;
		case STORE_TYPE_RESOURCE: {
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

		case STORE_TYPE_CLOSURE: {
			char *name;
			size_t name_len;
			zend_string *zname;
			const pthreads_closure_storage_t* closure_data = (const pthreads_closure_storage_t* )storage->data;
			const zend_closure *closure_obj = closure_data->closure;
			zend_function *closure = pthreads_copy_function(&closure_data->owner, &closure_obj->func);

			PTHREADS_ZG(hard_copy_interned_strings) = 1;
			zend_create_closure(
				pzval,
				closure,
				pthreads_prepare_single_class(&closure_data->owner, closure->common.scope),
				pthreads_prepare_single_class(&closure_data->owner, closure_obj->called_scope),
				NULL
			);
			PTHREADS_ZG(hard_copy_interned_strings) = 0;

			name_len = spprintf(&name, 0, "Closure@%p", zend_get_closure_method_def(Z_OBJ_P(pzval)));
			zname = zend_string_init(name, name_len, 0);

			if (!zend_hash_update_ptr(EG(function_table), zname, closure)) {
				result = FAILURE;
				zval_dtor(pzval);
			} else result = SUCCESS;
			efree(name);
			zend_string_release(zname);
		} break;

		case STORE_TYPE_PTHREADS: {
			pthreads_zend_object_t* threaded = storage->data;

			if (!pthreads_globals_object_connect(threaded, NULL, pzval)) {
				zend_throw_exception_ex(
					pthreads_ce_ThreadedConnectionException, 0,
					"pthreads detected an attempt to connect to an object which has already been destroyed");
				result = FAILURE;
			}
		} break;

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
		case STORE_TYPE_SOCKET: {
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
#if PHP_VERSION_ID >= 80100
		case STORE_TYPE_ENUM: {
			pthreads_enum_storage_t* enum_data = (pthreads_enum_storage_t*)storage->data;

			zend_class_entry* enum_ce = zend_lookup_class(enum_data->class_name);

			if (enum_ce && enum_ce->ce_flags & ZEND_ACC_ENUM && zend_hash_exists(CE_CONSTANTS_TABLE(enum_ce), enum_data->member_name)) {
				zend_object* enum_member = zend_enum_get_case(enum_ce, enum_data->member_name);
				ZEND_ASSERT(enum_member);

				ZVAL_OBJ_COPY(pzval, enum_member);
				result = SUCCESS;
			} else {
				//this might happen if the class failed to load on this thread, or if a different version of the class
				//was loaded than on the origin thread
				zend_throw_error(
					NULL,
					"pthreads failed to restore enum case %s::%s because either it or the class does not exist",
					ZSTR_VAL(enum_data->class_name),
					ZSTR_VAL(enum_data->member_name)
				);
			}
			break;
		}
#endif
		default: ZEND_ASSERT(0);
	}

	if (result == FAILURE) {
		ZVAL_NULL(pzval);
	}

	return result;
}
/* }}} */

/* {{{ */
static void pthreads_store_restore_zval_ex(zval *unstore, zval *zstorage, zend_bool *was_pthreads_storage) {
	*was_pthreads_storage = 0;
	switch (Z_TYPE_P(zstorage)) {
		case IS_NULL:
		case IS_FALSE:
		case IS_TRUE:
		case IS_LONG:
		case IS_DOUBLE:
			/* simple values are stored directly */
			ZVAL_COPY(unstore, zstorage);
			break;
		case IS_STRING:
			/* permanent interned string, or persisted string from a dead thread */
			ZVAL_STR(unstore, pthreads_store_restore_string(Z_STR_P(zstorage)));
			break;
		case IS_PTR:
			{
				/* threaded object, serialized object, resource */
				pthreads_storage *storage = (pthreads_storage *) Z_PTR_P(zstorage);
				*was_pthreads_storage = pthreads_store_storage_is_cacheable(zstorage);
				pthreads_store_convert(storage, unstore);
			}
			break;
		default:
			ZEND_ASSERT(0);
	}
} /* }}} */

/* {{{ */
static void pthreads_store_restore_zval(zval *unstore, zval *zstorage) {
	zend_bool dummy;
	pthreads_store_restore_zval_ex(unstore, zstorage, &dummy);
} /* }}} */

/* {{{ */
static void pthreads_store_hard_copy_storage(zval *new_zstorage, zval *zstorage) {
	if (Z_TYPE_P(zstorage) == IS_PTR) {
		pthreads_storage *storage = (pthreads_storage *) Z_PTR_P(zstorage);
		if (storage->type == STORE_TYPE_STRING_PTR) {
			//hard-copy string ptrs here, since the destination object might not exist on the thread which owns the string
			//this means that the owning thread may not be aware that this new ref now exists and won't persist the string when it dies
			pthreads_string_storage_t* string = (pthreads_string_storage_t*)storage->data;
			ZVAL_STR(new_zstorage, pthreads_store_save_string(string->string));

		} else {
			pthreads_storage* copy = malloc(sizeof(pthreads_storage));

			memcpy(copy, storage, sizeof(pthreads_storage));

			//if we add new store types, their internal data might need to be copied here

			ZVAL_PTR(new_zstorage, copy);
		}
	} else if (Z_TYPE_P(zstorage) == IS_STRING) {
		ZVAL_STR(new_zstorage, pthreads_store_save_string(Z_STR_P(zstorage)));
	} else {
		ZEND_ASSERT(!Z_REFCOUNTED_P(zstorage));
		ZVAL_COPY(new_zstorage, zstorage);
	}
} /* }}} */

/* {{{ */
int pthreads_store_merge(zend_object *destination, zval *from, zend_bool overwrite, zend_bool coerce_array_to_threaded) {
	if (Z_TYPE_P(from) != IS_ARRAY &&
		Z_TYPE_P(from) != IS_OBJECT) {
		return FAILURE;
	}

	switch (Z_TYPE_P(from)) {
		case IS_OBJECT: {
			if (IS_PTHREADS_THREADED_ARRAY(Z_OBJCE_P(from))) {
				pthreads_object_t* threaded[2] = {PTHREADS_FETCH_TS_FROM(destination), PTHREADS_FETCH_TS_FROM(Z_OBJ_P(from))};

				if (pthreads_monitor_lock(threaded[0]->monitor)) {
					if (pthreads_monitor_lock(threaded[1]->monitor)) {
						HashPosition position;
						zval *storage;
						HashTable *tables[2] = {&threaded[0]->props->hash, &threaded[1]->props->hash};
						zval key;
						zend_bool overwrote_pthreads_object = 0;

						for (zend_hash_internal_pointer_reset_ex(tables[1], &position);
							 (storage = zend_hash_get_current_data_ex(tables[1], &position));
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

							if (!overwrote_pthreads_object) {
								overwrote_pthreads_object = pthreads_store_member_is_cacheable(destination, &key);
							}

							zval new_zstorage;
							pthreads_store_hard_copy_storage(&new_zstorage, storage);
							pthreads_store_update_shared_property(threaded[0], &key, &new_zstorage);
							if (destination->properties != NULL) {
								if (Z_TYPE(key) == IS_LONG) {
									zend_hash_index_del(destination->properties, Z_LVAL(key));
								} else {
									zend_hash_del(destination->properties, Z_STR(key));
								}
							}
						}
						if (overwrote_pthreads_object) {
							_pthreads_store_bump_modcount_nolock(PTHREADS_FETCH_FROM(destination));
						}
						//TODO: sync local properties?

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
							if (!overwrite && zend_hash_index_exists(&ts_obj->props->hash, Z_LVAL(key))) {
								goto next;
							}
							pthreads_store_write(destination, &key, pzval, coerce_array_to_threaded);
						break;

						case IS_STRING:
							if (!overwrite && zend_hash_exists(&ts_obj->props->hash, Z_STR(key))) {
								goto next;
							}
							pthreads_store_write(destination, &key, pzval, coerce_array_to_threaded);
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
static void pthreads_store_storage_dtor (zval *zstorage){
	if (!zstorage) return;

	if (Z_TYPE_P(zstorage) == IS_PTR) {
		pthreads_storage *storage = (pthreads_storage *) Z_PTR_P(zstorage);
		switch (storage->type) {
			case STORE_TYPE_STRING_PTR:
			case STORE_TYPE_RESOURCE:
			case STORE_TYPE_SOCKET:
			case STORE_TYPE_CLOSURE:
				if (storage->data) {
					free(storage->data);
				}
				break;
#if PHP_VERSION_ID >= 80100
			case STORE_TYPE_ENUM:
				if (storage->data) {
					pthreads_enum_storage_t* data = (pthreads_enum_storage_t*) storage->data;
					zend_string_free(data->class_name);
					zend_string_free(data->member_name);
					free(storage->data);
				}
				break;
#endif
			default: break;
		}
		free(storage);
	} else if (Z_TYPE_P(zstorage) == IS_STRING) {
		zend_string *str = Z_STR_P(zstorage);
		zend_string_release_ex(str, 1);
	} else {
		/* no action necessary */
	}
} /* }}} */

/* {{{ iteration helpers */
void pthreads_store_reset(zend_object *object, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_hash_internal_pointer_reset_ex(&ts_obj->props->hash, position);
		if (zend_hash_has_more_elements_ex(&ts_obj->props->hash, position) == FAILURE) { //empty
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
		switch (zend_hash_get_current_key_ex(&ts_obj->props->hash, &str_key, &num_key, position)) {
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
		zend_hash_get_current_key_zval_ex(&ts_obj->props->hash, &key, position);

		if (pthreads_store_read(object, &key, BP_VAR_R, value) == FAILURE) {
			ZVAL_UNDEF(value);
		}
		if (Z_TYPE(key) == IS_STRING) {
			zend_string_release(Z_STR(key));
		}

		pthreads_monitor_unlock(ts_obj->monitor);
	}
}

void pthreads_store_forward(zend_object *object, HashPosition *position) {
	pthreads_object_t *ts_obj = PTHREADS_FETCH_TS_FROM(object);

	if (pthreads_monitor_lock(ts_obj->monitor)) {
		zend_hash_move_forward_ex(
			&ts_obj->props->hash, position);
		if (zend_hash_has_more_elements_ex(&ts_obj->props->hash, position) == FAILURE) {
			*position = HT_INVALID_IDX;
		}
		pthreads_monitor_unlock(ts_obj->monitor);
	}
} /* }}} */
