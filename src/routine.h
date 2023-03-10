#ifndef HAVE_PTHREADS_ROUTINE_H
#define HAVE_PTHREADS_ROUTINE_H

#include <src/thread.h>

/* {{{ */
zend_bool pthreads_start(pthreads_zend_object_t* thread, zend_ulong thread_options); /* }}} */

/* {{{ */
zend_bool pthreads_join(pthreads_zend_object_t* thread); /* }}} */
#endif
