#include "bbzvm.h"

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

void bbzvm_process_inmsgs(bbzvm_t* vm) {
    // TODO
}

/****************************************/
/****************************************/

void bbzvm_process_outmsgs(bbzvm_t* vm) {
    // TODO
}

/****************************************/
/****************************************/

void bbzvm_construct(bbzvm_t* vm, uint16_t robot) {
    vm->bcode_fetch_fun = NULL;
    vm->bcode_size = 0;
    vm->pc = 0;
    vm->state = BBZVM_STATE_NOCODE;
    vm->error = BBZVM_ERROR_NONE;

    // Setup the heap
    bbzheap_clear(&vm->heap);

    // Allocate singleton objects
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_NIL, &vm->nil);
    bbzdarray_new(&vm->heap, &vm->dflt_actrec);
    bbzdarray_push(&vm->heap, vm->dflt_actrec, vm->nil);
    
    // Create various arrays
    bbzdarray_new(&vm->heap, &vm->lsymts);
    bbzdarray_new(&vm->heap, &vm->flist);

    // Create global symbols table
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_TABLE, &vm->gsyms);
    bbzheap_idx_t s;
    bbzheap_tseg_alloc(&vm->heap, &s);
    bbzheap_obj_at(&vm->heap, vm->gsyms)->t.value = s;

    // Setup stack
    vm->stackptr = -1;

    // TODO Reset error message
    vm->robot = robot;
}

/****************************************/
/****************************************/

void bbzvm_destruct(bbzvm_t* vm) {
    // Destroy heap ;
    // everything else is destroyed along with it.
    bbzheap_clear(&vm->heap);
}

/****************************************/
/****************************************/

void bbzvm_seterror(bbzvm_t* vm, bbzvm_error errcode) {
    vm->state = BBZVM_STATE_ERROR;
    vm->error = errcode;
}

/****************************************/
/****************************************/

uint16_t strlen(char* c) {
	uint16_t len = 0;
	while(*(c + len) != 0) ++len;
	return len;
}

int bbzvm_set_bcode(bbzvm_t* vm, bbzvm_bcode_fetch_fun bcode_fetch_fun, uint32_t bcode_size) {
    // 1) Reset the VM
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZVM_ERROR_NONE;
    vm->pc = 0;
    // TODO Reset string manager

    // 2) Set the bytecode
    vm->bcode_fetch_fun = bcode_fetch_fun;
    vm->bcode_size = bcode_size;

    // 3) Register global strings
    uint16_t strCount = *vm->bcode_fetch_fun(0, sizeof(uint16_t));
    uint16_t charCount = 0;
    const uint8_t* c;
	bbzheap_idx_t o, o2;
    for (int i = 0; i < strCount; ++i) {
    	do {
    		c = vm->bcode_fetch_fun(2+(charCount++), sizeof(uint8_t));
    	} while(*c != 0);
    	bbzheap_obj_alloc(&vm->heap, BBZTYPE_INT, &o);
    	bbzheap_obj_alloc(&vm->heap, BBZTYPE_STRING, &o2);
    	bbzvm_obj_at(vm, o)->i.value = i;
    	bbzvm_obj_at(vm, o2)->s.value = i;
    	bbztable_set(&vm->heap, vm->gsyms, o, o2);
    }
    vm->pc = 2 + charCount;

    // 4) Register Buzz's built-in functions
    // TODO

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { bbzvm_seterror(vm, BBZVM_ERROR_PC); return BBZVM_STATE_ERROR; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)vm->bcode_fetch_fun(vm->pc, sizeof(TYPE))); vm->pc += sizeof(TYPE);

/**
 * @brief Runs the VM's garbage collector.
 * @param[in|out] vm The VM.
 */
__attribute__((always_inline)) inline
void bbzvm_gc(bbzvm_t* vm) {
    // TODO Take some of these element out of the heap.
    bbzvm_push(vm, vm->lsymts);
    bbzvm_push(vm, vm->gsyms);
    bbzvm_push(vm, vm->nil);
    bbzvm_push(vm, vm->dflt_actrec);
    bbzvm_push(vm, vm->flist);
    bbzheap_gc(&vm->heap, vm->stack, bbzvm_stack_size(vm));
    for (int i = 5; i > 0; --i) {
        bbzvm_pop(vm);
    }
}

/**
 * @brief Executes a single Buzz instruction.
 * @param[in,out] vm The VM.
 */
__attribute__((always_inline)) inline
bbzvm_state bbzvm_exec_instr(bbzvm_t* vm) {
    uint8_t instr = *(*vm->bcode_fetch_fun)(vm->pc, 1);
    switch(instr) {
        case BBZVM_INSTR_NOP: {
            inc_pc();
            break;
        }
        case BBZVM_INSTR_DONE: {
            bbzvm_done(vm);
            break;
        }
        case BBZVM_INSTR_PUSHNIL: {
            inc_pc();
            bbzvm_pushnil(vm);
            break;
        }
        case BBZVM_INSTR_DUP: {
            inc_pc();
            bbzvm_dup(vm);
            break;
        }
        case BBZVM_INSTR_POP: {
            if(bbzvm_pop(vm) != BBZVM_STATE_READY) return vm->state;
            inc_pc();
            break;
        }
        case BBZVM_INSTR_RET0: {
            if(bbzvm_ret0(vm) != BBZVM_STATE_READY) return vm->state;
            assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_RET1: {
            if(bbzvm_ret1(vm) != BBZVM_STATE_READY) return vm->state;
            assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_ADD: {
            // TODO bbzvm_add(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_SUB: {
            // TODO bbzvm_sub(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_MUL: {
            // TODO bbzvm_mul(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_DIV: {
            // TODO bbzvm_div(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_MOD: {
            // TODO bbzvm_mod(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_POW: {
            // TODO bbzvm_pow(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_UNM: {
            // TODO bbzvm_unm(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_AND: {
            // TODO bbzvm_and(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_OR: {
            // TODO bbzvm_or(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_NOT: {
            // TODO bbzvm_not(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_EQ: {
            // TODO bbzvm_eq(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_NEQ: {
            // TODO bbzvm_neq(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_GT: {
            // TODO bbzvm_gt(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_GTE: {
            // TODO bbzvm_gte(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_LT: {
            // TODO bbzvm_lt(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_LTE: {
            // TODO bbzvm_lte(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_GLOAD: {
            inc_pc();
            bbzvm_gload(vm);
            break;
        }
        case BBZVM_INSTR_GSTORE: {
            inc_pc();
            if(bbzvm_gstore(vm) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHT: {
            // TODO bbzvm_pusht(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_TPUT: {
            if(bbzvm_tput(vm) != BBZVM_STATE_READY) return vm->state;
            inc_pc();
            break;
        }
        case BBZVM_INSTR_TGET: {
            if(bbzvm_tget(vm) != BBZVM_STATE_READY) return vm->state;
            inc_pc();
            break;
        }
        case BBZVM_INSTR_CALLC: {
            inc_pc();
            // TODO if(bbzvm_callc(vm) != BBZVM_STATE_READY) return vm->state;
            // assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_CALLS: {
            inc_pc();
            // TODO if(bbzvm_calls(vm) != BBZVM_STATE_READY) return vm->state;
            // assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_PUSHF: {
            inc_pc();
            get_arg(float);
            if(bbzvm_pushf(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHI: {
            inc_pc();
            get_arg(int32_t);
            uint16_t arg16 = arg;
            if(bbzvm_pushi(vm, arg16) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHS: {
            inc_pc();
            get_arg(int32_t);
            // TODO if(bbzvm_pushs(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHCN: {
            inc_pc();
            get_arg(uint32_t);
            // TODO if(bbzvm_pushcn(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHCC: {
            inc_pc();
            get_arg(uint32_t);
            // TODO if(bbzvm_pushcc(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHL: {
            inc_pc();
            get_arg(uint32_t);
            // TODO if(bbzvm_pushl(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_LLOAD: {
            inc_pc();
            get_arg(uint32_t);
            // TODO bbzvm_lload(vm, arg);
            break;
        }
        case BBZVM_INSTR_LSTORE: {
            inc_pc();
            get_arg(uint32_t);
            // TODO bbzvm_lstore(vm, arg);
            break;
        }
        case BBZVM_INSTR_JUMP: {
            inc_pc();
            get_arg(uint32_t);
            vm->pc = arg;
            assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_JUMPZ: {
            inc_pc();
            get_arg(uint32_t);
            bbzvm_stack_assert(vm, 1);
            bbzobj_t* o = bbzheap_obj_at(&vm->heap, bbzvm_stack_at(vm, 0));
            if(bbztype_isnil(*o) ||
              (bbztype_isint(*o) && o->i.value == 0)) {
                vm->pc = arg;
                assert_pc(vm->pc);
            }
            bbzvm_pop(vm);
            break;
        }
        case BBZVM_INSTR_JUMPNZ: {
            inc_pc();
            get_arg(uint32_t);
            bbzvm_stack_assert(vm, 1);
            bbzobj_t* o = bbzheap_obj_at(&vm->heap, bbzvm_stack_at(vm, 0));
            if(!bbztype_isnil(*o) &&
              (!bbztype_isint(*o) || o->i.value != 0)) {
                vm->pc = arg;
                assert_pc(vm->pc);
            }
            bbzvm_pop(vm);
            break;
        }
        default:
            bbzvm_seterror(vm, BBZVM_ERROR_INSTR);
            break;
    }
    return vm->state;
}

bbzvm_state bbzvm_step(bbzvm_t* vm) {
    if(vm->state == BBZVM_STATE_READY) {
        bbzvm_gc(vm);
        bbzvm_state ret = bbzvm_exec_instr(vm);
        return ret;
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_execute_script(bbzvm_t* vm) {
    while(bbzvm_step(vm) == BBZVM_STATE_READY) { }
    return vm->state;
}


// ======================================
// =         BYTECODE FUNCTIONS         =
// ======================================


bbzvm_state bbzvm_closure_call(bbzvm_t* vm, uint32_t argc) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_function_call(bbzvm_t* vm, const char* fname, uint32_t argc) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

int bbzvm_function_cmp(const void* a, const void* b) {
    //TODO
    return 0;
}

uint32_t bbzvm_function_register(bbzvm_t* vm, bbzvm_funp funp) {
    // TODO
    uint32_t fpos = 0;
    return fpos;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_call(bbzvm_t* vm, int isswrm) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pop(bbzvm_t* vm) {
    if(bbzvm_stack_size(vm) > 0) {
    	--vm->stackptr;
    }
    else {
        bbzvm_seterror(vm, BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_dup(bbzvm_t* vm) {
    if(bbzvm_stack_size(vm) > 0) {
        bbzvm_push(vm, bbzvm_stack_at(vm, 0));
    }
    else {
        bbzvm_seterror(vm, BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_push(bbzvm_t* vm, bbzheap_idx_t v) {
	if (bbzvm_stack_size(vm) > BBZSTACK_SIZE) {
		bbzvm_seterror(vm, BBZVM_ERROR_STACK);
		return (vm)->state;
	}
	vm->stack[++vm->stackptr] = v;
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushu(bbzvm_t* vm, void* v) {
    bbzheap_idx_t o;
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_USERDATA, &o);
    bbzheap_obj_at(&vm->heap, o)->u.value = v;
    bbzvm_push(vm, o);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushnil(bbzvm_t* vm) {
    bbzvm_push(vm, vm->nil);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushc(bbzvm_t* vm, int32_t rfrnc, int32_t nat) {
    // TODO
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushi(bbzvm_t* vm, int16_t v) {
    bbzheap_idx_t o;
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_INT, &o);
    bbzvm_obj_at(vm, o)->i.value = v;
    bbzvm_push(vm, o);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushf(bbzvm_t* vm, float v) {
    bbzheap_idx_t o;
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_FLOAT, &o);
    bbzheap_obj_at(&vm->heap, o)->f.value = v;
    bbzvm_push(vm, o);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushs(bbzvm_t* vm, uint16_t strid) {
    // TODO
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushl(bbzvm_t* vm, int32_t addr) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tput(bbzvm_t* vm) {
    // Get value, key and table, and pop them.
    bbzvm_stack_assert(vm, 3);
    bbzvm_type_assert(vm, 3, BBZTYPE_TABLE);
    bbzheap_idx_t v = bbzvm_stack_at(vm, 0);
    bbzheap_idx_t k = bbzvm_stack_at(vm, 1);
    bbzheap_idx_t t = bbzvm_stack_at(vm, 2);
    bbzvm_pop(vm);
    bbzvm_pop(vm);
    bbzvm_pop(vm);

    bbzobj_t* vObj = bbzheap_obj_at(&vm->heap, v);
    if(bbztype_isclosure(*vObj)) {
        // Method call
        // TODO Clone the object and add the table to its activation record.
    }
    else {
        bbztable_set(&vm->heap, t, k, v);
    }

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tget(bbzvm_t* vm) {
    // Get & pop the arguments
    bbzvm_stack_assert(vm, 2);
    bbzvm_type_assert(vm, 2, BBZTYPE_TABLE);
    bbzheap_idx_t k = bbzvm_stack_at(vm, 0);
    bbzheap_idx_t t = bbzvm_stack_at(vm, 1);
    bbzvm_pop(vm);
    bbzvm_pop(vm);

    // Get the value and push it
    bbzheap_idx_t idx;
    if(bbztable_get(&vm->heap, t, k, &idx)){
        bbzvm_push(vm, idx);
    }
    else {
        bbzvm_pushnil(vm);
    }

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gload(bbzvm_t* vm) {
    // Get and pop the string
    bbzvm_stack_assert(vm, 1);
    bbzvm_type_assert(vm, 1, BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at(vm, 0);
    bbzvm_pop(vm);

    // Get and push the associated value
    bbzheap_idx_t o;
    if(bbztable_get(&vm->heap, vm->gsyms, str, &o)) {
        bbzvm_push(vm, o);
    }
    else {
        bbzvm_pushnil(vm);
    }
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gstore(bbzvm_t* vm) {
    // Get and pop the arguments    
    bbzvm_stack_assert((vm), 2);
    bbzvm_type_assert((vm), 2, BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at((vm), 1);
    bbzheap_idx_t o = bbzvm_stack_at((vm), 0);
    bbzvm_pop(vm);
    bbzvm_pop(vm);

    // Store the value
    bbztable_set(&vm->heap, vm->gsyms, str, o);
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret0(bbzvm_t* vm) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_ret1(bbzvm_t* vm) {
    // TODO
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/
