/*
  +----------------------------------------------------------------------+
  | pmmpthread                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2015                                       |
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
#ifndef HAVE_PMMPTHREAD_MONITOR_H
#define HAVE_PMMPTHREAD_MONITOR_H

typedef unsigned long pmmpthread_monitor_state_t;

typedef struct _pmmpthread_monitor_t {
	pmmpthread_monitor_state_t state;
	pthread_mutex_t          mutex;
	pthread_cond_t           cond;
} pmmpthread_monitor_t;

#define PMMPTHREAD_MONITOR_NOTHING         (0)
#define PMMPTHREAD_MONITOR_STARTED         (1<<0)
#define PMMPTHREAD_MONITOR_RUNNING         (1<<1)
#define PMMPTHREAD_MONITOR_JOINED          (1<<2)
#define PMMPTHREAD_MONITOR_ERROR           (1<<3)
#define PMMPTHREAD_MONITOR_READY           (1<<4)
#define PMMPTHREAD_MONITOR_COLLECT_GARBAGE (1<<5)
#define PMMPTHREAD_MONITOR_EXIT            (1<<6)
#define PMMPTHREAD_MONITOR_AWAIT_JOIN      (1<<7)

zend_result pmmpthread_monitor_init(pmmpthread_monitor_t* m);
void pmmpthread_monitor_destroy(pmmpthread_monitor_t* m);
zend_bool pmmpthread_monitor_lock(pmmpthread_monitor_t *m);
zend_bool pmmpthread_monitor_unlock(pmmpthread_monitor_t *m);
pmmpthread_monitor_state_t pmmpthread_monitor_check(pmmpthread_monitor_t *m, pmmpthread_monitor_state_t state);
int pmmpthread_monitor_wait(pmmpthread_monitor_t *m, long timeout);
int pmmpthread_monitor_notify(pmmpthread_monitor_t *m);
int pmmpthread_monitor_notify_one(pmmpthread_monitor_t *m);
void pmmpthread_monitor_wait_until(pmmpthread_monitor_t *m, pmmpthread_monitor_state_t state);
void pmmpthread_monitor_add(pmmpthread_monitor_t *m, pmmpthread_monitor_state_t state);
void pmmpthread_monitor_remove(pmmpthread_monitor_t *m, pmmpthread_monitor_state_t state);
#endif
