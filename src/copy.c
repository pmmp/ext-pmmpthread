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
#include <src/object.h>
#include <src/prepare.h>

static HashTable* pthreads_copy_hash(const pthreads_ident_t* owner, HashTable* source);
static zend_ast_ref* pthreads_copy_ast(const pthreads_ident_t* owner, zend_ast* ast);
static void* pthreads_copy_ast_tree(const pthreads_ident_t* owner, zend_ast* ast, void* buf);

zend_string* pthreads_copy_string(zend_string* s) {
	zend_string* ret;
	if (ZSTR_IS_INTERNED(s)) {
		if (GC_FLAGS(s) & IS_STR_PERMANENT) { /* usually opcache SHM */
			return s;
		}

		//we can no longer risk sharing request-local interned strings in 8.1, because their CE_CACHE may be populated
		//and cause bad stuff to happen when opcache is not used. This sucks for memory usage, but we don't have a choice.
		ret = zend_new_interned_string(zend_string_init(ZSTR_VAL(s), ZSTR_LEN(s), GC_FLAGS(s) & IS_STR_PERSISTENT));
	}
	else {
		ret = zend_string_dup(s, GC_FLAGS(s) & IS_STR_PERSISTENT);
	}
	ZSTR_H(ret) = ZSTR_H(s);
	return ret;
}

int pthreads_copy_zval(const pthreads_ident_t* owner, zval* dest, zval* source) {
	if (Z_TYPE_P(source) == IS_INDIRECT)
		return pthreads_copy_zval(owner, dest, Z_INDIRECT_P(source));
	if (Z_TYPE_P(source) == IS_REFERENCE)
		return pthreads_copy_zval(owner, dest, &Z_REF_P(source)->val);

	int result = FAILURE;
	switch (Z_TYPE_P(source)) {
	case IS_NULL:
	case IS_TRUE:
	case IS_FALSE:
	case IS_LONG:
	case IS_DOUBLE:
		ZVAL_DUP(dest, source);
		result = SUCCESS;
		break;

	case IS_STRING:
		ZVAL_STR(dest, pthreads_copy_string(Z_STR_P(source)));
		result = SUCCESS;
		break;

	case IS_ARRAY:
		ZVAL_ARR(dest, pthreads_copy_hash(owner, Z_ARRVAL_P(source)));
		result = SUCCESS;
		break;

	case IS_OBJECT:
		if (instanceof_function(Z_OBJCE_P(source), pthreads_threaded_base_entry)) {
			pthreads_globals_object_connect(PTHREADS_FETCH_FROM(Z_OBJ_P(source)), NULL, dest);
			result = SUCCESS;
		}
		else if (instanceof_function(Z_OBJCE_P(source), zend_ce_closure)) {
			//no exceptions here - we're probably in a class copy context
			result = pthreads_copy_closure(owner, (zend_closure*) Z_OBJ_P(source), 1, dest);
		}
		break;

	case IS_CONSTANT_AST:
		ZVAL_AST(dest, pthreads_copy_ast(owner, GC_AST(Z_AST_P(source))));
		result = SUCCESS;
		break;
	default:
		result = FAILURE;
	}
	return result;
}

static HashTable* pthreads_copy_hash(const pthreads_ident_t* owner, HashTable* source) {
	zval newzval;

	zend_ulong h;
	zend_string* key;
	zval* val;

	//TODO: what about IS_ARRAY_IMMUTABLE?
	HashTable* ht = (HashTable*)pemalloc(sizeof(HashTable), GC_FLAGS(source) & IS_ARRAY_PERSISTENT);
	zend_hash_init(ht, source->nNumUsed, NULL, source->pDestructor, GC_FLAGS(source) & IS_ARRAY_PERSISTENT);

	ZEND_HASH_FOREACH_KEY_VAL(source, h, key, val) {
		if (pthreads_copy_zval(owner, &newzval, val) == FAILURE) {
			continue;
		}

		if (key) {
			zend_hash_update(ht, pthreads_copy_string(key), &newzval);
		}
		else {
			zend_hash_index_update(ht, h, &newzval);
		}
	} ZEND_HASH_FOREACH_END();

	return ht;
}

static inline size_t zend_ast_list_size(uint32_t children) {
	//this is an exact copy of zend_ast_list_size() in zend_ast.c, which we can't use because it's static :(
	return sizeof(zend_ast_list) - sizeof(zend_ast*) + sizeof(zend_ast*) * children;
}

static size_t zend_ast_tree_size(zend_ast* ast) {
	//this is an exact copy of zend_ast_tree_size() in zend_ast.c, which we can't use because it's static :(
	size_t size;

	if (ast->kind == ZEND_AST_ZVAL || ast->kind == ZEND_AST_CONSTANT) {
		size = sizeof(zend_ast_zval);
	}
	else if (zend_ast_is_list(ast)) {
		uint32_t i;
		zend_ast_list* list = zend_ast_get_list(ast);

		size = zend_ast_list_size(list->children);
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				size += zend_ast_tree_size(list->child[i]);
			}
		}
	}
	else {
		uint32_t i, children = zend_ast_get_num_children(ast);

		size = zend_ast_size(children);
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				size += zend_ast_tree_size(ast->child[i]);
			}
		}
	}
	return size;
}

static void* pthreads_copy_ast_tree(const pthreads_ident_t* owner, zend_ast* ast, void* buf)
{
	//this code is adapted from zend_ast_tree_copy() in zend_ast.c
	//sadly we have to duplicate all of this even though we only need to change a couple of lines

	if (ast->kind == ZEND_AST_ZVAL) {
		zend_ast_zval* new = (zend_ast_zval*)buf;
		new->kind = ZEND_AST_ZVAL;
		new->attr = ast->attr;
		pthreads_copy_zval(owner, &new->val, zend_ast_get_zval(ast)); //changed
		buf = (void*)((char*)buf + sizeof(zend_ast_zval));
	}
	else if (ast->kind == ZEND_AST_CONSTANT) {
		zend_ast_zval* new = (zend_ast_zval*)buf;
		new->kind = ZEND_AST_CONSTANT;
		new->attr = ast->attr;
		ZVAL_STR(&new->val, pthreads_copy_string(zend_ast_get_constant_name(ast))); //changed
		buf = (void*)((char*)buf + sizeof(zend_ast_zval));
	}
	else if (zend_ast_is_list(ast)) {
		zend_ast_list* list = zend_ast_get_list(ast);
		zend_ast_list* new = (zend_ast_list*)buf;
		uint32_t i;
		new->kind = list->kind;
		new->attr = list->attr;
		new->children = list->children;
		buf = (void*)((char*)buf + zend_ast_list_size(list->children));
		for (i = 0; i < list->children; i++) {
			if (list->child[i]) {
				new->child[i] = (zend_ast*)buf;
				buf = pthreads_copy_ast_tree(owner, list->child[i], buf); //changed
			}
			else {
				new->child[i] = NULL;
			}
		}
	}
	else {
		uint32_t i, children = zend_ast_get_num_children(ast);
		zend_ast* new = (zend_ast*)buf;
		new->kind = ast->kind;
		new->attr = ast->attr;
		buf = (void*)((char*)buf + zend_ast_size(children));
		for (i = 0; i < children; i++) {
			if (ast->child[i]) {
				new->child[i] = (zend_ast*)buf;
				buf = pthreads_copy_ast_tree(owner, ast->child[i], buf); //changed
			}
			else {
				new->child[i] = NULL;
			}
		}
	}
	return buf;
}

static zend_ast_ref* pthreads_copy_ast(const pthreads_ident_t* owner, zend_ast* ast) {
	//this code is adapted from zend_ast_copy() in zend_ast.c
	//sadly we have to duplicate all of this even though we only need to change a couple of lines

	size_t tree_size;
	zend_ast_ref* ref;

	ZEND_ASSERT(ast != NULL);
	tree_size = zend_ast_tree_size(ast) + sizeof(zend_ast_ref);
	ref = emalloc(tree_size);
	pthreads_copy_ast_tree(owner, ast, GC_AST(ref));
	GC_SET_REFCOUNT(ref, 1);
	GC_TYPE_INFO(ref) = GC_CONSTANT_AST;
	return ref;
}

static void pthreads_copy_attribute(const pthreads_ident_t* owner, HashTable **new, const zend_attribute *attr, zend_string *filename) {
	uint32_t i;
	zend_attribute *copy = zend_add_attribute(new, pthreads_copy_string(attr->name), attr->argc, attr->flags, attr->offset, attr->lineno);

	for (i = 0; i < attr->argc; i++) {
		if (pthreads_copy_zval(owner, &copy->args[i].value, &attr->args[i].value) == FAILURE) {
			//TODO: show a more useful error message - if we actually see this we're going to have no idea what code caused it
			zend_error_at_noreturn(
				E_CORE_ERROR,
				filename,
				attr->lineno,
				"pthreads encountered a non-copyable attribute argument %s of type %s",
				ZSTR_VAL(attr->name),
				zend_zval_type_name(&attr->args[i].value)
			);
		}
		copy->args[i].name = attr->args[i].name ? pthreads_copy_string(attr->args[i].name) : NULL;
	}
}

/* {{{ */
HashTable* pthreads_copy_attributes(const pthreads_ident_t* owner, HashTable *old, zend_string *filename) {
	if (!old) {
		return NULL;
	}

	zval *v;

	//zend_add_attribute() will allocate this for us with the correct flags and destructor
	HashTable *new = NULL;

	ZEND_HASH_FOREACH_VAL(old, v) {
		pthreads_copy_attribute(owner, &new, Z_PTR_P(v), filename);
	} ZEND_HASH_FOREACH_END();

	return new;
} /* }}} */

/* {{{ */
static HashTable* pthreads_copy_statics(const pthreads_ident_t* owner, HashTable *old) {
	HashTable *statics = NULL;

	if (old) {
		zend_string *key;
		zval *value;

		ALLOC_HASHTABLE(statics);
		zend_hash_init(statics,
			zend_hash_num_elements(old),
			NULL, ZVAL_PTR_DTOR, 0);

		ZEND_HASH_FOREACH_STR_KEY_VAL(old, key, value) {
			zend_string *name = pthreads_copy_string(key);
			zval *next = value;
			zval copy;
			while (Z_TYPE_P(next) == IS_REFERENCE)
				next = &Z_REF_P(next)->val;

			if (pthreads_copy_zval(owner, &copy, next) == SUCCESS) {
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
			pthreads_copy_string(old[it]);
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
static zval* pthreads_copy_literals(const pthreads_ident_t* owner, zval *old, int last, void *memory) {
	zval *literals = (zval*) memory;
	zval *literal = literals,
		 *end = literals + last;
	zval *old_literal = old;

	memcpy(memory, old, sizeof(zval) * last);
	while (literal < end) {
		if (pthreads_copy_zval(owner, literal, old_literal) == FAILURE) {
			ZEND_ASSERT(0); //literals should always be copyable
			ZVAL_NULL(literal);
		}
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
#if PHP_VERSION_ID < 80200
				case ZEND_JMPZNZ:
					/* relative extended_value don't have to be changed */
					/* break omitted intentionally */
#endif
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
		if (ZEND_TYPE_HAS_LIST(*single_type)) {
			pthreads_copy_zend_type(single_type, single_type);
		} else if (ZEND_TYPE_HAS_NAME(*single_type)) {
			ZEND_TYPE_SET_PTR(*single_type, pthreads_copy_string(ZEND_TYPE_NAME(*single_type)));
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
			info[it].name = pthreads_copy_string(old[it].name);

		pthreads_copy_zend_type(&old[it].type, &info[it].type);
		it++;
	}

	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		info++;
	}

	return info;
} /* }}} */

/* {{{ */
static zend_op_array** pthreads_copy_dynamic_func_defs(const pthreads_ident_t* owner, const zend_op_array** old, uint32_t num_dynamic_func_defs) {
	zend_op_array** new = (zend_op_array**) emalloc(num_dynamic_func_defs * sizeof(zend_op_array*));

	for (int i = 0; i < num_dynamic_func_defs; i++) {
		//assume this is OK?
		new[i] = (zend_op_array*) pthreads_copy_function(owner, old[i]);
	}

	return new;
} /* }}} */

/* {{{ */
static inline zend_function* pthreads_copy_user_function(const pthreads_ident_t* owner, const zend_function *function, zend_bool copy_static_variables) {
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

	op_array->function_name = pthreads_copy_string(op_array->function_name);
	/* we don't care about prototypes */
	op_array->prototype = NULL;
	if (function->op_array.refcount) { //refcount will be NULL if opcodes are allocated on SHM
		op_array->refcount = emalloc(sizeof(uint32_t));
		(*op_array->refcount) = 1;
	}

	/*
	 * we never want to share the same runtime cache
	 * since we're producing function definitions here, runtime cache should never be allocated on the heap
	 * we may be producing this definition from a closure's copy of the original definition, which might have used a heap-allocated cache
	 * heap runtime caches are only used by closures' copies of function definitions, not the definition itself
	 */
	op_array->fn_flags &= ~ZEND_ACC_HEAP_RT_CACHE;
#if PHP_VERSION_ID >= 80200
	ZEND_MAP_PTR_INIT(op_array->run_time_cache, NULL);
#else
	ZEND_MAP_PTR_INIT(op_array->run_time_cache, zend_arena_alloc(&CG(arena), sizeof(void*)));
	ZEND_MAP_PTR_SET(op_array->run_time_cache, NULL);
#endif

	if (op_array->doc_comment) {
		op_array->doc_comment = pthreads_copy_string(op_array->doc_comment);
	}

	if (!(filename_copy = zend_hash_find_ptr(&PTHREADS_ZG(filenames), op_array->filename))) {
		filename_copy = pthreads_copy_string(op_array->filename);
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

		if (op_array->literals) op_array->literals = pthreads_copy_literals (owner, literals, op_array->last_literal, literals_memory);

		op_array->opcodes = pthreads_copy_opcodes(op_array, literals, opcodes_memory);

		if (op_array->arg_info) 	op_array->arg_info = pthreads_copy_arginfo(op_array, arg_info, op_array->num_args);
		if (op_array->live_range)		op_array->live_range = pthreads_copy_live(op_array->live_range, op_array->last_live_range);
		if (op_array->try_catch_array)  op_array->try_catch_array = pthreads_copy_try(op_array->try_catch_array, op_array->last_try_catch);
		if (op_array->vars) 		op_array->vars = pthreads_copy_variables(variables, op_array->last_var);
		if (op_array->attributes) op_array->attributes = pthreads_copy_attributes(owner, op_array->attributes, op_array->filename);

		if (op_array->num_dynamic_func_defs) op_array->dynamic_func_defs = pthreads_copy_dynamic_func_defs(owner, op_array->dynamic_func_defs, op_array->num_dynamic_func_defs);
	}

	if (copy_static_variables && op_array->static_variables) {
		op_array->static_variables = pthreads_copy_statics(owner, op_array->static_variables);
	} else {
		op_array->static_variables = NULL;
	}
#if PHP_VERSION_ID >= 80200
	ZEND_MAP_PTR_INIT(op_array->static_variables_ptr, NULL);
#else
	ZEND_MAP_PTR_INIT(op_array->static_variables_ptr, &op_array->static_variables);
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

static inline void pthreads_add_function_ref(zend_function* function) {
	if (function) {
		if (function->type == ZEND_USER_FUNCTION && function->op_array.refcount) {
			(*function->op_array.refcount)++;
		}
		zend_string_addref(function->op_array.function_name);
	}
}

/* {{{ */
static zend_function* pthreads_copy_function_ex(const pthreads_ident_t* owner, const zend_function* function, zend_bool copy_static_variables) {
	zend_function *copy;

	if (function->common.fn_flags & ZEND_ACC_IMMUTABLE) {
		ZEND_ASSERT(function->type == ZEND_USER_FUNCTION);
		return function;
	}

	if (!(function->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
		copy = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong)function);
	} else {
		ZEND_ASSERT(function->type == ZEND_USER_FUNCTION);
		copy = zend_hash_index_find_ptr(&PTHREADS_ZG(closure_base_op_arrays), (zend_ulong)function->op_array.opcodes);
	}

	if (copy) {
		pthreads_add_function_ref(copy);
		return copy;
	}

	if (function->type == ZEND_USER_FUNCTION) {
		copy = pthreads_copy_user_function(owner, function, copy_static_variables);
	} else {
		copy = pthreads_copy_internal_function(function);
	}

	if (function->op_array.fn_flags & ZEND_ACC_CLOSURE) {
		//closure objects copy their base zend_function, but don't copy the opcodes
		//this means we can use the opcodes to identify the base op_array a closure should use for zend_create_closure()
		//instead of creating a bunch of useless copies
		zend_hash_index_add_ptr(&PTHREADS_ZG(closure_base_op_arrays), (zend_ulong)function->op_array.opcodes, copy);

		//this may be the only place the definition is referenced from - make sure it doesn't get destroyed
		pthreads_add_function_ref(copy);
	} else {
		zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong)function, copy);
	}

	return copy;
} /* }}} */

/* {{{ */
zend_function* pthreads_copy_function(const pthreads_ident_t* owner, const zend_function* function) {
	return pthreads_copy_function_ex(owner, function, 1);
} /* }}} */

/* {{{ */
zend_result pthreads_copy_closure(const pthreads_ident_t* owner, zend_closure* closure_obj, zend_bool silent, zval *pzval) {
	char* name;
	size_t name_len;
	zend_string* zname;
	zend_function* func_def = NULL;
	zval this_zv;

	if (IS_PTHREADS_OBJECT(&closure_obj->this_ptr)) {
		if (!pthreads_globals_object_connect(PTHREADS_FETCH_FROM(Z_OBJ(closure_obj->this_ptr)), NULL, &this_zv)) {
			if (!silent) {
				zend_throw_exception_ex(
					pthreads_ce_ThreadedConnectionException, 0,
					"Closure $this references a thread-safe object from another thread which no longer exists");
			}
			return FAILURE;
		}
	} else {
		ZVAL_UNDEF(&this_zv);
	}

	if (closure_obj->func.common.fn_flags & ZEND_ACC_FAKE_CLOSURE) {
		//fake closures behave as a pointer to another function, and share the original function's static variables
		//they may also point to other, real closures, but we can't do much about that case.
		zend_class_entry* origin_class = NULL;

		if (closure_obj->called_scope != NULL) {
			origin_class = closure_obj->called_scope;
		} else if (closure_obj->func.common.scope != NULL) {
			origin_class = closure_obj->func.common.scope;
		}

		if (origin_class != NULL) {
			zend_class_entry* local_class = pthreads_prepare_single_class(owner, origin_class);
			func_def = (zend_function*)zend_hash_find_ptr(&local_class->function_table, closure_obj->func.common.function_name);

			if (func_def == NULL) {
				//this could happen if a different version of the class was autoloaded onto this thread
				if (!silent) {
					zend_throw_exception_ex(
						pthreads_ce_ThreadedConnectionException,
						0,
						"First-class callable references an unknown class method %s::%s()",
						ZSTR_VAL(local_class->name),
						ZSTR_VAL(closure_obj->func.common.function_name)
					);
				}

				return FAILURE;
			}
		} else {
			func_def = (zend_function*)zend_hash_find_ptr(EG(function_table), closure_obj->func.common.function_name);

			if (func_def == NULL) {
				//the referenced function isn't available in the local EG(function_table)
				//we can't add this closure to EG(function_table) because it might have fake static variables (uses) and we
				//can't tell them apart from real static variables, so we don't know which to remove
				if (!silent) {
					zend_throw_exception_ex(
						pthreads_ce_ThreadedConnectionException,
						0,
						"First-class callable references an unknown function %s()",
						ZSTR_VAL(closure_obj->func.common.function_name)
					);
				}
				return FAILURE;
			}
		}

		zend_create_fake_closure(
			pzval,
			func_def,
			pthreads_prepare_single_class(owner, closure_obj->func.common.scope),
			pthreads_prepare_single_class(owner, closure_obj->called_scope),
			&this_zv
		);
	} else {
		HashTable* static_variables = NULL;
		if (closure_obj->func.type == ZEND_USER_FUNCTION && closure_obj->func.op_array.static_variables != NULL) {
			//if this is a real closure, we need to update the static_variables from the original closure object
			//so that the copied closure has the correct use()d variables
			//this should not modify the original closure
			//these have to be copied before the closure is created, to maintain the original behaviour
			static_variables = pthreads_copy_statics(owner, closure_obj->func.op_array.static_variables);
		}

		//we don't know where the definition for this closure is, so create a definition from this copy of it
		//assume there are no real static variables and don't copy them from this definition (thread-safe
		//closures are not allowed to have static variables anyway)
		func_def = pthreads_copy_function_ex(owner, &closure_obj->func, 0);

		zend_create_closure(
			pzval,
			func_def,
			pthreads_prepare_single_class(owner, closure_obj->func.common.scope),
			pthreads_prepare_single_class(owner, closure_obj->called_scope),
			&this_zv
		);
		if (static_variables != NULL) {
			zend_closure* new_closure = (zend_closure*)Z_OBJ_P(pzval);
			ZEND_ASSERT(new_closure->func.type == ZEND_USER_FUNCTION);
			if (new_closure->func.op_array.static_variables != NULL) {
				//the closure may have static_variables allocated from its original creation by zend_compile.c
				zend_array_release(new_closure->func.op_array.static_variables);
			}
			new_closure->func.op_array.static_variables = static_variables;
		}

		//pthreads_copy_function() adds a ref to any cached function returned - make sure we don't leak the original definition
		destroy_zend_function(func_def);
	}

	zval_ptr_dtor(&this_zv);

	return SUCCESS;
} /* }}} */
