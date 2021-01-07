/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2016                                       |
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
#ifndef HAVE_PTHREADS_CLASS_SOCKET_H
#define HAVE_PTHREADS_CLASS_SOCKET_H
PHP_METHOD(ThreadedSocket, __construct);

PHP_METHOD(ThreadedSocket, setOption);
PHP_METHOD(ThreadedSocket, getOption);

PHP_METHOD(ThreadedSocket, bind);
PHP_METHOD(ThreadedSocket, listen);
PHP_METHOD(ThreadedSocket, accept);
PHP_METHOD(ThreadedSocket, connect);
PHP_METHOD(ThreadedSocket, select);

PHP_METHOD(ThreadedSocket, read);
PHP_METHOD(ThreadedSocket, write);
PHP_METHOD(ThreadedSocket, send);
PHP_METHOD(ThreadedSocket, recvfrom);
PHP_METHOD(ThreadedSocket, sendto);

PHP_METHOD(ThreadedSocket, setBlocking);
PHP_METHOD(ThreadedSocket, getPeerName);
PHP_METHOD(ThreadedSocket, getSockName);

PHP_METHOD(ThreadedSocket, getLastError);
PHP_METHOD(ThreadedSocket, clearError);
PHP_METHOD(ThreadedSocket, strerror);

PHP_METHOD(ThreadedSocket, close);

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket___construct, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_setOption, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_getOption, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_bind, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_listen, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_connect, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_read, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_send, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_recvfrom, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(1, buffer, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_sendto, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, addr, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_setBlocking, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_getHost, 0, 0, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, port, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_select, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(1, read, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, write, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, except, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, sec, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, usec, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, error, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_accept, 0, 0, 0)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_getLastError, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, clear, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(ThreadedSocket_strerror, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, error, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ThreadedSocket_void, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_socket_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_SOCKET
#	define HAVE_PTHREADS_CLASS_SOCKET
zend_function_entry pthreads_socket_methods[] = {
	PHP_ME(ThreadedSocket, __construct, ThreadedSocket___construct, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, setOption, ThreadedSocket_setOption, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, getOption, ThreadedSocket_getOption, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, bind, ThreadedSocket_bind, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, listen, ThreadedSocket_listen, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, accept, ThreadedSocket_accept, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, connect, ThreadedSocket_connect, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, select, ThreadedSocket_select, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(ThreadedSocket, read, ThreadedSocket_read, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, write, ThreadedSocket_write, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, send, ThreadedSocket_send, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, recvfrom, ThreadedSocket_recvfrom, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, sendto, ThreadedSocket_sendto, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, setBlocking, ThreadedSocket_setBlocking, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, getPeerName, ThreadedSocket_getHost, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, getSockName, ThreadedSocket_getHost, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, close, ThreadedSocket_void, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, getLastError, ThreadedSocket_getLastError, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, clearError, ThreadedSocket_void, ZEND_ACC_PUBLIC)
	PHP_ME(ThreadedSocket, strerror, ThreadedSocket_strerror, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/* {{{ proto ThreadedSocket::__construct(int domain, int type, int protocol)
	Create a Threaded ThreadedSocket */
PHP_METHOD(ThreadedSocket, __construct) {
	zend_long domain = AF_INET;
	zend_long type = SOCK_STREAM;
	zend_long protocol = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lll", &domain, &type, &protocol) != SUCCESS) {
		return;
	}

	pthreads_socket_construct(getThis(), domain, type, protocol);
} /* }}} */

/* {{{ proto bool ThreadedSocket::setOption(int level, int name, int value)
	Sets long socket option */
PHP_METHOD(ThreadedSocket, setOption) {
	zend_long level = 0;
	zend_long name = 0;
	zend_long value = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lll", &level, &name, &value) != SUCCESS) {
		return;
	}

	pthreads_socket_set_option(getThis(), level, name, value, return_value);
} /* }}} */

/* {{{ proto int ThreadedSocket::getOption(int level, int name)
	Get long socket option */
PHP_METHOD(ThreadedSocket, getOption) {
	zend_long level = 0;
	zend_long name = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "ll", &level, &name) != SUCCESS) {
		return;
	}

	pthreads_socket_get_option(getThis(), level, name, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::bind(string host [, int port]) */
PHP_METHOD(ThreadedSocket, bind) {
	zend_string *host = NULL;
	zend_long port = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S|l", &host, &port) != SUCCESS) {
		return;
	}

	pthreads_socket_bind(getThis(), host, port, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::listen([int backlog = 0]) */
PHP_METHOD(ThreadedSocket, listen) {
	zend_long backlog = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|l", &backlog) != SUCCESS) {
		return;
	}

	pthreads_socket_listen(getThis(), backlog, return_value);
} /* }}} */

/* {{{ proto ThreadedSocket|bool ThreadedSocket::accept([string class = self::class]) */
PHP_METHOD(ThreadedSocket, accept) {
	zend_class_entry *ce = zend_get_called_scope(execute_data);

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|C", &ce) != SUCCESS) {
		return;
	}

	pthreads_socket_accept(getThis(), ce, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::connect(string host[, int port]) */
PHP_METHOD(ThreadedSocket, connect) {
	zend_string *host = NULL;
	zend_long port = 0;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters_throw(argc, "S|l", &host, &port) != SUCCESS) {
		return;
	}

	pthreads_socket_connect(getThis(), argc, host, port, return_value);
} /* }}} */

/* {{{ proto int|bool ThreadedSocket::select(array &read, array &write, array &except, int sec [, int usec = 0 [, int &error]]) */
PHP_METHOD(ThreadedSocket, select) {
	zval *read, *write, *except, *sec, *errorno = NULL;
	zend_long usec = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "a/!a/!a/!z!|lz/", &read, &write, &except, &sec, &usec, &errorno) != SUCCESS) {
		return;
	}
	pthreads_socket_select(read, write, except, sec, usec, errorno, return_value);
} /* }}} */

/* {{{ proto string|bool ThreadedSocket::read(int length [, int flags = 0 [, int type = ThreadedSocket::BINARY_READ]]) */
PHP_METHOD(ThreadedSocket, read) {
	zend_long length = 0;
	zend_long flags = 0;
	zend_long type = PTHREADS_BINARY_READ;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l|ll", &length, &flags, &type) != SUCCESS) {
		return;
	}

	pthreads_socket_read(getThis(), length, flags, type, return_value);
} /* }}} */

/* {{{ proto int|bool ThreadedSocket::write(string buffer [, int length]) */
PHP_METHOD(ThreadedSocket, write) {
	zend_string *buffer = NULL;
	zend_long length = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "S|l", &buffer, &length) != SUCCESS) {
		return;
	}

	pthreads_socket_write(getThis(), buffer, length, return_value);
} /* }}} */

/* {{{ proto int|bool ThreadedSocket::send(string buffer, int length, int flags) */
PHP_METHOD(ThreadedSocket, send) {
	zend_string *buffer = NULL;
	zend_long length = 0;
	zend_long flags = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "Sll", &buffer, &length, &flags) != SUCCESS) {
		return;
	}

	pthreads_socket_send(getThis(), buffer, length, flags, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::recvfrom(string &buf, int length, int flags, string &name [, int &port ]) */
PHP_METHOD(ThreadedSocket, recvfrom) {
	zval		*buffer, *name, *port = NULL;
	zend_long	len, flags;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z/llz/|z/", &buffer, &len, &flags, &name, &port) == FAILURE) {
		return;
	}

	/* overflow check */
	if ((len + 2) < 3) {
		RETURN_FALSE;
	}

	pthreads_socket_recvfrom(getThis(), buffer, len, flags, name, port, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::sendto(string buf, int length, int flags, string addr [, int port ]) */
PHP_METHOD(ThreadedSocket, sendto) {
	zend_string *buffer, *address = NULL;
	zend_long	len, flags, port = 0;
	int	argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters_throw(argc, "SllS|l", &buffer, &len, &flags, &address, &port) == FAILURE) {
		return;
	}

	pthreads_socket_sendto(getThis(), argc, buffer, len, flags, address, port, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::setBlocking(bool blocking) */
PHP_METHOD(ThreadedSocket, setBlocking) {
	zend_bool blocking = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "b", &blocking) != SUCCESS) {
		return;
	}

	pthreads_socket_set_blocking(getThis(), blocking, return_value);
} /* }}} */

/* {{{ proto array ThreadedSocket::getPeerName([bool port = true]) */
PHP_METHOD(ThreadedSocket, getPeerName) {
	zend_bool port = 1;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|b", &port) != SUCCESS) {
		return;
	}

	pthreads_socket_get_peer_name(getThis(), port, return_value);
} /* }}} */

/* {{{ proto array ThreadedSocket::getSockName([bool port = true]) */
PHP_METHOD(ThreadedSocket, getSockName) {
	zend_bool port = 1;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|b", &port) != SUCCESS) {
		return;
	}

	pthreads_socket_get_sock_name(getThis(), port, return_value);
} /* }}} */

/* {{{ proto int|bool ThreadedSocket::getLastError([bool clear = false]) */
PHP_METHOD(ThreadedSocket, getLastError) {
	zend_bool clear = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|b", &clear) != SUCCESS) {
		return;
	}

	pthreads_socket_get_last_error(getThis(), clear, return_value);
} /* }}} */

/* {{{ proto void ThreadedSocket::clearError() */
PHP_METHOD(ThreadedSocket, clearError) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_clear_error(getThis());
} /* }}} */

/* {{{ proto string|null ThreadedSocket::strerror(int error) */
PHP_METHOD(ThreadedSocket, strerror) {
	zend_long error = 0;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &error) != SUCCESS) {
		return;
	}

	pthreads_socket_strerror(error, return_value);
} /* }}} */

/* {{{ proto bool ThreadedSocket::close(void) */
PHP_METHOD(ThreadedSocket, close) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_close(getThis(), return_value);
} /* }}} */
#	endif
#endif
