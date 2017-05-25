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
    vm.bcode_fetch_fun = NULL;
    vm.bcode_size = 0;
    vm.pc = 0;
    vm.state = BBZVM_STATE_NOCODE;
    vm.error = BBZVM_ERROR_NONE;
    vm.error_notifier_fun = NULL;

    // Setup the heap
    bbzheap_clear();

    // Allocate singleton objects
    bbzheap_obj_alloc(BBZTYPE_NIL, &vm.nil);
    bbzdarray_new(&vm.dflt_actrec);
    bbzdarray_push(vm.dflt_actrec, vm.nil);
    
    // Create various arrays
    bbzdarray_new(&vm.lsymts);
    bbzdarray_new(&vm.flist);

    // Create global symbols table
    bbzheap_obj_alloc(BBZTYPE_TABLE, &vm.gsyms);

    // Setup stack
    vm.stackptr = -1;
    vm.blockptr = vm.stackptr;

    // Set up other variables...
    vm.lsyms = 0;

    vm.robot = robot;
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
    vm.state = BBZVM_STATE_ERROR;
    vm.error = errcode;
    // Call the user's notifier function.
    if (vm.error_notifier_fun) {
        (*vm.error_notifier_fun)(errcode);
    }
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_set_bcode(bbzvm_bcode_fetch_fun bcode_fetch_fun, uint16_t bcode_size) {
    // 1) Reset the VM
    vm.state = BBZVM_STATE_READY;
    vm.error = BBZVM_ERROR_NONE;
    vm.pc = 0;

    // 2) Set the bytecode
    vm.bcode_fetch_fun = bcode_fetch_fun;
    vm.bcode_size = bcode_size;

    // 3) Register global strings
    uint16_t strCount = *vm.bcode_fetch_fun(0, sizeof(uint16_t));
    vm.pc = sizeof(uint16_t);
    const uint8_t* c;
    for (uint16_t i = 0; i < strCount; ++i) {
#ifndef BBZVM_USE_BBO
    	do {
    		c = vm.bcode_fetch_fun(vm.pc++, sizeof(uint8_t));
    	} while(*c != 0);
#endif
        bbzvm_gsym_register(i, vm.nil);
    }

    // 4) Register Buzz's built-in functions
    while(*vm.bcode_fetch_fun(vm.pc, sizeof(uint8_t)) != BBZVM_INSTR_NOP) {
        if(bbzvm_step() != BBZVM_STATE_READY) return vm.state;
    }
    bbzvm_step();

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm.bcode_size) { bbzvm_seterror(BBZVM_ERROR_PC); return BBZVM_STATE_ERROR; }

#define inc_pc() assert_pc(vm.pc); ++vm.pc;

#define get_arg(TYPE) assert_pc(vm.pc + sizeof(TYPE)); TYPE arg = *((TYPE*)vm.bcode_fetch_fun(vm.pc, sizeof(TYPE))); vm.pc += sizeof(TYPE);

#ifndef BBZVM_USE_BBO
#define get_integer() get_arg(int32_t)
#else
#define get_integer() get_arg(int16_t);
#endif

/**
 * @brief Runs the VM's garbage collector.
 * @param[in|out] vm The VM.
 */
__attribute__((always_inline)) static inline
uint8_t bbzvm_gc() {
    // TODO Take some of these element out of the heap.
    if (BBZSTACK_SIZE - bbzvm_stack_size() < 6) return 0;
    bbzvm_push(vm.lsyms);
    bbzvm_push(vm.lsymts);
    bbzvm_push(vm.gsyms);
    bbzvm_push(vm.nil);
    bbzvm_push(vm.dflt_actrec);
    bbzvm_push(vm.flist);
    bbzheap_gc(vm.stack, bbzvm_stack_size());
    for (int8_t i = 6; i > 0; --i) {
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
    uint16_t instrOffset = vm.pc; // Save PC in case of error or DONE.

    uint8_t instr = *(*vm.bcode_fetch_fun)(vm.pc, 1);
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
            if (vm.state == BBZVM_STATE_READY) {
                assert_pc(vm.pc);
            }
            break;
        }
        case BBZVM_INSTR_RET1: {
            bbzvm_ret1();
            if (vm.state == BBZVM_STATE_READY) {
                assert_pc(vm.pc);
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
            if (vm.state == BBZVM_STATE_READY) {
                assert_pc(vm.pc);
            }
            break;
        }
        case BBZVM_INSTR_CALLS: {
            bbzvm_calls();
            if (vm.state == BBZVM_STATE_READY) {
                assert_pc(vm.pc);
            }
            break;
        }
        case BBZVM_INSTR_PUSHF: {
#ifndef BBZVM_USE_BBO
            get_arg(float);
#else
            get_arg(bbzfloat);
#endif
            bbzvm_pushf(arg);
            break;
        }
        case BBZVM_INSTR_PUSHI: {
            get_integer();
            bbzvm_pushi(arg);
            break;
        }
        case BBZVM_INSTR_PUSHS: {
            get_integer();
            bbzvm_pushs(arg);
            break;
        }
        case BBZVM_INSTR_PUSHCN: {
            get_integer();
            bbzvm_pushcn(arg);
            break;
        }
        case BBZVM_INSTR_PUSHCC: { // FIXME I don't think that a buzz script should/would ever use this instruction... Neither is it used in the buzz parser.
            get_integer();
            bbzvm_pushcc((bbzvm_funp)(intptr_t)arg);
            break;
        }
        case BBZVM_INSTR_PUSHL: {
            get_integer();
            bbzvm_pushl(arg);
            break;
        }
        case BBZVM_INSTR_LLOAD: {
            get_integer();
            bbzvm_lload(arg);
            break;
        }
        case BBZVM_INSTR_LSTORE: {
            get_integer();
            bbzvm_lstore(arg);
            break;
        }
        case BBZVM_INSTR_JUMP: {
            get_integer();
            bbzvm_jump((uint16_t)arg);
            break;
        }
        case BBZVM_INSTR_JUMPZ: {
            get_integer();
            bbzvm_jumpz((uint16_t)arg);
            break;
        }
        case BBZVM_INSTR_JUMPNZ: {
            get_integer();
            bbzvm_jumpnz((uint16_t)arg);
            break;
        }
        default:
            bbzvm_seterror(BBZVM_ERROR_INSTR);
            break;
    }

    if (vm.state != BBZVM_STATE_READY) {
        // Stay on the instruction that caused the error,
        // or, in the case of BBZVM_INSTR_DONE, loop on it.
        vm.pc = instrOffset;
    }

    return vm.state;
}

bbzvm_state bbzvm_step() {
    if(vm.state == BBZVM_STATE_READY) {
        if(!bbzvm_gc()) {
            bbzvm_seterror(BBZVM_ERROR_STACK);
            return BBZVM_STATE_ERROR;
        }
        bbzvm_state ret = bbzvm_exec_instr();
        return ret;
    }
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_execute_script() {
    while(bbzvm_step() == BBZVM_STATE_READY) { }
    return vm.state;
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
    return vm.state;
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
	return vm.state;
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

    uint8_t lhs_bool, rhs_bool;

    switch(bbztype(*lhs)) {
        case BBZTYPE_INT: lhs_bool = (lhs->i.value != 0); break;
        case BBZTYPE_NIL: lhs_bool = 0; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm.state;
    }
    switch(bbztype(*rhs)) {
        case BBZTYPE_INT: rhs_bool = (rhs->i.value != 0); break;
        case BBZTYPE_NIL: rhs_bool = 0; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm.state;
    }

    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = (*op)(lhs_bool, rhs_bool);
    bbzvm_push(idx);

    return vm.state;
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
    return vm.state;
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

    return vm.state;
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
    bbzvm_push(idx);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lload(uint16_t idx) {
    bbzheap_idx_t id = vm.nil;
    if (!bbzdarray_get(vm.lsyms, idx, &id)) {
        bbzvm_seterror(BBZVM_ERROR_LNUM);
    }
    bbzvm_push(id);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_lstore(uint16_t idx) {
    bbzheap_idx_t o = bbzvm_stack_at(0);
    uint16_t size = bbzdarray_size(vm.lsyms);
    while (size++ <= idx) {
        bbzdarray_push(vm.lsyms, vm.nil);
    }
    bbzdarray_set(vm.lsyms, idx, o);
    return bbzvm_pop();
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jump(uint16_t offset) {
    vm.pc = offset;
    assert_pc(vm.pc);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jumpz(uint16_t offset) {
    bbzvm_stack_assert(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_INT: if (o->i.value == 0) vm.pc = offset; break;
        case BBZTYPE_NIL: vm.pc = offset; break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm.state;
    }
    assert_pc(vm.pc);
    bbzvm_pop();
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_jumpnz(uint16_t offset) {
    bbzvm_stack_assert(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_INT: if (o->i.value != 0) vm.pc = offset; assert_pc(vm.pc); break;
        case BBZTYPE_NIL: break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return vm.state;
    }
    bbzvm_pop();
    return vm.state;
}

/****************************************/
/****************************************/

uint8_t bbzvm_gsym_register(uint16_t sid, bbzheap_idx_t v) {
    bbzvm_pushs(sid);
    bbzheap_idx_t str = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzobj_t* o = bbzvm_obj_at(v);
    uint8_t oldType = bbztype(*o);
    /* If we want to put a nil, ... */
    if (bbztype_isnil(*o)) {
        // ... Cast the nil into an int
        bbztype_cast(*o, BBZTYPE_INT);
    }
    /* Put the value in the global symbols table */
    if (!bbztable_set(vm.gsyms, str, v)) return 0;
    bbztype_cast(*o, oldType);
    return 1;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_closure_call(uint16_t argc) {
    bbzvm_pushi(argc);
    int16_t blockptr = vm.blockptr;
    bbzvm_callc();
    while(blockptr < vm.blockptr) if(bbzvm_step() != BBZVM_STATE_READY) return vm.state;
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_function_call(bbzheap_idx_t fname, uint16_t argc) {
	/* Reset the VM state if it's DONE */
	if(vm.state == BBZVM_STATE_DONE)
		vm.state = BBZVM_STATE_READY;
	/* Don't continue if the VM has an error */
	if(vm.state != BBZVM_STATE_READY)
		return vm.state;
	/* Push the function name (return with error if not found) */
	if(bbzvm_pushs(fname) != BBZVM_STATE_READY)
		return vm.state;
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
			vm.stack[vm.stackptr - i] = bbzvm_stack_at(i + 1);
		}
		vm.stack[vm.stackptr - argc] = c;
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
       ((c->l.value.ref) >= bbzdarray_size(vm.flist))) {
		bbzvm_seterror(BBZVM_ERROR_FLIST);
		return vm.state;
	}
    /* Create a new local symbol list copying the parent's */
    if (!bbztype_isclosurelambda(*c) ||
        (c->l.value.actrec) == 0xFF) {
    	bbzdarray_clone(vm.dflt_actrec, &vm.lsyms);
    }
    else {
    	bbzdarray_clone(c->l.value.actrec, &vm.lsyms);
    }
    if (isswrm) {
    	bbzdarray_mark_swarm((bbzdarray_t*)bbzvm_obj_at(vm.lsyms));
    }
    bbzdarray_push(vm.lsymts, vm.lsyms);
    /* Add function arguments to the local symbols */
    /* and */
    /* Get rid of the function arguments */
    int16_t i;
    for (i = argn; i > 0; --i) {
    	bbzdarray_push(vm.lsyms, bbzvm_stack_at(i-1));
    }
    vm.stackptr -= argn + 1;// Get rid of the closure's reference on the stack.
    /* Push return address */
    bbzvm_pushi(vm.pc);
    /* Push block pointer */
    bbzvm_pushi(vm.blockptr);
    vm.blockptr = vm.stackptr;
    /* Jump to/execute the function */
    if (bbztype_isclosurenative(*c)) {
        if (bbztype_isclosurelambda(*c)) {
            bbzdarray_get(vm.flist, c->l.value.ref, (uint16_t*)&i);
            i = bbzvm_obj_at((uint16_t)i)->i.value;
        }
        else {
            i = (int16_t)c->i.value;
        }
        vm.pc = i;
    }
    else {
        if (bbztype_isclosurelambda(*c)) {
            bbzdarray_get(vm.flist, c->l.value.ref, (uint16_t*)&i);
            ((bbzvm_funp)bbzvm_obj_at((uint16_t)i)->u.value)();
        }
        else {
            ((bbzvm_funp)c->c.value)();
        }
    }
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pop() {
    if(bbzvm_stack_size() > 0) {
    	--vm.stackptr;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_STACK);
    }
    return vm.state;
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
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_push(bbzheap_idx_t v) {
	if (bbzvm_stack_size() >= BBZSTACK_SIZE) {
		bbzvm_seterror(BBZVM_ERROR_STACK);
		return vm.state;
	}
	vm.stack[++vm.stackptr] = v;
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushu(void* v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
    bbzheap_obj_at(o)->u.value = v;
    bbzvm_push(o);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushnil() {
    bbzvm_push(vm.nil);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushc(intptr_t rfrnc, int16_t nat) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc((uint8_t)(BBZTYPE_CLOSURE | ((nat & 1) << 1)), &o);
    bbzvm_obj_at(o)->c.value = (void*)rfrnc;
    bbzvm_push(o);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushi(int16_t v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &o);
    bbzvm_obj_at(o)->i.value = v;
    bbzvm_push(o);
    return vm.state;
}

/****************************************/
/****************************************/

#ifndef BBZVM_USE_BBO
    bbzvm_state bbzvm_pushf(float v) {
#else
    bbzvm_state bbzvm_pushf(bbzfloat v) {
#endif
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_FLOAT, &o);
#ifndef BBZVM_USE_BBO
    bbzheap_obj_at(o)->f.value = bbzfloat_fromfloat(v);
#else
    bbzheap_obj_at(o)->f.value = v;
#endif
    bbzvm_push(o);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushs(uint16_t strid) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_STRING, &o);
    bbzheap_obj_at(o)->s.value = strid;
    strid = bbzdarray_find(vm.gsyms, bbztype_cmp, o);
    if (strid < bbzdarray_size(vm.gsyms)) {
        obj_makeinvalid(*bbzvm_obj_at(o));
        bbzdarray_get(vm.gsyms, strid, &o);
    }
    bbzvm_push(o);
    return vm.state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushl(int16_t addr) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_NCLOSURE, &o);
    bbzclosure_make_lambda(*bbzvm_obj_at(o));
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = addr;
    addr = bbzdarray_find(vm.flist, bbztype_cmp, idx);
    /* If the function isn't in the list yet, ... */
    if (addr == bbzdarray_size(vm.flist)) {
        /* ... Add the bbzuserdata_t to the function list */
        bbzdarray_push(vm.flist, idx);
    }
    else {
        /* ... else, Free the memory used by the buffer */
        obj_makeinvalid(*bbzvm_obj_at(idx));
    }
    bbzvm_obj_at(o)->l.value.ref = (uint8_t)addr;
    if (vm.lsyms) {
        bbzdarray_lambda_alloc(vm.lsyms, &bbzvm_obj_at(o)->l.value.actrec);
    }
    bbzvm_push(o);
    return BBZVM_STATE_READY;
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
            bbzdarray_get(vm.flist, (uint16_t)vObj->l.value.ref, &o2);
            bbzvm_obj_at(v)->u.value = bbzvm_obj_at(o2)->u.value;
        }
        else {
            ar = vm.dflt_actrec;
            bbzvm_obj_at(v)->c.value = vObj->c.value;
        }
        if (bbzdarray_find(vm.flist, bbztype_cmp, v) == bbzdarray_size(vm.flist)) {
            bbzdarray_push(vm.flist, v);
        }
        else {
            obj_makeinvalid(*bbzvm_obj_at(v));
        }
        bbzdarray_lambda_alloc(ar, &bbzvm_obj_at(o)->l.value.actrec);
        bbzdarray_set(bbzvm_obj_at(o)->l.value.actrec, 0, t);
        bbztable_set(t, k, o);
    }
    else {
        bbztable_set(t, k, v);
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

    // Get the value and push it
    bbzheap_idx_t idx;
    if(bbztable_get(t, k, &idx)){
        bbzvm_push(idx);
    }
    else {
        bbzvm_pushnil();
    }

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

    // Get and push the associated value
    bbzheap_idx_t o;
    if(bbztable_get(vm.gsyms, str, &o)) {
        bbzvm_push(o);
    }
    else {
        bbzvm_pushnil();
    }
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

    // Store the value
    bbztable_set(vm.gsyms, str, o);
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret0() {
	/* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm.lsyms)->t)) {
    	//TODO pop the swarm stack.
    }
    /* Pop local symbol table */
    bbzdarray_pop(vm.lsymts);
    /* Set local symbol table pointer */
    bbzdarray_destroy(vm.lsyms);
    if (!bbzdarray_isempty(vm.lsymts)) {
    	bbzdarray_last(vm.lsymts, &vm.lsyms);
    }
    else {
    	vm.lsyms = 0;
    }
    /* Pop block pointer and stack */
    vm.stackptr = vm.blockptr;
    vm.blockptr = bbzvm_obj_at(vm.stack[vm.stackptr])->i.value;
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Make sure that element is an integer */
	bbzvm_type_assert(0, BBZTYPE_INT);
	/* Use that element as program counter */
	vm.pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    return bbzvm_pop();
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret1() {
	/* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm.lsyms)->t)) {
    	//TODO pop the swarm stack.
    }
    /* Pop local symbol table */
    bbzdarray_pop(vm.lsymts);
    /* Set local symbol table pointer */
    bbzdarray_destroy(vm.lsyms);
    if (!bbzdarray_isempty(vm.lsymts)) {
    	bbzdarray_last(vm.lsymts, &vm.lsyms);
    }
    else {
    	vm.lsyms = 0;
    }
    /* Make sure there's an element on the stack */
    bbzvm_stack_assert(1);
    /* Save it, it's the return value to pass to the lower stack */
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    /* Pop block pointer and stack */
    vm.stackptr = vm.blockptr;
    vm.blockptr = bbzvm_obj_at(vm.stack[vm.stackptr])->i.value;
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_stack_assert(1);
    /* Make sure that element is an integer */
	bbzvm_type_assert(0, BBZTYPE_INT);
	/* Use that element as program counter */
	vm.pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
    /* Push the return value */
    return bbzvm_push(ret);
}

/****************************************/
/****************************************/
