#include "bbzvm.h"
#include "bbzheap.h"
#include <stdio.h>
#include "bbztype.h"

bbzvm_t* vm; // Global extern variable 'vm'.

/****************************************/
/****************************************/

void bbzvm_process_inmsgs() {
    bbzvm_assert_state();
    /* Go through the messages */
    uint8_t count = 0;
    while(!bbzinmsg_queue_isempty() && count++ < BBZMSG_IN_PROC_MAX) {
        bbzvm_gc();
        bbzvm_assert_state();
        /* Extract the message data */
        bbzmsg_t* msg = bbzinmsg_queue_extract();
        switch(msg->type) {
            case BBZMSG_BROADCAST:
                bbzmsg_process_broadcast(msg);
                break;
            case BBZMSG_VSTIG_QUERY: // fallthrough
            case BBZMSG_VSTIG_PUT:
                bbzmsg_process_vstig(msg);
                break;
            case BBZMSG_SWARM: {
                bbzmsg_process_swarm(msg);
                break;
            }
            default:
                break;
        }
    }
}

/****************************************/
/****************************************/

void bbzvm_process_outmsgs() {
#ifndef BBZ_DISABLE_NEIGHBORS
    if (!(vm->neighbors.clear_counter--)) {
        vm->neighbors.clear_counter = BBZNEIGHBORS_CLR_PERIOD;
        // Execute the neighbors' data garbage-collector.
        bbzneighbors_data_gc();
    }
#endif // !BBZ_DISABLE_NEIGHBORS

#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
    // TODO Send swarm message
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
}

/****************************************/
/****************************************/

ALWAYS_INLINE
void bbzvm_register_globals() {
    bbzvm_pushi(vm->robot);
    bbzvm_gsym_register(__BBZSTRID_id, bbzvm_stack_at(0));
    bbzvm_pop();
}

ALWAYS_INLINE
void bbzvm_clear_stack() {
    for (uint16_t i = BBZSTACK_SIZE-1; i; --i) {
        vm->stack[i] ^= vm->stack[i];
    }
}

extern char* _state_desc[];
extern char* _error_desc[];
extern char* _instr_desc[];
#if defined(DEBUG) && !defined(BBZ_XTREME_MEMORY)
char* _state_desc[] = {"BBZVM_STATE_NOCODE", "BBZVM_STATE_READY", "BBZVM_STATE_STOPPED", "BBZVM_STATE_DONE", "BBZVM_STATE_ERROR",
                       "BBZVM_STATE_COUNT"};
char* _error_desc[] = {"BBZVM_ERROR_NONE", "BBZVM_ERROR_INSTR", "BBZVM_ERROR_STACK", "BBZVM_ERROR_LNUM", "BBZVM_ERROR_PC",
                       "BBZVM_ERROR_FLIST", "BBZVM_ERROR_TYPE", "BBZVM_ERROR_OUTOFRANGE", "BBZVM_ERROR_NOTIMPL",
                       "BBZVM_ERROR_RET", "BBZVM_ERROR_STRING", "BBZVM_ERROR_SWARM", "BBZVM_ERROR_VSTIG", "BBZVM_ERROR_MEM",
                       "BBZVM_ERROR_MATH"};
char* _instr_desc[] = {"NOP", "DONE", "PUSHNIL", "DUP", "POP", "RET0", "RET1", "ADD", "SUB", "MUL", "DIV", "MOD", "POW",
                       "UNM", "LAND", "LOR", "LNOT","BAND","BOR","BNOT","EQ", "NEQ", "GT", "GTE", "LT", "LTE", "GLOAD", "GSTORE", "PUSHT", "TPUT",
                       "TGET", "CALLC", "CALLS", "PUSHF", "PUSHI", "PUSHS", "PUSHCN", "PUSHCC", "PUSHL", "LLOAD", "LSTORE","LREMOVE",
                       "JUMP", "JUMPZ", "JUMPNZ", "COUNT"};
#endif // DEBUG && !BBZ_XTREME_MEMORY

#pragma GCC diagnostic ignored "-Wunused-parameter"
ALWAYS_INLINE void dftl_error_receiver(bbzvm_error errcode) {
#ifdef DEBUG
    bbzheap_print();
#ifndef BBZ_XTREME_MEMORY
    printf("VM:\n\tstate: %s\n\tpc: %d\n\tinstr: %s\n\terror state: %s\n", _state_desc[vm->state], vm->dbg_pc,
           vm->bcode_fetch_fun ? _instr_desc[*vm->bcode_fetch_fun(vm->dbg_pc, 1)] : "N/A", _error_desc[errcode]);
#endif // !BBZ_XTREME_MEMORY
#endif // DEBUG
}

int bbzvm_construct(bbzrobot_id_t robot) {
    vm->bcode_fetch_fun = NULL;
    vm->bcode_size = 0;
    vm->pc = 0;
    vm->state = BBZVM_STATE_NOCODE;
    vm->error = BBZVM_ERROR_NONE;
    vm->error_receiver_fun = dftl_error_receiver;
    vm->stackptr = -1;
    vm->blockptr = vm->stackptr;
    vm->lsyms = 0;
    vm->robot = robot;
    vm->flist = 0;

    // Setup things
    bbzheap_clear();
    bbzinmsg_queue_construct();
    bbzoutmsg_queue_construct();

    // Allocate singleton objects
    bbzheap_obj_alloc(BBZTYPE_NIL, &vm->nil);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->nil));
    bbzheap_obj_at(vm->nil)->i.value = 0;
    bbzdarray_new(&vm->dflt_actrec);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->dflt_actrec));
    bbzdarray_push(vm->dflt_actrec, vm->nil);

    // Create various arrays
    bbzdarray_new(&vm->flist);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->flist));

    // Create global symbols table
    bbzheap_obj_alloc(BBZTYPE_TABLE, &vm->gsyms);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->gsyms));
    
    // For debugging purposes:
    //     bbzheap_obj_at(vm->gsyms)->i.value = 4;
        int16_t si = bbzheap_obj_at(vm->gsyms)->t.value;
    /* Get segment data */
    bbzheap_tseg_t* sd = bbzheap_tseg_at(si);
    if(bbzheap_obj_alloc(BBZTYPE_TABLE, &vm->gsyms))
{
    bbzheap_print();
    printf("[BBZVM] is bbzheap_obj_at(vm->gsyms) a table? -> %d [1 if yes].\n)", bbztype_istable(*bbzheap_obj_at(vm->gsyms)));
    printf("[BBZVM] is  bbzheap_tseg_hasnext(sd) true? -> %d [1 if yes].\n)", bbzheap_tseg_hasnext(sd));
    printf("[testheap] Allocated table object at position %" PRIu16 "\n", vm->gsyms);
    printf("[testheap] Allocated table segment at position %" PRIu16 "\n", bbzheap_obj_at(vm->gsyms)->t.value);     
}
    printf("[BBZVM] Done\n");
    // End of debugging block

    bbzvm_register_globals();

    // Register things
    bbzvstig_register();
    bbzswarm_register();
    bbzneighbors_register();
    
    // Debugging:
    if(bbzheap_obj_alloc(BBZTYPE_TABLE, &vm->gsyms) == 0 || bbzheap_obj_alloc(BBZTYPE_NIL, &vm->nil) == 0)
{
    return 0;
}
    else
    return 1;
    // End debugging.
}

/****************************************/
/****************************************/

void bbzvm_destruct() {
    // Destroy heap ; Buzz objects are destroyed along with it.
    bbzheap_clear();

    bbzoutmsg_queue_destruct();
    bbzinmsg_queue_destruct();
}

/****************************************/
/****************************************/

void bbzvm_seterror(bbzvm_error errcode) {
    // Set the error
    vm->state = BBZVM_STATE_ERROR;
    vm->error = errcode;
    // Call the error receiver function.
    (*vm->error_receiver_fun)(errcode);
}

/****************************************/
/****************************************/

void bbzvm_set_bcode(bbzvm_bcode_fetch_fun bcode_fetch_fun, uint16_t bcode_size) {
    // 1) Set the bytecode
    vm->bcode_fetch_fun = bcode_fetch_fun;
    vm->bcode_size = bcode_size;

    // 2) Reset the VM
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZVM_ERROR_NONE;

    // 3) Register global strings
    vm->pc = sizeof(uint16_t);

    // 4) Register Buzz's built-in functions
    while(*vm->bcode_fetch_fun(vm->pc, sizeof(uint8_t)) != BBZVM_INSTR_NOP) {
        bbzvm_step();
        if(vm->state != BBZVM_STATE_READY) return;
    }

    bbzvm_step();
}

/****************************************/
/****************************************/

#define assert_pc(IDX) if((IDX) > vm->bcode_size) { bbzvm_seterror(BBZVM_ERROR_PC); return; }

#define inc_pc() assert_pc(vm->pc); ++vm->pc;

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg; {TYPE* parg = ((TYPE*)vm->bcode_fetch_fun(vm->pc, sizeof(TYPE))); bbzvm_assign(&arg, parg);} vm->pc += sizeof(TYPE);

void bbzvm_gc() {
    bbzheap_gc(vm->stack, (uint16_t)bbzvm_stack_size());
}

/**
 * @brief Executes a single Buzz instruction.
 */
//ALWAYS_INLINE
static void bbzvm_exec_instr() {
    bbzpc_t instrOffset = vm->pc; // Save PC in case of error or DONE.

    uint8_t instr = *(*vm->bcode_fetch_fun)(vm->pc, 1);
#ifdef DEBUG
    vm->dbg_pc = vm->pc;
    vm->instr = (bbzvm_instr)instr;
#endif
    if (instr != BBZVM_INSTR_DONE) inc_pc();
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
        case BBZVM_INSTR_LAND: {
	    bbzvm_land();
            break;
        }
        case BBZVM_INSTR_LOR: {
            bbzvm_lor();
            break;
        }
        case BBZVM_INSTR_BAND: {
            bbzvm_band();
            break;
        }
        case BBZVM_INSTR_BOR: {
            bbzvm_bor();
            break;
        }
        case BBZVM_INSTR_BNOT: {
            bbzvm_bnot();
            break;
        }
        case BBZVM_INSTR_LNOT: {
            bbzvm_lnot();
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
        case BBZVM_INSTR_CALLS: { // For compatibility only
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
        case BBZVM_INSTR_PUSHCC: { // _FIXME I don't think that a buzz script should/would ever use this instruction... Neither is it used in the buzz parser.
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
        case BBZVM_INSTR_LREMOVE: {
            get_arg(uint16_t);
            bbzvm_lremove(arg);
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
}

void bbzvm_step() {
    if(vm->state == BBZVM_STATE_READY) {
        bbzvm_gc();
        bbzvm_exec_instr();
    }
}

/****************************************/
/****************************************/


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
 * @param[in] op The operation to perform.
 */
static void bbzvm_binary_op_arith(binary_op_arith op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzheap_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // MCUs usually only support integer operations.
    // Disallow floating-point operations.
//    uint8_t ok = (uint8_t) (!bbztype_isfloat(*lhs) && !bbztype_isfloat(*rhs));
    bbzvm_assert_exec(!bbztype_isfloat(*lhs) && !bbztype_isfloat(*rhs), BBZVM_ERROR_TYPE);
    bbzvm_pushi((*op)(lhs->i.value, rhs->i.value));
}

static int16_t add(int16_t lhs, int16_t rhs) { return lhs + rhs; }
static int16_t sub(int16_t lhs, int16_t rhs) { return lhs - rhs; }
static int16_t mul(int16_t lhs, int16_t rhs) { return lhs * rhs; }
static int16_t div(int16_t lhs, int16_t rhs) { return lhs / rhs; }
static int16_t mod(int16_t lhs, int16_t rhs) { return lhs % rhs; }
static int16_t bbzpow(int16_t lhs, int16_t rhs) {
    int16_t ret;
    if (rhs >= 0) {
        int32_t res = 1;
        while (rhs--) {
            res *= lhs;
        }
        ret = (int16_t)res;
    }
    else if (lhs == 1) {
        ret = 1;
    }
    else if (lhs == -1) {
        ret = -1;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_MATH);
        ret = 0;
    }
    return ret;
}

/****************************************/
/****************************************/

void bbzvm_add() {
    return bbzvm_binary_op_arith(&add);
}

/****************************************/
/****************************************/

void bbzvm_sub() {
    return bbzvm_binary_op_arith(&sub);
}

/****************************************/
/****************************************/

void bbzvm_mul() {
    return bbzvm_binary_op_arith(&mul);
}

/****************************************/
/****************************************/

void bbzvm_div() {
    return bbzvm_binary_op_arith(&div);
}

/****************************************/
/****************************************/

void bbzvm_mod() {
    return bbzvm_binary_op_arith(&mod);
}

/****************************************/
/****************************************/

void bbzvm_pow() {
    return bbzvm_binary_op_arith(&bbzpow);
}


// --------------------------------
// -         Unary Minus          -
// --------------------------------

void bbzvm_unm() {
    bbzvm_assert_stack(1);
    bbzobj_t* operand = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    bbzvm_pop();

    // We *could* implement the unary minus on a float, but we
    // disallow it, since we can do nothing with it anyway.
    bbzvm_pushi(-operand->i.value);
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
 * @param[in] op The operation to perform.
 */
static void bbzvm_binary_op_logic(binary_op_logic op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzheap_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    uint8_t
            lhs_bool = bbztype_tobool(lhs),
            rhs_bool = bbztype_tobool(rhs);

    bbzvm_pushi((*op)(lhs_bool, rhs_bool));
}

static uint8_t bbzand(uint8_t lhs, uint8_t rhs) { return lhs && rhs; }
static uint8_t bbzor (uint8_t lhs, uint8_t rhs) { return lhs || rhs; }
static uint8_t bbzband(uint8_t lhs, uint8_t rhs) { return lhs & rhs; }
static uint8_t bbzbor (uint8_t lhs, uint8_t rhs) { return lhs | rhs; }


/****************************************/
/****************************************/

void bbzvm_land() {
    return bbzvm_binary_op_logic(&bbzand);
}

/****************************************/
/****************************************/

void bbzvm_lor() {
    return bbzvm_binary_op_logic(&bbzor);
}

/****************************************/
/****************************************/

void bbzvm_band() {
    return bbzvm_binary_op_logic(&bbzband);
}

/****************************************/
/****************************************/

void bbzvm_bor() {
    return bbzvm_binary_op_logic(&bbzbor);
}

/****************************************/
/****************************************/

void bbzvm_lnot() {
    bbzvm_assert_stack(1);
    bbzobj_t* operand = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();
    switch(bbztype(*operand)) {
        case BBZTYPE_NIL: // fallthrough
        case BBZTYPE_INT: bbzvm_pushi(operand->i.value != 0); break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
}

/****************************************/
/****************************************/

void bbzvm_bnot() {
    bbzvm_assert_stack(1);
    bbzobj_t* operand = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();
    switch(bbztype(*operand)) {
        case BBZTYPE_NIL: // fallthrough
        case BBZTYPE_INT: bbzvm_pushi(~operand->i.value); break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
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
 * @param[in] op The operation to perform.
 */
static void bbzvm_binary_op_cmp(binary_op_cmp op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzheap_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzheap_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();

    bbzvm_pushi((*op)(bbztype_cmp(lhs, rhs)));
}

static uint8_t bbzeq (int8_t cmp) { return (uint8_t) (cmp == 0); }
static uint8_t bbzneq(int8_t cmp) { return (uint8_t) (cmp != 0); }
static uint8_t bbzgt (int8_t cmp) { return (uint8_t) (cmp >  0); }
static uint8_t bbzgte(int8_t cmp) { return (uint8_t) (cmp >= 0); }
static uint8_t bbzlt (int8_t cmp) { return (uint8_t) (cmp <  0); }
static uint8_t bbzlte(int8_t cmp) { return (uint8_t) (cmp <= 0); }

/****************************************/
/****************************************/

void bbzvm_eq() {
    return bbzvm_binary_op_cmp(&bbzeq);
}

/****************************************/
/****************************************/

void bbzvm_neq() {
    return bbzvm_binary_op_cmp(&bbzneq);
}

/****************************************/
/****************************************/

void bbzvm_gt() {
    return bbzvm_binary_op_cmp(&bbzgt);
}

/****************************************/
/****************************************/

void bbzvm_gte() {
    return bbzvm_binary_op_cmp(&bbzgte);
}

/****************************************/
/****************************************/

void bbzvm_lt() {
    return bbzvm_binary_op_cmp(&bbzlt);
}

/****************************************/
/****************************************/

void bbzvm_lte() {
    return bbzvm_binary_op_cmp(&bbzlte);
}

/****************************************/
/****************************************/

void bbzvm_pusht() {
    bbzvm_push(bbztable_new());
}

/****************************************/
/****************************************/

void bbzvm_lload(uint16_t idx) {
    bbzheap_idx_t id = vm->nil;
    bbzvm_assert_exec(bbzdarray_get(vm->lsyms, idx, &id), BBZVM_ERROR_LNUM);
    return bbzvm_push(id);
}

/****************************************/
/****************************************/

void bbzvm_lstore(uint16_t idx) {
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

void bbzvm_lremove(uint16_t num) {
    uint16_t i;
    for( i = 0 ; i < num ; i++ )
      bbzvm_assert_exec(bbzdarray_pop(vm->lsyms), BBZVM_ERROR_LNUM);
}


/****************************************/
/****************************************/

void bbzvm_jump(uint16_t offset) {
    vm->pc = offset;
    assert_pc(vm->pc);
}

/****************************************/
/****************************************/

void bbzvm_jumpz(uint16_t offset) {
    bbzvm_assert_stack(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_NIL: // fallthrough
        case BBZTYPE_INT: if (o->i.value == 0) vm->pc = offset; assert_pc(vm->pc); break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return;
    }
    assert_pc(vm->pc);
    return bbzvm_pop();
}

/****************************************/
/****************************************/

void bbzvm_jumpnz(uint16_t offset) {
    bbzvm_assert_stack(1);
    bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(0));

    switch(bbztype(*o)) {
        case BBZTYPE_NIL: // fallthrough
        case BBZTYPE_INT: if (o->i.value != 0) vm->pc = offset; assert_pc(vm->pc); break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return;
    }
    return bbzvm_pop();
}

/****************************************/
/****************************************/

uint8_t bbzvm_gsym_register(uint16_t sid, bbzheap_idx_t v) {
    bbzvm_pushs(sid);
    bbzvm_push(v);
    bbzvm_gstore();
    bbzvm_assert_state(0);
    return 1;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzint_new(int16_t val) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &o, vm->nil);
    bbzheap_obj_at(o)->i.value = val;
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzfloat_new(bbzfloat val) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_FLOAT, &o, vm->nil);
    bbzheap_obj_at(o)->f.value = val;
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzstring_get(uint16_t val) {
    bbzheap_idx_t o = val;
    bbzvm_assert_mem_alloc(BBZTYPE_STRING, &o, vm->nil);
    bbzheap_obj_at(o)->s.value = val;
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbztable_new() {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_TABLE, &o, vm->nil);
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzclosure_new(intptr_t val) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_CLOSURE, &o, vm->nil);
    bbzheap_obj_at(o)->c.value = (void(*)())val;
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzuserdata_new(void* val) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o, vm->nil);
    bbzheap_obj_at(o)->u.value = (uintptr_t)val;
    return o;
}

/****************************************/
/****************************************/

bbzheap_idx_t bbzvm_stack_at(int16_t idx) { return vm->stack[vm->stackptr - idx]; }

/****************************************/
/****************************************/

void bbzvm_closure_call(uint16_t argc) {
    bbzvm_assert_state();
    bbzvm_pushi(argc);
    int16_t blockptr = vm->blockptr;
    bbzvm_callc();
    while(blockptr < vm->blockptr) {
        if(vm->state != BBZVM_STATE_READY) return;
        bbzvm_step();
    }
}

/****************************************/
/****************************************/

void bbzvm_function_call(uint16_t fname, uint16_t argc) {
    /* Reset the VM state if it's DONE */
    if (vm->state == BBZVM_STATE_DONE)
        vm->state = BBZVM_STATE_READY;
    /* Don't continue if the VM has an error */
    bbzvm_assert_state();
    /* Push the function name (return with error if not found) */
    bbzvm_pushs(fname);
    bbzvm_assert_state();
    /* Get associated symbol */
    bbzvm_gload();
    /* Make sure it's a closure */
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_CLOSURE);
    /* Move closure before arguments */
    if(argc > 0) {
        bbzheap_idx_t c = bbzvm_stack_at(0);
        for (uint16_t i = 0;
             i < argc; ++i) {
            vm->stack[vm->stackptr - i] = bbzvm_stack_at(i + (uint16_t)1);
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
    bbzvm_pushcc(funp);
    bbzvm_assert_state(0);
    /* Register the closure in the global symbols */
    bbzheap_idx_t cpos = bbzvm_stack_at(0);
    bbzvm_pop();
    if (fnameid >= 0) {
        bbzvm_gsym_register((uint16_t)fnameid, cpos);
    }
    /* Return the closure's position */
    return cpos;
}

/****************************************/
/****************************************/

void bbzvm_callc() {
    /* Get argument number and pop it */
    bbzvm_assert_stack(1);
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    uint16_t argn = (uint16_t)bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    /* Make sure the stack has enough elements */
    bbzvm_assert_stack(argn+1);
    /* Make sure the closure is where expected */
    bbzvm_assert_type(bbzvm_stack_at(argn), BBZTYPE_CLOSURE);
    bbzobj_t* c = bbzheap_obj_at(bbzvm_stack_at(argn));
    /* Make sure that the data about lambda closures is correct */
    bbzvm_assert_exec(!(bbztype_isclosurelambda(*c) && ((c->l.value.ref) >= bbzdarray_size(vm->flist))),
                      BBZVM_ERROR_FLIST);
    /* Keep a refererence to the old local symbol list */
    bbzheap_idx_t oldLsyms = vm->lsyms;
    /* Create a new local symbol list copying the parent's */
    if (!bbztype_isclosurelambda(*c) ||
        (c->l.value.actrec) == BBZHEAP_CLOSURE_DFLT_ACTREC) {
        bbzvm_assert_exec(bbzdarray_clone(vm->dflt_actrec, &vm->lsyms), BBZVM_ERROR_MEM);
    }
    else {
        bbzvm_assert_exec(bbzdarray_clone(c->l.value.actrec, &vm->lsyms), BBZVM_ERROR_MEM);
    }
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->lsyms));
    /* Add function arguments to the local symbols */
    /* and */
    /* Get rid of the function arguments */
    uint16_t i;
    for (i = argn; i; --i) {
        bbzdarray_push(vm->lsyms, bbzvm_stack_at(i - (uint16_t)1));
    }
    vm->stackptr -= argn + 1; // Get rid of the closure's reference on the stack.
    /* Recover and pop the self table */
    if (!bbztype_isclosurelambda(*c) ||
        c->l.value.actrec == BBZHEAP_CLOSURE_DFLT_ACTREC ||
        !bbztype_darray_hasself(*bbzheap_obj_at(c->l.value.actrec))) {
        bbzdarray_set(vm->lsyms, 0, bbzvm_stack_at(0));
    }
    bbzvm_pop();
    /* Push return address */
    bbzvm_pushi(vm->pc);
    bbzvm_assert_state();
    /* Push old local symbol list */
    bbzvm_push(oldLsyms);
    bbzvm_assert_state();
    /* Push block pointer */
    bbzvm_pushi(vm->blockptr);
    bbzvm_assert_state();
    vm->blockptr = vm->stackptr;
    /* Jump to/execute the function */
    uintptr_t x;
    if (bbztype_isclosurelambda(*c)) {
        bbzdarray_get(vm->flist, c->l.value.ref, &i);
        x = bbzheap_obj_at((uint16_t)i)->biggest.value;
    }
    else {
        x = c->biggest.value;
    }
    if (bbztype_isclosurenative(*c)) {
        vm->pc = (bbzpc_t)x;
    }
    else {
        ((bbzvm_funp)x)();
    }
}

/****************************************/
/****************************************/

void bbzvm_pop() {
    bbzvm_assert_exec(bbzvm_stack_size() > 0, BBZVM_ERROR_STACK);
    --vm->stackptr;
}

/****************************************/
/****************************************/

void bbzvm_dup() {
    uint16_t stack_size = (uint16_t)bbzvm_stack_size();
    bbzvm_assert_exec(stack_size > 0 && stack_size < BBZSTACK_SIZE, BBZVM_ERROR_STACK);
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &idx);
    bbzheap_obj_copy(bbzvm_stack_at(0), idx);
    bbzheap_obj_makevalid(*bbzheap_obj_at(idx));
    bbzheap_obj_unmake_permanent(*bbzheap_obj_at(idx));
    bbzvm_push(idx);
}

/****************************************/
/****************************************/

void bbzvm_push(bbzheap_idx_t v) {
    bbzvm_assert_exec(bbzvm_stack_size() < BBZSTACK_SIZE, BBZVM_ERROR_STACK);
    vm->stack[++vm->stackptr] = v;
}

/****************************************/
/****************************************/

void bbzvm_pushu(void* v) {
    bbzvm_push(bbzuserdata_new(v));
}

/****************************************/
/****************************************/

void bbzvm_pushnil() {
    bbzvm_push(vm->nil);
}

/****************************************/
/****************************************/

void bbzvm_pushc(intptr_t rfrnc, int16_t nat) {
    bbzheap_idx_t o = bbzclosure_new(rfrnc);
    if (nat) bbzclosure_make_native(*bbzheap_obj_at(o));
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_pushi(int16_t v) {
    bbzvm_push(bbzint_new(v));
}

/****************************************/
/****************************************/

void bbzvm_pushf(bbzfloat v) {
    bbzvm_push(bbzfloat_new(v));
}

/****************************************/
/****************************************/

void bbzvm_pushs(uint16_t strid) {
    bbzvm_push(bbzstring_get(strid));
}

/****************************************/
/****************************************/

void bbzvm_pushl(uint16_t addr) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_CLOSURE, &o);
    bbzclosure_make_native(*bbzheap_obj_at(o));
    bbzclosure_make_lambda(*bbzheap_obj_at(o));
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzheap_obj_at(idx)->i.value = addr;
    addr = bbzdarray_find(vm->flist, bbztype_cmp, idx);
    /* If the function isn't in the list yet, ... */
    if (addr == bbzdarray_size(vm->flist)) {
        /* ... Add the bbzuserdata_t to the function list */
        bbzvm_assert_exec(bbzdarray_push(vm->flist, idx), BBZVM_ERROR_MEM);
    }
    else {
        /* ... else, Free the memory used by the buffer */
        bbzheap_obj_makeinvalid(*bbzheap_obj_at(idx));
    }
    bbzheap_obj_at(o)->l.value.ref = (uint8_t)addr;
    if (vm->lsyms) {
        bbzvm_assert_exec(
                bbzdarray_lambda_alloc(vm->lsyms, &bbzheap_obj_at(o)->l.value.actrec),
                BBZVM_ERROR_MEM);
    }

    bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_tput() {
    // Get value, key and table, and pop them.
    bbzvm_assert_stack(3);
    bbzheap_idx_t v = bbzvm_stack_at(0);
    bbzheap_idx_t k = bbzvm_stack_at(1);
    bbzheap_idx_t t = bbzvm_stack_at(2);
    bbzvm_assert_type(t, BBZTYPE_TABLE);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    bbzobj_t* vObj = bbzheap_obj_at(v);
    if (bbztype_isclosure(*vObj) &&
        bbztype_isclosurelambda(*vObj) &&
        vObj->l.value.actrec != BBZHEAP_CLOSURE_DFLT_ACTREC &&
        !bbztype_darray_hasself(*bbzheap_obj_at(vObj->l.value.actrec))) {
        // Method call
        bbzheap_idx_t o, ar, o2;
        bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
        bbzheap_obj_copy(v, o);
        bbzclosure_make_lambda(*bbzheap_obj_at(o));

        bbzvm_assert_mem_alloc(bbztype_isclosurenative(*vObj) ? BBZTYPE_INT : BBZTYPE_USERDATA, &v);
        if(bbztype_isclosurelambda(*vObj)) {
            ar = vObj->l.value.actrec;
            bbzvm_assert_exec(bbzdarray_get(vm->flist, (uint16_t)vObj->l.value.ref, &o2), BBZVM_ERROR_FLIST);
            bbzheap_obj_at(v)->u.value = bbzheap_obj_at(o2)->u.value;
        }
        else {
            ar = vm->dflt_actrec;
            bbzheap_obj_at(v)->c.value = vObj->c.value;
        }
        o2 = bbzdarray_find(vm->flist, bbztype_cmp, v);
        if (o2 == bbzdarray_size(vm->flist)) {
            bbzvm_assert_exec(bbzdarray_push(vm->flist, v), BBZVM_ERROR_MEM);
        }
        else {
            bbzheap_obj_makeinvalid(*bbzheap_obj_at(v));
        }

        bbzvm_assert_exec(
                bbzdarray_lambda_alloc(ar, &bbzheap_obj_at(o)->l.value.actrec),
                BBZVM_ERROR_MEM);
        bbztype_darray_markself(*bbzheap_obj_at(bbzheap_obj_at(o)->l.value.actrec));
        bbzheap_obj_at(o)->l.value.ref = (uint8_t)o2;
        bbzvm_assert_exec(bbzdarray_set(bbzheap_obj_at(o)->l.value.actrec, 0, t), BBZVM_ERROR_FLIST);
        bbzvm_assert_exec(bbztable_set(t, k, o), BBZVM_ERROR_MEM);
    }
    else {
        bbzvm_assert_exec(bbztable_set(t, k, v), BBZVM_ERROR_MEM);
    }
}

/****************************************/
/****************************************/

void bbzvm_tget() {
    // Get & pop the arguments
    bbzvm_assert_stack(2);
    bbzheap_idx_t k = bbzvm_stack_at(0);
    bbzheap_idx_t t = bbzvm_stack_at(1);
    bbzvm_assert_type(t, BBZTYPE_TABLE);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // Get the value and push it
    bbzheap_idx_t idx = vm->nil;
    bbztable_get(t, k, &idx);
    bbzvm_push(idx);
}

/****************************************/
/****************************************/

void bbzvm_gload() {
    // Get and pop the string
    bbzvm_assert_stack(1);
    bbzheap_idx_t str = bbzvm_stack_at(0);
    bbzvm_assert_type(str, BBZTYPE_STRING);
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
}

/****************************************/
/****************************************/

void bbzvm_gstore() {
    // Get and pop the arguments
    bbzvm_assert_stack(2);
    bbzheap_idx_t str = bbzvm_stack_at(1);
    bbzheap_idx_t o = bbzvm_stack_at(0);
    bbzvm_assert_type(str, BBZTYPE_STRING);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // Store the value
    bbzvm_assert_exec(bbztable_set(vm->gsyms, str, o), BBZVM_ERROR_MEM);
}

/****************************************/
/****************************************/

void bbzvm_ret0() {
    /* Make sure there's enough elements on the stack */
    bbzvm_assert_stack(3);
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzheap_obj_at(vm->stack[vm->stackptr])->i.value;
    bbzvm_pop();
    /* Pop local symbol table */
    bbzheap_obj_unmake_permanent(*bbzheap_obj_at(vm->lsyms));
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_assert_stack(1);
    /* Make sure that element is an integer */
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    /* Use that element as program counter */
    vm->pc = (bbzpc_t)bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
}

/****************************************/
/****************************************/

void bbzvm_ret1() {
    /* Make sure there's enough elements on the stack */
    bbzvm_assert_stack(4);
    /* Save it, it's the return value to pass to the lower stack */
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzheap_obj_at(vm->stack[vm->blockptr])->i.value;
    bbzvm_pop();
    /* Pop local symbol table */
    bbzheap_obj_unmake_permanent(*bbzheap_obj_at(vm->lsyms));
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure that element is an integer */
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    /* Use that element as program counter */
    vm->pc = (bbzpc_t)bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
    /* Push the return value */
    bbzvm_push(ret);
}

/****************************************/
/****************************************/
