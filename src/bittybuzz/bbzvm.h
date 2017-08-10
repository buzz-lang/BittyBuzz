/**
 * @file bbzvm.h
 * @brief Definition of the BittyBuzz Virtual Machine,
 *  as well as of other types required by it.
 */

#ifndef BBZVM_H
#define BBZVM_H

#include <inttypes.h>

#include "bbzinclude.h"
#include "bbzheap.h"
#include "bbzdarray.h"
#include "bbztable.h"
#include "bbzstrids.h"
#include "bbzneighbors.h"
#include "bbzswarm.h"
#include "bbzvstig.h"
#include "bbzoutmsg.h"
#include "bbzinmsg.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    // ======================================
    // =       DATA TYPE DEFNINITIONS       =
    // ======================================

    /**
     * @brief Type for the pointer to a function which fetches bytecode data.
     * @warning The function provider should take endianness
     * into account if copying byte-by-byte.
     * @param[in] offset Bytecode offset for the data to fetch.
     * @param[in] size Size of the data to set. The VM will at most
     * try to read 4 bytes (uint32_t).
     * @return A pointer to the data to the data.
     */
    typedef const uint8_t* (*bbzvm_bcode_fetch_fun)(bbzpc_t offset, uint8_t size);

    /**
     * @brief Type for the pointer to a function that is called whenever the
     * VM meets an error.
     * @details This function may perform operations based on the type of the
     * errors that occur. Typically, this involves notifying the outside world
     * that an error has occurred (such as stopping the actuators and sending a
     * signal), or handling the error manually.
     * @param[in] errcode The type of the error that occurred.
     */
    typedef void (*bbzvm_error_receiver_fun)(bbzvm_error errcode);

    /**
     * @brief Type for the pointer to a C-closure.
     */
    typedef void (*bbzvm_funp)();

    /**
     * @brief The BittyBuzz Virtual Machine.
     *
     * @details Responsibilities:
     *
     *  1) Load and run bytecode.
     */
    typedef struct PACKED bbzvm_t {
        bbzvm_error_receiver_fun error_receiver_fun; /**< @brief Error receiver. */
        bbzvm_bcode_fetch_fun bcode_fetch_fun; /**< @brief Bytecode fetcher function */
        uint16_t bcode_size;       /**< @brief Size of the loaded bytecode */
        bbzpc_t pc;                /**< @brief Program counter */
        bbzheap_idx_t lsyms;       /**< @brief Current local variable table */
        bbzheap_idx_t gsyms;       /**< @brief Global symbols */
        bbzheap_t heap;            /**< @brief Heap content */
        bbzheap_idx_t nil;         /**< @brief Singleton bbznil_t */
        bbzheap_idx_t dflt_actrec; /**< @brief Singleton bbzdarray_t for the default activations record */
        bbzheap_idx_t flist;       /**< @brief Registered functions */
        /* List of known swarms */
        // TODO
        /* List of known swarms */
        // TODO
        bbzswarm_t swarm;          /**< @brief Swarm data */
        /* Counter for swarm membership broadcasting */
        // TODO
        bbzinmsg_queue_t inmsgs;   /**< @brief Input messages FIFO */
        bbzoutmsg_queue_t outmsgs; /**< @brief Output messages FIFO */
        bbzvstig_t vstig;          /**< @brief Virtual stigmergy single instance. */
        bbzneighbors_t neighbors;  /**< @brief Neighbor data. */
        bbzvm_state state;         /**< @brief Current VM state */
        bbzvm_error error;         /**< @brief Current VM error */
        /* Current VM error message */
        // TODO ... or not TODO? That is the question...
        bbzrobot_id_t robot;       /**< @brief This robot's id */
#ifdef DEBUG
        bbzpc_t dbg_pc;            /**< @brief PC value used for debugging purpose. */
        bbzvm_instr instr;         /**< @brief Current instruction */
#endif
        int16_t stackptr;          /**< @brief Stack pointer (Index of the last valid element of the stack) */
        int16_t blockptr;          /**< @brief Block pointer (Index of the previous block pointer in the stack) */
        bbzheap_idx_t stack[BBZSTACK_SIZE]; /**< @brief Current stack content */
    } bbzvm_t;

    /**
     * @brief Virtual Machine instance. Available from anywhere.
     */
    extern bbzvm_t* vm;



    // ======================================
    // =  GENERAL VM FUNCTION DEFINITIONS   =
    // ======================================

    /**
     * @brief Sets up the VM.
     * @param[in] robot The robot id.
     */
    void bbzvm_construct(bbzrobot_id_t robot);

    /**
     * @brief Destroys the VM.
     */
    void bbzvm_destruct();

    /**
     * @brief Sets the error state of the VM.
     * @note It is possible for a user to create and pass their own error
     * type to this function.
     * @param[in] errcode The code of the error.
     * @see bbzvm_error
     */
    void bbzvm_seterror(bbzvm_error errcode);

    /**
     * @brief Sets the bytecode function in the VM.
     * @warning The passed buffer should not be deleted until the VM is done with it.
     * @warning The function provider should take endianness
     * into account if copying byte-by-byte.
     * @param[in] bcode_fetch_fun The function to call to read bytecode data.
     * @param[in] bcode_size The size (in bytes) of the bytecode.
     * @return 0 if everything OK, a non-zero value in case of error
     */
    void bbzvm_set_bcode(bbzvm_bcode_fetch_fun bcode_fetch_fun, uint16_t bcode_size);

    /**
     * @brief Sets the error receiver.
     * @see bbzvm_error_receiver_fun
     * @param[in] error_receiver_fun Function recieving the error notification.
     */
    ALWAYS_INLINE
    void bbzvm_set_error_receiver(bbzvm_error_receiver_fun error_receiver_fun) { vm->error_receiver_fun = error_receiver_fun; }

    /**
     * @brief Processes the input message queue.
     */
    void bbzvm_process_inmsgs();

    /**
     * @brief Processes the output message queue.
     */
    void bbzvm_process_outmsgs();

    /**
     * @brief Runs the VM's garbage collector.
     */
    void bbzvm_gc();

    /**
     * @brief Executes the next step in the bytecode, if possible.
     * @details Should there be an error during stepping, the VM's
     * program counter will point to the instruction that caused the error.
     */
    void bbzvm_step();



    // ======================================
    // =         BYTECODE FUNCTIONS         =
    // ======================================

    /**
     * @brief Terminates the current Buzz script.
     * @see BBZVM_INSTR_DONE
     */
    ALWAYS_INLINE
    void bbzvm_done() { vm->state = BBZVM_STATE_DONE; }

    /**
     * @brief Pushes nil on the stack.
     * @see BBZVM_INSTR_PUSHNIL
     */
    void bbzvm_pushnil();

    /**
     * @brief Duplicates the current stack top.
     * @see BBZVM_INSTR_DUP
     */
    void bbzvm_dup();

    /**
     * @brief Pops the stack.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_POP
     */
    void bbzvm_pop();

    /**
     * @brief Returns from a closure without setting a return value.
     * @details Internally checks whether the operation is valid.
     *
     * This function expects at least two stacks to be present. The
     * first stack is popped. The stack beneath, now the top stack, is
     * expected to have at least one element: the return address at
     * stack #0. The return address is popped and used to update the program
     * counter.
     * @see BBZVM_INSTR_RET0
     */
    void bbzvm_ret0();

    /**
     * @brief Returns from a closure setting a return value.
     * @details Internally checks whether the operation is valid.
     *
     * This function expects at least two stacks to be present. The
     * first stack must have at least one element, which is saved as
     * the return value of the call. The stack is then popped. The
     * stack beneath, now the top stack, is expected to have at least
     * one element: the return address at stack #0. The return address is
     * popped and used to update the program counter. Then, the saved
     * return value is pushed on the stack.
     * @see BBZVM_INSTR_RET1
     */
    void bbzvm_ret1();

    /**
     * @brief Performs an addition.
     * @see BBZVM_INSTR_ADD
     */
    void bbzvm_add();

    /**
     * @brief Performs a subtraction.
     * @see BBZVM_INSTR_SUB
     */
    void bbzvm_sub();

    /**
     * @brief Performs a multiplication.
     * @see BBZVM_INSTR_MUL
     */
    void bbzvm_mul();

    /**
     * @brief Performs a division.
     * @see BBZVM_INSTR_DIV
     */
    void bbzvm_div();

    /**
     * @brief Performs a modulo.
     * @see BBZVM_INSTR_MOD
     */
    void bbzvm_mod();

    /**
     * @brief Performs a power.
     * @see BBZVM_INSTR_POW
     */
    void bbzvm_pow();

    /**
     * @brief Performs a unary minus.
     * @see BBZVM_INSTR_UNM
     */
    void bbzvm_unm();

    /**
     * @brief Performs a logical and.
     * @see BBZVM_INSTR_AND
     */
    void bbzvm_and();

    /**
     * @brief Performs a logical or.
     * @see BBZVM_INSTR_OR
     */
    void bbzvm_or();

    /**
     * @brief Performs a logical inversion.
     * @see BBZVM_INSTR_NOT
     */
    void bbzvm_not();

    /**
     * @brief Performs a equality check.
     * @see BBZVM_INSTR_EQ
     */
    void bbzvm_eq();

    /**
     * @brief Performs a difference check.
     * @see BBZVM_INSTR_NEQ
     */
    void bbzvm_neq();

    /**
     * @brief Performs a greater-than comparison.
     * @see BBZVM_INSTR_GT
     */
    void bbzvm_gt();

    /**
     * @brief Performs a greater-than-or-equal comparison.
     * @see BBZVM_INSTR_GTE
     */
    void bbzvm_gte();

    /**
     * @brief Performs a less-than comparison.
     * @see BBZVM_INSTR_LT
     */
    void bbzvm_lt();

    /**
     * @brief Performs a less-than-or-equal comparison.
     * @see BBZVM_INSTR_LTE
     */
    void bbzvm_lte();

    /**
     * @brief Pushes the global variable located at the given stack index.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_GLOAD
     */
    void bbzvm_gload();

    /**
     * @brief Stores the object located at the stack top into the a global variable, pops operand.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_GSTORE
     * @bug Gstore-ing lambda functions will not change its activation record (if we call it as a global object, it will
     * be executed as if it was still in its former self-table object).<br/>Example:
     * @code
     *      t1 = {
     *          .id = 1,
     *          .foo = function () {
     *              log(self.id)
     *          }
     *      }
     *      t1.foo() # Will display "1"
     *      bar = t1.foo
     *      bar() # Will also display "1", instead of throwing an error
     * @endcode
     */
    void bbzvm_gstore();


    /**
     * @brief Pushes a table object on the stack
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_PUSHT
     */
    void bbzvm_pusht();

    /**
     * @brief Stores a (idx,value) pair in a table.
     * @details Internally checks whether the operation is valid.
     *
     * The stack is expected to be as follows:
     * 0   -> value
     * 1   -> idx
     * 2   -> table
     * This operation pops stack #0 and stack #1, leaving the table at the stack top.
     * @see BBZVM_INSTR_TPUT
     */
    void bbzvm_tput();

    /**
     * @brief Fetches a (idx,value) pair from a table.
     * @details Internally checks whether the operation is valid.
     *
     * The stack is expected to be as follows:
     * 0   -> idx
     * 1   -> table
     * This operation pops stack #0 and pushes the value, leaving the table at
     * stack #1. If the element for the given idx is not found, nil is
     * pushed as value.
     * @see BBZVM_INSTR_TGET
     */
    void bbzvm_tget();


    /**
     * @brief Calls a Buzz closure.
     * @details It expects the stack to be as follows:<br/>
     * 0   -> argN<br/>
     * 1   -> argN-1<br/>
     * ...<br/>
     * N-2 -> arg2<br/>
     * N-1 -> arg1<br/>
     * N   -> closure<br/>
     * This function pops all arguments.
     * @param[in] argc The number of arguments.
     * @return 0 if everything OK, a non-zero value in case of error
     */
    void bbzvm_closure_call(uint16_t argc);

    /**
     * @brief Calls a function defined in Buzz.
     * @details It expects the stack to be as follows:<br/>
     * 0   -> argN<br/>
     * 1   -> argN-1<br/>
     * ...<br/>
     * N-2 -> arg2<br/>
     * N-1 -> arg1<br/>
     * This function pops all arguments.
     * @param[in] fname The function name (bbzheap_idx_t pointing to a bbzstring_t).
     * @param[in] argc The number of arguments.
     */
    void bbzvm_function_call(uint16_t fname, uint16_t argc);

    /**
     * @brief Registers a function in the VM.
     * @param[in] fnameid The symbol ID of the function's name.
     * @param[in] funp The function pointer to register.
     * @return The function's closure position in the heap.
     */
    bbzheap_idx_t bbzvm_function_register(int16_t fnameid, bbzvm_funp funp);

    /**
     * @brief Calls a closure.
     * @details Internally checks whether the operation is valid.
     *
     * This function expects the stack to be as follows:
     * 0   -> An integer for the number of closure parameters N<br/>
     * 1   -> Closure argN<br/>
     * ...<br/>
     * N   -> Closure arg1<br/>
     * N+1 -> Closure
     *
     * This function pushes a new stack and a new local variable table filled with the
     * activation record entries and the closure arguments. In addition, it leaves the stack
     * beneath as follows:
     * 0   -> The previous value of the block pointer (used when returning from a call)
     * 1   -> The parent local symbol table
     * 2   -> An integer for the return address
     */
    void bbzvm_callc();

    /**
     * @brief Pushes a variable on the stack.
     * @param[in] v The variable.
     */
    void bbzvm_push(bbzheap_idx_t v);

    /**
     * @brief Pushes a float value on the stack.
     * @see BBZVM_INSTR_PUSHF
     * @param[in] v The value.
     */
    void bbzvm_pushf(bbzfloat v);

    /**
     * @brief Pushes a 32 bit signed int value on the stack.
     * @see BBZVM_INSTR_PUSHI
     * @param[in] v The value.
     */
    void bbzvm_pushi(int16_t v);

    /**
     * @brief Pushes a string on the stack.
     * @see BBZVM_INSTR_PUSHS
     * @param[in] strid The string id.
     */
    void bbzvm_pushs(uint16_t strid);

    /**
     * @brief Pushes a closure on the stack.
     * @details Internally checks whether the operation is valid.
     * @param[in] rfrnc The closure reference.
     * @param[in] nat 1 if the closure in native, 0 if not
     */
    void bbzvm_pushc(intptr_t rfrnc, int16_t nat);

    /**
     * @brief Pushes a native closure on the stack.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_PUSHCN
     * @param[in] addr The closure address.
     */
    ALWAYS_INLINE
    void bbzvm_pushcn(uint16_t addr) { return bbzvm_pushc((intptr_t)addr, 1); }

    /**
     * @brief Pushes a c-function closure on the stack.
     * @details Internally checks whether the operation is valid.
     *
     * This function expects the function id in the stack top.
     * It pops the function id and pushes the c-function closure.
     * @see BBZVM_INSTR_PUSHCC
     * @param[in] cid The closure id.
     */
    ALWAYS_INLINE
    void bbzvm_pushcc(bbzvm_funp cid) { return bbzvm_pushc((intptr_t)cid, 0); }

    /**
     * @brief Pushes a lambda native closure on the stack.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_PUSHL
     * @param[in] addr The closure address.
     */
    void bbzvm_pushl(uint16_t addr);

    /**
     * @brief Pushes a userdata on the stack.
     * @param[in,out] v The C pointer to the user data.
     */
    void bbzvm_pushu(void* v);

    /**
     * @brief Pushes the local variable located at the given stack index.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_LLOAD
     * @param[in] idx The index of the local variable to load.
     */
    void bbzvm_lload(uint16_t idx);

    /**
     * @brief Stores the object located at the stack top into the a local variable, pops operand.
     * @details Internally checks whether the operation is valid.
     * @see BBZVM_INSTR_LSTORE
     * @param[in] idx The local variable index.
     */
    void bbzvm_lstore(uint16_t idx);

    /**
     * @brief Sets the program counter to jump to a specificed bytecode offset.
     * @see BBZVM_INSTR_JUMP
     * @param[in] offset The offset to jump to.
     */
    void bbzvm_jump(uint16_t offset);

    /**
     * @brief Sets the program counter to jump to a specificed bytecode
     * offset, if stack top is zero.
     * @note A nil object is considered zero.
     * @see BBZVM_INSTR_JUMPZ
     * @param[in] offset The offset to jump to.
     */
    void bbzvm_jumpz(uint16_t offset);

    /**
     * @brief Sets the program counter to jump to a specificed bytecode
     * offset, if stack top is not zero.
     * @note A nil object is considered zero.
     * @see BBZVM_INSTR_JUMPNZ
     * @param[in] offset The offset to jump to.
     */
    void bbzvm_jumpnz(uint16_t offset);

    /**
     * @brief Register a global symbol.
     * @param[in] sid The string ID representing the global symbol.
     * @param[in] v The value of the global symbol.
     * @return 1 for success, 0 for failure (out of memory).
     */
    uint8_t bbzvm_gsym_register(uint16_t sid, bbzheap_idx_t v);



    // ======================================
    // =      MISC FUNCTIONS AND MACROS     =
    // ======================================

    /**
     * @brief Returns the vm's nil instance.
     * @return The index of the object.
     */
    bbzheap_idx_t bbznil_get();

    /**
     * @brief Allocates a Buzz integer and returns its index on the heap.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @param[in] val The value to assign to the object.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbzint_new(int16_t val);

    /**
     * @brief Allocates a Buzz float and returns its index on the heap.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @param[in] val The value to assign to the object.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbzfloat_new(bbzfloat val);

    /**
     * @brief Fetches an instance of the requested Buzz string and returns its index on the heap.
     * Allocates it if it does not already exist.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @warning You shouldn't change the string id of the returned object.
     * @param[in] val The string ID to assign to the object.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbzstring_get(uint16_t val);

    /**
     * @brief Allocates a Buzz table and returns its index on the heap.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbztable_new();

    /**
     * @brief Allocates a Buzz closure and returns its index on the heap.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @warning You shouldn't change the string id of the returned object.
     * @param[in] val The value to assign to the object.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbzclosure_new(intptr_t val);

    /**
     * @brief Allocates a Buzz userdata and returns its index on the heap.
     * @warning This function may throw a #BBZVM_ERROR_MEM error.
     * @warning You shouldn't change the string id of the returned object.
     * @param[in] val The value to assign to the object.
     * @return The index of the allocated object. UINT16_MAX in case of error.
     */
    bbzheap_idx_t bbzuserdata_new(void* val);

    /**
     * Returns the size of the stack.
     * The most recently pushed element in the stack is at size - 1.
     * @return The size of the VM's current stack.
     */
    #define bbzvm_stack_size() (vm->stackptr + (int16_t)1)

    /**
     * @brief Gets the element at given stack position,
     * where 0 is the top of the stack and >0 goes down the stack.
     * @warning This function performs no sanity check on the passed index.
     * @param[in] idx The stack index.
     * @return The heap index of the element at given stack index.
     */
    bbzheap_idx_t bbzvm_stack_at(int16_t idx);

    /**
     * @brief Gets the element at given local symbols position,
     * where 0 is the self table (the table we are calling the closure on) and
     * >0 are the closure's arguments (e.g., 3 -> third argument).
     * @warning This function performs no sanity check on the passed index.
     * @param[in] idx The local symbols index.
     * @return The heap index of the element at given local symbols index.
     */
    #define bbzvm_lsym_at(idx) ({ bbzheap_idx_t ret; bbzdarray_get(vm->lsyms, idx, &ret); ret; })

    /**
     * @brief Determines how many arguments were passed to the closure that
     * is being executed.
     * @return The number of arguments.
     */
    #define bbzvm_lsym_size() (bbzdarray_size(vm->lsyms) - 1)

    /**
     * @brief Assert the correct execution of a boolean returning function.
     * Typically used with memory allocating functions such as bbzheap_obj_alloc()
     * or bbzdarray_lambda_alloc().
     * @param[in] expr The expression (function call) to assert.
     * @param[in] ERR_TYPE The kind of error to "throw" if the assertion fails.
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_exec(expr, ERR_TYPE, RET...)                   \
        if(!(expr)) {                                                   \
            bbzvm_seterror(ERR_TYPE);                                   \
            return RET;                                                 \
        }

    /**
     * @brief Checks whether the given stack's size is >= to the passed size.
     * If the size is not valid, it updates the VM state.
     * @param[in] size The stack index, where 0 is the stack top and >0 goes down the stack.
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_stack(size, RET...)                            \
        bbzvm_assert_exec(bbzvm_stack_size() >= (size), BBZVM_ERROR_STACK, RET)

    /**
     * @brief Checks whether the type at the given stack position is correct.
     * If the type is wrong, it updates the VM state and exits the current function.
     * @param[in] idx The heap index of the object whose type to assert.
     * @param[in] tpe The type to check.
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_type(idx, tpe, RET...)                         \
        bbzvm_assert_exec(bbztype_is(*bbzheap_obj_at(idx), tpe), BBZVM_ERROR_TYPE, RET)

    /**
     * @brief Allocate memory on the heap. If the heap is out of memory,
     * it updates the VM state and exits the current function.
     * @param[in] type The type to allocate.
     * @param[out] idx A buffer for the index of the allocated object.
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_mem_alloc(type, idx, RET...)                   \
        bbzvm_assert_exec(bbzheap_obj_alloc(type, idx), BBZVM_ERROR_MEM, RET)

    /**
     * @brief Checks whether the current closure was passed exactly
     * certain number of parameters.
     * @param[in] num The number of parameters expected.
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_lnum(num, RET...)                              \
        bbzvm_assert_exec(bbzvm_lsym_size() == (num), BBZVM_ERROR_LNUM, RET)

    /**
     * @brief Assert the state of the VM. To be used after explicit usage of
     * instructions such as bbzvm_pop() or bbzvm_gload();
     * @param[in] RET (optional) The value returned if the assertion fails.
     */
    #define bbzvm_assert_state(RET...)                                  \
        if(vm->state == BBZVM_STATE_ERROR) return RET

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzutil.h" // Include AFTER bbzvm.h because of circular dependencies.

#endif // !BBZVM_H
