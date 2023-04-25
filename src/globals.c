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

#include <src/globals.h>
#include <src/prepare.h>

struct _pthreads_globals pthreads_globals;

#ifndef PTHREADS_G
#	define PTHREADS_G () ?  : (void***) &pthreads_globals
#endif

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
static void pthreads_globals_shared_sockets_dtor_func(zval *pDest) {
	close(Z_LVAL_P(pDest));
}
#endif

static void pthreads_globals_string_dtor_func(zval* pDest) {
	free(Z_STR_P(pDest));
}

zend_string* pthreads_globals_find_interned_string(zend_string* string) {
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

	if (pthreads_globals_lock()) {
		zval *zv = zend_hash_find(&PTHREADS_G(interned_strings), string);
		if (zv != NULL) {
			result = Z_STR_P(zv);
		}

		pthreads_globals_unlock();
	}

	return result;
}

zend_string* pthreads_globals_add_interned_string(zend_string* string) {
	if (GC_FLAGS(string) & IS_STR_PERMANENT) {
		//permanent strings should always safe to share
		return string;
	}

	zend_string* result = NULL;

	zend_string_hash_val(string); //interned strings must always have their hash values known
	if (pthreads_globals_lock()) {
		result = pthreads_globals_find_interned_string(string);
		if (result == NULL) {
			zval value;

			result = zend_string_init(ZSTR_VAL(string), ZSTR_LEN(string), 1);
			GC_ADD_FLAGS(result, IS_STR_INTERNED | IS_STR_PERMANENT);
			GC_SET_REFCOUNT(result, 1);
			ZSTR_H(result) = ZSTR_H(string);

			ZVAL_INTERNED_STR(&value, result);

			zend_hash_add_new(&PTHREADS_G(interned_strings), result, &value);
		}

		pthreads_globals_unlock();
	}

	return result;
}

/* {{{ */
zend_bool pthreads_globals_init(){
	if (!PTHREADS_G(init)&&!PTHREADS_G(failed)) {
		PTHREADS_G(init)=1;
		if (pthreads_monitor_init(&PTHREADS_G(monitor)) == FAILURE)
			PTHREADS_G(failed)=1;
		if (PTHREADS_G(failed)) {
			PTHREADS_G(init)=0;
		} else {
			zend_hash_init(
				&PTHREADS_G(objects), 64, NULL, (dtor_func_t) NULL, 1);
#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
			zend_hash_init(
				&PTHREADS_G(shared_sockets), 16, NULL, (dtor_func_t) pthreads_globals_shared_sockets_dtor_func, 1);
#endif
			zend_hash_init(
				&PTHREADS_G(interned_strings),
				1024,
				NULL,
				(dtor_func_t)pthreads_globals_string_dtor_func,
				1
			);
			ZVAL_UNDEF(&PTHREADS_G(undef_zval));

		}

#define INIT_STRING(n, v) do { \
	PTHREADS_G(strings).n = zend_new_interned_string(zend_string_init(v, 1)); \
} while(0)

		INIT_STRING(run, ZEND_STRL("run"));
		INIT_STRING(session.cache_limiter, ZEND_STRL("cache_limiter"));
		INIT_STRING(session.use_cookies, ZEND_STRL("use_cookies"));
#undef INIT_STRING

		return PTHREADS_G(init);
	} else return 0;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_lock(){
	return pthreads_monitor_lock(&PTHREADS_G(monitor));
} /* }}} */

/* {{{ */
void pthreads_globals_unlock() {
	pthreads_monitor_unlock(&PTHREADS_G(monitor));
} /* }}} */

/* {{{ */
pthreads_zend_object_t* pthreads_globals_object_alloc(size_t length) {
	pthreads_zend_object_t *bucket = (pthreads_zend_object_t*) ecalloc(1, length);

	if (pthreads_globals_lock()) {
		zend_hash_index_update_ptr(
			&PTHREADS_G(objects),
			(zend_ulong) bucket, bucket);
		pthreads_globals_unlock();
	}

	memset(bucket, 0, length);

	return bucket;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_valid(pthreads_zend_object_t *address) {
	zend_bool valid = 0;

	if (!address)
		return valid;

	if (pthreads_globals_lock()) {
		if (zend_hash_index_exists(&PTHREADS_G(objects), (zend_ulong) address)) {
			valid = 1;
		}
		pthreads_globals_unlock();
	}

	return valid;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(pthreads_zend_object_t *address) {
	zend_bool deleted = 0;

	if (!address)
		return deleted;

	if (pthreads_globals_lock()) {
		deleted = zend_hash_index_del(
			&PTHREADS_G(objects), (zend_ulong) address);
		pthreads_globals_unlock();
	}

	return deleted;
} /* }}} */

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
void pthreads_globals_shared_socket_track(PHP_SOCKET socket) {
	if (socket < 0) {
		return;
	}

	if (pthreads_globals_lock()) {
		zval value;

		ZVAL_LONG(&value, (zend_long) socket);
		zend_hash_index_add(&PTHREADS_G(shared_sockets), (zend_ulong) socket, &value);

		pthreads_globals_unlock();
	}
}

zend_bool pthreads_globals_socket_shared(PHP_SOCKET socket) {
	zend_bool result = 0;
	if (socket < 0) {
		return result;
	}

	if (pthreads_globals_lock()) {
		result = zend_hash_index_find(&PTHREADS_G(shared_sockets), (zend_ulong) socket) != NULL;
	
		pthreads_globals_unlock();
	}

	return result;
}
#endif

/* {{{ */
void pthreads_globals_shutdown() {
	if (PTHREADS_G(init)) {
		PTHREADS_G(init)=0;
		PTHREADS_G(failed)=0;
		/* we allow proc shutdown to destroy tables, and global strings */
		pthreads_monitor_destroy(&PTHREADS_G(monitor));
		zend_hash_destroy(&PTHREADS_G(objects));
#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
		zend_hash_destroy(&PTHREADS_G(shared_sockets));
#endif
		zend_hash_destroy(&PTHREADS_G(interned_strings));
	}
} /* }}} */
