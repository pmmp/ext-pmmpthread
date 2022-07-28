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

#include <src/copy.h>

static void pthreads_copy_attribute(HashTable **new, const zend_attribute *attr, zend_string *filename) {
	uint32_t i;
	zend_attribute *copy = zend_add_attribute(new, zend_string_new(attr->name), attr->argc, attr->flags, attr->offset, attr->lineno);

	for (i = 0; i < attr->argc; i++) {
		if (pthreads_store_separate(&attr->args[i].value, &copy->args[i].value) == FAILURE) {
			//TODO: show a more useful error message - if we actually see this we're going to have no idea what code caused it
			zend_error_at_noreturn(
				E_CORE_ERROR,
				filename,
				attr->lineno,
				"pthreads encountered a non-copyable attribute argument %s of type %s",
				ZSTR_VAL(attr->name),
				zend_get_type_by_const(Z_TYPE(attr->args[i].value))
			);
		}
		copy->args[i].name = attr->args[i].name ? zend_string_new(attr->args[i].name) : NULL;
	}
}

/* {{{ */
HashTable* pthreads_copy_attributes(HashTable *old, zend_string *filename) {
	if (!old) {
		return NULL;
	}

	zval *v;

	//zend_add_attribute() will allocate this for us with the correct flags and destructor
	HashTable *new = NULL;

	ZEND_HASH_FOREACH_VAL(old, v) {
		pthreads_copy_attribute(&new, Z_PTR_P(v), filename);
	} ZEND_HASH_FOREACH_END();

	return new;
} /* }}} */

/* {{{ */
static HashTable* pthreads_copy_statics(HashTable *old) {
	HashTable *statics = NULL;

	if (old) {
		zend_string *key;
		zval *value;

		ALLOC_HASHTABLE(statics);
		zend_hash_init(statics,
			zend_hash_num_elements(old),
			NULL, ZVAL_PTR_DTOR, 0);

		ZEND_HASH_FOREACH_STR_KEY_VAL(old, key, value) {
			zend_string *name = zend_string_new(key);
			zval *next = value;
			zval copy;
			while (Z_TYPE_P(next) == IS_REFERENCE)
				next = &Z_REF_P(next)->val;

			if (pthreads_store_separate(next, &copy) == SUCCESS) {
				zend_hash_add(statics, name, &copy);
			} else {
				zend_hash_add_empty_element(statics, name);
			}
			zend_string_release(name);
		} ZEND_HASH_FOREACH_END();
	}

	return statics;
} /* }}} */

/* {{{ */
static zend_string** pthreads_copy_variables(zend_string **old, int end) {
	zend_string **variables = safe_emalloc(end, sizeof(zend_string*), 0);
	int it = 0;

	while (it < end) {
		variables[it] =
			zend_string_new(old[it]);
		it++;
	}

	return variables;
} /* }}} */

/* {{{ */
static zend_try_catch_element* pthreads_copy_try(zend_try_catch_element *old, int end) {
	zend_try_catch_element *try_catch = safe_emalloc(end, sizeof(zend_try_catch_element), 0);

	memcpy(
		try_catch,
		old,
		sizeof(zend_try_catch_element) * end);

	return try_catch;
} /* }}} */

/* {{{ */
static zend_live_range* pthreads_copy_live(zend_live_range *old, int end) {
	zend_live_range *range = safe_emalloc(end, sizeof(zend_live_range), 0);

	memcpy(
		range,
		old,
		sizeof(zend_live_range) * end);

	return range;
} /* }}} */

/* {{{ */
static zval* pthreads_copy_literals(zval *old, int last, void *memory) {
	zval *literals = (zval*) memory;
	zval *literal = literals,
		 *end = literals + last;
	zval *old_literal = old;

	memcpy(memory, old, sizeof(zval) * last);
	while (literal < end) {
		pthreads_store_separate(old_literal, literal);
		old_literal++;
		literal++;
	}

	return literals;
} /* }}} */

/* {{{ */
static zend_op* pthreads_copy_opcodes(zend_op_array *op_array, zval *literals, void *memory) {
	zend_op *copy = memory;
	memcpy(copy, op_array->opcodes, sizeof(zend_op) * op_array->last);

	/* The following code comes from ext/opcache/zend_persist.c */
	zend_op *opline = copy;
	zend_op *end	= copy + op_array->last;

	for (; opline < end; opline++) {
#if ZEND_USE_ABS_CONST_ADDR
			if (opline->op1_type == IS_CONST) {
				opline->op1.zv = (zval*)((char*)opline->op1.zv + ((char*)op_array->literals - (char*)literals));
				if (opline->opcode == ZEND_SEND_VAL
				 || opline->opcode == ZEND_SEND_VAL_EX
				 || opline->opcode == ZEND_QM_ASSIGN) {
					/* Update handlers to eliminate REFCOUNTED check */
					zend_vm_set_opcode_handler_ex(opline, 1 << Z_TYPE_P(opline->op1.zv), 0, 0);
				}
			}
			if (opline->op2_type == IS_CONST) {
				opline->op2.zv = (zval*)((char*)opline->op2.zv + ((char*)op_array->literals - (char*)literals));
			}
#else
			if (opline->op1_type == IS_CONST) {
				opline->op1.constant =
					(char*)(op_array->literals +
						((zval*)((char*)(op_array->opcodes + (opline - copy)) +
						(int32_t)opline->op1.constant) - literals)) -
					(char*)opline;
				if (opline->opcode == ZEND_SEND_VAL
				 || opline->opcode == ZEND_SEND_VAL_EX
				 || opline->opcode == ZEND_QM_ASSIGN) {
					zend_vm_set_opcode_handler_ex(opline, 0, 0, 0);
				}
			}
			if (opline->op2_type == IS_CONST) {
				opline->op2.constant =
					(char*)(op_array->literals +
						((zval*)((char*)(op_array->opcodes + (opline - copy)) +
						(int32_t)opline->op2.constant) - literals)) -
					(char*)opline;
			}
#endif
#if ZEND_USE_ABS_JMP_ADDR
		if (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO) {
			/* fix jumps to point to new array */
			switch (opline->opcode) {
				case ZEND_JMP:
				case ZEND_FAST_CALL:
					opline->op1.jmp_addr = &copy[opline->op1.jmp_addr - op_array->opcodes];
					break;
				case ZEND_JMPZNZ:
					/* relative extended_value don't have to be changed */
					/* break omitted intentionally */
				case ZEND_JMPZ:
				case ZEND_JMPNZ:
				case ZEND_JMPZ_EX:
				case ZEND_JMPNZ_EX:
				case ZEND_JMP_SET:
				case ZEND_COALESCE:
				case ZEND_FE_RESET_R:
				case ZEND_FE_RESET_RW:
				case ZEND_ASSERT_CHECK:
				case ZEND_JMP_NULL:
					opline->op2.jmp_addr = &copy[opline->op2.jmp_addr - op_array->opcodes];
					break;
				case ZEND_CATCH:
					if (!(opline->extended_value & ZEND_LAST_CATCH)) {
						opline->op2.jmp_addr = &copy[opline->op2.jmp_addr - op_array->opcodes];
					}
					break;
				case ZEND_FE_FETCH_R:
				case ZEND_FE_FETCH_RW:
				case ZEND_SWITCH_LONG:
				case ZEND_SWITCH_STRING:
				case ZEND_MATCH:
					/* relative extended_value don't have to be changed */
					break;
			}
		}
#endif
	}

	return copy;
} /* }}} */

/* {{{ */
static void pthreads_copy_zend_type(const zend_type *old_type, zend_type *new_type) {
	memcpy(new_type, old_type, sizeof(zend_type));

	//This code is based on zend_persist_type() in ext/opcache/zend_persist.c
	if (ZEND_TYPE_HAS_LIST(*old_type)) {
		const zend_type_list *old_list = ZEND_TYPE_LIST(*old_type);
		zend_type_list *new_list;
		if (ZEND_TYPE_USES_ARENA(*old_type)) {
			new_list = zend_arena_alloc(&CG(arena), ZEND_TYPE_LIST_SIZE(old_list->num_types));
		} else {
			new_list = emalloc(ZEND_TYPE_LIST_SIZE(old_list->num_types));
		}
		memcpy(new_list, old_list, ZEND_TYPE_LIST_SIZE(old_list->num_types));
		ZEND_TYPE_SET_PTR(*new_type, new_list);
	}

	zend_type *single_type;
	ZEND_TYPE_FOREACH(*new_type, single_type) {
		if (ZEND_TYPE_HAS_NAME(*single_type)) {
			ZEND_TYPE_SET_PTR(*single_type, zend_string_new(ZEND_TYPE_NAME(*single_type)));
		}
	} ZEND_TYPE_FOREACH_END();
} /* }}} */

/* {{{ */
static zend_arg_info* pthreads_copy_arginfo(zend_op_array *op_array, zend_arg_info *old, uint32_t end) {
	zend_arg_info *info;
	uint32_t it = 0;

	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		old--;
		end++;
	}

	if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
		end++;
	}

	info = safe_emalloc
		(end, sizeof(zend_arg_info), 0);
	memcpy(info, old, sizeof(zend_arg_info) * end);

	while (it < end) {
		if (info[it].name)
			info[it].name = zend_string_new(old[it].name);

		pthreads_copy_zend_type(&old[it].type, &info[it].type);
		it++;
	}

	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		info++;
	}

	return info;
} /* }}} */

#if PHP_VERSION_ID >= 80100
/* {{{ */
static zend_op_array** pthreads_copy_dynamic_func_defs(const zend_op_array** old, uint32_t num_dynamic_func_defs) {
	zend_op_array** new = (zend_op_array**) emalloc(num_dynamic_func_defs * sizeof(zend_op_array*));

	for (int i = 0; i < num_dynamic_func_defs; i++) {
		//assume this is OK?
		new[i] = (zend_op_array*) pthreads_copy_function(old[i]);
	}

	return new;
} /* }}} */
#endif

/* {{{ */
static inline zend_function* pthreads_copy_user_function(const zend_function *function) {
	zend_function  *copy;
	zend_op_array  *op_array;
	zend_string   **variables, *filename_copy;
	zval           *literals;
	zend_arg_info  *arg_info;

	copy = (zend_function*)
		zend_arena_alloc(&CG(arena), sizeof(zend_op_array));
	memcpy(copy, function, sizeof(zend_op_array));

	op_array = &copy->op_array;
	variables = op_array->vars;
	literals = op_array->literals;
	arg_info = op_array->arg_info;

	op_array->function_name = zend_string_new(op_array->function_name);
	/* we don't care about prototypes */
	op_array->prototype = NULL;
	if (function->op_array.refcount) { //refcount will be NULL if opcodes are allocated on SHM
		op_array->refcount = emalloc(sizeof(uint32_t));
		(*op_array->refcount) = 1;
	}
	/* we never want to share the same runtime cache */
	if (op_array->fn_flags & ZEND_ACC_HEAP_RT_CACHE) {
		//TODO: we really shouldn't need to initialize this here, but right now it's the most convenient way to do it.
		//the primary problem is zend_create_closure(), which doesn't like being given an op_array that has a NULL
		//map_ptr. However, when allocated on heap, Zend expects the map ptr and the runtime cache to be part of the
		//same contiguous memory block freed with a single call to efree(), and the cache won't be resized.
		void *ptr = ecalloc(1, sizeof(void*) + op_array->cache_size);
		ZEND_MAP_PTR_INIT(op_array->run_time_cache, ptr);
		ptr = (char*)ptr + sizeof(void*);
		ZEND_MAP_PTR_SET(op_array->run_time_cache, ptr);
	} else {
		ZEND_MAP_PTR_INIT(op_array->run_time_cache, zend_arena_alloc(&CG(arena), sizeof(void*)));
		ZEND_MAP_PTR_SET(op_array->run_time_cache, NULL);
	}

	if (op_array->doc_comment) {
		op_array->doc_comment = zend_string_new(op_array->doc_comment);
	}

	if (!(filename_copy = zend_hash_find_ptr(&PTHREADS_ZG(filenames), op_array->filename))) {
		filename_copy = zend_string_new(op_array->filename);
		zend_hash_add_ptr(&PTHREADS_ZG(filenames), filename_copy, filename_copy);
		zend_string_release(filename_copy);
	}
	//php/php-src@7620ea15807a84e76cb1cb2f9d5234ea787aae2e - filenames are no longer always interned
	//opcache might intern them, but in the absence of opcache this won't be the case
	//if this string is interned, the following will be a no-op
	zend_string_addref(filename_copy);

	op_array->filename = filename_copy;

	if (op_array->refcount) {
		//NULL refcount means this op_array's parts are allocated on SHM, don't mess with it
		//sometimes opcache caches part of an op_array without marking it as immutable
		//in these cases we can (and should) use the opcache version directly without copying it
		void *opcodes_memory;
		void *literals_memory = NULL;
#if !ZEND_USE_ABS_CONST_ADDR
		if(op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO){
			opcodes_memory = emalloc(ZEND_MM_ALIGNED_SIZE_EX(sizeof (zend_op) * op_array->last, 16) + sizeof (zval) * op_array->last_literal);
			if (op_array->literals) {
				literals_memory = ((char*) opcodes_memory) + ZEND_MM_ALIGNED_SIZE_EX(sizeof (zend_op) * op_array->last, 16);
			}
		} else {
#endif
			opcodes_memory = safe_emalloc(op_array->last, sizeof(zend_op), 0);
			if(op_array->literals) {
				literals_memory = safe_emalloc(op_array->last_literal, sizeof(zval), 0);
			}
#if !ZEND_USE_ABS_CONST_ADDR
		}
#endif

		if (op_array->literals) op_array->literals = pthreads_copy_literals (literals, op_array->last_literal, literals_memory);

		op_array->opcodes = pthreads_copy_opcodes(op_array, literals, opcodes_memory);

		if (op_array->arg_info) 	op_array->arg_info = pthreads_copy_arginfo(op_array, arg_info, op_array->num_args);
		if (op_array->live_range)		op_array->live_range = pthreads_copy_live(op_array->live_range, op_array->last_live_range);
		if (op_array->try_catch_array)  op_array->try_catch_array = pthreads_copy_try(op_array->try_catch_array, op_array->last_try_catch);
		if (op_array->vars) 		op_array->vars = pthreads_copy_variables(variables, op_array->last_var);
		if (op_array->attributes) op_array->attributes = pthreads_copy_attributes(op_array->attributes, op_array->filename);
	}

	//closures realloc static vars even if they were already persisted, so they always have to be copied (I guess for use()?)
	//TODO: we should be able to avoid copying this in some cases (sometimes already persisted by opcache, check GC_COLLECTABLE)
	if (op_array->static_variables) op_array->static_variables = pthreads_copy_statics(op_array->static_variables);
	ZEND_MAP_PTR_INIT(op_array->static_variables_ptr, &op_array->static_variables);

#if PHP_VERSION_ID >= 80100
	if (op_array->num_dynamic_func_defs) op_array->dynamic_func_defs = pthreads_copy_dynamic_func_defs(op_array->dynamic_func_defs, op_array->num_dynamic_func_defs);
#endif

	return copy;
} /* }}} */

/* {{{ */
static inline zend_function* pthreads_copy_internal_function(const zend_function *function) {
	zend_function *copy = zend_arena_alloc(&CG(arena), sizeof(zend_internal_function));
	memcpy(copy, function, sizeof(zend_internal_function));
	copy->common.fn_flags |= ZEND_ACC_ARENA_ALLOCATED;
	return copy;
} /* }}} */

/* {{{ */
zend_function* pthreads_copy_function(const zend_function *function) {
	zend_function *copy;

	if (function->common.fn_flags & ZEND_ACC_IMMUTABLE) {
		ZEND_ASSERT(function->type == ZEND_USER_FUNCTION);
		return function;
	}

	if (!(function->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
		copy = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong)function);

		if (copy) {
			if (copy->type == ZEND_USER_FUNCTION && copy->op_array.refcount) {
				(*copy->op_array.refcount)++;
			}
			//TODO: I think we're actually supposed to dup the entire structure (see zend_inheritance.c zend_duplicate_function)
			//the only time functions usually get reused is for inheritance and we're not generally supposed to reuse the actual
			//structure for that, just its members ...
			zend_string_addref(copy->op_array.function_name);
			return copy;
		}
	}

	if (function->type == ZEND_USER_FUNCTION) {
		copy = pthreads_copy_user_function(function);
	} else {
		copy = pthreads_copy_internal_function(function);
	}

	if (function->op_array.fn_flags & ZEND_ACC_CLOSURE) { //don't cache closures
		return copy;
	}

	return zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong) function, copy);
} /* }}} */

