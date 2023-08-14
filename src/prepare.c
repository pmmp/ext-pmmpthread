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

#include <src/prepare.h>
#include <src/object.h>
#include <src/globals.h>
#include <src/copy.h>
#include <Zend/zend_enum.h>
#include <Zend/zend_observer.h>

#define PMMPTHREAD_PREPARATION_BEGIN_CRITICAL() pmmpthread_globals_lock();
#define PMMPTHREAD_PREPARATION_END_CRITICAL()   pmmpthread_globals_unlock()

/* {{{ */
static zend_class_entry* pmmpthread_prepared_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate);
static zend_class_entry* pmmpthread_create_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate, int do_late_bindings);
static zend_trait_alias * pmmpthread_preparation_copy_trait_alias(zend_trait_alias *alias);
static zend_trait_precedence * pmmpthread_preparation_copy_trait_precedence(zend_trait_precedence *precedence);
static void pmmpthread_preparation_copy_trait_method_reference(zend_trait_method_reference *reference, zend_trait_method_reference *copy);

/* {{{ Initializes an enum case object.
 * Adapted from zend_enum_new(), which is sadly not exported. */
zend_object* prepare_enum_constant(const pmmpthread_ident_t* source, zend_class_entry* prepared, zend_object* enum_obj) {
	zend_object* new_obj = zend_objects_new(prepared);

	pmmpthread_copy_zval(source, OBJ_PROP_NUM(new_obj, 0), zend_enum_fetch_case_name(enum_obj));
	if (prepared->enum_backing_type != IS_UNDEF) {
		zval* backing_value_zv = zend_enum_fetch_case_value(enum_obj);
		ZEND_ASSERT(backing_value_zv != NULL);
		if (backing_value_zv != NULL) {
			pmmpthread_copy_zval(source, OBJ_PROP_NUM(new_obj, 1), backing_value_zv);
		}
	}

	new_obj->handlers = enum_obj->handlers;

	return new_obj;
} /* }}} */

/* {{{ */
static void prepare_class_constants(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {

	zend_string *key;
	zval *value;

	ZEND_HASH_FOREACH_STR_KEY_VAL(&candidate->constants_table, key, value) {
		zend_string *name;

		if (zend_hash_exists(&prepared->constants_table, key)) {
			continue;
		}

		zend_class_constant *zc = Z_PTR_P(value);
		zend_class_constant* rc;

		name = pmmpthread_copy_string(key);

		if (zc->ce->type == ZEND_INTERNAL_CLASS) {
			rc = pemalloc(sizeof(zend_class_constant), 1);
		} else {
			rc = zend_arena_alloc(&CG(arena), sizeof(zend_class_constant));
		}

		memcpy(rc, zc, sizeof(zend_class_constant));
		if (ZEND_CLASS_CONST_FLAGS(zc) & ZEND_CLASS_CONST_IS_CASE && Z_TYPE(zc->value) == IS_OBJECT) {
			//resolved enum members require special treatment, because serializing and unserializing them just gives
			//back the original enum member.
			ZVAL_OBJ(&rc->value, prepare_enum_constant(source, prepared, Z_OBJ(zc->value)));
		} else if (pmmpthread_copy_zval(source, &rc->value, &zc->value) != SUCCESS) {
			zend_error_at_noreturn(
				E_CORE_ERROR,
				prepared->info.user.filename,
				0,
				"pmmpthread encountered a non-copyable class constant %s::%s with value of type %s",
				ZSTR_VAL(prepared->name),
				ZSTR_VAL(name),
				zend_zval_type_name(&zc->value)
			);
		}
		rc->ce = pmmpthread_prepared_entry(source, zc->ce);

		zend_hash_add_ptr(&prepared->constants_table, name, rc);

		zend_string_release(name);

		if (zc->doc_comment != NULL) {
			rc->doc_comment = pmmpthread_copy_string(zc->doc_comment);
		}
		if (zc->attributes) {
			rc->attributes = pmmpthread_copy_attributes(source, zc->attributes, zc->ce->type == ZEND_INTERNAL_CLASS ? NULL : zc->ce->info.user.filename);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static void prepare_class_statics(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {
	if (candidate->default_static_members_count) {
		/* this code is adapted from ext/opcache/zend_accelerator_util_funcs.c */
		int i, end;
		zend_class_entry *parent = !(prepared->ce_flags & ZEND_ACC_LINKED) ? NULL : prepared->parent;

		if (prepared->default_static_members_table) {
			//if this is an anonymous class, we may have already copied declared statics for this class (but not inherited ones)
			for (i = 0; i < prepared->default_static_members_count; i++) {
				zval_ptr_dtor(&prepared->default_static_members_table[i]);
			}
			efree(prepared->default_static_members_table);
		}
		prepared->default_static_members_table = (zval*) ecalloc(
			sizeof(zval), candidate->default_static_members_count);
		prepared->default_static_members_count = candidate->default_static_members_count;
		memcpy(prepared->default_static_members_table,
		       candidate->default_static_members_table,
			sizeof(zval) * candidate->default_static_members_count);

		i = candidate->default_static_members_count - 1;
		/* Copy static properties in this class */
		end = parent ? parent->default_static_members_count : 0;
		for (; i >= end; i--) {
			if (pmmpthread_copy_zval(source,
				&prepared->default_static_members_table[i], &candidate->default_static_members_table[i])) {
				ZEND_ASSERT(0);
			}
		}

		/* Create indirections to static properties from parent classes */
		while (parent && parent->default_static_members_table) {
			end = parent->parent ? parent->parent->default_static_members_count : 0;
			for (; i >= end; i--) {
				zval *p = &prepared->default_static_members_table[i];
				ZVAL_INDIRECT(p, &parent->default_static_members_table[i]);
			}

			parent = parent->parent;
		}

#if PHP_VERSION_ID >= 80200
		//zend_initialize_class_data() already inits the MAP_PTR(static_members_table) to NULL, so nothing to do here
#else
		if (!ZEND_MAP_PTR(prepared->static_members_table)) {
			ZEND_MAP_PTR_INIT(prepared->static_members_table, zend_arena_alloc(&CG(arena), sizeof(zval*)));
			ZEND_MAP_PTR_SET(prepared->static_members_table, NULL);
		}
#endif
	} else prepared->default_static_members_count = 0;
} /* }}} */

/* {{{ */
static void prepare_class_function_table(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {

	zend_string *key;
	zend_function *value;

	ZEND_HASH_FOREACH_STR_KEY_PTR(&candidate->function_table, key, value) {
		if (!zend_hash_exists(&prepared->function_table, key)) {
			zend_string *name = pmmpthread_copy_string(key);

			value = pmmpthread_copy_function(source, value);
			zend_hash_add_ptr(&prepared->function_table, name, value);
			zend_string_release(name);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static void prepare_class_property_table(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {

	zend_property_info *info;
	zend_string *name;
	ZEND_HASH_FOREACH_STR_KEY_PTR(&candidate->properties_info, name, info) {
		zend_property_info *dup;

		if (info->ce->type == ZEND_INTERNAL_CLASS) {
			dup = pemalloc(sizeof(zend_property_info), 1);
		} else {
			dup = zend_arena_alloc(&CG(arena), sizeof(zend_property_info));
		}
		memcpy(dup, info, sizeof(zend_property_info));

		dup->name = pmmpthread_copy_string(info->name);
		if (info->doc_comment) {
			if (PMMPTHREAD_ZG(options) & PMMPTHREAD_INHERIT_COMMENTS) {
				dup->doc_comment = pmmpthread_copy_string(info->doc_comment);
			} else dup->doc_comment = NULL;
		}

		if (info->ce) {
			if (info->ce == candidate) {
				dup->ce = prepared;
			} else dup->ce = pmmpthread_prepared_entry(source, info->ce);
		}

		memcpy(&dup->type, &info->type, sizeof(zend_type));

		//This code is based on zend_persist_type() in ext/opcache/zend_persist.c
		if (ZEND_TYPE_HAS_LIST(info->type)) {
			const zend_type_list *old_list = ZEND_TYPE_LIST(info->type);
			zend_type_list *new_list;
			if (ZEND_TYPE_USES_ARENA(info->type)) {
				new_list = zend_arena_alloc(&CG(arena), ZEND_TYPE_LIST_SIZE(old_list->num_types));
			} else {
				new_list = emalloc(ZEND_TYPE_LIST_SIZE(old_list->num_types));
			}
			memcpy(new_list, old_list, ZEND_TYPE_LIST_SIZE(old_list->num_types));
			ZEND_TYPE_SET_PTR(dup->type, new_list);
		}

		zend_type *single_type;
		ZEND_TYPE_FOREACH(dup->type, single_type) {
			if (ZEND_TYPE_HAS_NAME(*single_type)) {
				ZEND_TYPE_SET_PTR(*single_type, pmmpthread_copy_string(ZEND_TYPE_NAME(*single_type)));
			}
		} ZEND_TYPE_FOREACH_END();

		if (info->attributes) {
			dup->attributes = pmmpthread_copy_attributes(source, info->attributes, info->ce->type == ZEND_INTERNAL_CLASS ? NULL : info->ce->info.user.filename);
		}

		if (!zend_hash_str_add_ptr(&prepared->properties_info, name->val, name->len, dup)) {
			if (dup->doc_comment)
				zend_string_release(dup->doc_comment);
		}
	} ZEND_HASH_FOREACH_END();

	if (candidate->default_properties_count) {
		int i;

		if(prepared->default_properties_table != NULL) {
			efree(prepared->default_properties_table);
		}
		prepared->default_properties_table = emalloc(
			sizeof(zval) * candidate->default_properties_count);

		memcpy(
			prepared->default_properties_table,
			candidate->default_properties_table,
			sizeof(zval) * candidate->default_properties_count);

		for (i=0; i<candidate->default_properties_count; i++) {
			if (Z_REFCOUNTED(prepared->default_properties_table[i])) {
				if (pmmpthread_copy_zval(source,
					&prepared->default_properties_table[i], &candidate->default_properties_table[i]) == FAILURE) {
					ZEND_ASSERT(0);
					ZVAL_NULL(&prepared->default_properties_table[i]);
				}
			}
		}
		prepared->default_properties_count = candidate->default_properties_count;
		if (prepared->ce_flags & ZEND_ACC_LINKED) {
			prepared->properties_info_table = zend_arena_calloc(&CG(arena), 1, sizeof(zend_property_info *) * candidate->default_properties_count);

			if (prepared->parent && prepared->parent->default_properties_count != 0) {
				memcpy(
					prepared->properties_info_table, prepared->parent->properties_info_table,
					sizeof(zend_property_info*) * prepared->parent->default_properties_count
				);
			}

			ZEND_HASH_FOREACH_PTR(&prepared->properties_info, info) {
				if (info->ce == prepared && (info->flags & ZEND_ACC_STATIC) == 0) {
					prepared->properties_info_table[OBJ_PROP_TO_NUM(info->offset)] = info;
				}
			} ZEND_HASH_FOREACH_END();
		} else prepared->properties_info_table = NULL;
	} else prepared->default_properties_count = 0;
} /* }}} */

/* {{{ */
static void prepare_class_handlers(zend_class_entry *candidate, zend_class_entry *prepared) {
	prepared->create_object = candidate->create_object;
	prepared->serialize = candidate->serialize;
	prepared->unserialize = candidate->unserialize;
	prepared->get_iterator = candidate->get_iterator;
	prepared->interface_gets_implemented = candidate->interface_gets_implemented;
	prepared->get_static_method = candidate->get_static_method;
} /* }}} */

static void prepare_class_interceptors(zend_class_entry *candidate, zend_class_entry *prepared) {
	zend_function *func;

	if (!prepared->constructor && zend_hash_num_elements(&prepared->function_table)) {
		if ((func = zend_hash_str_find_ptr(&prepared->function_table, "__construct", sizeof("__construct")-1))) {
			prepared->constructor = func;
		} else {
			if ((func = zend_hash_find_ptr(&prepared->function_table, prepared->name))) {
				prepared->constructor = func;
			}
		}
	}

#define FIND_AND_SET(f, n) do {\
	if (!prepared->f && zend_hash_num_elements(&prepared->function_table)) { \
		if ((func = zend_hash_str_find_ptr(&prepared->function_table, n, sizeof(n)-1))) { \
			prepared->f = func; \
		} \
	} \
} \
while(0)

	FIND_AND_SET(clone, "__clone");
	FIND_AND_SET(__get, "__get");
	FIND_AND_SET(__set, "__set");
	FIND_AND_SET(__unset, "__unset");
	FIND_AND_SET(__isset, "__isset");
	FIND_AND_SET(__call, "__call");
	FIND_AND_SET(__callstatic, "__callstatic");
	FIND_AND_SET(__serialize, "__serialize");
	FIND_AND_SET(__unserialize, "__unserialize");
	FIND_AND_SET(__tostring, "__tostring");
	FIND_AND_SET(destructor, "__destruct");
#undef FIND_AND_SET

#define SET_FUNC_REF(group, funcname) do { \
	prepared->group->zf_##funcname = zend_hash_str_find_ptr( \
		&prepared->function_table, #funcname, sizeof(#funcname) - 1); \
} while (0)

	if (candidate->iterator_funcs_ptr) {
		prepared->iterator_funcs_ptr = zend_arena_alloc(&CG(arena), sizeof(zend_class_iterator_funcs));
		SET_FUNC_REF(iterator_funcs_ptr, new_iterator);
		SET_FUNC_REF(iterator_funcs_ptr, valid);
		SET_FUNC_REF(iterator_funcs_ptr, current);
		SET_FUNC_REF(iterator_funcs_ptr, key);
		SET_FUNC_REF(iterator_funcs_ptr, next);
		SET_FUNC_REF(iterator_funcs_ptr, rewind);
	} else prepared->iterator_funcs_ptr = NULL;

#if PHP_VERSION_ID >= 80200
	if (candidate->arrayaccess_funcs_ptr) {
		prepared->arrayaccess_funcs_ptr = zend_arena_alloc(&CG(arena), sizeof(zend_class_arrayaccess_funcs));
		SET_FUNC_REF(arrayaccess_funcs_ptr, offsetget);
		SET_FUNC_REF(arrayaccess_funcs_ptr, offsetexists);
		SET_FUNC_REF(arrayaccess_funcs_ptr, offsetset);
		SET_FUNC_REF(arrayaccess_funcs_ptr, offsetunset);
	} else prepared->arrayaccess_funcs_ptr = NULL;
#endif
#undef SET_FUNC_REF
} /* }}} */

/* {{{ */
static void prepare_class_traits(zend_class_entry *candidate, zend_class_entry *prepared) {

	if (candidate->num_traits) {
		unsigned int trait;

		prepared->trait_names = emalloc(sizeof(zend_class_name) * candidate->num_traits);
		for (trait = 0; trait < candidate->num_traits; trait++) {
			prepared->trait_names[trait].lc_name = pmmpthread_copy_string(candidate->trait_names[trait].lc_name);
			prepared->trait_names[trait].name = pmmpthread_copy_string(candidate->trait_names[trait].name);
		}
		prepared->num_traits = candidate->num_traits;
	} else prepared->num_traits = 0;

	if (candidate->trait_aliases) {
		size_t alias = 0;

		while (candidate->trait_aliases[alias]) {
			alias++;
		}

		//TODO: some stuff may already have been copied, they will leak if this is the second pass on an anonymous class
		prepared->trait_aliases = emalloc(sizeof(zend_trait_alias*) * (alias+1));
		alias = 0;

		while (candidate->trait_aliases[alias]) {
			prepared->trait_aliases[alias] = pmmpthread_preparation_copy_trait_alias(candidate->trait_aliases[alias]);
			alias++;
		}
		prepared->trait_aliases[alias]=NULL;
	} else prepared->trait_aliases = NULL;

	if (candidate->trait_precedences) {
		size_t precedence = 0;

		while (candidate->trait_precedences[precedence]) {
			precedence++;
		}
		prepared->trait_precedences = emalloc(sizeof(zend_trait_precedence*) * (precedence+1));
		//TODO: some stuff may already have been copied, they will leak if this is the second pass on an anonymous class
		precedence = 0;

		while (candidate->trait_precedences[precedence]) {
			prepared->trait_precedences[precedence] = pmmpthread_preparation_copy_trait_precedence(candidate->trait_precedences[precedence]);
			precedence++;
		}
		prepared->trait_precedences[precedence]=NULL;
	} else prepared->trait_precedences = NULL;
} /* }}} */

/* {{{ */
static zend_class_entry* pmmpthread_complete_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {
	int old_ce_flags = prepared->ce_flags;
	prepared->ce_flags = candidate->ce_flags;
	if (candidate->ce_flags & ZEND_ACC_LINKED) {
		if(!(old_ce_flags & ZEND_ACC_LINKED) && prepared->parent_name){ //we're updating an unlinked copy with information from a newly linked copy
			zend_string_release(prepared->parent_name);
		}
		if (candidate->parent) {
			prepared->parent = pmmpthread_prepared_entry(source, candidate->parent);
		}
	} else if (candidate->parent_name) {
		prepared->parent_name = pmmpthread_copy_string(candidate->parent_name);
	}

	if (candidate->num_interfaces) {
		unsigned int interface;
		if (candidate->ce_flags & ZEND_ACC_LINKED) {
			if(!(old_ce_flags & ZEND_ACC_LINKED)){
				for (interface = 0; interface < prepared->num_interfaces; interface++) {
					zend_string_release(prepared->interface_names[interface].name);
					zend_string_release(prepared->interface_names[interface].lc_name);
				}
				efree(prepared->interfaces);
			}
			prepared->interfaces = emalloc(sizeof(zend_class_entry*) * candidate->num_interfaces);
			for (interface = 0; interface < candidate->num_interfaces; interface++)
				prepared->interfaces[interface] = pmmpthread_prepared_entry(source, candidate->interfaces[interface]);
		} else {
			prepared->interface_names = emalloc(sizeof(zend_class_name) * candidate->num_interfaces);
			for (interface = 0; interface < candidate->num_interfaces; interface++) {
				prepared->interface_names[interface].name = pmmpthread_copy_string(candidate->interface_names[interface].name);
				prepared->interface_names[interface].lc_name = pmmpthread_copy_string(candidate->interface_names[interface].lc_name);
			}
		}
		prepared->num_interfaces = candidate->num_interfaces;
	} else prepared->num_interfaces = 0;

	prepare_class_traits(candidate, prepared);
	prepare_class_handlers(candidate, prepared);
	prepare_class_function_table(source, candidate, prepared);
	prepare_class_interceptors(candidate, prepared);

	return prepared;
} /* }}} */

/* {{{ */
static HashTable* prepare_backed_enum_table(const pmmpthread_ident_t* owner, const HashTable *candidate_table) {
	if (!candidate_table) {
		return NULL;
	}

	HashTable *result = emalloc(sizeof(HashTable));
	zend_hash_init(result, 0, NULL, ZVAL_PTR_DTOR, 0);

	HashPosition h;
	zend_string* key;
	zval* val;
	ZEND_HASH_FOREACH_KEY_VAL(candidate_table, h, key, val) {
		zval new_val;
		if (pmmpthread_copy_zval(owner, &new_val, val) == FAILURE) {
			ZEND_ASSERT(0);
			continue;
		}
		if (key) {
			zend_string* new_key = pmmpthread_copy_string(key);
			zend_hash_add_new(result, key, &new_val);
			zend_string_release(new_key);
		}
		else {
			zend_hash_index_add_new(result, h, &new_val);
		}
	} ZEND_HASH_FOREACH_END();

	return result;
}
/* }}} */

/* {{{ */
static zend_class_entry* pmmpthread_copy_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate) {
	zend_class_entry *prepared;

	prepared = zend_arena_alloc(&CG(arena), sizeof(zend_class_entry));
	prepared->name = pmmpthread_copy_string(candidate->name);
	prepared->type = candidate->type;

	zend_initialize_class_data(prepared, 1);

	prepared->ce_flags = candidate->ce_flags;
	prepared->refcount = 1;

	memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));

	if ((PMMPTHREAD_ZG(options) & PMMPTHREAD_INHERIT_COMMENTS) &&
	   (candidate->info.user.doc_comment)) {
			prepared->info.user.doc_comment = pmmpthread_copy_string(candidate->info.user.doc_comment);
		} else prepared->info.user.doc_comment = NULL;
	
	if (candidate->attributes) {
		prepared->attributes = pmmpthread_copy_attributes(source, candidate->attributes, prepared->info.user.filename);
	}

	prepared->enum_backing_type = candidate->enum_backing_type;
	if (candidate->backed_enum_table) {
		prepared->backed_enum_table = prepare_backed_enum_table(source, candidate->backed_enum_table);
	}

	if (prepared->info.user.filename) {
		zend_string *filename_copy;
		if (!(filename_copy = zend_hash_find_ptr(&PMMPTHREAD_ZG(filenames), candidate->info.user.filename))) {
			filename_copy = pmmpthread_copy_string(candidate->info.user.filename);
			zend_hash_add_ptr(&PMMPTHREAD_ZG(filenames), filename_copy, filename_copy);
			zend_string_release(filename_copy);
		}
		//php/php-src@7620ea15807a84e76cb1cb2f9d5234ea787aae2e - filenames are no longer always interned
		//opcache might intern them, but in the absence of opcache this won't be the case
		//if this string is interned, the following will be a no-op
		zend_string_addref(filename_copy);

		prepared->info.user.filename = filename_copy;
	}

	return pmmpthread_complete_entry(source, candidate, prepared);
} /* }}} */

/* {{{ */
static inline int pmmpthread_prepared_entry_function_prepare(zval *bucket, int argc, va_list argv, zend_hash_key *key) {
	zend_function *function = (zend_function*) Z_PTR_P(bucket);
	const pmmpthread_ident_t* source = va_arg(argv, const pmmpthread_ident_t*);
	zend_class_entry *prepared = va_arg(argv, zend_class_entry*);
	zend_class_entry *candidate = va_arg(argv, zend_class_entry*);
	zend_class_entry *scope = function->common.scope;

	if (scope == candidate) {
		function->common.scope = prepared;
	} else {
		if (function->common.scope && function->common.scope->type == ZEND_USER_CLASS) {
			function->common.scope = pmmpthread_prepared_entry(source, function->common.scope);
		}
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ */
zend_class_entry* pmmpthread_prepare_single_class(const pmmpthread_ident_t* source, zend_class_entry *candidate) {
	//this has to be synchronized every time we copy a new class after initial thread bootup, in case new immutable classes want to refer to new offsets in it
	zend_map_ptr_extend(PMMPTHREAD_CG(source->ls, map_ptr_last));
	return pmmpthread_prepared_entry(source, candidate);
} /* }}} */

/* {{{ */
static zend_class_entry* pmmpthread_prepared_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate) {
	return pmmpthread_create_entry(source, candidate, 1);
} /* }}} */

static void pmmpthread_prepare_immutable_class_dependencies(const pmmpthread_ident_t* source, zend_class_entry* candidate, int do_late_bindings) {
	//assume that all dependencies of immutable classes are themselves immutable

	if (candidate->ce_flags & ZEND_ACC_LINKED) {
		if (candidate->parent) {
			pmmpthread_create_entry(source, candidate->parent, do_late_bindings);
		}
		if (candidate->num_interfaces) {
			for (int i = 0; i < candidate->num_interfaces; i++) {
				pmmpthread_create_entry(source, candidate->interfaces[i], do_late_bindings);
			}
		}
	}

	zend_property_info* info;
	zend_type *type;
	zend_class_entry* ce;
	ZEND_HASH_FOREACH_PTR(&candidate->properties_info, info) {
		ZEND_TYPE_FOREACH(info->type, type) {
			if (ZEND_TYPE_HAS_NAME(*type)) {
				zend_string* lookup = zend_string_tolower(ZEND_TYPE_NAME(*type));
				ce = zend_hash_find_ptr(PMMPTHREAD_EG(source->ls, class_table), lookup);
				zend_string_release(lookup);
			} else {
				ce = NULL;
			}
			if (ce != NULL) {
				pmmpthread_create_entry(source, ce, do_late_bindings);
			}
		} ZEND_TYPE_FOREACH_END();
	} ZEND_HASH_FOREACH_END();
}

/* {{{ */
static zend_class_entry* pmmpthread_create_entry(const pmmpthread_ident_t* source, zend_class_entry *candidate, int do_late_bindings) {
	zend_class_entry *prepared = NULL;
	zend_string *lookup = NULL;

	if (!candidate) {
		return NULL;
	}

	if (candidate->type == ZEND_INTERNAL_CLASS
		|| candidate->ce_flags & ZEND_ACC_PRELOADED
	) {
		return zend_lookup_class(candidate->name);
	}

	lookup = zend_string_tolower(candidate->name);

	prepared = zend_hash_find_ptr(EG(class_table), lookup);
	if(
		prepared &&
		(prepared->ce_flags & (ZEND_ACC_ANON_CLASS|ZEND_ACC_LINKED)) == ZEND_ACC_ANON_CLASS &&
		(candidate->ce_flags & (ZEND_ACC_ANON_CLASS|ZEND_ACC_LINKED)) == (ZEND_ACC_ANON_CLASS|ZEND_ACC_LINKED)
	){
		//anonymous class that was unbound at initial copy, now bound on another thread (worker task stack?)

		if (prepared->ce_flags & ZEND_ACC_IMMUTABLE) {
			//we can't modify an immutable class; fallthru to full copy
			prepared = NULL;
		} else {
			pmmpthread_complete_entry(source, candidate, prepared);
			if (do_late_bindings) {
				//linking might cause new statics and constants to become visible
				pmmpthread_prepared_entry_late_bindings(source, candidate, prepared);
			}
		}
	}

	if (prepared) {
		zend_string_release(lookup);
		return prepared;
	}

	if (candidate->ce_flags & ZEND_ACC_IMMUTABLE) {
		//IMMUTABLE classes don't need to be copied and should not be modified
		//this may overwrite previously inserted immutable classes on 8.1 (e.g. unlinked opcached class -> linked opcached class)
		zend_hash_update_ptr(EG(class_table), lookup, candidate);
		zend_string_release(lookup);
		pmmpthread_prepare_immutable_class_dependencies(source, candidate, do_late_bindings);
		if (do_late_bindings) {
			//this is needed to copy non-default statics from the origin thread
			pmmpthread_prepared_entry_late_bindings(source, candidate, candidate);
		}
		return candidate;
	}

	if (!(prepared = pmmpthread_copy_entry(source, candidate))) {
		zend_string_release(lookup);
		return NULL;
	}
	zend_hash_update_ptr(EG(class_table), lookup, prepared);

	if(do_late_bindings) {
		pmmpthread_prepared_entry_late_bindings(source, candidate, prepared);
	}

	zend_hash_apply_with_arguments(
		&prepared->function_table,
		pmmpthread_prepared_entry_function_prepare,
		3, source, prepared, candidate);

	zend_string_release(lookup);

	return prepared;
} /* }}} */

/* {{{ */
void pmmpthread_prepared_entry_late_bindings(const pmmpthread_ident_t* source, zend_class_entry *candidate, zend_class_entry *prepared) {
	if (!(candidate->ce_flags & ZEND_ACC_IMMUTABLE)) {
		prepare_class_property_table(source, candidate, prepared);
		prepare_class_statics(source, candidate, prepared);
		prepare_class_constants(source, candidate, prepared);
	}
} /* }}} */


/* {{{ */
void pmmpthread_context_late_bindings(const pmmpthread_ident_t* source) {
	zend_class_entry *entry;
	zend_string *name;

	ZEND_HASH_FOREACH_STR_KEY_PTR(CG(class_table), name, entry) {
		if (entry->type != ZEND_INTERNAL_CLASS) {
			pmmpthread_prepared_entry_late_bindings(source, zend_hash_find_ptr(PMMPTHREAD_CG(source->ls, class_table), name), entry);
		}
	} ZEND_HASH_FOREACH_END();
}

/* {{{ */
static inline zend_bool pmmpthread_constant_exists(zend_string *name) {
	int retval = 1;
	zend_string *lookup;

	if (!zend_hash_exists(EG(zend_constants), name)) {
		lookup = zend_string_tolower(name);
		retval = zend_hash_exists(EG(zend_constants), lookup);
		zend_string_release(lookup);
	}

	return retval;
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_ini(const pmmpthread_ident_t* source) {
	zend_ini_entry *entry[2];
	zend_string *name;
	HashTable *table[2] = {PMMPTHREAD_EG(source->ls, ini_directives), EG(ini_directives)};

	if (!(PMMPTHREAD_ZG(options) & PMMPTHREAD_ALLOW_HEADERS)) {
		zend_alter_ini_entry_chars(
			PMMPTHREAD_G(strings).session.cache_limiter,
			"nocache", sizeof("nocache")-1,
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
		zend_alter_ini_entry_chars(
			PMMPTHREAD_G(strings).session.use_cookies,
			"0", sizeof("0")-1,
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
	}

	ZEND_HASH_FOREACH_STR_KEY_PTR(table[0], name, entry[0]) {
		if ((entry[1] = zend_hash_find_ptr(table[1], name))) {
			if (entry[0]->value && entry[1]->value) {
				if (strcmp(ZSTR_VAL(entry[0]->value), ZSTR_VAL(entry[1]->value)) != SUCCESS) {
					zend_bool resmod = entry[1]->modifiable;
					zend_string *copied = pmmpthread_copy_string(name);

					if (!EG(modified_ini_directives)) {
						ALLOC_HASHTABLE(EG(modified_ini_directives));
						zend_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
					}

					if (!entry[1]->modified) {
						entry[1]->orig_value = entry[1]->value;
						entry[1]->orig_modifiable = entry[1]->modifiable;
						entry[1]->modified = 1;
						zend_hash_add_ptr(EG(modified_ini_directives), copied, entry[1]);
					}

					entry[1]->modifiable = 1;
					entry[1]->on_modify(entry[1], entry[0]->value, entry[1]->mh_arg1, entry[1]->mh_arg2, entry[1]->mh_arg3, ZEND_INI_SYSTEM);
					if (entry[1]->modified && entry[1]->orig_value != entry[1]->value) {
						zend_string_release(entry[1]->value);
					}
					entry[1]->value = pmmpthread_copy_string(entry[0]->value);
					entry[1]->modifiable = resmod;

					zend_string_release(copied);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_constants(const pmmpthread_ident_t* source) {
	zend_constant *zconstant;
	zend_string *name;

	ZEND_HASH_FOREACH_STR_KEY_PTR(PMMPTHREAD_EG(source->ls, zend_constants), name, zconstant) {
		if (zconstant->name) {
			if (Z_TYPE(zconstant->value) == IS_RESOURCE){
				//we can't copy these
				continue;
			} else {
				zend_constant constant;

				if (!pmmpthread_constant_exists(name)) {
					constant.name = pmmpthread_copy_string(name);

					if (pmmpthread_copy_zval(source, &constant.value, &zconstant->value) != SUCCESS) {
						zend_error_noreturn(
							E_CORE_ERROR,
							"pmmpthread encountered an unknown non-copyable constant %s of type %s",
							ZSTR_VAL(zconstant->name),
							zend_zval_type_name(&zconstant->value)
						);
					}
					ZEND_CONSTANT_SET_FLAGS(&constant, ZEND_CONSTANT_FLAGS(zconstant), ZEND_CONSTANT_MODULE_NUMBER(zconstant));
					zend_register_constant(&constant);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_functions(const pmmpthread_ident_t* source) {
	zend_string *key, *name;
	zend_function *value = NULL, *prepared = NULL;

	ZEND_HASH_FOREACH_STR_KEY_PTR(PMMPTHREAD_CG(source->ls, function_table), key, value) {
		if (value->type == ZEND_INTERNAL_FUNCTION ||
			zend_hash_exists(CG(function_table), key))
			continue;

		name = pmmpthread_copy_string(key);
		prepared = pmmpthread_copy_function(source, value);

		if (!zend_hash_add_ptr(CG(function_table), name, prepared)) {
			destroy_op_array((zend_op_array*)prepared);
		}

		zend_string_release(name);
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_classes(const pmmpthread_ident_t* source) {
	zend_class_entry *entry;
	zend_string *name;

	ZEND_HASH_FOREACH_STR_KEY_PTR(PMMPTHREAD_CG(source->ls, class_table), name, entry) {
		if (!zend_hash_exists(CG(class_table), name) && ZSTR_VAL(name)[0] != '\0') {
			pmmpthread_create_entry(source, entry, 0);
		}
	} ZEND_HASH_FOREACH_END();

	pmmpthread_context_late_bindings(source);
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_includes(const pmmpthread_ident_t* source) {
	zend_string *file;
	ZEND_HASH_FOREACH_STR_KEY(&PMMPTHREAD_EG(source->ls, included_files), file) {
		zend_string *name = pmmpthread_copy_string(file);
		zend_hash_add_empty_element(&EG(included_files), name);
		zend_string_release(name);
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pmmpthread_prepare_sapi(const pmmpthread_ident_t* source) {
	SG(sapi_started) = 0;

	if (!(PMMPTHREAD_ZG(options) & PMMPTHREAD_ALLOW_HEADERS)) {
		SG(headers_sent)=1;
		SG(request_info).no_headers = 1;
	}
} /* }}} */

/* {{{ */
int pmmpthread_prepared_startup(pmmpthread_object_t* thread, pmmpthread_monitor_t *ready, zend_class_entry *thread_ce, zend_ulong thread_options) {

	PMMPTHREAD_PREPARATION_BEGIN_CRITICAL() {
		thread->local.id = pmmpthread_self();
		thread->local.ls = ts_resource(0);
		TSRMLS_CACHE_UPDATE();

		SG(server_context) =
			PMMPTHREAD_SG(thread->creator.ls, server_context);

		SG(request_info).argc = PMMPTHREAD_SG(thread->creator.ls, request_info).argc;

		SG(request_info).argv = PMMPTHREAD_SG(thread->creator.ls, request_info).argv;

		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;

		php_request_startup();
		PG(during_request_startup) = 0;

		zend_auto_global *auto_global;
		ZEND_HASH_FOREACH_PTR(CG(auto_globals), auto_global) {
			if (auto_global->jit && auto_global->armed) {
				if (auto_global->auto_global_callback) {
					//compiler would normally JIT these globals, but that won't happen for copied code
					//ideally we would do manual JIT on function copy, but this will work as a stopgap for now
					auto_global->armed = auto_global->auto_global_callback(auto_global->name);
				} else auto_global->armed = 0;
			}
		} ZEND_HASH_FOREACH_END();

		PMMPTHREAD_ZG(options) = thread_options;

		pmmpthread_prepare_sapi(&thread->creator);

		if (thread_options & PMMPTHREAD_INHERIT_INI)
			pmmpthread_prepare_ini(&thread->creator);

		if (thread_options & PMMPTHREAD_INHERIT_CONSTANTS)
			pmmpthread_prepare_constants(&thread->creator);

		zend_map_ptr_extend(PMMPTHREAD_CG(thread->creator.ls, map_ptr_last));

		if (thread_options & PMMPTHREAD_INHERIT_FUNCTIONS)
			pmmpthread_prepare_functions(&thread->creator);

		if (thread_options & PMMPTHREAD_INHERIT_CLASSES) {
			pmmpthread_prepare_classes(&thread->creator);
		} else {
			pmmpthread_create_entry(&thread->creator, thread_ce, 0);
			pmmpthread_context_late_bindings(&thread->creator);
		}

		if (thread_options & PMMPTHREAD_INHERIT_INCLUDES)
			pmmpthread_prepare_includes(&thread->creator);

		pmmpthread_monitor_add(ready, PMMPTHREAD_MONITOR_READY);
	} PMMPTHREAD_PREPARATION_END_CRITICAL();

	return SUCCESS;
} /* }}} */

/* {{{ Calls user shutdown functions before entering into the AWAIT_JOIN state.
* This is necessary to ensure that any object created by a shutdown handler can be recovered by the parent thread.
*
* This function replicates some logic from php_request_shutdown() as seen here: https://github.com/php/php-src/blob/PHP-8.0/main/main.c#L1781-L1842
* We call all the shutdown logic up to (but not including) zend_call_destructors.
*/
void pmmpthread_call_shutdown_functions(void) {
	EG(flags) |= EG_FLAGS_IN_SHUTDOWN;

	/* EG(current_execute_data) points into nirvana and therefore cannot be safely accessed
	 * inside zend_executor callback functions.
	 */
	EG(current_execute_data) = NULL;

	/* 0. Call any open observer end handlers that are still open after a zend_bailout */
	if (ZEND_OBSERVER_ENABLED) {
		zend_observer_fcall_end_all();
	}

	/* 1. Call all possible shutdown functions registered with register_shutdown_function() */
	if (PG(modules_activated)) {
		php_call_shutdown_functions();
	}

	/* 7. Free shutdown functions */
	//we don't want these to be called by php_request_shutdown() as we've already called them, so get rid of them here
	if (PG(modules_activated)) {
		php_free_shutdown_functions();
	}
} /* }}} */

/* {{{ */
int pmmpthread_prepared_shutdown(void) {
	PMMPTHREAD_PREPARATION_BEGIN_CRITICAL() {
		PG(report_memleaks) = 0;

		php_request_shutdown((void*)NULL);

		ts_free_thread();
	} PMMPTHREAD_PREPARATION_END_CRITICAL();

	return SUCCESS;
} /* }}} */

/* {{{ */
static zend_trait_alias * pmmpthread_preparation_copy_trait_alias(zend_trait_alias *alias) {
	zend_trait_alias *copy = ecalloc(1, sizeof(zend_trait_alias));

	pmmpthread_preparation_copy_trait_method_reference(&alias->trait_method, &copy->trait_method);

	if (alias->alias) {
		copy->alias = pmmpthread_copy_string(alias->alias);
	}

	copy->modifiers = alias->modifiers;

	return copy;
} /* }}} */

/* {{{ */
static zend_trait_precedence * pmmpthread_preparation_copy_trait_precedence(zend_trait_precedence *precedence) {
	zend_trait_precedence *copy = ecalloc(1, sizeof(zend_trait_precedence) + (precedence->num_excludes - 1) * sizeof(zend_string *));

	pmmpthread_preparation_copy_trait_method_reference(&precedence->trait_method, &copy->trait_method);
	copy->num_excludes = precedence->num_excludes;
	int i;
	for (i = 0; i < precedence->num_excludes; ++i) {
		copy->exclude_class_names[i] = pmmpthread_copy_string(precedence->exclude_class_names[i]);
	}

	return copy;
} /* }}} */

/* {{{ */
static void pmmpthread_preparation_copy_trait_method_reference(zend_trait_method_reference *reference, zend_trait_method_reference *copy) {
	if (reference->method_name) {
		copy->method_name = pmmpthread_copy_string(reference->method_name);
	}
	if (reference->class_name) {
		copy->class_name = pmmpthread_copy_string(reference->class_name);
	}
} /* }}} */
