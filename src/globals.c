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

#include <src/globals.h>
#include <src/prepare.h>

struct _pmmpthread_globals pmmpthread_globals;

#ifndef PMMPTHREAD_G
#	define PMMPTHREAD_G () ?  : (void***) &pmmpthread_globals
#endif

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
static void pmmpthread_globals_shared_sockets_dtor_func(zval *pDest) {
	close(Z_LVAL_P(pDest));
}
#endif

static void pmmpthread_globals_string_dtor_func(zval* pDest) {
	free(Z_STR_P(pDest));
}

zend_string* pmmpthread_globals_find_interned_string(zend_string* string) {
	if (GC_FLAGS(string) & IS_STR_PERMANENT) {
		//permanent strings should always safe to share
		return string;
	}

	zend_string_hash_val(string); //interned strings must always have their hash values known

	//try zend's table first
	zend_string* result = zend_interned_string_find_permanent(string);
	if (result != NULL) {
		return result;
	}

	if (pmmpthread_globals_lock()) {
		zval *zv = zend_hash_find(&PMMPTHREAD_G(interned_strings), string);
		if (zv != NULL) {
			result = Z_STR_P(zv);
		}

		pmmpthread_globals_unlock();
	}

	return result;
}

zend_string* pmmpthread_globals_add_interned_string(zend_string* string) {
	if (GC_FLAGS(string) & IS_STR_PERMANENT) {
		//permanent strings should always safe to share
		return string;
	}

	zend_string* result = NULL;

	zend_string_hash_val(string); //interned strings must always have their hash values known
	if (pmmpthread_globals_lock()) {
		result = pmmpthread_globals_find_interned_string(string);
		if (result == NULL) {
			zval value;

			result = zend_string_init(ZSTR_VAL(string), ZSTR_LEN(string), 1);
			GC_ADD_FLAGS(result, IS_STR_INTERNED | IS_STR_PERMANENT);
			GC_SET_REFCOUNT(result, 1);
			ZSTR_H(result) = ZSTR_H(string);

			ZVAL_INTERNED_STR(&value, result);

			zend_hash_add_new(&PMMPTHREAD_G(interned_strings), result, &value);
		}

		pmmpthread_globals_unlock();
	}

	return result;
}

/* {{{ */
zend_bool pmmpthread_globals_init(){
	if (!PMMPTHREAD_G(init)&&!PMMPTHREAD_G(failed)) {
		PMMPTHREAD_G(init)=1;
		if (pmmpthread_monitor_init(&PMMPTHREAD_G(monitor)) == FAILURE)
			PMMPTHREAD_G(failed)=1;
		if (PMMPTHREAD_G(failed)) {
			PMMPTHREAD_G(init)=0;
		} else {
			zend_hash_init(
				&PMMPTHREAD_G(objects), 64, NULL, (dtor_func_t) NULL, 1);
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
			zend_hash_init(
				&PMMPTHREAD_G(shared_sockets), 16, NULL, (dtor_func_t) pmmpthread_globals_shared_sockets_dtor_func, 1);
#endif
			zend_hash_init(
				&PMMPTHREAD_G(interned_strings),
				1024,
				NULL,
				(dtor_func_t)pmmpthread_globals_string_dtor_func,
				1
			);
			ZVAL_UNDEF(&PMMPTHREAD_G(undef_zval));

		}

#define INIT_STRING(n, v) do { \
	PMMPTHREAD_G(strings).n = zend_new_interned_string(zend_string_init(v, 1)); \
} while(0)

		INIT_STRING(run, ZEND_STRL("run"));
		INIT_STRING(session.cache_limiter, ZEND_STRL("cache_limiter"));
		INIT_STRING(session.use_cookies, ZEND_STRL("use_cookies"));
#undef INIT_STRING

		return PMMPTHREAD_G(init);
	} else return 0;
} /* }}} */

/* {{{ */
zend_bool pmmpthread_globals_lock(){
	return pmmpthread_monitor_lock(&PMMPTHREAD_G(monitor));
} /* }}} */

/* {{{ */
void pmmpthread_globals_unlock() {
	pmmpthread_monitor_unlock(&PMMPTHREAD_G(monitor));
} /* }}} */

/* {{{ */
pmmpthread_zend_object_t* pmmpthread_globals_object_alloc(size_t length) {
	pmmpthread_zend_object_t *bucket = (pmmpthread_zend_object_t*) ecalloc(1, length);

	if (pmmpthread_globals_lock()) {
		zend_hash_index_update_ptr(
			&PMMPTHREAD_G(objects),
			(zend_ulong) bucket, bucket);
		pmmpthread_globals_unlock();
	}

	memset(bucket, 0, length);

	return bucket;
} /* }}} */

/* {{{ */
zend_bool pmmpthread_globals_object_valid(pmmpthread_zend_object_t *address) {
	zend_bool valid = 0;

	if (!address)
		return valid;

	if (pmmpthread_globals_lock()) {
		if (zend_hash_index_exists(&PMMPTHREAD_G(objects), (zend_ulong) address)) {
			valid = 1;
		}
		pmmpthread_globals_unlock();
	}

	return valid;
} /* }}} */

/* {{{ */
zend_bool pmmpthread_globals_object_delete(pmmpthread_zend_object_t *address) {
	zend_bool deleted = 0;

	if (!address)
		return deleted;

	if (pmmpthread_globals_lock()) {
		deleted = zend_hash_index_del(
			&PMMPTHREAD_G(objects), (zend_ulong) address);
		pmmpthread_globals_unlock();
	}

	return deleted;
} /* }}} */

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
void pmmpthread_globals_shared_socket_track(PHP_SOCKET socket) {
	if (socket < 0) {
		return;
	}

	if (pmmpthread_globals_lock()) {
		zval value;

		ZVAL_LONG(&value, (zend_long) socket);
		zend_hash_index_add(&PMMPTHREAD_G(shared_sockets), (zend_ulong) socket, &value);

		pmmpthread_globals_unlock();
	}
}

zend_bool pmmpthread_globals_socket_shared(PHP_SOCKET socket) {
	zend_bool result = 0;
	if (socket < 0) {
		return result;
	}

	if (pmmpthread_globals_lock()) {
		result = zend_hash_index_find(&PMMPTHREAD_G(shared_sockets), (zend_ulong) socket) != NULL;
	
		pmmpthread_globals_unlock();
	}

	return result;
}
#endif

/* {{{ */
void pmmpthread_globals_shutdown() {
	if (PMMPTHREAD_G(init)) {
		PMMPTHREAD_G(init)=0;
		PMMPTHREAD_G(failed)=0;
		/* we allow proc shutdown to destroy tables, and global strings */
		pmmpthread_monitor_destroy(&PMMPTHREAD_G(monitor));
		zend_hash_destroy(&PMMPTHREAD_G(objects));
#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
		zend_hash_destroy(&PMMPTHREAD_G(shared_sockets));
#endif
		zend_hash_destroy(&PMMPTHREAD_G(interned_strings));
	}
} /* }}} */
