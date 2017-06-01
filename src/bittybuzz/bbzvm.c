#include "bbzvm.h"
#include "bbztype.h"

/****************************************/
/****************************************/

//bbzvm_lsyms_t bbzvm_lsyms_new(uint8_t isswarm, bbzdarray_t syms) {
    // TODO
//}

/****************************************/
/****************************************/

//void bbzvm_lsyms_destroy(uint32_t pos, void*  void* params) {
    // TODO
//}

/****************************************/
/****************************************/

void bbzvm_process_inmsgs() {
    // TODO
}

/****************************************/
/****************************************/

void bbzvm_process_outmsgs() {
    // TODO
}

/****************************************/
/****************************************/

void bbzvm_construct(bbzvm_rid_t robot) {

    vm->bcode_fetch_fun = NULL;
    vm->bcode_size = 0;
    vm->pc = 0;
    vm->state = BBZVM_STATE_NOCODE;
    vm->error = BBZVM_ERROR_NONE;
    vm->error_notifier_fun = NULL;

    // Setup the heap
    bbzheap_clear();

    // Allocate singleton objects
    bbzheap_obj_alloc(BBZTYPE_NIL, &vm->nil);
    bbzdarray_new(&vm->dflt_actrec);
    bbzdarray_push(vm->dflt_actrec, vm->nil);

    vm->flist = 0;
    
    // Create various arrays
    bbzdarray_new(&vm->flist);

    // Create global symbols table
    bbzheap_obj_alloc(BBZTYPE_TABLE, &vm->gsyms);

    // Setup stack
    vm->stackptr = -1;
    vm->blockptr = vm->stackptr;

    // Set up other variables...
    vm->lsyms = 0;


    vm->robot = robot;
}

/****************************************/
/****************************************/

void bbzvm_destruct() {
    // Destroy heap ;
    // everything else is destroyed along with it.
    bbzheap_clear();
}

/****************************************/
/****************************************/

void bbzvm_seterror(bbzvm_error errcode) {
    // Set the error
    vm->state = BBZVM_STATE_ERROR;
    vm->error = errcode;
    // Call the user's notifier function.
    if (vm->error_notifier_fun) {
        (*vm->error_notifier_fun)(errcode);
    }
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_set_bcode(bbzvm_bcode_fetch_fun bcode_fetch_fun, uint16_t bcode_size) {
    // 1) Reset the VM
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZVM_ERROR_NONE;
    vm->pc = 0;

    // 2) Set the bytecode
    vm->bcode_fetch_fun = bcode_fetch_fun;
    vm->bcode_size = bcode_size;

    // 3) Register global strings
    uint16_t strCount = *vm->bcode_fetch_fun(0, sizeof(uint16_t));
    vm->pc = sizeof(uint16_t);

    // 4) Register Buzz's built-in functions
    while(*vm->bcode_fetch_fun(vm->pc, sizeof(uint8_t)) != BBZVM_INSTR_NOP) {
        if(bbzvm_step() != BBZVM_STATE_READY) return vm->state;
    }

    bbzvm_step();

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { bbzvm_seterror(BBZVM_ERROR_PC); return BBZVM_STATE_ERROR; }

#define inc_pc() assert_pc(vm->pc); ++vm->pc;

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)vm->bcode_fetch_fun(vm->pc, sizeof(TYPE))); vm->pc += sizeof(TYPE);

uint8_t bbzvm_gc() {
    if (BBZSTACK_SIZE - bbzvm_stack_size() < 5) return 0;
    bbzvm_push(vm->lsyms);
    bbzvm_push(vm->gsyms);
    bbzvm_push(vm->nil);
    bbzvm_push(vm->dflt_actrec);
    bbzvm_push(vm->flist);
    bbzheap_gc(vm->stack, bbzvm_stack_size());
    for (int8_t i = 5; i > 0; --i) {
        bbzvm_pop();
    }
    return 1;
}

/**
 * @brief Executes a single Buzz instruction.
 * @param[in,out] vm The VM.
 */
__attribute__((always_inline)) static inline
bbzvm_state bbzvm_exec_instr() {
    int16_t instrOffset = vm->pc; // Save PC in case of error or DONE.

    uint8_t instr = *(*vm->bcode_fetch_fun)(vm->pc, 1);
    #ifdef DEBUG
    vm->dbg_pc = vm->pc;
    vm->instr = instr;
    #endif
    inc_pc();
    switch(instr) {
        case BBZVM_INSTR_NOP: {
            break;
        }
        case BBZVM_INSTR_DONE: {
            bbzvm_done();
            break;
        }
        case BBZVM_INSTR_PUSHNIL: {
            bbzvm_pushnil();
            break;
        }
        case BBZVM_INSTR_DUP: {
            bbzvm_dup();
            break;
        }
        case BBZVM_INSTR_POP: {
            bbzvm_pop();
            break;
        }
        case BBZVM_INSTR_RET0: {
            bbzvm_ret0();
            if (vm->state == BBZVM_STATE_READY) {
                assert_pc(vm->pc);
            }
            break;
        }
        case BBZVM_INSTR_RET1: {
            bbzvm_ret1();
            if (vm->state == BBZVM_STATE_READY) {
                assert_pc(vm->pc);
            }
            break;
        }
        case BBZVM_INSTR_ADD: {
            bbzvm_add();
            break;
        }
        case BBZVM_INSTR_SUB: {
            bbzvm_sub();
            break;
        }
        case BBZVM_INSTR_MUL: {
            bbzvm_mul();
            break;
        }
        case BBZVM_INSTR_DIV: {
            bbzvm_div();
            break;
        }
        case BBZVM_INSTR_MOD: {
            bbzvm_mod();
            break;
        }
        case BBZVM_INSTR_POW: {
            bbzvm_pow();
            break;
        }
        case BBZVM_INSTR_UNM: {
            bbzvm_unm();
            break;
        }
        case BBZVM_INSTR_AND: {
            bbzvm_and();
            break;
        }
        case BBZVM_INSTR_OR: {
            bbzvm_or();
            break;
        }
        case BBZVM_INSTR_NOT: {
            bbzvm_not();
            break;
        }
        case BBZVM_INSTR_EQ: {
            bbzvm_eq();
            break;
        }
        case BBZVM_INSTR_NEQ: {
            bbzvm_neq();
            break;
        }
        case BBZVM_INSTR_GT: {
            bbzvm_gt();
            break;
        }
        case BBZVM_INSTR_GTE: {
            bbzvm_gte();
            break;
        }
        case BBZVM_INSTR_LT: {
            bbzvm_lt();
            break;
        }
        case BBZVM_INSTR_LTE: {
            bbzvm_lte();
            break;
        }
        case BBZVM_INSTR_GLOAD: {
            bbzvm_gload();
            break;
        }
        case BBZVM_INSTR_GSTORE: {
            bbzvm_gstore();
            break;
        }
        case BBZVM_INSTR_PUSHT: {
            bbzvm_pusht();
            break;
        }
        case BBZVM_INSTR_TPUT: {
            bbzvm_tput();
            break;
        }
        case BBZVM_INSTR_TGET: {
            bbzvm_tget();
            break;
        }
        case BBZVM_INSTR_CALLC: {
            bbzvm_callc();
            if (vm->state == BBZVM_STATE_READY) {
                assert_pc(vm->pc);
            }
            break;
        }
        case BBZVM_INSTR_CALLS: {
            bbzvm_calls();
            if (vm->state == BBZVM_STATE_READY) {
                assert_pc(vm->pc);
            }
            break;
        }
        case BBZVM_INSTR_PUSHF: {
            get_arg(bbzfloat);
            bbzvm_pushf(arg);
            break;
        }
        case BBZVM_INSTR_PUSHI: {
            get_arg(int16_t);
            bbzvm_pushi(arg);
            break;
        }
        case BBZVM_INSTR_PUSHS: {
            get_arg(uint16_t);
            bbzvm_pushs(arg);
            break;
        }
        case BBZVM_INSTR_PUSHCN: {
            get_arg(uint16_t);
            bbzvm_pushcn(arg);
            break;
        }
        case BBZVM_INSTR_PUSHCC: { // FIXME I don't think that a buzz script should/would ever use this instruction... Neither is it used in the buzz parser.
            get_arg(int16_t);
            bbzvm_pushcc((bbzvm_funp)(intptr_t)arg);
            break;
        }
        case BBZVM_INSTR_PUSHL: {
            get_arg(uint16_t);
            bbzvm_pushl(arg);
            break;
        }
        case BBZVM_INSTR_LLOAD: {
            get_arg(uint16_t);
            bbzvm_lload(arg);
            break;
        }
        case BBZVM_INSTR_LSTORE: {
            get_arg(uint16_t);
            bbzvm_lstore(arg);
            break;
        }
        case BBZVM_INSTR_JUMP: {
            get_arg(uint16_t);
            bbzvm_jump(arg);
            break;
        }
        case BBZVM_INSTR_JUMPZ: {
            get_arg(uint16_t);
            bbzvm_jumpz(arg);
            break;
        }
        case BBZVM_INSTR_JUMPNZ: {
            get_arg(uint16_t);
            bbzvm_jumpnz(arg);
            break;
        }
        default:
            bbzvm_seterror(BBZVM_ERROR_INSTR);
            break;
    }

    if (vm->state != BBZVM_STATE_READY) {
        // Stay on the instruction that caused the error,
        // or, in the case of BBZVM_INSTR_DONE, loop on it.
        vm->pc = instrOffset;
    }

    return vm->state;
}

bbzvm_state bbzvm_step() {
    if(vm->state == BBZVM_STATE_READY) {
        if(!bbzvm_gc()) {
            bbzvm_seterror(BBZVM_ERROR_STACK);
            return BBZVM_STATE_ERROR;
        }
        bbzvm_state ret = bbzvm_exec_instr();
        return ret;
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_execute_script() {
    while(bbzvm_step() == BBZVM_STATE_READY) { }
    return vm->state;
}


// ======================================
// =         BYTECODE FUNCTIONS         =
// ======================================


// --------------------------------
// - Binary arithmetic operators  -
// --------------------------------

/**
 * @brief Type for an arithmetic binary function pointer.
 * @param[in] lhs Left-hand side of the operation.
 * @param[in] rhs Right-hand side of the operation.
 * @return The result of the operation.
 */
typedef int16_t (*binary_op_arith)(int16_t lhs,
                                   int16_t rhs);

/**
 * @brief Performs an arithmetic binary operation.
 * @details Pops both operand, and pushes the result.
 * @param[in,out] vm The VM.
 * @param[in] op The operation to perform.
 */
bbzvm_state bbzvm_binary_op_arith(binary_op_arith op) {
    bbzvm_stack_assert(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // MCUs usually only support integer operations.
    // Disallow floating-point operations.
    uint8_t ok = bbztype_isint(*lhs) && bbztype_isint(*rhs);
    if (ok) {
        bbzheap_idx_t idx;
        bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
        bbzvm_obj_at(idx)->i.value = (*op)(lhs->i.value, rhs->i.value);
        bbzvm_push(idx);
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
    return vm->state;
}

int16_t add(int16_t lhs, int16_t rhs) { return lhs + rhs; }
int16_t sub(int16_t lhs, int16_t rhs) { return lhs - rhs; }
int16_t mul(int16_t lhs, int16_t rhs) { return lhs * rhs; }
int16_t div(int16_t lhs, int16_t rhs) { return lhs / rhs; }
int16_t mod(int16_t lhs, int16_t rhs) { return lhs % rhs; }
int16_t bbzpow(int16_t lhs, int16_t rhs) {
    if (rhs >= 0) {
        int32_t res = 1;
        for (uint16_t i = 0; i < rhs; ++i) {
            res *= lhs;
        }
        return (int16_t)res;
    }
    else if (lhs == 1) {
        return 1;
    }
    else if (lhs == -1) {
        return -1;
    }
    else {
        // TODO Set an error when lhs == 0 ?
        return 0;
    }
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_add() {
	return bbzvm_binary_op_arith(&add);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_sub() {
	return bbzvm_binary_op_arith(&sub);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_mul() {
	return bbzvm_binary_op_arith(&mul);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_div() {
	return bbzvm_binary_op_arith(&div);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_mod() {
	return bbzvm_binary_op_arith(&mod);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pow() {
	return bbzvm_binary_op_arith(&bbzpow);
}


// --------------------------------
// -         Unary Minus          -
// --------------------------------

bbzvm_state bbzvm_unm() {
    bbzvm_stack_assert(1);
    bbzobj_t* operand = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();

    // We *could* implement the unary minus on a float, but we
    // disallow it, since we can do nothing with it anyway.
    uint8_t ok = bbztype_isint(*operand);
    if (ok) {
        bbzheap_idx_t idx;
        bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
        bbzvm_obj_at(idx)->i.value = -operand->i.value;
        bbzvm_push(idx);
    }
    else {
	    bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
	return vm->state;
}

// --------------------------------
// -   Binary logical operators   -
// --------------------------------

/**
 * @brief Type for a logic binary function pointer.
 * @param[in] lhs Left-hand side of the operation.
 * @param[in] rhs Right-hand side of the operation.
 * @return The result of the operation.
 */
typedef uint8_t (*binary_op_logic)(uint8_t lhs,
                                   uint8_t rhs);

/**
 * @brief Performs an arithmetic binary operation.
 * @details Pops both operand, and pushes the result.
 * @param[in,out] vm The VM.
 * @param[in] op The operation to perform.
 */
bbzvm_state bbzvm_binary_op_logic(binary_op_logic op) {
    bbzvm_stack_assert(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    uint8_t lhs_bool, rhs_bool;

    switch(bbztype(*lhs)) {
        case BBZTYPE_INT: lhs_bool = (lhs->i.value != 0); break;
        case BBZTYPE_NIL: lhs_bool = 0; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm->state;
    }
    switch(bbztype(*rhs)) {
        case BBZTYPE_INT: rhs_bool = (rhs->i.value != 0); break;
        case BBZTYPE_NIL: rhs_bool = 0; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm->state;
    }

    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = (*op)(lhs_bool, rhs_bool);
    return bbzvm_push(idx);
}

uint8_t bbzand(uint8_t lhs, uint8_t rhs) { return lhs & rhs; }
uint8_t bbzor (uint8_t lhs, uint8_t rhs) { return lhs | rhs; }

/****************************************/
/****************************************/

bbzvm_state bbzvm_and() {
	return bbzvm_binary_op_logic(&bbzand);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_or() {
	return bbzvm_binary_op_logic(&bbzor);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_not() {
    bbzvm_stack_assert(1);
    bbzobj_t* operand = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
	switch(bbztype(*operand)) {
        case BBZTYPE_INT: bbzvm_obj_at(idx)->i.value = (operand->i.value != 0); break;
        case BBZTYPE_NIL: bbzvm_obj_at(idx)->i.value = 0; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
    return vm->state;
}

/****************************************/
/****************************************/

// --------------------------------
// - Binary comparison operators  -
// --------------------------------

/**
 * @brief Type for a comparison binary function pointer.
 * @param[in] cmp Value of the comparison of the two hand sides.
 * @return The result of the operation.
 */
typedef uint8_t (*binary_op_cmp)(int8_t cmp);

/**
 * @brief Performs an arithmetic binary operation.
 * @details Pops both operand, and pushes the result.
 * @param[in,out] vm The VM.
 * @param[in] op The operation to perform.
 */
bbzvm_state bbzvm_binary_op_cmp(binary_op_cmp op) {
    bbzvm_stack_assert(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();

    int8_t cmp = bbztype_cmp(lhs, rhs);

    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = (*op)(cmp);
    bbzvm_push(idx);

    return vm->state;
}

uint8_t bbzeq (int8_t cmp) { return cmp == 0; }
uint8_t bbzneq(int8_t cmp) { return cmp != 0; }
uint8_t bbzgt (int8_t cmp) { return cmp >  0; }
uint8_t bbzgte(int8_t cmp) { return cmp >= 0; }
uint8_t bbzlt (int8_t cmp) { return cmp <  0; }
uint8_t bbzlte(int8_t cmp) { return cmp <= 0; }

/****************************************/
/****************************************/

bbzvm_state bbzvm_eq() {
	return bbzvm_binary_op_cmp(&bbzeq);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_neq() {
	return bbzvm_binary_op_cmp(&bbzneq);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gt() {
	return bbzvm_binary_op_cmp(&bbzgt);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gte() {
	return bbzvm_binary_op_cmp(&bbzgte);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lt() {
	return bbzvm_binary_op_cmp(&bbzlt);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lte() {
	return bbzvm_binary_op_cmp(&bbzlte);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pusht() {
    // Allocate the table
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_TABLE, &idx);
    return bbzvm_push(idx);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lload(uint16_t idx) {
    bbzheap_idx_t id = vm->nil;
    if (!bbzdarray_get(vm->lsyms, idx, &id)) {
        bbzvm_seterror(BBZVM_ERROR_LNUM);
    }
    return bbzvm_push(id);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lstore(uint16_t idx) {
    bbzheap_idx_t o = bbzvm_stack_at(0);
    uint16_t size = bbzdarray_size(vm->lsyms);
    while (size++ <= idx) {
        bbzvm_assert_exec(bbzdarray_push(vm->lsyms, vm->nil), BBZVM_ERROR_MEM);
    }
    bbzdarray_set(vm->lsyms, idx, o);
    return bbzvm_pop();
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jump(uint16_t offset) {
    vm->pc = offset;
    assert_pc(vm->pc);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jumpz(uint16_t offset) {
    bbzvm_stack_assert(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_INT: if (o->i.value == 0) vm->pc = offset; break;
        case BBZTYPE_NIL: vm->pc = offset; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm->state;
    }
    assert_pc(vm->pc);
    return bbzvm_pop();
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jumpnz(uint16_t offset) {
    bbzvm_stack_assert(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_INT: if (o->i.value != 0) vm->pc = offset; assert_pc(vm->pc); break;
        case BBZTYPE_NIL: break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm->state;
    }
    return bbzvm_pop();
}

/****************************************/
/****************************************/

uint8_t bbzvm_gsym_register(uint16_t sid, bbzheap_idx_t v) {
    bbzvm_pushs(sid);
    bbzheap_idx_t str = bbzvm_stack_at(0);
    bbzvm_pop();
    // Put the value in the global symbols table
    if (!bbztable_set(vm->gsyms, str, v)) return 0;
    return 1;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_closure_call(uint16_t argc) {
    bbzvm_pushi(argc);
    int16_t blockptr = vm->blockptr;
    bbzvm_callc();
    while(blockptr < vm->blockptr) if(bbzvm_step() != BBZVM_STATE_READY) return vm->state;
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_function_call(bbzheap_idx_t fname, uint16_t argc) {
	/* Reset the VM state if it's DONE */
	if(vm->state == BBZVM_STATE_DONE)
		vm->state = BBZVM_STATE_READY;
	/* Don't continue if the VM has an error */
    bbzvm_assert_state();
	/* Push the function name (return with error if not found) */
	bbzvm_pushs(fname);
    bbzvm_assert_state();
	/* Get associated symbol */
	bbzvm_gload();
	/* Make sure it's a closure */
	bbzobj_t* o = bbzvm_obj_at(bbzvm_stack_at(0));
	if(!bbztype_isclosure(*o)) {
		bbzvm_seterror(BBZVM_ERROR_TYPE);
		return BBZVM_STATE_ERROR;
	}
	/* Move closure before arguments */
	if(argc > 0) {
		bbzheap_idx_t c = bbzvm_stack_at( 0);
		for (uint16_t i = 0;
			 i < argc; ++i) {
			vm->stack[vm->stackptr - i] = bbzvm_stack_at(i + 1);
		}
		vm->stack[vm->stackptr - argc] = c;
	}
	/* Call the closure */
	return bbzvm_closure_call(argc);
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzvm_function_register(int16_t fnameid, bbzvm_funp funp) {
    /* Allocate a bbzclosure_t */
    if (bbzvm_pushcc(funp) == BBZVM_STATE_ERROR) return 0;
    /* Register the closure in the global symbols */
    bbzheap_idx_t cpos = bbzvm_stack_at(0);
    bbzvm_pop();
    if (fnameid >= 0) {
        bbzvm_gsym_register(fnameid, cpos);
    }
    /* Return the closure's position */
    return cpos;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_call(uint8_t isswrm) {
	/* Get argument number and pop it */
    bbzvm_stack_assert(1);
    bbzvm_type_assert(0, BBZTYPE_INT);
    uint16_t argn = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    /* Make sure the stack has enough elements */
    bbzvm_stack_assert(argn+1);
    /* Make sure the closure is where expected */
    bbzvm_type_assert(argn, BBZTYPE_CLOSURE);
    bbzobj_t* c = bbzvm_obj_at(bbzvm_stack_at(argn));
    /* Make sure that that data about lambda closures is correct */
    if(bbztype_isclosurelambda(*c) &&
       ((c->l.value.ref) >= bbzdarray_size(vm->flist))) {
		bbzvm_seterror(BBZVM_ERROR_FLIST);
		return vm->state;
	}
    /* Keep a refererence to the old local symbol list */
    bbzheap_idx_t oldLsyms = vm->lsyms;
    /* Create a new local symbol list copying the parent's */
    if (!bbztype_isclosurelambda(*c) ||
        (c->l.value.actrec) == 0xFF) {
        bbzvm_assert_exec(bbzdarray_clone(vm->dflt_actrec, &vm->lsyms), BBZVM_ERROR_MEM);
    }
    else {
        bbzvm_assert_exec(bbzdarray_clone(c->l.value.actrec, &vm->lsyms), BBZVM_ERROR_MEM);
    }
    if (isswrm) {
    	bbzdarray_mark_swarm((bbzdarray_t*)bbzvm_obj_at(vm->lsyms));
    }
    /* Add function arguments to the local symbols */
    /* and */
    /* Get rid of the function arguments */
    int16_t i;
    for (i = argn; i > 0; --i) {
    	bbzdarray_push(vm->lsyms, bbzvm_stack_at(i-1));
    }
    vm->stackptr -= argn + 1;// Get rid of the closure's reference on the stack.
    /* Push return address */
    bbzvm_pushi(vm->pc);
    bbzvm_assert_state();
    /* Push old local symbol list */
    bbzvm_push(oldLsyms);
    bbzvm_assert_state();
    /* Push block pointer */
    bbzvm_pushi(vm->blockptr);
    vm->blockptr = vm->stackptr;
    bbzvm_assert_state();
    /* Jump to/execute the function */
    if (bbztype_isclosurenative(*c)) {
        if (bbztype_isclosurelambda(*c)) {
            bbzdarray_get(vm->flist, c->l.value.ref, (uint16_t*)&i);
            i = bbzvm_obj_at((uint16_t)i)->i.value;
        }
        else {
            i = (int16_t)c->i.value;
        }
        vm->pc = i;
    }
    else {
        if (bbztype_isclosurelambda(*c)) {
            bbzdarray_get(vm->flist, c->l.value.ref, (uint16_t*)&i);
            ((bbzvm_funp)bbzvm_obj_at((uint16_t)i)->u.value)();
        }
        else {
            ((bbzvm_funp)c->c.value)();
        }
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pop() {
    if(bbzvm_stack_size() > 0) {
    	--vm->stackptr;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_dup() {
    uint16_t stack_size = bbzvm_stack_size();
    if (stack_size > 0 && stack_size < BBZSTACK_SIZE) {
        bbzheap_idx_t idx;
        bbzvm_assert_mem_alloc(BBZTYPE_NIL, &idx);
        bbzheap_obj_copy(bbzvm_stack_at(0), idx);
        bbzvm_push(idx);
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_push(bbzheap_idx_t v) {
	if (bbzvm_stack_size() >= BBZSTACK_SIZE) {
		bbzvm_seterror(BBZVM_ERROR_STACK);
		return vm->state;
	}
	vm->stack[++vm->stackptr] = v;
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushu(void* v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
    bbzheap_obj_at(o)->u.value = v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushnil() {
    return bbzvm_push(vm->nil);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushc(intptr_t rfrnc, int16_t nat) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc((uint8_t)(BBZTYPE_CLOSURE | ((nat & 1) << 1)), &o);
    bbzvm_obj_at(o)->c.value = (void*)rfrnc;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushi(int16_t v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &o);
    bbzvm_obj_at(o)->i.value = v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushf(bbzfloat v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_FLOAT, &o);
    bbzheap_obj_at(o)->f.value = v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushs(uint16_t strid) {
    bbzheap_idx_t o, v;
    bbzvm_assert_mem_alloc(BBZTYPE_STRING, &o);
    bbzheap_obj_at(o)->s.value = strid;
    strid = bbzdarray_find(vm->gsyms, bbztype_cmp, o);
    if (bbztable_get(vm->gsyms, o, &v)) {
        obj_makeinvalid(*bbzvm_obj_at(o));
        bbzdarray_get(vm->gsyms, strid, &o);
    }
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushl(uint16_t addr) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_NCLOSURE, &o);
    bbzclosure_make_lambda(*bbzvm_obj_at(o));
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = addr;
    addr = bbzdarray_find(vm->flist, bbztype_cmp, idx);
    /* If the function isn't in the list yet, ... */
    if (addr == bbzdarray_size(vm->flist)) {
        /* ... Add the bbzuserdata_t to the function list */
        bbzvm_assert_exec(bbzdarray_push(vm->flist, idx), BBZVM_ERROR_MEM);
    }
    else {
        /* ... else, Free the memory used by the buffer */
        obj_makeinvalid(*bbzvm_obj_at(idx));
    }
    bbzvm_obj_at(o)->l.value.ref = (uint8_t)addr;
    if (vm->lsyms) {
        bbzvm_assert_exec(
                bbzdarray_lambda_alloc(vm->lsyms, &bbzvm_obj_at(o)->l.value.actrec),
                BBZVM_ERROR_MEM);
    }
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tput() {
    // Get value, key and table, and pop them.
    bbzvm_stack_assert(3);
    bbzvm_type_assert(2, BBZTYPE_TABLE);
    bbzheap_idx_t v = bbzvm_stack_at(0);
    bbzheap_idx_t k = bbzvm_stack_at(1);
    bbzheap_idx_t t = bbzvm_stack_at(2);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    bbzobj_t* vObj = bbzheap_obj_at(v);
    if(bbztype_isclosure(*vObj)) {
        // Method call
        bbzheap_idx_t o, ar, o2;
        bbzvm_assert_mem_alloc(BBZTYPE_NIL, &o);
        bbzheap_obj_copy(v, o);
        bbzclosure_make_lambda(*bbzvm_obj_at(o));

        bbzvm_assert_mem_alloc(bbztype_isclosurenative(*vObj) ? BBZTYPE_INT : BBZTYPE_USERDATA, &v);
        if(bbztype_isclosurelambda(*vObj)) {
            ar = vObj->l.value.actrec;
            bbzdarray_get(vm->flist, (uint16_t)vObj->l.value.ref, &o2);
            bbzvm_obj_at(v)->u.value = bbzvm_obj_at(o2)->u.value;
        }
        else {
            ar = vm->dflt_actrec;
            bbzvm_obj_at(v)->c.value = vObj->c.value;
        }
        o2 = bbzdarray_find(vm->flist, bbztype_cmp, v);
        if (o2 == bbzdarray_size(vm->flist)) {
            bbzvm_assert_exec(bbzdarray_push(vm->flist, v), BBZVM_ERROR_MEM);
        }
        else {
            obj_makeinvalid(*bbzvm_obj_at(v));
        }

        bbzvm_assert_exec(
                bbzdarray_lambda_alloc(ar, &bbzvm_obj_at(o)->l.value.actrec),
                BBZVM_ERROR_MEM);
        bbzvm_obj_at(o)->l.value.ref = (uint8_t)o2;
        bbzdarray_set(bbzvm_obj_at(o)->l.value.actrec, 0, t);
        bbzvm_assert_exec(bbztable_set(t, k, o), BBZVM_ERROR_MEM);
    }
    else {
        bbzvm_assert_exec(bbztable_set(t, k, v), BBZVM_ERROR_MEM);
    }

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tget() {
    // Get & pop the arguments
    bbzvm_stack_assert(2);
    bbzvm_type_assert(1, BBZTYPE_TABLE);
    bbzheap_idx_t k = bbzvm_stack_at(0);
    bbzheap_idx_t t = bbzvm_stack_at(1);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // Get the value and push it
    bbzheap_idx_t idx;
    if(bbztable_get(t, k, &idx)){
        bbzvm_push(idx);
    }
    else {
        bbzvm_pushnil();
    }
    bbzvm_assert_state();

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gload() {
    // Get and pop the string
    bbzvm_stack_assert(1);
    bbzvm_type_assert(0, BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzvm_assert_state();

    // Get and push the associated value
    bbzheap_idx_t o;
    if(bbztable_get(vm->gsyms, str, &o)) {
        bbzvm_push(o);
    }
    else {
        bbzvm_pushnil();
    }
    bbzvm_assert_state();
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gstore() {
    // Get and pop the arguments
    bbzvm_stack_assert(2);
    bbzvm_type_assert(1, BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at(1);
    bbzheap_idx_t o = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // Store the value
    if (!bbztable_set(vm->gsyms, str, o)) {
        bbzvm_seterror(BBZVM_ERROR_MEM);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret0() {
	/* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm->lsyms)->t)) {
    	//TODO pop the swarm stack.
    }
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzvm_obj_at(vm->stack[vm->stackptr])->i.value;
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Pop local symbol table */
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Make sure that element is an integer */
	bbzvm_type_assert(0, BBZTYPE_INT);
	/* Use that element as program counter */
	vm->pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    return bbzvm_pop();
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret1() {
	/* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm->lsyms)->t)) {
    	//TODO pop the swarm stack.
    }
    /* Make sure there's an element on the stack */
    bbzvm_stack_assert(1);
    /* Save it, it's the return value to pass to the lower stack */
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzvm_obj_at(vm->stack[vm->stackptr])->i.value;
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Pop local symbol table */
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Make sure that element is an integer */
	bbzvm_type_assert(0, BBZTYPE_INT);
	/* Use that element as program counter */
	vm->pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
    /* Push the return value */
    return bbzvm_push(ret);
}

/****************************************/
/****************************************/
