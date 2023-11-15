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
#include <src/copy.h>
#include <src/store.h>
#include <src/globals.h>
#include <src/prepare.h>
#include <src/store_types.h>
#include <src/thread.h>
#include <Zend/zend_ast.h>
#include <Zend/zend_enum.h>

#define PMMPTHREAD_STORAGE_EMPTY {0, 0, 0, 0, NULL}

/* {{{ */
static zend_result pmmpthread_store_save_zval(pmmpthread_ident_t* source, zval* zstorage, zval* write);
static void pmmpthread_store_restore_zval_ex(zval* unstore, zval* zstorage, zend_bool* may_be_locally_cached);
static void pmmpthread_store_restore_zval(zval* unstore, zval* zstorage); /* }}} */
static void pmmpthread_store_storage_dtor(zval* element);

/* {{{ */
static inline void pmmpthread_store_invalidate_bounds(pmmpthread_store_t* store) {
	store->first = HT_INVALID_IDX;
	store->last = HT_INVALID_IDX;
} /* }}} */

/* {{{ */
void pmmpthread_store_init(pmmpthread_store_t* store) {
	store->modcount = 0;
	zend_hash_init(
		&store->hash, 8, NULL,
		(dtor_func_t)pmmpthread_store_storage_dtor, 1);
	pmmpthread_store_invalidate_bounds(store);
} /* }}} */

/* {{{ */
void pmmpthread_store_destroy(pmmpthread_store_t* store) {
	zend_hash_destroy(&store->hash);
} /* }}} */

/* {{{ Prepares local property table to cache items.
We may use integer keys, so the ht must be explicitly initialized to avoid zend allocating it as packed, which will cause assert failures. */
static void pmmpthread_store_init_local_properties(zend_object* object) {
	rebuild_object_properties(object);
	if (HT_FLAGS(object->properties) & HASH_FLAG_UNINITIALIZED) {
		zend_hash_real_init_mixed(object->properties);
	}
} /* }}} */

void pmmpthread_store_sync_local_properties(zend_object* object) { /* {{{ */
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;
	zend_ulong idx;
	zend_string *name;
	zval *val;
	pmmpthread_storage *ts_val;
	zend_bool remove;

	if (threaded->local_props_modcount == ts_obj->props.modcount) {
		return;
	}

	if (threaded->std.properties) {
		ZEND_HASH_FOREACH_KEY_VAL(threaded->std.properties, idx, name, val) {
			if (!name) {
				ts_val = TRY_PMMPTHREAD_STORAGE_PTR_P(zend_hash_index_find(&ts_obj->props.hash, idx));
			} else {
				ts_val = TRY_PMMPTHREAD_STORAGE_PTR_P(zend_hash_find(&ts_obj->props.hash, name));
			}

			remove = 1;
			if (ts_val) {
				ZVAL_DEINDIRECT(val);
				if (ts_val->type == STORE_TYPE_THREADSAFE_OBJECT && IS_THREADSAFE_CLASS_INSTANCE(val)) {
					pmmpthread_zend_object_t* shared = ((pmmpthread_zend_object_storage_t*)ts_val)->object;
					pmmpthread_zend_object_t* local = PMMPTHREAD_FETCH_FROM(Z_OBJ_P(val));

					if (
						shared == local || //same object
						(pmmpthread_globals_object_valid(shared) && shared->ts_obj == local->ts_obj) //connection to a valid foreign object
					) {
						remove = 0;
					}
				} else if (ts_val->type == STORE_TYPE_CLOSURE && IS_CLOSURE_OBJECT(val)) {
					zend_closure* shared = ((pmmpthread_closure_storage_t*)ts_val)->closure;
					zend_closure* local = (zend_closure*)Z_OBJ_P(val);
					if (shared == local) {
						remove = 0;
					}
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
				} else if (ts_val->type == STORE_TYPE_SOCKET && IS_EXT_SOCKETS_OBJECT(val)) {
					pmmpthread_socket_storage_t* shared = (pmmpthread_socket_storage_t*)ts_val;
					php_socket* local = Z_SOCKET_P(val);
					if (shared->bsd_socket == local->bsd_socket) {
						remove = 0;
					}
#endif
				} else if (ts_val->type == STORE_TYPE_STRING_PTR && Z_TYPE_P(val) == IS_STRING) {
					pmmpthread_string_storage_t* string = (pmmpthread_string_storage_t*)ts_val;
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
	threaded->local_props_modcount = ts_obj->props.modcount;
} /* }}} */

static inline zend_bool pmmpthread_store_retain_in_local_cache(zval* val) {
	return IS_THREADSAFE_CLASS_INSTANCE(val) || IS_CLOSURE_OBJECT(val) || IS_EXT_SOCKETS_OBJECT(val) || Z_TYPE_P(val) == IS_STRING;
}

static inline zend_bool pmmpthread_store_valid_local_cache_item(zval* val) {
	//rebuild_object_properties() may add IS_INDIRECT zvals to point to the linear property table
	//we don't want that, because they aren't used by pmmpthread and are always uninitialized
	return Z_TYPE_P(val) != IS_INDIRECT;
}

/* {{{ */
static inline zend_bool pmmpthread_store_storage_is_cacheable(zval* zstorage) {
	if (zstorage == NULL) {
		return 0;
	}
	if (Z_TYPE_P(zstorage) == IS_STRING) {
		//permanent interned strings may be stored directly
		return 1;
	}
	pmmpthread_storage* storage = TRY_PMMPTHREAD_STORAGE_PTR_P(zstorage);
	return storage && (storage->type == STORE_TYPE_THREADSAFE_OBJECT || storage->type == STORE_TYPE_CLOSURE || storage->type == STORE_TYPE_SOCKET || storage->type == STORE_TYPE_STRING_PTR);
} /* }}} */

/* {{{ */
static inline zend_bool pmmpthread_store_storage_is_pmmpthread_obj(zval* zstorage) {
	pmmpthread_storage* storage = TRY_PMMPTHREAD_STORAGE_PTR_P(zstorage);
	return storage != NULL && (storage->type == STORE_TYPE_THREADSAFE_OBJECT);
} /* }}} */

/* {{{ Syncs all the cacheable properties from TS storage into local cache */
void pmmpthread_store_full_sync_local_properties(zend_object *object) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);

	if (GC_IS_RECURSIVE(&threaded->std)) {
		return;
	}

	if (!pmmpthread_monitor_lock(&threaded->ts_obj->monitor)) {
		return;
	}
	GC_PROTECT_RECURSION(&threaded->std);

	pmmpthread_store_sync_local_properties(object); //remove any outdated cache elements

	pmmpthread_object_t* ts_obj = threaded->ts_obj;

	zend_long idx;
	zend_string* name;
	zval* zstorage;

	pmmpthread_store_init_local_properties(&threaded->std);

	ZEND_HASH_FOREACH_KEY_VAL(&ts_obj->props.hash, idx, name, zstorage) {
		zval* cached;
		zval pzval;

		//we just synced local cache, so if something is already here, it doesn't need to be modified
		if (!name) {
			cached = zend_hash_index_find(threaded->std.properties, idx);
		} else {
			cached = zend_hash_find(threaded->std.properties, name);
		}
		if (cached && pmmpthread_store_valid_local_cache_item(cached)) {
			if (pmmpthread_store_storage_is_pmmpthread_obj(zstorage)) {
				pmmpthread_store_full_sync_local_properties(Z_OBJ_P(cached));
			}
			continue;
		}
		if (pmmpthread_store_storage_is_cacheable(zstorage)) {
			pmmpthread_store_restore_zval(&pzval, zstorage);

			if (pmmpthread_store_storage_is_pmmpthread_obj(zstorage)) {
				pmmpthread_store_full_sync_local_properties(Z_OBJ(pzval));
			}

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
		pmmpthread_worker_sync_collectable_tasks(threaded->worker_data);
	}

	GC_UNPROTECT_RECURSION(&threaded->std);
	pmmpthread_monitor_unlock(&threaded->ts_obj->monitor);
} /* }}} */

/* {{{ */
static inline void _pmmpthread_store_bump_modcount_nolock(pmmpthread_zend_object_t *threaded) {
	if (threaded->local_props_modcount == threaded->ts_obj->props.modcount) {
		/* It's possible we may have caused a modification via a connection whose property table was not in sync. This is OK
		 * for writes, because these cases usually cause destruction of outdated caches anyway, but we have to avoid
		 * incorrectly marking the local table as in-sync.
		 */
		threaded->local_props_modcount++;
	}
	threaded->ts_obj->props.modcount++;
} /* }}} */

static inline zend_bool pmmpthread_store_coerce(zval *key, zval *member) {
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
static inline zend_bool pmmpthread_store_member_is_cacheable(zend_object *object, zval *key) {
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	zval *zstorage;

	if (Z_TYPE_P(key) == IS_LONG) {
		zstorage = zend_hash_index_find(&threaded->ts_obj->props.hash, Z_LVAL_P(key));
	} else zstorage = zend_hash_find(&threaded->ts_obj->props.hash, Z_STR_P(key));

	return pmmpthread_store_storage_is_cacheable(zstorage);
} /* }}} */

/* {{{ */
int pmmpthread_store_delete(zend_object *object, zval *key) {
	int result = FAILURE;
	zval member;
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pmmpthread_store_coerce(key, &member);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zend_bool was_pmmpthread_object = pmmpthread_store_member_is_cacheable(object, &member);
		if (Z_TYPE(member) == IS_LONG) {
			result = zend_hash_index_del(&ts_obj->props.hash, Z_LVAL(member));
		} else result = zend_hash_del(&ts_obj->props.hash, Z_STR(member));

		if (result == SUCCESS && was_pmmpthread_object) {
			_pmmpthread_store_bump_modcount_nolock(threaded);
		}
		//TODO: it would be better if we can update this, if we deleted the first element
		pmmpthread_store_invalidate_bounds(&ts_obj->props);

		//TODO: sync local properties?
		pmmpthread_monitor_unlock(&ts_obj->monitor);
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
zend_bool pmmpthread_store_isset(zend_object *object, zval *key, int has_set_exists) {
	zend_bool isset = 0;
	zval member;
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pmmpthread_store_coerce(key, &member);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval *zstorage;

		if (Z_TYPE(member) == IS_LONG) {
			zstorage = zend_hash_index_find(&ts_obj->props.hash, Z_LVAL(member));
		} else zstorage = zend_hash_find(&ts_obj->props.hash, Z_STR(member));

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
						pmmpthread_storage* storage = TRY_PMMPTHREAD_STORAGE_PTR_P(zstorage);
						if (storage->type == STORE_TYPE_STRING_PTR) {
							pmmpthread_string_storage_t* string = (pmmpthread_string_storage_t*)storage;
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
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return isset;
} /* }}} */

static inline void pmmpthread_store_update_local_property(zend_object* object, zval* key, zval* value) {
	if (pmmpthread_store_retain_in_local_cache(value)) {
		pmmpthread_store_init_local_properties(object);
		if (Z_TYPE_P(key) == IS_LONG) {
			zend_hash_index_update(object->properties, Z_LVAL_P(key), value);
		} else {
			zend_string* str_key = Z_STR_P(key);
			if ((GC_FLAGS(str_key) & (IS_STR_PERSISTENT|IS_STR_INTERNED)) == IS_STR_PERSISTENT) {
				//refcounted persistent string from pmmpthread_store - we can't use it directly
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

static inline zend_bool pmmpthread_store_update_shared_property(pmmpthread_object_t* ts_obj, zval* key, zval* zstorage) {
	zend_bool result = FAILURE;
	if (Z_TYPE_P(key) == IS_LONG) {
		if (zend_hash_index_update(&ts_obj->props.hash, Z_LVAL_P(key), zstorage))
			result = SUCCESS;
	} else {
		zend_string* str_key = Z_STR_P(key);
		if (GC_FLAGS(str_key) & IS_STR_PERMANENT) {
			//only permanent strings can be used directly
			if (zend_hash_update(&ts_obj->props.hash, str_key, zstorage)) {
				result = SUCCESS;
			}
		} else {
			//refcounted or request-local interned string - this must be hard-copied, regardless of where it came from
			if (zend_hash_str_update(&ts_obj->props.hash, ZSTR_VAL(str_key), ZSTR_LEN(str_key), zstorage)) {
				result = SUCCESS;
			}
		}
	}

	return result;
}

/* {{{ */
int pmmpthread_store_read(zend_object *object, zval *key, int type, zval *read) {
	int result = FAILURE;
	zval member, *property = NULL;
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = pmmpthread_store_coerce(key, &member);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		if (threaded->std.properties) {
			pmmpthread_store_sync_local_properties(object);

			/* check if there's still a ref in local cache after sync - this ensures ref reuse for ThreadSafe and Closure objects */

			if (Z_TYPE(member) == IS_LONG) {
				property = zend_hash_index_find(threaded->std.properties, Z_LVAL(member));
			} else property = zend_hash_find(threaded->std.properties, Z_STR(member));

			if (property && pmmpthread_store_valid_local_cache_item(property)) {
				pmmpthread_monitor_unlock(&ts_obj->monitor);
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
			zstorage = zend_hash_index_find(&ts_obj->props.hash, Z_LVAL(member));
		} else zstorage = zend_hash_find(&ts_obj->props.hash, Z_STR(member));

		pmmpthread_storage *serialized = TRY_PMMPTHREAD_STORAGE_PTR_P(zstorage);
		/* strictly only reads are supported */
		if ((serialized == NULL || serialized->type != STORE_TYPE_THREADSAFE_OBJECT) && type != BP_VAR_R && type != BP_VAR_IS){
			zend_throw_error(zend_ce_error, "Indirect modification of non-ThreadSafe members of %s is not supported", ZSTR_VAL(object->ce->name));
			result = FAILURE;
		} else if (zstorage != NULL) {
			pmmpthread_store_restore_zval(read, zstorage);
			result = SUCCESS;
		}
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}

	if (result != SUCCESS) {
		ZVAL_UNDEF(read);
	} else {
		pmmpthread_store_update_local_property(&threaded->std, &member, read);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ Copies strings (as needed) for use in thread-safe object tables */
static zend_string* pmmpthread_store_save_string(zend_string* string) {
	zend_string* result;
	if (GC_FLAGS(string) & IS_STR_PERMANENT) { //interned by OPcache, or provided by builtin class
		result = string;
	}
	else {
		result = zend_string_init(ZSTR_VAL(string), ZSTR_LEN(string), 1);
	}

	return result;
} /* }}} */

static zend_string* pmmpthread_store_restore_string(zend_string* string) {
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
int pmmpthread_store_write(zend_object *object, zval *key, zval *write, zend_bool coerce_array_to_threaded) {
	int result = FAILURE;
	zval vol, member, zstorage;
	pmmpthread_zend_object_t *threaded =
		PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;
	zend_bool coerced = 0;

	if (Z_TYPE_P(write) == IS_ARRAY && coerce_array_to_threaded == PMMPTHREAD_STORE_COERCE_ARRAY) {
		/* coerce arrays into threaded objects */
		object_init_ex(
			&vol, pmmpthread_ce_array);
		pmmpthread_store_merge(Z_OBJ(vol), write, 1, PMMPTHREAD_STORE_COERCE_ARRAY);
		/* this will be addref'd when caching the object */
		Z_SET_REFCOUNT(vol, 0);
		write = &vol;
	}

	if (pmmpthread_store_save_zval(&threaded->owner, &zstorage, write) != SUCCESS) {
		return FAILURE;
	}

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		if (!key) {
			zend_ulong next = zend_hash_next_free_element(&ts_obj->props.hash);
			if (next == ZEND_LONG_MIN) next = 0;
			ZVAL_LONG(&member, next);
		} else {
			coerced = pmmpthread_store_coerce(key, &member);
		}

		zend_bool was_pmmpthread_object = pmmpthread_store_member_is_cacheable(object, &member);
		result = pmmpthread_store_update_shared_property(ts_obj, &member, &zstorage);
		if (result == SUCCESS && was_pmmpthread_object) {
			_pmmpthread_store_bump_modcount_nolock(threaded);
		}
		if (ts_obj->props.first != HT_INVALID_IDX && ts_obj->props.first != 0) {
			HashPosition start = 0;
			if (zend_hash_get_current_data_ex(&ts_obj->props.hash, &start) != NULL) {
				//a table rehash may have occurred, moving all elements to the start of the table
				//this is usually because the table size was increased to accommodate the new element
				pmmpthread_store_invalidate_bounds(&ts_obj->props);
			}
		}
		if (key) {
			//only invalidate position if an arbitrary key was used
			//if the item was appended, the first element was either unchanged or the position was invalid anyway
			pmmpthread_store_invalidate_bounds(&ts_obj->props);
		} else if (ts_obj->props.last != HT_INVALID_IDX) {
			//if we appended, the last element is now the new item
			if (zend_hash_move_forward_ex(&ts_obj->props.hash, &ts_obj->props.last) == FAILURE) {
				ZEND_ASSERT(0);
			}
		}
		//this isn't necessary for any specific property write, but since we don't have any other way to clean up local
		//cached ThreadSafe references that are dead, we have to take the opportunity
		pmmpthread_store_sync_local_properties(object);

		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}

	if (result != SUCCESS) {
		pmmpthread_store_storage_dtor(&zstorage);
	} else {
		pmmpthread_store_update_local_property(&threaded->std, &member, write);
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
} /* }}} */

/* {{{ */
int pmmpthread_store_count(zend_object *object, zend_long *count) {
	pmmpthread_object_t* ts_obj = PMMPTHREAD_FETCH_TS_FROM(object);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		(*count) = zend_hash_num_elements(&ts_obj->props.hash);
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	} else (*count) = 0L;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pmmpthread_store_shift(zend_object *object, zval *member) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval key;
		zval *zstorage;
		HashPosition position = ts_obj->props.first;

		if (position == HT_INVALID_IDX) {
			zend_hash_internal_pointer_reset_ex(&ts_obj->props.hash, &position);
		}
		if ((zstorage = zend_hash_get_current_data_ex(&ts_obj->props.hash, &position))) {
			zend_hash_get_current_key_zval_ex(&ts_obj->props.hash, &key, &position);

			if (zend_hash_num_elements(&ts_obj->props.hash) == 1) { //we're about to delete the last element
				pmmpthread_store_invalidate_bounds(&ts_obj->props);
			} else {
				zend_hash_move_forward_ex(&ts_obj->props.hash, &position);
				ts_obj->props.first = position;
			}

			zend_bool may_be_locally_cached;

			pmmpthread_store_restore_zval_ex(member, zstorage, &may_be_locally_cached);
			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_del(&ts_obj->props.hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				zend_hash_del(&ts_obj->props.hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}

			if (may_be_locally_cached) {
				_pmmpthread_store_bump_modcount_nolock(threaded);
			}
			//TODO: maybe we should be syncing local properties here?
		} else ZVAL_NULL(member);
		pmmpthread_monitor_unlock(&ts_obj->monitor);

		return SUCCESS;
	}

	return FAILURE;
} /* }}} */

/* {{{ */
int pmmpthread_store_chunk(zend_object *object, zend_long size, zend_bool preserve, zval *chunk) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval *zstorage;

		HashPosition position = ts_obj->props.first;
		if (position == HT_INVALID_IDX) {
			zend_hash_internal_pointer_reset_ex(&ts_obj->props.hash, &position);
		}
		array_init(chunk);
		zend_bool stale_local_cache = 0;
		while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
			zend_hash_num_elements(&ts_obj->props.hash) > 0) {
			zstorage = zend_hash_get_current_data_ex(&ts_obj->props.hash, &position);
			ZEND_ASSERT(zstorage != NULL);

			zval key, zv;

			zend_hash_get_current_key_zval_ex(&ts_obj->props.hash, &key, &position);

			if (zend_hash_num_elements(&ts_obj->props.hash) == 1) { //we're about to delete the last element
				pmmpthread_store_invalidate_bounds(&ts_obj->props);
			} else {
				zend_hash_move_forward_ex(&ts_obj->props.hash, &position);
				ts_obj->props.first = position;
			}

			zend_bool may_be_locally_cached;
			pmmpthread_store_restore_zval_ex(&zv, zstorage, &may_be_locally_cached);
			if (!stale_local_cache) {
				stale_local_cache = may_be_locally_cached;
			}
			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_update(
					Z_ARRVAL_P(chunk), Z_LVAL(key), &zv);
				zend_hash_index_del(&ts_obj->props.hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
				zend_hash_str_update(
					Z_ARRVAL_P(chunk), Z_STRVAL(key), Z_STRLEN(key), &zv);
				zend_hash_del(&ts_obj->props.hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}
		}
		if (stale_local_cache) {
			_pmmpthread_store_bump_modcount_nolock(threaded);
		}
		//TODO: sync local properties?
		pmmpthread_monitor_unlock(&ts_obj->monitor);

		return SUCCESS;
	}

	return FAILURE;
} /* }}} */

/* {{{ */
int pmmpthread_store_pop(zend_object *object, zval *member) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval key;
		zval *zstorage;
		HashPosition position = ts_obj->props.last;

		if (position == HT_INVALID_IDX) {
			zend_hash_internal_pointer_end_ex(&ts_obj->props.hash, &position);
		}
		if ((zstorage = zend_hash_get_current_data_ex(&ts_obj->props.hash, &position))) {
			zend_hash_get_current_key_zval_ex(&ts_obj->props.hash, &key, &position);

			if (zend_hash_num_elements(&ts_obj->props.hash) == 1) { //we're about to delete the last element
				pmmpthread_store_invalidate_bounds(&ts_obj->props);
			} else {
				zend_hash_move_backwards_ex(&ts_obj->props.hash, &position);
				ts_obj->props.last = position;
			}

			zend_bool may_be_locally_cached;
			pmmpthread_store_restore_zval_ex(member, zstorage, &may_be_locally_cached);

			if (Z_TYPE(key) == IS_LONG) {
				zend_hash_index_del(
					&ts_obj->props.hash, Z_LVAL(key));
				if (threaded->std.properties) {
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				}
			} else {
				zend_hash_del(
					&ts_obj->props.hash, Z_STR(key));
				if (threaded->std.properties) {
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
				zend_string_release(Z_STR(key));
			}
			if (may_be_locally_cached) {
				_pmmpthread_store_bump_modcount_nolock(threaded);
			}
			//TODO: sync local properties?
		} else ZVAL_NULL(member);

		pmmpthread_monitor_unlock(&ts_obj->monitor);

		return SUCCESS;
	}

   return FAILURE;
} /* }}} */

/* {{{ */
void pmmpthread_store_tohash(zend_object *object, HashTable *hash) {
	pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t *ts_obj = threaded->ts_obj;

	pmmpthread_store_init_local_properties(&threaded->std);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zend_string *name = NULL;
		zend_ulong idx;
		zval *zstorage;
		zend_bool changed = 0;

		pmmpthread_store_sync_local_properties(object);


		ZEND_HASH_FOREACH_KEY_VAL(&ts_obj->props.hash, idx, name, zstorage) {
			zval *cached;
			zval pzval;

			//we just synced local cache, so if something is already here, it doesn't need to be modified
			if (hash == threaded->std.properties) {
				if (!name) {
					cached = zend_hash_index_find(threaded->std.properties, idx);
				} else {
					cached = zend_hash_find(threaded->std.properties, name);
				}
				if (cached && pmmpthread_store_valid_local_cache_item(cached)) {
					continue;
				}
			} else {
				if (!name) {
					cached = zend_hash_index_find(threaded->std.properties, idx);
					if (cached && pmmpthread_store_valid_local_cache_item(cached)) {
						zend_hash_index_update(hash, idx, cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				} else {
					cached = zend_hash_find(threaded->std.properties, name);
					if (cached && pmmpthread_store_valid_local_cache_item(cached)) {
						/* we can't use zend_hash_update() here - the string from store.props must not be returned to user code */
						zend_hash_str_update(hash, ZSTR_VAL(name), ZSTR_LEN(name), cached);
						Z_TRY_ADDREF_P(cached);
						continue;
					}
				}
			}

			pmmpthread_store_restore_zval(&pzval, zstorage);

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
			threaded->local_props_modcount = ts_obj->props.modcount - 1;
		}

		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
} /* }}} */

/* {{{ */
void pmmpthread_store_persist_local_properties(zend_object* object) {
	pmmpthread_zend_object_t* threaded = PMMPTHREAD_FETCH_FROM(object);
	pmmpthread_object_t* ts_obj = threaded->ts_obj;

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval *zstorage;
		ZEND_HASH_FOREACH_VAL(&ts_obj->props.hash, zstorage) {
			pmmpthread_storage* storage = TRY_PMMPTHREAD_STORAGE_PTR_P(zstorage);

			if (storage != NULL && storage->type == STORE_TYPE_STRING_PTR) {
				pmmpthread_string_storage_t* string = (pmmpthread_string_storage_t*)storage;
				if (string->owner.ls == TSRMLS_CACHE) {
					//we can't guarantee this string will continue to be available once we stop referencing it on this thread,
					//so we must create a persistent copy now

					zend_string* persistent_string = pmmpthread_store_save_string(string->string);
					pmmpthread_store_storage_dtor(zstorage);

					ZVAL_STR(zstorage, persistent_string);
				}
			}
		} ZEND_HASH_FOREACH_END();

		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
} /* }}} */

static zend_bool pmmpthread_closure_thread_safe(zend_closure* closure) {
	if (
		!Z_ISUNDEF(closure->this_ptr) &&
		!(Z_TYPE(closure->this_ptr) == IS_OBJECT && instanceof_function(Z_OBJCE(closure->this_ptr), pmmpthread_ce_thread_safe))
	) {
		//closures must be unbound or static when assigned, because they won't be bound when restored onto another thread
		//however, this is OK for thread-safe objects which we can copy
		zend_throw_error(
			pmmpthread_ce_nts_value_error,
			"Closures with non-thread-safe $this cannot be made thread-safe"
		);
		return 0;
	}

	zend_function* func = &closure->func;
	if (
		func->common.type == ZEND_USER_FUNCTION &&
		!(func->common.fn_flags & ZEND_ACC_FAKE_CLOSURE)
	) {
		HashTable* static_variables = ZEND_MAP_PTR_GET(func->op_array.static_variables_ptr);
		if (static_variables != NULL) {
			//TODO: we need to check for non-thread-safe objects and other variables here

			//copied closures must not have static variables or use-by-ref - their state may change in ways that
			//we can't control, potentially making them unsafe after assignment (e.g. a static variable assigned
			//an object)
			//fake closures (first-class callables) are allowed to have statics, since they directly refer to the
			//statics of the original function, so we can just treat them as if they are a classic callable.
			zend_op* opline = func->op_array.opcodes;
			zend_op* last = func->op_array.opcodes + func->op_array.last;
			for (; opline < last; opline++) {
				if (opline->opcode == ZEND_BIND_STATIC && opline->extended_value & ZEND_BIND_REF) {
					zend_throw_error(
						pmmpthread_ce_nts_value_error,
						"Closures with local static variables or use-by-reference cannot be made thread-safe"
					);
					return 0;
				}
			}
		}
	}

	return 1;

}
/* {{{ */
static pmmpthread_storage* pmmpthread_store_create(pmmpthread_ident_t* source, zval *unstore){
	pmmpthread_storage *result = NULL;

#define MAKE_STORAGE(enum_type, struct_type) \
	struct_type *storage = malloc(sizeof(struct_type)); \
	if (storage == NULL) { \
		break; \
	} \
	storage->common.type = enum_type; \
	result = (pmmpthread_storage*) storage; \

	switch(Z_TYPE_P(unstore)){
		case IS_STRING: {
			MAKE_STORAGE(STORE_TYPE_STRING_PTR, pmmpthread_string_storage_t);
			storage->owner = *source;
			storage->string = Z_STR_P(unstore);
		} break;

		case IS_OBJECT:
			if (Z_OBJCE_P(unstore)->ce_flags & ZEND_ACC_ENUM) {
				MAKE_STORAGE(STORE_TYPE_ENUM, pmmpthread_enum_storage_t);
				zval* zname = zend_enum_fetch_case_name(Z_OBJ_P(unstore));

				storage->class_name = pmmpthread_store_save_string(Z_OBJCE_P(unstore)->name);
				storage->member_name = pmmpthread_store_save_string(Z_STR_P(zname));
				break;
			}

			if (instanceof_function(Z_OBJCE_P(unstore), zend_ce_closure)) {
				zend_closure* closure = (zend_closure*)Z_OBJ_P(unstore);

				if (!pmmpthread_closure_thread_safe(closure)) {
					break;
				}

				MAKE_STORAGE(STORE_TYPE_CLOSURE, pmmpthread_closure_storage_t);
				//TODO: this might result in faults because it's not copied properly
				//since we aren't copying this to persistent memory, a fault is going to
				//happen if it's dereferenced after the original closure is destroyed
				//(for what it's worth, this was always a problem.)
				if (Z_TYPE(closure->this_ptr) == IS_OBJECT) {
					pmmpthread_zend_object_t* this_object = PMMPTHREAD_FETCH_FROM(Z_OBJ(closure->this_ptr));
					storage->this_obj = this_object;
				} else {
					storage->this_obj = NULL;
				}
				storage->closure = closure;
				storage->owner = *source;
				break;
			}

			if (instanceof_function(Z_OBJCE_P(unstore), pmmpthread_ce_thread_safe)) {
				pmmpthread_zend_object_t *threaded = PMMPTHREAD_FETCH_FROM(Z_OBJ_P(unstore));
				MAKE_STORAGE(STORE_TYPE_THREADSAFE_OBJECT, pmmpthread_zend_object_storage_t);
				storage->object = threaded;
				break;
			}

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
			if (instanceof_function(Z_OBJCE_P(unstore), socket_ce)) {
				php_socket *socket = Z_SOCKET_P(unstore);
				if (!Z_ISUNDEF(socket->zstream)) {
					break; // we can't handle these; resources can't be shared safely
				}
				if (!IS_INVALID_SOCKET(socket)) {
					//we still copy invalid sockets, for consistency with normal objects
					pmmpthread_globals_shared_socket_track(socket->bsd_socket);
				}

				MAKE_STORAGE(STORE_TYPE_SOCKET, pmmpthread_socket_storage_t);
				storage->bsd_socket = socket->bsd_socket;
				storage->type = socket->type;
				storage->error = socket->error;
				storage->blocking = socket->blocking;
				break;
			}
#endif

			/* fallthru to default, non-threaded, non-closure objects cannot be stored */
		default:
			break;
	}
#undef MAKE_STORAGE
	return result;
}
/* }}} */

/* {{{ */
static zend_result pmmpthread_store_save_zval(pmmpthread_ident_t* source, zval *zstorage, zval *write) {
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
			pmmpthread_storage *storage = pmmpthread_store_create(source, write);
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
static int pmmpthread_store_convert(pmmpthread_storage *storage, zval *pzval){
	int result = SUCCESS;

	switch(storage->type) {
		case STORE_TYPE_STRING_PTR: {
			pmmpthread_string_storage_t* string = (pmmpthread_string_storage_t*)storage;

			if (string->owner.ls == TSRMLS_CACHE) {
				//this thread owns the string - we can use it directly
				ZVAL_STR_COPY(pzval, string->string);
			} else {
				//this thread does not own the string - create a copy
				ZVAL_STR(pzval, pmmpthread_store_restore_string(string->string));
			}
		} break;

		case STORE_TYPE_CLOSURE: {
			const pmmpthread_closure_storage_t* closure_data = (const pmmpthread_closure_storage_t* )storage;
			result = pmmpthread_copy_closure(&closure_data->owner, closure_data->closure, 0, pzval);
		} break;

		case STORE_TYPE_THREADSAFE_OBJECT: {
			pmmpthread_zend_object_t* threaded = ((pmmpthread_zend_object_storage_t*)storage)->object;

			if (!pmmpthread_object_connect(threaded, pzval)) {
				zend_throw_exception_ex(
					pmmpthread_ce_connection_exception, 0,
					"pmmpthread detected an attempt to connect to an object which has already been destroyed");
				result = FAILURE;
			}
		} break;

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
		case STORE_TYPE_SOCKET: {
			pmmpthread_socket_storage_t *stored_socket = (pmmpthread_socket_storage_t*)storage;

			object_init_ex(pzval, socket_ce);
			php_socket *socket = Z_SOCKET_P(pzval);
			socket->bsd_socket = stored_socket->bsd_socket;
			socket->type = stored_socket->type;
			socket->error = stored_socket->error;
			socket->blocking = stored_socket->blocking;
			result = SUCCESS;
		} break;
#endif
		case STORE_TYPE_ENUM: {
			pmmpthread_enum_storage_t* enum_data = (pmmpthread_enum_storage_t*)storage;

			zend_class_entry* enum_ce = zend_lookup_class(enum_data->class_name);

			result = pmmpthread_resolve_enum_reference(
				enum_ce,
				enum_data->member_name,
				pzval
			);

			break;
		}
		default: ZEND_ASSERT(0);
	}

	if (result == FAILURE) {
		ZVAL_NULL(pzval);
	}

	return result;
}
/* }}} */

/* {{{ */
static void pmmpthread_store_restore_zval_ex(zval *unstore, zval *zstorage, zend_bool *may_be_locally_cached) {
	*may_be_locally_cached = pmmpthread_store_storage_is_cacheable(zstorage);
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
			ZVAL_STR(unstore, pmmpthread_store_restore_string(Z_STR_P(zstorage)));
			break;
		case IS_PTR:
			{
				/* thread-safe object */
				pmmpthread_storage *storage = (pmmpthread_storage *) Z_PTR_P(zstorage);
				pmmpthread_store_convert(storage, unstore);
			}
			break;
		default:
			ZEND_ASSERT(0);
	}
} /* }}} */

/* {{{ */
static void pmmpthread_store_restore_zval(zval *unstore, zval *zstorage) {
	zend_bool dummy;
	pmmpthread_store_restore_zval_ex(unstore, zstorage, &dummy);
} /* }}} */

/* {{{ */
static void pmmpthread_store_hard_copy_storage(zval *new_zstorage, zval *zstorage) {
	if (Z_TYPE_P(zstorage) == IS_PTR) {
		pmmpthread_storage *storage = (pmmpthread_storage *) Z_PTR_P(zstorage);
		if (storage->type == STORE_TYPE_STRING_PTR) {
			//hard-copy string ptrs here, since the destination object might not exist on the thread which owns the string
			//this means that the owning thread may not be aware that this new ref now exists and won't persist the string when it dies
			pmmpthread_string_storage_t* string = (pmmpthread_string_storage_t*)storage;
			ZVAL_STR(new_zstorage, pmmpthread_store_save_string(string->string));

		} else {

#define CASE_STORAGE(enum_type, struct_type) \
				case enum_type: { \
					struct_type* copy = malloc(sizeof(struct_type)); \
					memcpy(copy, storage, sizeof(struct_type)); \
					ZVAL_PTR(new_zstorage, copy); \
				} break;

			switch (storage->type) {
				CASE_STORAGE(STORE_TYPE_CLOSURE, pmmpthread_closure_storage_t);
				CASE_STORAGE(STORE_TYPE_THREADSAFE_OBJECT, pmmpthread_zend_object_storage_t);
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
				CASE_STORAGE(STORE_TYPE_SOCKET, pmmpthread_socket_storage_t);
#endif
				CASE_STORAGE(STORE_TYPE_ENUM, pmmpthread_enum_storage_t); //TODO: we need to copy or intern these strings, otherwise UAF will occur
				default: {
					ZEND_ASSERT(0);
					ZVAL_UNDEF(new_zstorage);
				} break;
			}
		}
#undef CASE_STORAGE
	} else if (Z_TYPE_P(zstorage) == IS_STRING) {
		ZVAL_STR(new_zstorage, pmmpthread_store_save_string(Z_STR_P(zstorage)));
	} else {
		ZEND_ASSERT(!Z_REFCOUNTED_P(zstorage));
		ZVAL_COPY(new_zstorage, zstorage);
	}
} /* }}} */

/* {{{ */
int pmmpthread_store_merge(zend_object *destination, zval *from, zend_bool overwrite, zend_bool coerce_array_to_threaded) {
	if (Z_TYPE_P(from) != IS_ARRAY &&
		Z_TYPE_P(from) != IS_OBJECT) {
		return FAILURE;
	}

	switch (Z_TYPE_P(from)) {
		case IS_OBJECT: {
			if (instanceof_function(Z_OBJCE_P(from), pmmpthread_ce_array)) {
				pmmpthread_object_t* threaded[2] = {PMMPTHREAD_FETCH_TS_FROM(destination), PMMPTHREAD_FETCH_TS_FROM(Z_OBJ_P(from))};

				if (pmmpthread_monitor_lock(&threaded[0]->monitor)) {
					if (pmmpthread_monitor_lock(&threaded[1]->monitor)) {
						HashPosition position;
						zval *storage;
						HashTable *tables[2] = {&threaded[0]->props.hash, &threaded[1]->props.hash};
						zval key;
						zend_bool overwrote_pmmpthread_object = 0;

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

							if (!overwrote_pmmpthread_object) {
								overwrote_pmmpthread_object = pmmpthread_store_member_is_cacheable(destination, &key);
							}

							zval new_zstorage;
							pmmpthread_store_hard_copy_storage(&new_zstorage, storage);
							pmmpthread_store_update_shared_property(threaded[0], &key, &new_zstorage);
							if (destination->properties != NULL) {
								if (Z_TYPE(key) == IS_LONG) {
									zend_hash_index_del(destination->properties, Z_LVAL(key));
								} else {
									zend_hash_del(destination->properties, Z_STR(key));
								}
							}
						}
						if (overwrote_pmmpthread_object) {
							_pmmpthread_store_bump_modcount_nolock(PMMPTHREAD_FETCH_FROM(destination));
						}
						pmmpthread_store_invalidate_bounds(&threaded[0]->props);

						//TODO: sync local properties?

						pmmpthread_monitor_unlock(&threaded[1]->monitor);
					}

					pmmpthread_monitor_unlock(&threaded[0]->monitor);

					return SUCCESS;

				} else return FAILURE;
			}
		}

		/* fall through on purpose to handle normal objects and arrays */

		default: {
			pmmpthread_object_t* ts_obj = PMMPTHREAD_FETCH_TS_FROM(destination);

			if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
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

					while (Z_TYPE_P(pzval) == IS_INDIRECT) {
						pzval = Z_INDIRECT_P(pzval);
					}
					switch (Z_TYPE(key)) {
						case IS_LONG:
							if (!overwrite && zend_hash_index_exists(&ts_obj->props.hash, Z_LVAL(key))) {
								goto next;
							}
							if (pmmpthread_store_write(destination, &key, pzval, coerce_array_to_threaded) == FAILURE) {
								zend_throw_error(
									pmmpthread_ce_nts_value_error,
									"Cannot merge non-thread-safe value of type %s (input key %zd) into %s",
									zend_zval_type_name(pzval),
									Z_LVAL(key),
									ZSTR_VAL(destination->ce->name)
								);
							}
						break;

						case IS_STRING:
							if (!overwrite && zend_hash_exists(&ts_obj->props.hash, Z_STR(key))) {
								goto next;
							}
							if (pmmpthread_store_write(destination, &key, pzval, coerce_array_to_threaded) == FAILURE) {
								zend_throw_error(
									pmmpthread_ce_nts_value_error,
									"Cannot merge non-thread-safe value of type %s (input key \"%s\") into %s",
									zend_zval_type_name(pzval),
									Z_STRVAL(key),
									ZSTR_VAL(destination->ce->name)
								);
							}
						break;
					}

next:
					index++;
					if (EG(exception)) {
						break;
					}
				}

				pmmpthread_monitor_unlock(&ts_obj->monitor);
			}
		} break;
	}

	return EG(exception) ? FAILURE : SUCCESS;
} /* }}} */


/* {{{ Will free store element */
static void pmmpthread_store_storage_dtor (zval *zstorage){
	if (!zstorage) return;

	if (Z_TYPE_P(zstorage) == IS_PTR) {
		pmmpthread_storage *storage = (pmmpthread_storage *) Z_PTR_P(zstorage);
		switch (storage->type) {
			case STORE_TYPE_CLOSURE:
			case STORE_TYPE_THREADSAFE_OBJECT:
			case STORE_TYPE_SOCKET:
			case STORE_TYPE_STRING_PTR:
				/* no extra action necessary */
				break;
			case STORE_TYPE_ENUM: {
				pmmpthread_enum_storage_t* enum_storage = (pmmpthread_enum_storage_t*)storage;
				zend_string_free(enum_storage->class_name);
				zend_string_free(enum_storage->member_name);
			} break;
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
void pmmpthread_store_reset(zend_object *object, HashPosition *position) {
	pmmpthread_object_t *ts_obj = PMMPTHREAD_FETCH_TS_FROM(object);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		if (ts_obj->props.first == HT_INVALID_IDX) {
			zend_hash_internal_pointer_reset_ex(&ts_obj->props.hash, position);
			if (zend_hash_has_more_elements_ex(&ts_obj->props.hash, position) == FAILURE) { //empty
				*position = HT_INVALID_IDX;
			} else {
				ts_obj->props.first = *position;
			}
		} else {
			*position = ts_obj->props.first;
		}
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
}

void pmmpthread_store_key(zend_object *object, zval *key, HashPosition *position) {
	pmmpthread_object_t *ts_obj = PMMPTHREAD_FETCH_TS_FROM(object);
	zend_string *str_key;
	zend_ulong num_key;

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		switch (zend_hash_get_current_key_ex(&ts_obj->props.hash, &str_key, &num_key, position)) {
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
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
}

void pmmpthread_store_data(zend_object *object, zval *value, HashPosition *position) {
	pmmpthread_object_t *ts_obj = PMMPTHREAD_FETCH_TS_FROM(object);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zval key;
		zend_hash_get_current_key_zval_ex(&ts_obj->props.hash, &key, position);

		if (pmmpthread_store_read(object, &key, BP_VAR_R, value) == FAILURE) {
			ZVAL_UNDEF(value);
		}
		if (Z_TYPE(key) == IS_STRING) {
			zend_string_release(Z_STR(key));
		}

		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
}

void pmmpthread_store_forward(zend_object *object, HashPosition *position) {
	pmmpthread_object_t *ts_obj = PMMPTHREAD_FETCH_TS_FROM(object);

	if (pmmpthread_monitor_lock(&ts_obj->monitor)) {
		zend_hash_move_forward_ex(
			&ts_obj->props.hash, position);
		if (zend_hash_has_more_elements_ex(&ts_obj->props.hash, position) == FAILURE) {
			*position = HT_INVALID_IDX;
		}
		pmmpthread_monitor_unlock(&ts_obj->monitor);
	}
} /* }}} */
