#include <stdio.h>
#include <bittybuzz/bbztype.h>
#include <bittybuzz/bbzvm.h>

#define NUM_TEST_CASES 17
#define TEST_MODULE vm
#include "testingconfig.h"

    // ======================================
    // =                MISC                =
    // ======================================

#define FLIST_COUNT_DEFAULT 0
#define GSYMS_COUNT_DEFAULT 1
#ifndef BBZ_DISABLE_SWARMS
#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
#define FLIST_COUNT_SWARMS 0
#define GSYMS_COUNT_SWARMS 1
#else // !BBZ_DISABLE_SWARMLIST_BROADCASTS
#define FLIST_COUNT_SWARMS 0
#define GSYMS_COUNT_SWARMS 1
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
#else // !BBZ_DISABLE_SWARMS
#define FLIST_COUNT_SWARMS 0
#define GSYMS_COUNT_SWARMS 0
#endif // !BBZ_DISABLE_SWARMS
#ifndef BBZ_DISABLE_NEIGHBORS
#define FLIST_COUNT_NEIGHBORS 0
#define GSYMS_COUNT_NEIGHBORS 1
#else // !BBZ_DISABLE_NEIGHBORS
#define FLIST_COUNT_NEIGHBORS 0
#define GSYMS_COUNT_NEIGHBORS 0
#endif // !BBZ_DISABLE_NEIGHBORS
#ifndef BBZ_DISABLE_VSTIGS
#define FLIST_COUNT_VSTIGS 0
#define GSYMS_COUNT_VSTIGS 1
#else // !BBZ_DISABLE_VSTIGS
#define FLIST_COUNT_VSTIGS 0
#define GSYMS_COUNT_VSTIGS 0
#endif // !BBZ_DISABLE_VSTIGS
#define FLIST_COUNT ((FLIST_COUNT_DEFAULT)+(FLIST_COUNT_SWARMS)+(FLIST_COUNT_NEIGHBORS)+(FLIST_COUNT_VSTIGS))
#define GSYMS_COUNT ((GSYMS_COUNT_DEFAULT)+(GSYMS_COUNT_SWARMS)+(GSYMS_COUNT_NEIGHBORS)+(GSYMS_COUNT_VSTIGS))

FILE* fbcode;
int16_t fsize;
uint8_t buf[4];
bbzvm_error last_error;
static bbzvm_t vmObj;

char* state_desc[] = {"BBZVM_STATE_NOCODE", "BBZVM_STATE_READY", "BBZVM_STATE_STOPPED", "BBZVM_STATE_DONE", "BBZVM_STATE_ERROR"};
char* error_desc[] = {"BBZVM_ERROR_NONE", "BBZVM_ERROR_INSTR", "BBZVM_ERROR_STACK", "BBZVM_ERROR_LNUM", "BBZVM_ERROR_PC",
                      "BBZVM_ERROR_FLIST", "BBZVM_ERROR_TYPE", "BBZVM_ERROR_OUTOFRANGE", "BBZVM_ERROR_NOTIMPL",
                      "BBZVM_ERROR_RET", "BBZVM_ERROR_STRING", "BBZVM_ERROR_SWARM", "BBZVM_ERROR_VSTIG", "BBZVM_ERROR_MEM",
                      "BBZVM_ERROR_MATH"};
char* instr_desc[] = {"NOP", "DONE", "PUSHNIL", "DUP", "POP", "RET0", "RET1", "ADD", "SUB", "MUL", "DIV", "MOD", "POW",
                      "UNM", "LAND", "LOR", "LNOT","BAND","BOR","BNOT", "EQ", "NEQ", "GT", "GTE", "LT", "LTE", "GLOAD", "GSTORE", "PUSHT", "TPUT",
                      "TGET", "CALLC", "CALLS", "PUSHF", "PUSHI", "PUSHS", "PUSHCN", "PUSHCC", "PUSHL", "LLOAD", "LSTORE", "LREMOVE",
                      "JUMP", "JUMPZ", "JUMPNZ", "COUNT"};

/**
 * @brief Fetches bytecode from a FILE.
 * @param[in] offset Offset of the bytes to fetch.
 * @param[in] size Size of the data to fetch.
 * @return A pointer to the data fetched.
 */
const uint8_t* testBcode(bbzpc_t offset, uint8_t size) {
    if (offset + size - 2 >= fsize) {
        fprintf(stderr, "Trying to read outside of bytecode. Offset: %"
                        PRIu16 ", size: %" PRIu8 ".", offset, size);
    }
    else {
        switch(size) {
            case sizeof(uint8_t):  // Fallthrough
            case sizeof(uint16_t): // Fallthrough
            case sizeof(uint32_t): {
                fseek(fbcode, offset, SEEK_SET);
                RM_UNUSED_RETVAL_WARN(fread(buf, size, 1, fbcode));
                break;
            }
            default: {
                fprintf(stderr, "Bad bytecode size: %" PRIu8 ".", size);
                break;
            }
        }
    }
    return buf;
}

/**
 * @brief Skips a single instruction, with its operand, if any.
 */
void bbzvm_skip_instr() {
    uint8_t has_operand = (*testBcode(vm->pc, sizeof(uint8_t)) >= BBZVM_INSTR_PUSHF);
    vm->pc += sizeof(uint8_t); // Skip opcode
    if (has_operand) {
        vm->pc += sizeof(uint16_t); // Skip operand
    }
}

/**
 * @brief Sets the error code.
 * @param[in] errcode The error type.
 */
void set_last_error(bbzvm_error errcode) {
    last_error = errcode;
#ifdef DEBUG
    printf("VM:\n\tstate: %s\n\tpc: %d\n\tinstr: %s\n\terror state: %s\n", state_desc[vm->state], vm->dbg_pc, instr_desc[*vm->bcode_fetch_fun(vm->dbg_pc, 1)], error_desc[vm->error]);
    bbzheap_print();
#endif
}

void set_last_error_no_print(bbzvm_error errcode) {
    last_error = errcode;
}

/**
 * @brief Gets and clears the error code.
 * @return The error type.
 */
bbzvm_error get_last_error() {
    bbzvm_error e = last_error;
    last_error = BBZVM_ERROR_NONE;
    return e;
}

/**
 * @brief Resets the VM's state and error.
 */
 void bbzvm_reset_state() {
     vm->state = BBZVM_STATE_READY;
     vm->error = BBZVM_ERROR_NONE;
 }

/**
 * @brief Function used for testing C closures.
 */
void printIntVal() {
    bbzvm_assert_lnum(1);
#ifdef DEBUG
    bbzheap_idx_t idx = bbzvm_locals_at(1);
    printf("#%d taken as integer: %d\n", idx, bbzheap_obj_at(idx)->i.value);
#endif
    return bbzvm_ret0();
}


void logfunc() {
    int16_t argn = bbzheap_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    for (int16_t i = 0; i < argn; ++i) {
        bbzobj_t* o = bbzheap_obj_at(bbzvm_stack_at(i));
        switch(bbztype(*o)) {
            case BBZTYPE_NIL:
                printf("[nil]");
                break;
            case BBZTYPE_INT:
                printf("%d", o->i.value);
                break;
            case BBZTYPE_FLOAT:
                printf("%f", bbzfloat_tofloat(o->f.value));
                break;
            case BBZTYPE_TABLE:
                printf("[t:%d]%" PRIu16, bbztable_size(bbzvm_stack_at(i)), o->t.value);
                break;
            case BBZTYPE_USERDATA:
                printf("[u]%" PRIXPTR, o->u.value);
                break;
            case BBZTYPE_STRING:
                printf("[s]%d", (o->s.value));
                break;
            case BBZTYPE_CLOSURE:
                if (bbztype_isclosurelambda(*o))
                    printf("[cl]%d", o->l.value.ref);
                else
                    printf("[c]%d", (int)(intptr_t)o->c.value);
                break;
            default:
                break;
        }
        if (i < argn - 1) printf("; ");
    }
    printf("\n");
}

void bbzvm_log() {
    uint16_t nArg = bbzdarray_size(vm->lsyms) - 1;
    for (uint16_t i = 0; i < nArg; ++i) {
        bbzvm_lload(nArg - i);
    }
#ifdef DEBUG
    bbzvm_pushi(nArg);
    logfunc();fflush(stdout);
#endif
    return bbzvm_ret0();
}

typedef enum {
    BBZVM_SYMID_PRINT = _BBZSTRID_COUNT_,
    BBZVM_SYMID_FORWARD,
    BBZVM_SYMID_STOP,
    BBZVM_SYMID_TURN,

    BBZVM_SYMID_LEFT,
    BBZVM_SYMID_RIGHT
} bbzvm_symid;

void bbzvm_dummy() {
    return bbzvm_ret0();
}

int8_t bbzvm_register_functions() {
    if (bbzvm_function_register(BBZVM_SYMID_PRINT, bbzvm_log) == 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_FORWARD, bbzvm_dummy) == 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_STOP, bbzvm_dummy) == 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_TURN, bbzvm_dummy) == 0) return -1;
    return 0;
}

    // ======================================
    // =             UNIT TESTS             =
    // ======================================

#define FILE_TEST1 "resources/1_InstrTest.bbo"
#define FILE_TEST2 "resources/2_IfTest.bbo"
#define FILE_TEST3 "resources/3_test1.bbo"
#define FILE_TEST4 "resources/sample.bbo"

TEST(vm_construct) {
    vm = &vmObj;

    uint16_t robot = 0;
    bbzvm_construct(robot);

    ASSERT_EQUAL(vm->pc, 0);
    ASSERT(bbztype_isdarray(*bbzheap_obj_at(vm->flist)));
    ASSERT(bbztype_istable (*bbzheap_obj_at(vm->gsyms)));
    ASSERT(bbztype_isnil   (*bbzheap_obj_at(vm->nil)));
    ASSERT(bbztype_isdarray(*bbzheap_obj_at(vm->dflt_actrec)));
    ASSERT_EQUAL(vm->state, BBZVM_STATE_NOCODE);
    ASSERT_EQUAL(vm->error, BBZVM_ERROR_NONE);
    ASSERT_EQUAL(vm->robot, robot);

    bbzvm_destruct();
}

TEST(vm_set_bytecode) {
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(&set_last_error);

    // 1) Open bytecode file.
    fbcode = fopen(FILE_TEST2, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    // 2) Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    ASSERT_EQUAL((uintptr_t)vm->bcode_fetch_fun, (uintptr_t)&testBcode);
    ASSERT_EQUAL(vm->bcode_size, fsize);
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(vm->error, BBZVM_ERROR_NONE);
    ASSERT_EQUAL(bbzdarray_size(vm->flist), FLIST_COUNT);
    ASSERT_EQUAL(bbztable_size(vm->gsyms), GSYMS_COUNT); // 'id', 'neighbors', 'stigmergy', 'swarm'
    ASSERT_EQUAL(*testBcode(vm->pc-1, 1), BBZVM_INSTR_NOP);

    bbzvm_destruct();
    fclose(fbcode);
}

#define vm_step_instr()                         \
    vm = &vmObj;                                \
    bbzvm_construct(0);                         \
    bbzvm_set_error_receiver(set_last_error);   \
    fbcode = fopen(FILE_TEST1, "rb");           \
    REQUIRE(fbcode != NULL);                    \
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);   \
    fsize = ftell(fbcode);                      \
    REQUIRE(fsize > 0);                         \
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);   \
    vm->state = BBZVM_STATE_READY;              \
    vm->error = BBZVM_ERROR_NONE;               \
    vm->bcode_fetch_fun = testBcode;            \
    vm->bcode_size = fsize;

TEST(vm_step_nop) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 2;

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_NOP);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, 3);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_done) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 3;

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_DONE);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_DONE);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    // Make sure we are looping on DONE.
    ASSERT_EQUAL(vm->pc, 3);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_pushnil) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 4;

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_PUSHNIL);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 1);
    ASSERT(bbztype_isnil(*bbzheap_obj_at(bbzvm_stack_at(0))));
    ASSERT_EQUAL(vm->pc, 5);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_pop) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 5;

    vm->stack[++vm->stackptr] = vm->nil;

    REQUIRE(bbzvm_stack_size() == 1);
    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_POP);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, 6);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_pushi) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 6;

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_PUSHI);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 1);
    ASSERT(bbztype_isint(*bbzheap_obj_at(bbzvm_stack_at(0))));
    ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(0))->i.value, 0x42);
    ASSERT_EQUAL(vm->pc, 9);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_dup) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 9;

    REQUIRE(bbzheap_obj_alloc(BBZTYPE_INT, &vm->stack[++vm->stackptr]));
    bbzheap_obj_at(vm->stack[vm->stackptr])->i.value = 0x42;

    REQUIRE(bbzvm_stack_size() == 1);
    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_DUP);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 2);
    ASSERT(bbztype_isint(*bbzheap_obj_at(bbzvm_stack_at(1))));
    ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(1))->i.value, bbzheap_obj_at(bbzvm_stack_at(0))->i.value);
    ASSERT_EQUAL(vm->pc, 10);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_jump) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 10;
    bbzpc_t jumpAddr = *(bbzpc_t*)vm->bcode_fetch_fun(vm->pc + 1, sizeof(bbzpc_t));

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_JUMP);
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, jumpAddr);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_jumpz) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 13;
    bbzpc_t jumpAddr = *(bbzpc_t*)vm->bcode_fetch_fun(vm->pc + 1, sizeof(bbzpc_t));

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_JUMPZ);
    bbzvm_pushnil(); // Nil on the stack; Should jump
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, jumpAddr);

    vm->pc = 13;

    REQUIRE(vm->state == BBZVM_STATE_READY);
    bbzvm_pushi(0); // 0 on the stack; Should jump
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, jumpAddr);

    vm->pc = 13;

    REQUIRE(vm->state == BBZVM_STATE_READY);
    bbzvm_pushi(-1); // Non-0 on the stack; Shouldn't jump
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, 16);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_step_jumpnz) {
    vm_step_instr();

    // 3) Set the program counter
    vm->pc = 16;
    bbzpc_t jumpAddr = *(bbzpc_t*)vm->bcode_fetch_fun(vm->pc + 1, sizeof(bbzpc_t));

    REQUIRE(*testBcode(vm->pc, 1) == BBZVM_INSTR_JUMPNZ);
    bbzvm_pushi(-1); // Non-0 on the stack; Should jump
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, jumpAddr);

    vm->pc = 16;

    REQUIRE(vm->state == BBZVM_STATE_READY);
    bbzvm_pushnil(); // Nil on the stack; Should jump
    bbzvm_step();
    ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm->pc, 19);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_arith_logic) {
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(&set_last_error);

    fbcode = fopen(FILE_TEST1, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    // A) Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    // Set the program counter
    vm->pc = 19;

    {
        const int16_t LHS_INT = -21244, RHS_INT = 8384;
        bbzheap_idx_t lhs, rhs;
        bbzheap_obj_alloc(BBZTYPE_INT, &lhs);
        bbzheap_obj_alloc(BBZTYPE_INT, &rhs);
        bbzheap_obj_at(lhs)->i.value = LHS_INT;
        bbzheap_obj_at(rhs)->i.value = RHS_INT;

        const bbzvm_instr LAST_INSTR = (bbzvm_instr)-1;
        bbzvm_instr instrs[] = {
                BBZVM_INSTR_ADD, BBZVM_INSTR_SUB, BBZVM_INSTR_MUL, BBZVM_INSTR_DIV, BBZVM_INSTR_MOD,
                BBZVM_INSTR_LAND,  BBZVM_INSTR_LOR,  BBZVM_INSTR_EQ, BBZVM_INSTR_NEQ,  BBZVM_INSTR_GT,
                BBZVM_INSTR_GTE,  BBZVM_INSTR_LT, BBZVM_INSTR_LTE, LAST_INSTR
        };
        const int16_t results[] = {
                -12860, -29628, 0x4300, -2, -4476,
                    !0,     !0,      0, !0,     0,
                     0,     !0,     !0
        };
        uint16_t i = 0;
        bbzvm_instr curr_instr = instrs[i];
        while(curr_instr != LAST_INSTR) {
            bbzvm_push(lhs);
            bbzvm_push(rhs);
            REQUIRE(bbzvm_stack_size() == 2);
            REQUIRE((bbzvm_instr)*testBcode(vm->pc, 1) == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(0))->i.value, results[i]);
            ASSERT_EQUAL(vm->state, BBZVM_STATE_READY);
            ASSERT_EQUAL(vm->error, BBZVM_ERROR_NONE);

            bbzvm_pop();
            bbzvm_reset_state();
            curr_instr = instrs[++i];
        }
    }

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_stack_empty) {
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(&set_last_error_no_print);

    fbcode = fopen(FILE_TEST1, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    // Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    // Set the program counter
    vm->pc = 32;

    // ---- Test failing operations ----
    // Perform some basic operations when stack is empty
    {
        REQUIRE(bbzvm_stack_size() == 0);
        ASSERT_EQUAL(get_last_error(), BBZVM_ERROR_NONE);

        const bbzvm_instr LAST_INSTR = (bbzvm_instr)-1;
        bbzvm_instr failing_instr[] = {
                BBZVM_INSTR_POP, BBZVM_INSTR_DUP, BBZVM_INSTR_ADD, BBZVM_INSTR_SUB,   BBZVM_INSTR_MUL,
                BBZVM_INSTR_DIV, BBZVM_INSTR_MOD, BBZVM_INSTR_POW, BBZVM_INSTR_UNM,   BBZVM_INSTR_LAND,
                BBZVM_INSTR_LOR,  BBZVM_INSTR_LNOT, BBZVM_INSTR_EQ,  BBZVM_INSTR_NEQ,   BBZVM_INSTR_GT,
                BBZVM_INSTR_GTE, BBZVM_INSTR_LT,  BBZVM_INSTR_LTE, BBZVM_INSTR_JUMPZ, BBZVM_INSTR_JUMPNZ,
                LAST_INSTR
        };
        int16_t oldPc = vm->pc;
        uint16_t i = 0;
        bbzvm_instr curr_instr = failing_instr[i++];
        while(curr_instr != LAST_INSTR) {
            REQUIRE(bbzvm_stack_size() == 0);
            REQUIRE((bbzvm_instr)*testBcode(vm->pc, 1) == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(vm->state, BBZVM_STATE_ERROR);
            ASSERT_EQUAL(vm->error, BBZVM_ERROR_STACK);

            REQUIRE(vm->pc == oldPc);
            bbzvm_skip_instr();
            bbzvm_reset_state();
            curr_instr = failing_instr[i++];
            oldPc = vm->pc;
            ASSERT_EQUAL(get_last_error(), BBZVM_ERROR_STACK);
        }
    }

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_stack_full) {
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(set_last_error_no_print);

    fbcode = fopen(FILE_TEST1, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    // 1) Reset the VM state
    vm->state = BBZVM_STATE_READY;
    vm->error = BBZVM_ERROR_NONE;

    // 2) Set the bytecode
    vm->bcode_fetch_fun = testBcode;
    vm->bcode_size = fsize;

    // 3) Set the program counter
    vm->pc = 56;

    // 4) Set local symbols
    bbzdarray_new(&vm->lsyms);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->lsyms));
    bbzdarray_push(vm->lsyms, vm->nil);

    REQUIRE(bbzvm_stack_size() == 0);
    for (uint16_t i = 0; i < BBZSTACK_SIZE; ++i) {
        bbzvm_push(vm->nil);
    }

    {
        REQUIRE(bbzvm_stack_size() == BBZSTACK_SIZE);

        const bbzvm_instr LAST_INSTR = (bbzvm_instr)-1;
        bbzvm_instr failing_instr[] = {
                BBZVM_INSTR_DUP,    BBZVM_INSTR_PUSHNIL, BBZVM_INSTR_PUSHF, BBZVM_INSTR_PUSHI, BBZVM_INSTR_PUSHS,
                BBZVM_INSTR_PUSHCN, BBZVM_INSTR_PUSHCC,  BBZVM_INSTR_PUSHL, BBZVM_INSTR_LLOAD, LAST_INSTR
        };

        uint16_t i = 0;
        bbzvm_instr curr_instr = failing_instr[i++];
        while(curr_instr != LAST_INSTR) {
            REQUIRE(bbzvm_stack_size() == BBZSTACK_SIZE);
            bbzvm_instr instr = (bbzvm_instr)*testBcode(vm->pc, 1);
            REQUIRE(instr == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(vm->state, BBZVM_STATE_ERROR);
            ASSERT_EQUAL(vm->error, BBZVM_ERROR_STACK);
            bbzvm_skip_instr();
            bbzvm_reset_state();
            curr_instr = failing_instr[i++];
        }
    }

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_closures) {
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(&set_last_error);

    fbcode = fopen(FILE_TEST3, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    // A) Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    //ASSERT_EQUAL(bbztable_size(vm->gsyms), 5);

    // B) Register C closure
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    bbzheap_idx_t c = bbzvm_function_register(BBZVM_SYMID_PRINT, printIntVal);

    REQUIRE(c > 0);
    ASSERT_EQUAL(bbztype(*bbzheap_obj_at(c)), BBZTYPE_CLOSURE);
    ASSERT_EQUAL((intptr_t)bbzheap_obj_at(c)->c.value, (intptr_t)printIntVal);

    // C) Call registered C closure
    //REQUIRE(bbztable_size(vm->gsyms) == *(uint16_t*)vm->bcode_fetch_fun(0, 2));
    bbzvm_pushs(BBZVM_SYMID_PRINT);
    bbzvm_gload();
    c = bbzvm_stack_at(0);
    bbzvm_pop();
    REQUIRE(bbztype_isclosure(*bbzheap_obj_at(c)));
    bbzvm_pushnil(); // Push self table
    bbzvm_pushi(123);
    bbzvm_function_call(BBZVM_SYMID_PRINT, 1);
    ASSERT(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);

    // D) Execute the rest of the script to check if a call from a buzz script works properly
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    while(vm->state == BBZVM_STATE_READY) {
#ifdef DEBUG
        uint8_t instr = *vm->bcode_fetch_fun(vm->pc,1);
        if (instr > BBZVM_INSTR_CALLS) {
            printf("[%d: %s %d]\n", vm->pc, instr_desc[instr], *(int16_t*)vm->bcode_fetch_fun(vm->pc+1,2));
        }
        else {
            printf("[%d: %s]\n", vm->pc, instr_desc[instr]);
        }
#endif
        bbzvm_step();
    }
    ASSERT(vm->state != BBZVM_STATE_ERROR);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST(vm_message_processing) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzvm_construct(0);
    bbzvm_set_error_receiver(set_last_error);

    // Setup
    bbzobj_t obj1;
    bbztype_cast(obj1, BBZTYPE_INT);
    obj1.i.value = 0x2345;

    uint8_t buf1[10];
    bbzmsg_payload_t payload1;
    bbzringbuf_construct(&payload1, buf1, 1, 10);

    bbzmsg_serialize_u8 (&payload1, BBZMSG_VSTIG_PUT);
    bbzmsg_serialize_u16(&payload1, 42);
    bbzmsg_serialize_u16(&payload1, __BBZSTRID_put);
    bbzmsg_serialize_obj(&payload1, &obj1);
    bbzmsg_serialize_u8 (&payload1, 1);

    bbzinmsg_queue_append(&payload1);

    bbzvm_process_inmsgs();
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT(bbzinmsg_queue_isempty());
    REQUIRE(bbzinmsg_queue_size() == 0);
#ifndef BBZ_DISABLE_VSTIGS
    ASSERT_EQUAL(vm->vstig.data[0].robot, 42);
    ASSERT_EQUAL(vm->vstig.data[0].key, __BBZSTRID_put);
    ASSERT_EQUAL(bbztype(*bbzheap_obj_at(vm->vstig.data[0].value)), BBZTYPE_INT);
    ASSERT_EQUAL(bbzheap_obj_at(vm->vstig.data[0].value)->i.value, obj1.i.value);
    ASSERT_EQUAL(vm->vstig.data[0].timestamp, 1);
# endif // !BBZ_DISABLE_VSTIGS

    bbzvm_destruct();
}

TEST(vm_script_execution) {
    vm = &vmObj;

    uint16_t robot = 0;
    bbzvm_construct(robot);
    bbzvm_set_error_receiver(&set_last_error);
    fbcode = fopen(FILE_TEST4, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    REQUIRE(fsize > 0);
    REQUIRE(fseek(fbcode, 0, SEEK_SET) >= 0);

    bbzvm_set_bcode(&testBcode, fsize);

    REQUIRE(vm->state == BBZVM_STATE_READY);
    REQUIRE(bbzvm_register_functions() >= 0); // If this fails, it means that the heap doesn't have enough memory allocated to execute this test.

    while (vm->state == BBZVM_STATE_READY) {
#ifdef DEBUG
        uint8_t instr = *vm->bcode_fetch_fun(vm->pc,1);
        if (instr > BBZVM_INSTR_CALLS) {
            printf("[%d: %s %d]\n", vm->pc, instr_desc[instr], *(int16_t*)vm->bcode_fetch_fun(vm->pc+1,2));
        }
        else {
            printf("[%d: %s]\n", vm->pc, instr_desc[instr]);
        }
#endif
        bbzvm_step();
        ASSERT(vm->state != BBZVM_STATE_ERROR);
    }
    ASSERT_EQUAL(vm->state, BBZVM_STATE_DONE);
    ASSERT_EQUAL(vm->error, BBZVM_ERROR_NONE);

    bbzvm_destruct();
    fclose(fbcode);
}

TEST_LIST {
    ADD_TEST(vm_construct);
    ADD_TEST(vm_set_bytecode);
    ADD_TEST(vm_step_nop);
    ADD_TEST(vm_step_done);
    ADD_TEST(vm_step_pushnil);
    ADD_TEST(vm_step_pop);
    ADD_TEST(vm_step_pushi);
    ADD_TEST(vm_step_dup);
    ADD_TEST(vm_step_jump);
    ADD_TEST(vm_step_jumpz);
    ADD_TEST(vm_step_jumpnz);
    ADD_TEST(vm_arith_logic);
    ADD_TEST(vm_stack_empty);
    ADD_TEST(vm_stack_full);
    ADD_TEST(vm_closures);
    ADD_TEST(vm_message_processing);
    #if BBZHEAP_SIZE < 2048
    #warning\
    In test file "testvm.c": Running test of all features requires BBZHEAP_SIZE >= 2048\
    on 32 and 64 bit systems. The script execution test will be disabled.
    #else
    ADD_TEST(vm_script_execution);
    #endif
}
