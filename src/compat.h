#ifndef HAVE_PTHREADS_COMPAT_H
#define HAVE_PTHREADS_COMPAT_H

#include <src/pthreads.h>

#if PHP_VERSION_ID >= 80000
typedef zend_object pthreads_handler_context;
typedef zend_string pthreads_property_name;
#define PTHREADS_COMPAT_OBJECT_FROM_ZVAL(zv) Z_OBJ_P(zv)
#define PTHREADS_COMPAT_ZOBJ_FROM_HANDLER_CONTEXT(object) (object)
#else
typedef zval pthreads_handler_context;
typedef zval pthreads_property_name;
#define PTHREADS_COMPAT_OBJECT_FROM_ZVAL(zv) (zv)
#define PTHREADS_COMPAT_ZOBJ_FROM_HANDLER_CONTEXT(object) Z_OBJ_P(object)
#endif

#define PTHREADS_COMPAT_OBJECT_THIS() PTHREADS_COMPAT_OBJECT_FROM_ZVAL(getThis())

#endif
