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

/*
* These handlers provide thread-safe read/write/call/count/cast for pmmpthread objects
*/
#ifndef HAVE_PMMPTHREAD_HANDLERS_H
#define HAVE_PMMPTHREAD_HANDLERS_H

#include <src/pmmpthread.h>

#define PMMPTHREAD_COUNT_PASSTHRU_D zend_object *object, zend_long *count
#define PMMPTHREAD_COMPARE_PASSTHRU_D zval *op1, zval *op2

/* {{{  */
#define PMMPTHREAD_READ_DEBUG_PASSTHRU_D zend_object *object, int *is_temp
#define PMMPTHREAD_READ_PROPERTIES_PASSTHRU_D zend_object *object
#define PMMPTHREAD_READ_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, int type, void **cache, zval *rv
#define PMMPTHREAD_READ_DIMENSION_PASSTHRU_D zend_object *object, zval *member, int type, zval *rv

#define PMMPTHREAD_WRITE_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, zval *value, void **cache
#define PMMPTHREAD_WRITE_DIMENSION_PASSTHRU_D zend_object *object, zval *member, zval *value

#define PMMPTHREAD_HAS_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, int has_set_exists, void **cache
#define PMMPTHREAD_HAS_DIMENSION_PASSTHRU_D zend_object *object, zval *member, int has_set_exists

#define PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D zend_object *object, zend_string *member, void **cache
#define PMMPTHREAD_UNSET_DIMENSION_PASSTHRU_D zend_object *object, zval *member
/* }}} */

/* {{{ read proeprties from storage */
HashTable* pmmpthread_read_debug(PMMPTHREAD_READ_DEBUG_PASSTHRU_D); /* }}} */

/* {{{ read proeprties from storage */
HashTable* pmmpthread_read_properties(PMMPTHREAD_READ_PROPERTIES_PASSTHRU_D); /* }}} */

/* {{{ proxy get_property_ptr_ptr to read_property */
zval *pmmpthread_get_property_ptr_ptr_stub(zend_object *object, zend_string *member, int type, void **cache_slot); /* }}} */

/* {{{ read a property from the referenced object */
zval * pmmpthread_read_property(PMMPTHREAD_READ_PROPERTY_PASSTHRU_D);
zval * pmmpthread_read_property_deny(PMMPTHREAD_READ_PROPERTY_PASSTHRU_D);
zval * pmmpthread_read_dimension(PMMPTHREAD_READ_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ write a property to the referenced object */
zval* pmmpthread_write_property(PMMPTHREAD_WRITE_PROPERTY_PASSTHRU_D);
zval* pmmpthread_write_property_deny(PMMPTHREAD_WRITE_PROPERTY_PASSTHRU_D);
void pmmpthread_write_dimension(PMMPTHREAD_WRITE_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ check if the referenced object has a specific property */
int pmmpthread_has_property(PMMPTHREAD_HAS_PROPERTY_PASSTHRU_D);
int pmmpthread_has_property_deny(PMMPTHREAD_HAS_PROPERTY_PASSTHRU_D);
int pmmpthread_has_dimension(PMMPTHREAD_HAS_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ unset a property in the referenced object */
void pmmpthread_unset_property(PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D);
void pmmpthread_unset_property_deny(PMMPTHREAD_UNSET_PROPERTY_PASSTHRU_D);
void pmmpthread_unset_dimension(PMMPTHREAD_UNSET_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ count properties in storage */
int pmmpthread_count_properties(PMMPTHREAD_COUNT_PASSTHRU_D); /* }}} */

/* {{{ */
int pmmpthread_compare_objects(PMMPTHREAD_COMPARE_PASSTHRU_D); /* }}} */
#endif
