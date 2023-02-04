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
#ifndef HAVE_PTHREADS_GLOBALS_H
#define HAVE_PTHREADS_GLOBALS_H

#include <src/pthreads.h>

/* {{{ pthreads_globals */
struct _pthreads_globals {
	/*
	* Initialized flag
	*/
	volatile zend_bool init;

	/*
	* Failed flag
	*/
	zend_bool failed;

	/*
	* Global Monitor
	*/
	pthreads_monitor_t *monitor;

	/*
	* Global/Default Resource Destructor
	*/
	dtor_func_t (default_resource_dtor);

	/*
	* Objects Cache
	*/
	HashTable objects;

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
	/*
	* Sockets which have been shared between threads, and so musn't be closed by the destructor
	*/
	HashTable shared_sockets;
#endif
	/*
	* Interned strings which can be used from any thread, like OPcache interned strings
	* Used for long-lived things like defined property names
	*/
	HashTable interned_strings;

	zval undef_zval;

	/*
	* High Frequency Strings
	*/
	struct _strings {
		zend_string *run;
		zval         worker;
		struct _session {
			zend_string *cache_limiter;
			zend_string *use_cookies;
		} session;
	} strings;
}; /* }}} */

extern struct _pthreads_globals pthreads_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads)

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ */
zend_bool pthreads_globals_object_valid(pthreads_zend_object_t *address); /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(pthreads_zend_object_t *address); /* }}} */

#if HAVE_PTHREADS_EXT_SOCKETS_SUPPORT
void pthreads_globals_shared_socket_track(PHP_SOCKET socket);

zend_bool pthreads_globals_socket_shared(PHP_SOCKET socket);
#endif

/* {{{ Locate a permanent interned string, either in the Zend global table or pthreads' global table */
zend_string* pthreads_globals_find_interned_string(zend_string* string); /* }}} */

/* {{{ Add a new permanent interned string to pthreads' table, or returns an existing interned string if one already exists */
zend_string* pthreads_globals_add_interned_string(zend_string* string); /* }}} */

/* {{{ */
pthreads_zend_object_t* pthreads_globals_object_alloc(size_t length); /* }}} */

/* {{{ initialize (true) globals */
zend_bool pthreads_globals_init(); /* }}} */

/* {{{ acquire global lock */
zend_bool pthreads_globals_lock(); /* }}} */

/* {{{ release global lock */
void pthreads_globals_unlock(); /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(); /* }}} */

#endif /* HAVE_PTHREADS_GLOBALS_H */
