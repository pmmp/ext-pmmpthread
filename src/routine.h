#ifndef HAVE_PMMPTHREAD_ROUTINE_H
#define HAVE_PMMPTHREAD_ROUTINE_H

#include <src/thread.h>

/* {{{ */
zend_bool pmmpthread_start(pmmpthread_zend_object_t* thread, zend_ulong thread_options); /* }}} */

/* {{{ */
zend_bool pmmpthread_join(pmmpthread_zend_object_t* thread); /* }}} */
#endif
