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
#ifndef HAVE_PMMPTHREAD_PREPARE_H
#define HAVE_PMMPTHREAD_PREPARE_H

#include <src/pmmpthread.h>

/* {{{ fetch prepared class entry */
zend_class_entry* pmmpthread_prepare_single_class(const pmmpthread_ident_t* source, zend_class_entry *candidate); /* }}} */

/* {{{ */
void pmmpthread_prepared_entry_late_bindings(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared); /* }}} */

/* {{{ */
void pmmpthread_context_late_bindings(const pmmpthread_ident_t* source); /* }}} */

/* {{{ */
int pmmpthread_prepared_startup(pmmpthread_object_t* thread, pmmpthread_monitor_t *ready, zend_class_entry *thread_ce, zend_ulong thread_options); /* }}} */

/* {{{ */
void pmmpthread_call_shutdown_functions(void); /* }}} */

/* {{{ */
int pmmpthread_prepared_shutdown(void); /* }}} */
#endif
