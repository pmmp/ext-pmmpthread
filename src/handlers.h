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

/*
* These handlers provide thread-safe read/write/call/count/cast for pthreads objects
*/
#ifndef HAVE_PTHREADS_HANDLERS_H
#define HAVE_PTHREADS_HANDLERS_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#define PTHREADS_CAST_PASSTHRU_D zend_object *from, zval *to, int type
#define PTHREADS_COUNT_PASSTHRU_D zend_object *object, zend_long *count
#define PTHREADS_COMPARE_PASSTHRU_D zval *op1, zval *op2

/* {{{  */
#define PTHREADS_READ_DEBUG_PASSTHRU_D zend_object *object, int *is_temp
#define PTHREADS_READ_PROPERTIES_PASSTHRU_D zend_object *object
#define PTHREADS_READ_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, int type, void **cache, zval *rv
#define PTHREADS_READ_DIMENSION_PASSTHRU_D zend_object *object, zval *member, int type, zval *rv

#define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, zval *value, void **cache
#define PTHREADS_WRITE_DIMENSION_PASSTHRU_D zend_object *object, zval *member, zval *value

#define PTHREADS_HAS_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, int has_set_exists, void **cache
#define PTHREADS_HAS_DIMENSION_PASSTHRU_D zend_object *object, zval *member, int has_set_exists

#define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, void **cache
#define PTHREADS_UNSET_DIMENSION_PASSTHRU_D zend_object *object, zval *member
/* }}} */

/* {{{ read proeprties from storage */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D); /* }}} */

/* {{{ read proeprties from storage */
HashTable* pthreads_read_properties(PTHREADS_READ_PROPERTIES_PASSTHRU_D); /* }}} */

/* {{{ proxy get_property_ptr_ptr to read_property */
zval *pthreads_get_property_ptr_ptr_stub(zend_object *object, zend_string *member, int type, void **cache_slot); /* }}} */

/* {{{ read a property from the referenced object */
zval * pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D);
zval * pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ disallow read a property from the referenced object */
zval * pthreads_read_property_disallow(PTHREADS_READ_PROPERTY_PASSTHRU_D);
zval * pthreads_read_dimension_disallow(PTHREADS_READ_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ disallow write a property to the referenced object */
zval* pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D);
void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ disallow write a property to the referenced object */
zval* pthreads_write_property_disallow(PTHREADS_WRITE_PROPERTY_PASSTHRU_D);
void pthreads_write_dimension_disallow(PTHREADS_WRITE_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ check if the referenced object has a specific property */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D);
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ disallow check if the referenced object has a specific property */
int pthreads_has_property_disallow(PTHREADS_HAS_PROPERTY_PASSTHRU_D);
int pthreads_has_dimension_disallow(PTHREADS_HAS_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ unset a property in the referenced object */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D);
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ disallow unset a property in the referenced object */
void pthreads_unset_property_disallow(PTHREADS_UNSET_PROPERTY_PASSTHRU_D);
void pthreads_unset_dimension_disallow(PTHREADS_UNSET_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ count properties in storage */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D); /* }}} */

/* {{{ count properties in storage */
int pthreads_count_properties_disallow(PTHREADS_COUNT_PASSTHRU_D); /* }}} */

/* {{{ cast an object to a normal array helper */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D); /* }}} */

/* {{{ */
int pthreads_compare_objects(PTHREADS_COMPARE_PASSTHRU_D); /* }}} */
#endif
