#include "bbzvm.h"
#include "bbzutil.h"
#include "bbzvstig.h"
#include "bbzmsg.h"

bbzvm_t* vm; // Global extern variable 'vm'.

/****************************************/
/****************************************/

#define bbzvm_get(arg, TYPE) ({             \
    bbzvm_push ## TYPE(arg);                \
    bbzheap_idx_t __tmp = bbzvm_stack_at(0);\
    bbzvm_pop();                            \
    (bbzheap_idx_t)__tmp;                   \
})

uint8_t bbzlamport_isnewer(bbzlamport_t lamport, bbzlamport_t old_lamport) {
    // This function uses a circular Lamport model (0 == 255 + 1).
    // A Lamport clock is 'newer' than an old Lamport clock if its value
    // is less than 'LAMPORT_THRESHOLD' ticks ahead of the old clock.

    uint8_t lamport_overflow = (uint8_t)((uint16_t)UINT8_MAX - old_lamport < BBZLAMPORT_THRESHOLD);
    if (lamport_overflow) {
        return (uint8_t)(lamport > old_lamport ||
                         lamport <= old_lamport + (uint8_t)BBZLAMPORT_THRESHOLD);
    }
    else {
        return (uint8_t)(lamport > old_lamport &&
                         lamport <= old_lamport + (uint8_t)BBZLAMPORT_THRESHOLD);
    }
}

void bbzvm_process_inmsgs() {
//    if(vm->state != BBZVM_STATE_READY) return;
    /* Go through the messages */
    while(!bbzinmsg_queue_isempty()) {
        /* Extract the message data */
        bbzmsg_t* msg = bbzinmsg_queue_extract();
        bbzheap_idx_t rid, key;
        switch(msg->type) {
            case BBZMSG_BROADCAST:
                // Get the robot ID of the sender.
                bbzvm_pushi(msg->bc.rid);
                rid = bbzvm_stack_at(0);
                bbzvm_pop();
                // Get the topic
                bbzvm_pushs(msg->bc.topic);
                bbzheap_idx_t topic = bbzvm_stack_at(0);
                bbzvm_pop();
                // Check if the topic has a listener. Break out of the switch if not.
                bbzheap_idx_t l;
                if (!bbztable_get(vm->neighbors.listeners, topic, &l)) break;
                // Call the listener
                bbzvm_push(l);
                bbzvm_push(topic);
                bbzvm_push(msg->bc.value);
                bbzvm_push(rid);
                bbzvm_closure_call(3);
                break;
            case BBZMSG_VSTIG_QUERY: // fallthrough
            case BBZMSG_VSTIG_PUT: {
                // Get the robot ID of the sender.
                bbzvm_pushs(msg->vs.key);
                key = bbzvm_stack_at(0);
                bbzvm_pop();
                // Search the key in the vstig
                uint8_t inLocalVStig = 0;
                for (uint16_t i = 0; i < vm->vstig.size; ++i) {
                    if (bbztype_cmp(bbzheap_obj_at(key), bbzheap_obj_at(vm->vstig.data[i].key)) == 0) {
                        inLocalVStig = 1;
                        if (bbzlamport_isnewer(msg->vs.lamport, vm->vstig.data[i].timestamp)) {
                            // Update the value
                            vm->vstig.data[i].robot = msg->vs.rid;
                            vm->vstig.data[i].key = key;
                            vm->vstig.data[i].value = msg->vs.data;
                            vm->vstig.data[i].timestamp = msg->vs.lamport;
                            // Propagate the value.
                            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, vm->vstig.data[i].robot,
                                                         bbzheap_obj_at(vm->vstig.data[i].key)->s.value,
                                                         vm->vstig.data[i].value, vm->vstig.data[i].timestamp);
                        } // The following "else if" is only for VSTIG_QUERY mesages.
                        else if (msg->type == BBZMSG_VSTIG_QUERY &&
                                bbzlamport_isnewer(vm->vstig.data[i].timestamp, msg->vs.lamport)) {
                            /* Local element is newer */
                            /* Append a PUT message to the out message queue */
                            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, vm->robot, msg->vs.key,
                                                         vm->vstig.data[i].value, vm->vstig.data[i].timestamp);
                        }
                        else if (vm->vstig.data[i].timestamp == msg->vs.lamport &&
                                 vm->vstig.data[i].robot != msg->vs.rid) {
                            // Conflict! Call the onconflict callback closure.
                            bbzheap_idx_t tmp = vm->nil;
                            // Check if there is a callback closure.
                            if (bbztable_get(vm->vstig.hpos, bbzvm_get(__BBZSTRID___INTERNAL_1_DO_NOT_USE__, s),
                                             &tmp)) {
                                bbzvm_push(tmp);
                                bbzvm_push(key);
                                // push the local data
                                bbzvm_pusht();
                                bbztable_add_data(__BBZSTRID_robot, bbzvm_get(vm->vstig.data[i].robot, i));
                                bbztable_add_data(__BBZSTRID_data, vm->vstig.data[i].value);
                                bbztable_add_data(__BBZSTRID_timestamp, bbzvm_get(vm->vstig.data[i].timestamp, i));
                                // push the remote data
                                bbzvm_pusht();
                                bbzheap_idx_t rd = bbzvm_stack_at(0);
                                bbztable_add_data(__BBZSTRID_robot, bbzvm_get(msg->vs.rid, i));
                                bbztable_add_data(__BBZSTRID_data, msg->vs.data);
                                bbztable_add_data(__BBZSTRID_timestamp, bbzvm_get(msg->vs.lamport, i));
                                bbzvm_closure_call(3);
                                // Update the value with the table returned by the closure.
                                if (bbztype_istable(*bbzheap_obj_at(bbzvm_stack_at(0)))) {
                                    tmp = 0;
                                    bbztable_get(bbzvm_stack_at(0), bbzvm_get(__BBZSTRID_robot, s), &tmp);
                                    bbzrobot_id_t oldRID = vm->vstig.data[i].robot;
                                    vm->vstig.data[i].robot = tmp ?
                                                              (bbzrobot_id_t) bbzheap_obj_at(tmp)->i.value :
                                                              vm->vstig.data[i].robot;
                                    tmp = vm->nil;
                                    bbztable_get(bbzvm_stack_at(0), bbzvm_get(__BBZSTRID_data, s), &tmp);
                                    vm->vstig.data[i].value = tmp;
                                    vm->vstig.data[i].timestamp = msg->vs.lamport;
                                    // If this is the robot that lost, call the onconflictlost callback closure.
                                    if ((bbzrobot_id_t) bbzheap_obj_at(tmp)->i.value != vm->robot &&
                                        oldRID == vm->robot) {
                                        // Check if there is an onconflictlost callback closure.
                                        tmp = vm->nil;
                                        if (bbztable_get(vm->vstig.hpos,
                                                         bbzvm_get(__BBZSTRID___INTERNAL_2_DO_NOT_USE__, s), &tmp)) {
                                            bbzvm_push(tmp);
                                            bbzvm_push(key);
                                            bbzvm_push(rd);
                                            bbzvm_closure_call(2);
                                        }
                                    }
                                    // Propagate the winning value.
                                    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, vm->vstig.data[i].robot,
                                                                 bbzheap_obj_at(vm->vstig.data[i].key)->s.value,
                                                                 vm->vstig.data[i].value, vm->vstig.data[i].timestamp);
                                } else {
                                    // Error, either no value was returned, or the returned value is of the wrong type.
                                    bbzvm_seterror(BBZVM_ERROR_RET);
                                    return;
                                }
                            }
                            else {
                                // No conflict manager, use default behavior.
                                if (msg->vs.rid >= vm->vstig.data[i].robot) {
                                    vm->vstig.data[i].robot = msg->vs.rid;
                                    vm->vstig.data[i].key = key;
                                    vm->vstig.data[i].value = msg->vs.data;
                                    vm->vstig.data[i].timestamp = msg->vs.lamport;
                                }
                                // Propagate the winning value.
                                bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, vm->vstig.data[i].robot,
                                                             bbzheap_obj_at(vm->vstig.data[i].key)->s.value,
                                                             vm->vstig.data[i].value, vm->vstig.data[i].timestamp);
                            }
                        }
                        break;
                    }
                }
                if (!inLocalVStig) {
                    vm->vstig.data[vm->vstig.size].robot = msg->vs.rid;
                    vm->vstig.data[vm->vstig.size].key = key;
                    vm->vstig.data[vm->vstig.size].value = msg->vs.data;
                    vm->vstig.data[vm->vstig.size].timestamp = msg->vs.lamport;
                    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                                 vm->vstig.data[vm->vstig.size].robot,
                                                 msg->vs.key,
                                                 vm->vstig.data[vm->vstig.size].value,
                                                 vm->vstig.data[vm->vstig.size].timestamp);
                    ++vm->vstig.size;
                }
                break;
            }
            case BBZMSG_SWARM_CHUNK: {
                // TODO
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
    // TODO
}

/****************************************/
/****************************************/

ALWAYS_INLINE
void bbzvm_clear_stack() {
    for (uint16_t i = 0; i < BBZSTACK_SIZE; ++i) {
        vm->stack[i] = 0;
    }
}

ALWAYS_INLINE
void dftl_error_receiver(bbzvm_error errcode) { }

void bbzvm_construct(bbzrobot_id_t robot) {
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
    bbzvm_clear_stack();

    // Allocate singleton objects
    bbzheap_obj_alloc(BBZTYPE_NIL, &vm->nil);
    bbzdarray_new(&vm->dflt_actrec);
    bbzdarray_push(vm->dflt_actrec, vm->nil);

    // Create various arrays
    bbzdarray_new(&vm->flist);

    // Create global symbols table
    bbzheap_obj_alloc(BBZTYPE_TABLE, &vm->gsyms);

    // Register things
    // bbzswarm_register(); // TODO
    bbzneighbors_register();
    bbzvstig_register();
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
    // 1) Reset the VM
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZVM_ERROR_NONE;
    vm->pc = 0;

    // 2) Set the bytecode
    vm->bcode_fetch_fun = bcode_fetch_fun;
    vm->bcode_size = bcode_size;

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

#define assert_pc(IDX) if((IDX) < 0 || (IDX) >= vm->bcode_size) { bbzvm_seterror(BBZVM_ERROR_PC); return; }

#define inc_pc() assert_pc(vm->pc); ++vm->pc;

#define get_arg(TYPE) assert_pc(vm->pc + sizeof(TYPE)); TYPE arg = *((TYPE*)vm->bcode_fetch_fun(vm->pc, sizeof(TYPE))); vm->pc += sizeof(TYPE);

uint8_t bbzvm_gc() {
    if (BBZSTACK_SIZE - bbzvm_stack_size() < 8 + 2*vm->vstig.size) {
        bbzvm_seterror(BBZVM_ERROR_STACK);
        return 0;
    }
    uint16_t stackSize = bbzvm_stack_size();

    // Push VM values that we don't want to garbage-collect
    bbzvm_push(vm->lsyms);
    bbzvm_push(vm->gsyms);
    bbzvm_push(vm->nil);
    bbzvm_push(vm->dflt_actrec);
    bbzvm_push(vm->flist);
    bbzvm_push(vm->neighbors.hpos);
    bbzvm_push(vm->neighbors.listeners);
    bbzvm_push(vm->vstig.hpos);
    for (uint8_t i = 0; i < vm->vstig.size; ++i) {
        bbzvm_push(vm->vstig.data[i].key);
        bbzvm_push(vm->vstig.data[i].value);
    }

    bbzheap_gc(vm->stack, bbzvm_stack_size());
    while (stackSize < bbzvm_stack_size()) {
        bbzvm_pop();
    }
    return 1;
}

/**
 * @brief Executes a single Buzz instruction.
 */
ALWAYS_INLINE
void bbzvm_exec_instr() {
    int16_t instrOffset = vm->pc; // Save PC in case of error or DONE.

    uint8_t instr = *(*vm->bcode_fetch_fun)(vm->pc, 1);
#ifdef DEBUG
    vm->dbg_pc = vm->pc;
    vm->instr = (bbzvm_instr)instr;
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
        if(!bbzvm_gc()) {
            bbzvm_seterror(BBZVM_ERROR_STACK);
            return;
        }
        bbzvm_exec_instr();
    }
}

/****************************************/
/****************************************/

void bbzvm_execute_script() {
    while(vm->state == BBZVM_STATE_READY) {
        bbzvm_step();
    }
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
 * @param[in] op The operation to perform.
 */
void bbzvm_binary_op_arith(binary_op_arith op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // MCUs usually only support integer operations.
    // Disallow floating-point operations.
    uint8_t ok = (uint8_t) (bbztype_isint(*lhs) && bbztype_isint(*rhs));
    if (ok) {
        bbzheap_idx_t idx;
        bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
        bbzvm_obj_at(idx)->i.value = (*op)(lhs->i.value, rhs->i.value);
        bbzvm_push(idx);
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
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
        bbzvm_seterror(BBZVM_ERROR_MATH);
        return 0;
    }
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
    bbzobj_t* operand = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();

    // We *could* implement the unary minus on a float, but we
    // disallow it, since we can do nothing with it anyway.
    uint8_t ok = (uint8_t)bbztype_isint(*operand);
    if (ok) {
        bbzheap_idx_t idx;
        bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
        bbzvm_obj_at(idx)->i.value = -operand->i.value;
        bbzvm_push(idx);
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_TYPE);
    }
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
void bbzvm_binary_op_logic(binary_op_logic op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    uint8_t
            lhs_bool = bbztype_tobool(lhs),
            rhs_bool = bbztype_tobool(rhs);

    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = (*op)(lhs_bool, rhs_bool);
    return bbzvm_push(idx);
}

uint8_t bbzand(uint8_t lhs, uint8_t rhs) { return lhs & rhs; }
uint8_t bbzor (uint8_t lhs, uint8_t rhs) { return lhs | rhs; }

/****************************************/
/****************************************/

void bbzvm_and() {
    return bbzvm_binary_op_logic(&bbzand);
}

/****************************************/
/****************************************/

void bbzvm_or() {
    return bbzvm_binary_op_logic(&bbzor);
}

/****************************************/
/****************************************/

void bbzvm_not() {
    bbzvm_assert_stack(1);
    bbzobj_t* operand = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzvm_pop();
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    switch(bbztype(*operand)) {
        case BBZTYPE_INT: bbzvm_obj_at(idx)->i.value = (operand->i.value != 0); break;
        case BBZTYPE_NIL: bbzvm_obj_at(idx)->i.value = 0; break;
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
void bbzvm_binary_op_cmp(binary_op_cmp op) {
    bbzvm_assert_stack(2);
    bbzobj_t* rhs = bbzvm_obj_at(bbzvm_stack_at(0));
    bbzobj_t* lhs = bbzvm_obj_at(bbzvm_stack_at(1));
    bbzvm_pop();
    bbzvm_pop();

    int8_t cmp = bbztype_cmp(lhs, rhs);

    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = (*op)(cmp);
    bbzvm_push(idx);
}

uint8_t bbzeq (int8_t cmp) { return (uint8_t) (cmp == 0); }
uint8_t bbzneq(int8_t cmp) { return (uint8_t) (cmp != 0); }
uint8_t bbzgt (int8_t cmp) { return (uint8_t) (cmp >  0); }
uint8_t bbzgte(int8_t cmp) { return (uint8_t) (cmp >= 0); }
uint8_t bbzlt (int8_t cmp) { return (uint8_t) (cmp <  0); }
uint8_t bbzlte(int8_t cmp) { return (uint8_t) (cmp <= 0); }

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
    // Allocate the table
    bbzheap_idx_t idx;
    bbzvm_assert_mem_alloc(BBZTYPE_TABLE, &idx);
    return bbzvm_push(idx);
}

/****************************************/
/****************************************/

void bbzvm_lload(uint16_t idx) {
    bbzheap_idx_t id = vm->nil;
    if (!bbzdarray_get(vm->lsyms, idx, &id)) {
        bbzvm_seterror(BBZVM_ERROR_LNUM);
    }
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
        case BBZTYPE_INT: if (o->i.value == 0) vm->pc = offset; break;
        case BBZTYPE_NIL: vm->pc = offset; break;
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
        case BBZTYPE_INT: if (o->i.value != 0) vm->pc = offset; assert_pc(vm->pc); break;
        case BBZTYPE_NIL: break;
        default: bbzvm_seterror(BBZVM_ERROR_TYPE); return;
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

void bbzvm_closure_call(uint16_t argc) {
    bbzvm_pushi(argc);
    int16_t blockptr = vm->blockptr;
    bbzvm_callc();
    while(blockptr < vm->blockptr) {
        if(vm->state != BBZVM_STATE_READY) return;
        bbzvm_step();
    }
    return;
}

/****************************************/
/****************************************/

void bbzvm_function_call(uint16_t fname, uint16_t argc) {
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
        return;
    }
    /* Move closure before arguments */
    if(argc > 0) {
        bbzheap_idx_t c = bbzvm_stack_at( 0);
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
    if (vm->state == BBZVM_STATE_ERROR) return 0;
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

void bbzvm_call(uint8_t isswrm) {
    /* Get argument number and pop it */
    bbzvm_assert_stack(1);
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    uint16_t argn = (uint16_t)bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    /* Make sure the stack has enough elements */
    bbzvm_assert_stack(argn+1);
    /* Make sure the closure is where expected */
    bbzvm_assert_type(bbzvm_stack_at(argn), BBZTYPE_CLOSURE);
    bbzobj_t* c = bbzvm_obj_at(bbzvm_stack_at(argn));
    /* Make sure that that data about lambda closures is correct */
    if(bbztype_isclosurelambda(*c) &&
       ((c->l.value.ref) >= bbzdarray_size(vm->flist))) {
        bbzvm_seterror(BBZVM_ERROR_FLIST);
        return;
    }
    /* Keep a refererence to the old local symbol list */
    bbzheap_idx_t oldLsyms = vm->lsyms;
    /* Create a new local symbol list copying the parent's */
    if (!bbztype_isclosurelambda(*c) ||
        (c->l.value.actrec) == BBZ_DFLT_ACTREC) {
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
        bbzdarray_push(vm->lsyms, bbzvm_stack_at(i - (uint16_t)1));
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
}

/****************************************/
/****************************************/

void bbzvm_pop() {
    if(bbzvm_stack_size() > 0) {
        --vm->stackptr;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_STACK);
    }
}

/****************************************/
/****************************************/

void bbzvm_dup() {
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
}

/****************************************/
/****************************************/

void bbzvm_push(bbzheap_idx_t v) {
    if (bbzvm_stack_size() >= BBZSTACK_SIZE) {
        bbzvm_seterror(BBZVM_ERROR_STACK);
        return;
    }
    vm->stack[++vm->stackptr] = v;
}

/****************************************/
/****************************************/

void bbzvm_pushu(void* v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
    bbzheap_obj_at(o)->u.value = (uintptr_t)v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_pushnil() {
    return bbzvm_push(vm->nil);
}

/****************************************/
/****************************************/

void bbzvm_pushc(intptr_t rfrnc, int16_t nat) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc((uint8_t)(BBZTYPE_CLOSURE | ((nat & 1) << 1)), &o);
    bbzvm_obj_at(o)->c.value = (void*)rfrnc;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_pushi(int16_t v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_INT, &o);
    bbzvm_obj_at(o)->i.value = v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_pushf(bbzfloat v) {
    bbzheap_idx_t o;
    bbzvm_assert_mem_alloc(BBZTYPE_FLOAT, &o);
    bbzheap_obj_at(o)->f.value = v;
    return bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_pushs(uint16_t strid) {
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

void bbzvm_pushl(uint16_t addr) {
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

    bbzvm_push(o);
}

/****************************************/
/****************************************/

void bbzvm_tput() {
    // Get value, key and table, and pop them.
    bbzvm_assert_stack(3);
    bbzvm_assert_type(bbzvm_stack_at(2), BBZTYPE_TABLE);
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
}

/****************************************/
/****************************************/

void bbzvm_tget() {
    // Get & pop the arguments
    bbzvm_assert_stack(2);
    bbzvm_assert_type(bbzvm_stack_at(1), BBZTYPE_TABLE);
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
}

/****************************************/
/****************************************/

void bbzvm_gload() {
    // Get and pop the string
    bbzvm_assert_stack(1);
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_STRING);
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
}

/****************************************/
/****************************************/

void bbzvm_gstore() {
    // Get and pop the arguments
    bbzvm_assert_stack(2);
    bbzvm_assert_type(bbzvm_stack_at(1), BBZTYPE_STRING);
    bbzheap_idx_t str = bbzvm_stack_at(1);
    bbzheap_idx_t o = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzvm_pop();
    bbzvm_assert_state();

    // Store the value
    if (!bbztable_set(vm->gsyms, str, o)) {
        bbzvm_seterror(BBZVM_ERROR_MEM);
    }
}

/****************************************/
/****************************************/

void bbzvm_ret0() {
    /* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm->lsyms)->t)) {
        //TODO pop the swarm stack.
    }
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzvm_obj_at(vm->stack[vm->stackptr])->i.value;
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_assert_stack(1);
    /* Pop local symbol table */
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure the stack contains at least one element */
    bbzvm_assert_stack(1);
    /* Make sure that element is an integer */
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    /* Use that element as program counter */
    vm->pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
}

/****************************************/
/****************************************/

void bbzvm_ret1() {
    /* Pop swarm stack */
    if (bbzdarray_isswarm(&bbzvm_obj_at(vm->lsyms)->t)) {
        //TODO pop the swarm stack.
    }
    /* Make sure there's enough elements on the stack */
    bbzvm_assert_stack(3);
    /* Save it, it's the return value to pass to the lower stack */
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    /* Pop block pointer and stack */
    vm->stackptr = vm->blockptr;
    vm->blockptr = bbzvm_obj_at(vm->stack[vm->blockptr])->i.value;
    bbzvm_pop();
    /* Pop local symbol table */
    bbzdarray_destroy(vm->lsyms);
    vm->lsyms = bbzvm_stack_at(0);
    bbzvm_pop();
    /* Make sure that element is an integer */
    bbzvm_assert_type(bbzvm_stack_at(0), BBZTYPE_INT);
    /* Use that element as program counter */
    vm->pc = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    /* Pop the return address */
    bbzvm_pop();
    /* Push the return value */
    bbzvm_push(ret);
}

/****************************************/
/****************************************/
