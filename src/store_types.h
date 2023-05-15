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
#ifndef HAVE_PMMPTHREAD_STORE_TYPES_H
#define HAVE_PMMPTHREAD_STORE_TYPES_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

typedef enum _pmmpthread_store_type {
	STORE_TYPE_CLOSURE,
	STORE_TYPE_THREADSAFE_OBJECT,
	STORE_TYPE_SOCKET,
	STORE_TYPE_ENUM,
	STORE_TYPE_STRING_PTR,
} pmmpthread_store_type;

typedef struct _pmmpthread_storage {
	pmmpthread_store_type type;
} pmmpthread_storage;

typedef struct _pmmpthread_closure_storage_t {
	pmmpthread_storage common;
	zend_closure* closure;
	pmmpthread_zend_object_t* this_obj;
	pmmpthread_ident_t owner;
} pmmpthread_closure_storage_t;

typedef struct _pmmpthread_zend_object_storage_t {
	pmmpthread_storage common;
	pmmpthread_zend_object_t* object;
} pmmpthread_zend_object_storage_t;

#if HAVE_PMMPTHREAD_EXT_SOCKETS_SUPPORT
typedef struct _pmmpthread_socket_storage {
	pmmpthread_storage common;
	PHP_SOCKET bsd_socket;
	int        type;
	int        error;
	int        blocking;
} pmmpthread_socket_storage_t;
#endif

typedef struct _pmmpthread_enum_storage_t {
	pmmpthread_storage common;
	zend_string* class_name;
	zend_string* member_name;
} pmmpthread_enum_storage_t;

typedef struct _pmmpthread_string_storage_t {
	pmmpthread_storage common;
	zend_string* string;
	pmmpthread_ident_t owner;
} pmmpthread_string_storage_t;
#endif
