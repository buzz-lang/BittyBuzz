#include "testing/testingconfig.h"
#ifdef BBZ_USE_AUTOMATED_TESTS
#define BOOST_TEST_DYN_LINK // Use Boost as a dynamic library
#define BOOST_TEST_MODULE BBZVM_TEST
#include <boost/test/unit_test.hpp>
#endif // BBZ_USE_AUTOMATED_TESTS

//#define BBZVM_USE_BBO

#include <stdio.h>
#include <bittybuzz/bbztype.h>
#include <bittybuzz/bbzvm.h>

#include "bittybuzz/bbzvm.c"

    // ======================================
    // =                MISC                =
    // ======================================

FILE* fbcode;
uint16_t fsize;
uint8_t buf[4];
bbzvm_error last_error;

char* state_desc[] = {"BBZVM_STATE_NOCODE", "BBZVM_STATE_READY", "BBZVM_STATE_DONE", "BBZVM_STATE_ERROR", "BBZVM_STATE_STOPPED"};
char* error_desc[] = {"BBZVM_ERROR_NONE", "BBZVM_ERROR_INSTR", "BBZVM_ERROR_STACK", "BBZVM_ERROR_LNUM", "BBZVM_ERROR_PC",
                      "BBZVM_ERROR_FLIST", "BBZVM_ERROR_TYPE", "BBZVM_ERROR_STRING", "BBZVM_ERROR_SWARM", "BBZVM_ERROR_MEM"};
char* instr_desc[] = {"NOP", "DONE", "PUSHNIL", "DUP", "POP", "RET0", "RET1", "ADD", "SUB", "MUL", "DIV", "MOD", "POW",
                      "UNM", "AND", "OR", "NOT", "EQ", "NEQ", "GT", "GTE", "LT", "LTE", "GLOAD", "GSTORE", "PUSHT", "TPUT",
                      "TGET", "CALLC", "CALLS", "PUSHF", "PUSHI", "PUSHS", "PUSHCN", "PUSHCC", "PUSHL", "LLOAD", "LSTORE",
                      "JUMP", "JUMPZ", "JUMPNZ", "COUNT"};

/**
 * @brief Fetches bytecode from a FILE.
 * @param[in] offset Offset of the bytes to fetch.
 * @param[in] size Size of the data to fetch.
 * @return A pointer to the data fetched.
 */
const uint8_t* testBcode(int16_t offset, uint8_t size) {
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
                fread(buf, size, 1, fbcode);
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
 * @param[in,out] vm The VM.
 */
 void bbzvm_skip_instr() {
     uint8_t has_operand = (*testBcode(vm.pc, sizeof(uint8_t)) >= BBZVM_INSTR_PUSHF);
     vm.pc += sizeof(uint8_t); // Skip opcode
     if (has_operand) {
#ifndef BBZVM_USE_BBO
        vm.pc += sizeof(uint32_t); // Skip operand
#else
        vm.pc += sizeof(uint16_t); // Skip operand
#endif
     }
 }

static const char* bbztype_desc[] = { "nil", "integer", "float", "string", "table", "closure", "userdata", "native closure" };

#define obj_isvalid(x) ((x).o.mdata & 0x10)

/**
 * @brief Prints the heap.
 */
void bbzheap_print() {
   /* Object-related stuff */
   int objimax = (vm.heap.rtobj - vm.heap.data) / sizeof(bbzobj_t);
   printf("Max object index: %d\n", objimax);
   int objnum = 0;
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(i))) ++objnum;
   printf("Valid objects: %d\n", objnum);
   for(int i = 0; i < objimax; ++i)
      if(obj_isvalid(*bbzheap_obj_at(i))) {
         printf("\t#%d: [%s]", i, bbztype_desc[bbztype(*bbzheap_obj_at(i))]);
         switch(bbztype(*bbzheap_obj_at(i))) {
             case BBZTYPE_NIL:
                 break;
             case BBZTYPE_STRING: // fallthrough
             case BBZTYPE_INT:
                 printf(" %d", bbzheap_obj_at(i)->i.value);
                 break;
             case BBZTYPE_FLOAT:
                 printf(" %f", bbzfloat_tofloat(bbzheap_obj_at(i)->f.value));
                 break;
             case BBZTYPE_TABLE:
                 printf(" %" PRIu16, bbzheap_obj_at(i)->t.value);
                 break;
             case BBZTYPE_USERDATA:
                 printf(" %" PRIXPTR, (uintptr_t)bbzheap_obj_at(i)->u.value);
                 break;
             case BBZTYPE_NCLOSURE: // fallthrough
             case BBZTYPE_CLOSURE:
                 if (bbztype_isclosurelambda(*bbzheap_obj_at(i)))
                     printf("[l] %d", (uint8_t)bbzheap_obj_at(i)->l.value.ref);
                 else
                     printf(" %d", (int)(intptr_t)bbzheap_obj_at(i)->c.value);
                 break;
             default:
                 break;
         }
         printf("\n");
      }
   /* Segment-related stuff */
   int tsegimax = (vm.heap.data + BBZHEAP_SIZE - vm.heap.ltseg) / sizeof(bbzheap_tseg_t);
   printf("Max table segment index: %d\n", tsegimax);
   int tsegnum = 0;
   for(int i = 0; i < tsegimax; ++i)
      if(bbzheap_tseg_isvalid(*bbzheap_tseg_at(i))) ++tsegnum;
   printf("Valid table segments: %d\n", tsegnum);
   bbzheap_tseg_t* seg;
   for(int i = 0; i < tsegimax; ++i) {
      seg = bbzheap_tseg_at(i);
      if(bbzheap_tseg_isvalid(*seg)) {
         printf("\t#%d: {", i);
         for(int j = 0; j < BBZHEAP_ELEMS_PER_TSEG; ++j)
            if(bbzheap_tseg_elem_isvalid(seg->keys[j]))
               printf(" (%d,%d)",
                      bbzheap_tseg_elem_get(seg->keys[j]),
                      bbzheap_tseg_elem_get(seg->values[j]));
         printf(" /next=%x }\n", bbzheap_tseg_next_get(seg));
      }
   }
   printf("\n");fflush(stdout);
}

/**
 * @brief Sets the error code.
 * @param[in] errcode The error type.
 */
void set_last_error(bbzvm_error errcode) {
    last_error = errcode;
#ifdef DEBUG
    printf("VM:\n\tstate: %s\n\tpc: %d\n\tinstr: %s\n\terror state: %s\n", state_desc[vm.state], vm.dbg_pc, instr_desc[*vm.bcode_fetch_fun(vm.dbg_pc, 1)], error_desc[vm.error]);
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
 * @param[in,out] vm The VM.
 */
 void bbzvm_reset_state() {
     vm.state = BBZVM_STATE_READY;
     vm.error = BBZVM_ERROR_NONE;
 }

/**
 * @brief Function used for testing C closures.
 * @param[in,out] vm The current VM.
 * @return The state of the VM.
 */
bbzvm_state printIntVal() {
    bbzheap_idx_t idx;
    bbzvm_lload(1);
    idx = bbzvm_stack_at(0);
    printf("#%d: (%s) %d\n", idx, bbztype_desc[bbztype(*bbzvm_obj_at(idx))], bbzvm_obj_at(idx)->i.value);
    bbzvm_pop();
    return bbzvm_ret0();
}

#define STRBUF_SIZE 32
static char strBuf[STRBUF_SIZE+1];
char* getString(uint16_t sid) {
    uint16_t* strcnt = (uint16_t*)vm.bcode_fetch_fun(0, sizeof(uint16_t));
    if (sid >= *strcnt) return strBuf;
    uint16_t charCount = 0;
    for (uint16_t i = 0; i < sid; ++i) {
        char c = *vm.bcode_fetch_fun((charCount++)+sizeof(uint16_t), sizeof(char));
        while(c != 0) c = *vm.bcode_fetch_fun((charCount++)+sizeof(uint16_t), sizeof(char));
    }
    for (uint16_t i = 0; i < STRBUF_SIZE; ++i) {
        strBuf[i] = *vm.bcode_fetch_fun(sizeof(uint16_t)+charCount+i, sizeof(char));
        if (strBuf[i] == 0) break;
    }
    strBuf[STRBUF_SIZE] = 0;
    return strBuf;
}

void logfunc() {
    int16_t argn = bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    bbzvm_pop();
    for (int16_t i = 0; i < argn; ++i) {
        bbzobj_t* o = bbzvm_obj_at(bbzvm_stack_at(i));
        switch(bbztype(*o)) {
            case BBZTYPE_NIL:
                printf("nil");
                break;
            case BBZTYPE_INT:
                printf("%d", o->i.value);
                break;
            case BBZTYPE_FLOAT:
                printf("%f", bbzfloat_tofloat(o->f.value));
                break;
            case BBZTYPE_TABLE:
                printf("[t]%" PRIu16, o->t.value);
                break;
            case BBZTYPE_USERDATA:
                printf("[u]%" PRIXPTR, (uintptr_t)o->u.value);
                break;
            case BBZTYPE_STRING:
#ifndef BBZVM_USE_BBO
                printf("%s", getString(o->s.value));
#else
                //printf("[s]%d", (o->s.value));
#endif
                break;
            case BBZTYPE_NCLOSURE: // fallthrough
            case BBZTYPE_CLOSURE:
                if (bbztype_isclosurelambda(*o))
                    printf("[cl]%d", o->l.value.ref);
                else
                    printf("[c]%d", (int)(intptr_t)o->c.value);
                break;
            default:
                break;
        }
#ifdef BBZVM_USE_BBO
        if (bbztype(*o) != BBZTYPE_STRING) printf("; ");
#endif
    }
    printf("\n");
}

bbzvm_state bbzvm_log() {
    uint16_t nArg = bbzdarray_size(vm.lsyms) - 1;
    for (uint16_t i = 0; i < nArg; ++i) {
        bbzvm_lload(nArg - i);
    }
    bbzvm_pushi(nArg);
    logfunc();fflush(stdout);
    return bbzvm_ret0();
}

typedef enum {
    BBZVM_SYMID_LOG = 0,
    BBZVM_SYMID_FORWARD,
    BBZVM_SYMID_STOP,
    BBZVM_SYMID_TURN,

    BBZVM_SYMID_ID,
    BBZVM_SYMID_SWARM,
    BBZVM_SYMID_CREATE,
    BBZVM_SYMID_SELECT,
    BBZVM_SYMID_JOIN,
    BBZVM_SYMID_UNSELECT,
    BBZVM_SYMID_LEAVE,
    BBZVM_SYMID_IN,
    BBZVM_SYMID_EXEC,
    BBZVM_SYMID_INTERSECTION,
    BBZVM_SYMID_UNION,
    BBZVM_SYMID_DIFFERENCE,
    BBZVM_SYMID_OTHERS,
    BBZVM_SYMID_NEIGHBORS,
    BBZVM_SYMID_DISTANCE,
    BBZVM_SYMID_AZIMUTH,
    BBZVM_SYMID_ELEVATION,
    BBZVM_SYMID_FOREACH,
    BBZVM_SYMID_MAP,
    BBZVM_SYMID_REDUCE,
    BBZVM_SYMID_FILTER,
    BBZVM_SYMID_LISTEN,
    BBZVM_SYMID_IGNORE,
    BBZVM_SYMID_BROADCAST,
    BBZVM_SYMID_STIGMERGY,
    BBZVM_SYMID_PUT,
    BBZVM_SYMID_GET,
    BBZVM_SYMID_SIZE,
    BBZVM_SYMID_LEFT,
    BBZVM_SYMID_RIGHT
} bbzvm_symid;

bbzvm_state bbzvm_dummy() {
    return bbzvm_ret0();
}

bbzvm_state bbzvm_dummy_int() {
    bbzvm_pushi(0);
    return bbzvm_ret1();
}

bbzheap_idx_t swarm_select;
bbzheap_idx_t swarm_join;
bbzheap_idx_t swarm_unselect;
bbzheap_idx_t swarm_leave;
bbzheap_idx_t swarm_in;
bbzheap_idx_t swarm_exec;

bbzvm_state bbzvm_swarm_create() {
    bbzvm_pusht();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_SELECT);
    bbzvm_push(swarm_select);
    bbzvm_tput();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_JOIN);
    bbzvm_push(swarm_join);
    bbzvm_tput();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_UNSELECT);
    bbzvm_push(swarm_unselect);
    bbzvm_tput();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_LEAVE);
    bbzvm_push(swarm_leave);
    bbzvm_tput();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_IN);
    bbzvm_push(swarm_in);
    bbzvm_tput();
    bbzvm_dup();
    bbzvm_pushs(BBZVM_SYMID_EXEC);
    bbzvm_push(swarm_exec);
    bbzvm_tput();
    return bbzvm_ret1();
}

int8_t bbzvm_register_functions() {
    if (bbzvm_function_register(BBZVM_SYMID_LOG, bbzvm_log) < 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_FORWARD, bbzvm_dummy) < 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_STOP, bbzvm_dummy) < 0) return -1;
    if (bbzvm_function_register(BBZVM_SYMID_TURN, bbzvm_dummy) < 0) return -1;
    swarm_select = bbzvm_function_register(-1, bbzvm_dummy); if (swarm_select < 0) return -1;
    swarm_join = bbzvm_function_register(-1, bbzvm_dummy); if (swarm_join < 0) return -1;
    swarm_unselect = bbzvm_function_register(-1, bbzvm_dummy); if (swarm_unselect < 0) return -1;
    swarm_leave = bbzvm_function_register(-1, bbzvm_dummy); if (swarm_leave < 0) return -1;
    swarm_in = bbzvm_function_register(-1, bbzvm_dummy_int); if (swarm_in < 0) return -1;
    swarm_exec = bbzvm_function_register(-1, bbzvm_dummy); if (swarm_exec < 0) return -1;
    return 0;
}

void bbzvm_register_gsyms() {
    bbzheap_idx_t o;
    // Register 'id'
    bbzvm_pushs(BBZVM_SYMID_ID);
    bbzvm_pushi(vm.robot);
    bbzvm_gstore();

    // Register 'swarm'
    bbzvm_pushs(BBZVM_SYMID_SWARM);
    bbzvm_pusht();
    o = bbzvm_stack_at(0);
    bbzvm_gstore();
    //   Register 'swarm.create'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_CREATE);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy_int));
    bbzvm_tput();
    //   Register 'swarm.intersection'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_INTERSECTION);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy_int));
    bbzvm_tput();
    //   Register 'swarm.union'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_UNION);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy_int));
    bbzvm_tput();
    //   Register 'swarm.difference'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_DIFFERENCE);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy_int));
    bbzvm_tput();

    //bbzvm_gc();

    // Register neighbors
    bbzvm_pushs(BBZVM_SYMID_NEIGHBORS);
    bbzvm_pusht();
    o = bbzvm_stack_at(0);
    bbzvm_gstore();
    //   Register 'neighbors.foreach'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_FOREACH);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();
    //   Register 'neighbors.map'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_MAP);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();
    //   Register 'neighbors.filter'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_FILTER);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();
    //   Register 'neighbors.listen'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_LISTEN);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();
    //   Register 'neighbors.ignore'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_IGNORE);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();
    //   Register 'neighbors.broadcast'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_BROADCAST);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy));
    bbzvm_tput();

    //bbzvm_gc();

    // Register stigmergy
    bbzvm_pushs(BBZVM_SYMID_STIGMERGY);
    bbzvm_pusht();
    o = bbzvm_stack_at(0);
    bbzvm_gstore();
    //   Register 'stigmergy.create'
    bbzvm_push(o);
    bbzvm_pushs(BBZVM_SYMID_CREATE);
    bbzvm_push(bbzvm_function_register(-1, bbzvm_dummy_int));
    bbzvm_tput();

    //bbzvm_gc();
}

    // ======================================
    // =             UNIT TEST              =
    // ======================================

bbzvm_t vm;
#ifdef BBZVM_USE_BBO
#define FILE_TEST1 "ressources/1_InstrTest.bbo"
#define FILE_TEST2 "ressources/2_IfTest.bbo"
#define FILE_TEST3 "ressources/3_test1.bbo"
#define FILE_TEST4 "ressources/4_AllFeaturesTest.bbo"
#define SKIP_JUMP  sizeof(uint8_t) + sizeof(uint16_t)
#else
#define FILE_TEST1 "ressources/1_InstrTest.bo"
#define FILE_TEST2 "ressources/2_IfTest.bo"
#define FILE_TEST3 "ressources/3_test1.bo"
#define FILE_TEST4 "ressources/4_AllFeaturesTest.bo"
#define SKIP_JUMP  sizeof(uint8_t) + sizeof(uint32_t)
#endif

TEST(bbzvm) {
    
    // ------------------------
    // - Test bbzvm_construct -
    // ------------------------

    uint16_t robot = 0;
    bbzvm_construct(robot);

    ASSERT_EQUAL(vm.pc, 0);
    //ASSERT(bbztype_isdarray(*bbzvm_obj_at(vm.lsymts)));
    ASSERT(bbztype_isdarray(*bbzvm_obj_at(vm.flist)));
    ASSERT(bbztype_istable (*bbzvm_obj_at(vm.gsyms)));
    ASSERT(bbztype_isnil   (*bbzvm_obj_at(vm.nil)));
    ASSERT(bbztype_isdarray(*bbzvm_obj_at(vm.dflt_actrec)));
    ASSERT_EQUAL(vm.state, BBZVM_STATE_NOCODE);
    ASSERT_EQUAL(vm.error, BBZVM_ERROR_NONE);
    ASSERT_EQUAL(vm.robot, robot);

    // Also set the error notifier.
    bbzvm_set_error_notifier(&set_last_error);

    // ------------------------
    // - Test bbzvm_set_bcode -
    // ------------------------

    // 1) Open bytecode file.
    fbcode = fopen(FILE_TEST2, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    fseek(fbcode, 0, SEEK_SET);

    // 2) Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    ASSERT(vm.bcode_fetch_fun == &testBcode);
    ASSERT_EQUAL(vm.bcode_size, fsize);
    ASSERT_EQUAL(vm.state, BBZVM_STATE_READY);
    ASSERT_EQUAL(vm.error, BBZVM_ERROR_NONE);
    ASSERT_EQUAL(bbzdarray_size(vm.flist), 0);
    ASSERT_EQUAL(bbztable_size(vm.gsyms), 0);
    ASSERT_EQUAL(*testBcode(vm.pc-1, 1), BBZVM_INSTR_NOP);

    // -------------------
    // - Test bbzvm_step -
    // -------------------

    // 1) Open instruction test file
    fclose(fbcode);
    fbcode = fopen(FILE_TEST1, "rb");
    REQUIRE(fbcode != NULL);
    fseek(fbcode, 0, SEEK_END);
    fsize = ftell(fbcode);
    fseek(fbcode, 0, SEEK_SET);

    vm.bcode_size = fsize;

    vm.pc = 2;

    // 2) Nop
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_NOP);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 0);

    // Save PC.
    uint16_t labelDone = vm.pc;

    // 3) Done
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_DONE);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 0);
    ASSERT_EQUAL(vm.state, BBZVM_STATE_DONE);
    // Make sure we are looping on DONE.
    ASSERT_EQUAL(vm.pc, labelDone);
    // Reset VM state and go to next test.
    bbzvm_reset_state();
    vm.pc += sizeof(uint8_t);

    // 3) Pushnil
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_PUSHNIL);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 1);
    ASSERT(bbztype_isnil(*bbzvm_obj_at(0)));

    // 4) Pop
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_POP);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 0);

    // Save PC for jump tests.
    uint16_t pushiLabel = vm.pc;

    // 5) Pushi
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_PUSHI);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 1);
    ASSERT(bbztype_isint(*bbzvm_obj_at(bbzvm_stack_at(0))));
    bbzobj_t* o = bbzvm_obj_at(bbzvm_stack_at(0));
    ASSERT_EQUAL(o->i.value, 0x42);

    // 6) Dup
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_DUP);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 2);
    bbzobj_t* o1 = bbzvm_obj_at(bbzvm_stack_at(0));
    ASSERT(bbztype_isint(*o1));
    
    ASSERT_EQUAL(bbzvm_obj_at(bbzvm_stack_at(0))->i.value, 0x42);
    ASSERT(bbztype_isint(*bbzvm_obj_at(bbzvm_stack_at(1))));
    ASSERT_EQUAL(bbzvm_obj_at(bbzvm_stack_at(1))->i.value, 0x42);

    // Save PC
    uint16_t jumpLabel = vm.pc;

    // 7) Jump
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMP);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 2);
    ASSERT_EQUAL(vm.pc, pushiLabel);
    
    // Re-execute instructions until the jump.
    while (vm.pc != jumpLabel) {
        bbzvm_step();
    }
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    // Skip jump instruction.
    vm.pc += SKIP_JUMP;

    // Save PC
    uint16_t jumpzLabel = vm.pc;

    // 8) Jumpz when operand is BBZTYPE_NIL. Should jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPZ);
    bbzvm_pushnil();
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, pushiLabel);

    // Do the jumpz again.
    vm.pc = jumpzLabel;

    // 9) Jumpz when operand is BBZTYPE_INT and its value is zero. Should jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPZ);
    bbzheap_idx_t idx;
    bbzheap_obj_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = 0;
    bbzvm_push(idx);
    bbzvm_step();
    // Nothing should have happened ; we should have gone to the next instruction.
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, pushiLabel);

    // Do the jumpz again.
    vm.pc = jumpzLabel;

    // 10) Jumpz when operand is BBZTYPE_INT and its value is not zero. Should not jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPZ);
    bbzheap_obj_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = -1;
    bbzvm_push(idx);
    bbzvm_step();
    // Nothing should have happened ; we should have gone to the next instruction.
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, jumpzLabel + SKIP_JUMP);

    // Save PC.
    uint16_t jumpnzLabel = vm.pc;

    // 11) Jumpnz when operand is BBZTYPE_INT and its value is not zero. Should jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPNZ);
    bbzheap_obj_alloc(BBZTYPE_INT, &idx);
    bbzvm_obj_at(idx)->i.value = -1;
    bbzvm_push(idx);
    bbzvm_step();
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, pushiLabel);

    // Do the jumpnz again.
    vm.pc = jumpnzLabel;

    // 12) Jumpnz when operand is BBZTYPE_NIL. Should not jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPNZ);
    bbzvm_pushnil();
    bbzvm_step();
    // Nothing should have happened ; we should have gone to the next instruction.
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, jumpnzLabel + SKIP_JUMP);

    // Do the jumpnz again.
    vm.pc = jumpnzLabel;

    // 13) Jumpnz when operand is BBZTYPE_NIL. Should not jump.
    REQUIRE(*testBcode(vm.pc, 1) == BBZVM_INSTR_JUMPNZ);
    bbzvm_pushnil();
    bbzvm_step();
    // Nothing should have happened ; we should have gone to the next instruction.
    ASSERT_EQUAL(bbzvm_stack_size(), 4);
    ASSERT_EQUAL(vm.pc, jumpnzLabel + SKIP_JUMP);

    // 14) Empty the stack
    while (bbzvm_stack_size() != 0) {
        bbzvm_pop();
    }
    bbzvm_reset_state();


    // 15) Test arith and logical operators
    {
        const int16_t LHS_INT = -21244, RHS_INT = 8384;
        bbzheap_idx_t lhs, rhs;
        bbzheap_obj_alloc(BBZTYPE_INT, &lhs);
        bbzheap_obj_alloc(BBZTYPE_INT, &rhs);
        bbzvm_obj_at(lhs)->i.value = LHS_INT;
        bbzvm_obj_at(rhs)->i.value = RHS_INT;

        const bbzvm_instr LAST_INSTR = (bbzvm_instr)-1;
        bbzvm_instr instrs[] = {
            BBZVM_INSTR_ADD, BBZVM_INSTR_SUB, BBZVM_INSTR_MUL, BBZVM_INSTR_DIV, BBZVM_INSTR_MOD,
            BBZVM_INSTR_AND, BBZVM_INSTR_OR,  BBZVM_INSTR_EQ,  BBZVM_INSTR_NEQ, BBZVM_INSTR_GT,
            BBZVM_INSTR_GTE, BBZVM_INSTR_LT,  BBZVM_INSTR_LTE, LAST_INSTR
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
            REQUIRE((bbzvm_instr)*testBcode(vm.pc, 1) == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(bbzvm_obj_at(bbzvm_stack_at(0))->i.value, results[i]);
            ASSERT_EQUAL(vm.state, BBZVM_STATE_READY);
            ASSERT_EQUAL(vm.error, BBZVM_ERROR_NONE);

            bbzvm_pop();
            bbzvm_reset_state();
            curr_instr = instrs[++i];
        }
    }

    bbzvm_set_error_notifier(set_last_error_no_print);
    // ---- Test failing operations ----
    // 16) Perform some basic operations when stack is empty
    {
        REQUIRE(bbzvm_stack_size() == 0);
        ASSERT_EQUAL(get_last_error(), BBZVM_ERROR_NONE);

        const bbzvm_instr LAST_INSTR = (bbzvm_instr)-1;
        bbzvm_instr failing_instr[] = {
            BBZVM_INSTR_POP, BBZVM_INSTR_DUP, BBZVM_INSTR_ADD, BBZVM_INSTR_SUB,   BBZVM_INSTR_MUL,
            BBZVM_INSTR_DIV, BBZVM_INSTR_MOD, BBZVM_INSTR_POW, BBZVM_INSTR_UNM,   BBZVM_INSTR_AND,
            BBZVM_INSTR_OR,  BBZVM_INSTR_NOT, BBZVM_INSTR_EQ,  BBZVM_INSTR_NEQ,   BBZVM_INSTR_GT,
            BBZVM_INSTR_GTE, BBZVM_INSTR_LT,  BBZVM_INSTR_LTE, BBZVM_INSTR_JUMPZ, BBZVM_INSTR_JUMPNZ,
            LAST_INSTR
        };
        int16_t oldPc = vm.pc;
        uint16_t i = 0;
        bbzvm_instr curr_instr = failing_instr[i++];
        while(curr_instr != LAST_INSTR) {
            REQUIRE(bbzvm_stack_size() == 0);
            REQUIRE((bbzvm_instr)*testBcode(vm.pc, 1) == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(vm.state, BBZVM_STATE_ERROR);
            ASSERT_EQUAL(vm.error, BBZVM_ERROR_STACK);

            REQUIRE(vm.pc == oldPc);
            bbzvm_skip_instr();
            bbzvm_reset_state();
            curr_instr = failing_instr[i++];
            oldPc = vm.pc;
            ASSERT_EQUAL(get_last_error(), BBZVM_ERROR_STACK);
        }
    }

    // Fill the stack
    REQUIRE(bbzvm_stack_size() == 0);
    for (uint16_t i = 0; i < BBZSTACK_SIZE; ++i) {
        bbzvm_push(vm.nil);
    }

    // 17) Perform push operations when stack is full
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
            bbzvm_instr instr = (bbzvm_instr)*testBcode(vm.pc, 1);
            REQUIRE(instr == curr_instr);
            bbzvm_step();
            ASSERT_EQUAL(vm.state, BBZVM_STATE_ERROR);
            ASSERT_EQUAL(vm.error, BBZVM_ERROR_STACK);
            bbzvm_skip_instr();
            bbzvm_reset_state();
            curr_instr = failing_instr[i++];
        }
    }
    bbzvm_reset_state();

    bbzvm_set_error_notifier(&set_last_error);

    // -----------------------
    // - Test bbzvm_destruct -
    // -----------------------

    bbzvm_destruct();

    // -----------------
    // - Closure tests -
    // -----------------

    // - Set up -
    bbzvm_construct(robot);
    bbzvm_set_error_notifier(&set_last_error);

    fclose(fbcode);
    fbcode = fopen(FILE_TEST3, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    fseek(fbcode, 0, SEEK_SET);

    // A) Set the bytecode in the VM.
    bbzvm_set_bcode(&testBcode, fsize);

    //ASSERT_EQUAL(bbztable_size(vm.gsyms), 5);

    // B) Register C closure
    REQUIRE(vm.state != BBZVM_STATE_ERROR);
    bbzheap_idx_t c = bbzvm_function_register(0, printIntVal);

    REQUIRE(c >= 0);
    ASSERT_EQUAL(bbztype(*bbzvm_obj_at(c)), BBZTYPE_CLOSURE);
    ASSERT_EQUAL((intptr_t)bbzvm_obj_at(c)->c.value, (intptr_t)printIntVal);

    // C) Call registered C closure
    //REQUIRE(bbztable_size(vm.gsyms) == *(uint16_t*)vm.bcode_fetch_fun(0, 2));
    bbzvm_pushs(0);
    bbzvm_gload();
    c = bbzvm_stack_at(0);
    bbzvm_pop();
    REQUIRE(bbztype_isclosure(*bbzvm_obj_at(c)));
    bbzvm_pushi(123);
    ASSERT(bbzvm_function_call(0, 1) != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzvm_stack_size(), 0);

    // D) Execute the rest of the script
    REQUIRE(vm.state != BBZVM_STATE_ERROR);
    while(bbzvm_step() == BBZVM_STATE_READY);
    ASSERT(vm.state != BBZVM_STATE_ERROR);
    bbzvm_pushs(4);
    bbzvm_gload();
    ASSERT_EQUAL(bbzvm_obj_at(bbzvm_stack_at(0))->i.value, 63);

    bbzvm_destruct();

    // --------------------------
    // - Script execution tests -
    // --------------------------

    #if BBZHEAP_SIZE < 2048
    #warning Test file "testvm.c": Running test of all features requires BBZHEAP_SIZE >= 2048\
    on 32 and 64 bit systems. The script execution test will be disabled.
    #else

    bbzvm_construct(robot);
    bbzvm_set_error_notifier(&set_last_error);

    fclose(fbcode);
    fbcode = fopen(FILE_TEST4, "rb");
    REQUIRE(fbcode != NULL);
    REQUIRE(fseek(fbcode, 0, SEEK_END) == 0);
    fsize = ftell(fbcode);
    fseek(fbcode, 0, SEEK_SET);

    bbzvm_set_bcode(&testBcode, fsize);
    
    REQUIRE(vm.state == BBZVM_STATE_READY);
    REQUIRE(bbzvm_register_functions() >= 0); // If this fails, it means that the heap doesn't have enough memory allocated to execute this test.
    bbzvm_register_gsyms();
    while (vm.state == BBZVM_STATE_READY) {
    #ifdef DEBUG
        printf("[%d: %s]\n", vm.pc, instr_desc[*vm.bcode_fetch_fun(vm.pc,1)]);
    #endif
        bbzvm_step();
        ASSERT(vm.state != BBZVM_STATE_ERROR);
    }
    ASSERT_EQUAL(vm.state, BBZVM_STATE_DONE);
    ASSERT_EQUAL(vm.error, BBZVM_ERROR_NONE);

    bbzvm_destruct();
    #endif
    fclose(fbcode);

    TEST_END();
}