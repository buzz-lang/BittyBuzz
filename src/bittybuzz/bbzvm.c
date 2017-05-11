#include "bbzvm.h"

/****************************************/
/****************************************/

buzzvm_lsyms_t buzzvm_lsyms_new(uint8_t isswarm, buzzdarray_t syms) {
    // TODO
}

/****************************************/
/****************************************/

void buzzvm_lsyms_destroy(uint32_t pos, void*  void* params) {
    // TODO
}

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

void bbzvm_darray_destroy(uint32_t pos, void* data, void* params) {
    bbzdarray_t* s = (bbzdarray_t*)data;
    bbzdarray_destroy(s);
}

void bbzvm_construct(bbzvm_t* vm, uint16_t robot) {
    vm->bcode = NULL;
    vm->bcode_size = 0;
    vm->pc = 0;

    // Create the heap
    bbzheap_clear(&vm->heap);
    
    // Create various arrays
    bbzdarray_new(&vm->heap, &vm->stack);
    bbzdarray_new(&vm->heap, &vm->stacks);
    bbzdarray_new(&vm->heap, &vm->lsymts);
    bbzdarray_new(&vm->heap, &vm->flist);

    // Create global symbols table
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_TABLE, &vm->gsyms);
    uint16_t s;
    bbzheap_tseg_alloc(&vm->heap, &s);
    vm->gsyms.t.value = s;

    vm->errormsg = NULL;
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

int bbzvm_set_bcode(bbzvm_t* vm, const uint8_t* bcode, uint32_t bcode_size) {
    // 1) Reset the VM
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZ_ERROR_NONE;
    vm->pc = 0;

    // 2) Set the bytecode
    vm->bcode = bcode;
    vm->bcode_size = bcode_size;

    // 3) Register global strings
    // TODO

    // 4) Register Buzz's built-in functions
    // TODO

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { bbzvm_seterror(vm, BBZVM_ERROR_PC); return vm->state; }

#define inc_pc() ++vm->pc; assert_pc(vm->pc);

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)(vm->bcode + vm->pc)); vm->pc += sizeof(TYPE);

/**
 * @brief Runs the VM's garbage collector.
 * @param[in|out] vm The VM.
 */
__attribute__((always_inlide))
void bbzvm_gc(bbzvm_t* vm) {
    bbzheap_gc(&vm->heap);
}

__attribute__((always_inline))
void bbzvm_exec_instr(bbzvm_t* vm) {
    uint8_t instr = vm->bcode[vm->pc];
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
            bbzvm_add(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_SUB: {
            bbzvm_sub(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_MUL: {
            bbzvm_mul(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_DIV: {
            bbzvm_div(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_MOD: {
            bbzvm_mod(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_POW: {
            bbzvm_pow(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_UNM: {
            bbzvm_unm(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_AND: {
            bbzvm_and(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_OR: {
            bbzvm_or(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_NOT: {
            bbzvm_not(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_EQ: {
            bbzvm_eq(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_NEQ: {
            bbzvm_neq(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_GT: {
            bbzvm_gt(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_GTE: {
            bbzvm_gte(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_LT: {
            bbzvm_lt(vm);
            inc_pc();
            break;
        }
        case BBZVM_INSTR_LTE: {
            bbzvm_lte(vm);
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
            bbzvm_pusht(vm);
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
            if(bbzvm_callc(vm) != BBZVM_STATE_READY) return vm->state;
            assert_pc(vm->pc);
            break;
        }
        case BBZVM_INSTR_CALLS: {
            inc_pc();
            if(bbzvm_calls(vm) != BBZVM_STATE_READY) return vm->state;
            assert_pc(vm->pc);
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
            if(bbzvm_pushi(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHS: {
            inc_pc();
            get_arg(int32_t);
            if(bbzvm_pushs(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHCN: {
            inc_pc();
            get_arg(uint32_t);
            if(bbzvm_pushcn(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHCC: {
            inc_pc();
            get_arg(uint32_t);
            if(bbzvm_pushcc(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_PUSHL: {
            inc_pc();
            get_arg(uint32_t);
            if(bbzvm_pushl(vm, arg) != BBZVM_STATE_READY) return vm->state;
            break;
        }
        case BBZVM_INSTR_LLOAD: {
            inc_pc();
            get_arg(uint32_t);
            bbzvm_lload(vm, arg);
            break;
        }
        case BBZVM_INSTR_LSTORE: {
            inc_pc();
            get_arg(uint32_t);
            bbzvm_lstore(vm, arg);
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
            if(bbzvm_stack_at(vm, 1)->o.type == BBZTYPE_NIL ||
                (bbzvm_stack_at(vm, 1)->o.type == BBZTYPE_INT &&
                 bbzvm_stack_at(vm, 1)->i.value == 0)) {
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
            if(bbzvm_stack_at(vm, 1)->o.type != BBZTYPE_NIL &&
                (bbzvm_stack_at(vm, 1)->o.type != BBZTYPE_INT ||
                 bbzvm_stack_at(vm, 1)->i.value != 0)) {
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
}

bbzvm_state bbzvm_step(bbzvm_t* vm) {
    if(vm->state == BBZVM_STATE_READY) {
        bbzvm_gc(vm);
        bbzvm_exec_instr(vm);
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
    if(!bbzdarray_isempty(vm->stack)) {
        bbzdarray_pop(vm->stack);
    }
    else {
        bbzvm_seterror(vm, BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_dup(bbzvm_t* vm) {
    if(!bbzdarray_isempty(vm->stack)) {
        bbzobj_t x = bbzvm_stack_at(vm, 1);
        bbzdarray_push(vm->stack, &x);
    }
    else {
        bbzvm_seterror(vm, BBZVM_ERROR_STACK);
    }
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_push(bbzvm_t* vm, bbzheap_idx_t v) {
    bbzdarray_push(&vm->heap, vm->stack, v);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushu(bbzvm_t* vm, void* v) {
    bbzheap_idx_t o;
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_USERDATA, o);
    bbzheap_obj_at(&vm->heap, o)->value = v;
    bbzvm_push(vm, o);
    return vm->state;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_pushnil(bbzvm_t* vm) {
    bbzheap_idx_t o;
    bbzheap_obj_alloc(&vm->heap, BBZTYPE_NIL, &o);
    bbzvm_push(vm, o);
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
    bbzheap_obj_at(&vm->heap, o)->i.value = v;
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
    return bbzvm_push(vm, o);
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tput(bbzvm_t* vm) {
    bbzvm_stack_assert(vm, 3);
    bbzvm_type_assert(vm, 3, BBZTYPE_TABLE);
    bbzheap_idx_t v = bbzvm_stack_at(vm, 1);
    bbzheap_idx_t k = bbzvm_stack_at(vm, 2);
    bbzheap_idx_t t = bbzvm_stack_at(vm, 3);
    bbzvm_pop(vm);
    bbzvm_pop(vm);
    bbzvm_pop(vm);
    if(v->o.type == BBZTYPE_NIL) {
        // Nil, erase entry
        // TODO bbztable_remove(&vm-heap, t->t.value, &k);
    }
    else if(v->o.type == BBZTYPE_CLOSURE) {
        // Method call
        // TODO Clone the object and add the table to its activation record.
    }
    else {
        bbztable_set(t->t.value, &k, &v);
    }
    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_tget(bbzvm_t* vm) {
    // Get & pop the arguments
    bbzvm_stack_assert(vm, 2);
    bbzvm_type_assert(vm, 2, BBZTYPE_TABLE);
    bbzheap_idx_t k = bbzvm_stack_at(vm, 1);
    bbzheap_idx_t t = bbzvm_stack_at(vm, 2);
    bbzvm_pop(vm);
    bbzvm_pop(vm);

    // Get the value and push it
    bbzheap_idx_t idx;
    if(bbztable_get(&vm->heap, t->t.value, k, &idx)) bbzvm_push(vm, idx);
    else bbzvm_pushnil(vm);

    return BBZVM_STATE_READY;
}

/****************************************/
/****************************************/

bbzvm_state bbzvm_gload(bbzvm_t* vm) {
    // Get and pop the string
    bbzvm_stack_assert(vm, 1);
    bbzvm_type_assert(vm, 1, BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at(vm, 1);
    bbzvm_pop(vm);

    // Get and push the associated value
    bbzheap_idx_t o;
    if(bbztable_get(&vm->heap, vm->gsyms, &(str->s.value.sid), &o)) {
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
    bbzobj_t str = bbzvm_stack_at((vm), 2);
    bbzobj_t o = bbzvm_stack_at((vm), 1);
    bbzvm_pop(vm);
    bbzvm_pop(vm);

    // Store the value
    bbztable_set(&vm->heap, vm->gsyms, &str->s.value.sid, &o);
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
