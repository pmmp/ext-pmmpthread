/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
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
#ifndef HAVE_PHP_PMMPTHREAD_H
#define HAVE_PHP_PMMPTHREAD_H
#define PHP_PMMPTHREAD_EXTNAME "pmmpthread"
#define PHP_PMMPTHREAD_VERSION "6.0.2"

PHP_MINIT_FUNCTION(pmmpthread);
PHP_MSHUTDOWN_FUNCTION(pmmpthread);
PHP_RINIT_FUNCTION(pmmpthread);
PHP_RSHUTDOWN_FUNCTION(pmmpthread);
PHP_MINFO_FUNCTION(pmmpthread);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pmmpthread);

extern zend_module_entry pmmpthread_module_entry;
#define phpext_pmmpthread_ptr &pmmpthread_module_entry

#endif
