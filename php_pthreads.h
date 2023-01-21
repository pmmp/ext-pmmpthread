/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                         |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PHP_PTHREADS_H
#define HAVE_PHP_PTHREADS_H
#define PHP_PTHREADS_EXTNAME "pthreads"
#define PHP_PTHREADS_VERSION "5.0.1"

PHP_MINIT_FUNCTION(pthreads);
PHP_MSHUTDOWN_FUNCTION(pthreads);
PHP_RINIT_FUNCTION(pthreads);
PHP_RSHUTDOWN_FUNCTION(pthreads);
PHP_MINFO_FUNCTION(pthreads);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pthreads);

extern zend_module_entry pthreads_module_entry;
#define phpext_pthreads_ptr &pthreads_module_entry

#endif
