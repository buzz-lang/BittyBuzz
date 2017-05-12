#ifndef BBZVM_H
#define BBZVM_H

#include <inttypes.h>

#include "bbzheap.h"
#include "bbzdarray.h"
#include "bbztable.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



    // ======================================
    // =       DATA TYPE DEFNINITIONS       =
    // ======================================



    /**
     * @brief VM states
     */
    typedef enum {
        BBZVM_STATE_NOCODE = 0, // No code loaded
        BBZVM_STATE_READY,      // Ready to execute next instruction
        BBZVM_STATE_DONE,       // Program finished
        BBZVM_STATE_ERROR,      // Error occurred
        BBZVM_STATE_STOPPED     // Stopped due to a breakpoint
    } bbzvm_state;

    /**
     * @brief VM error codes
     */
    typedef enum {
        BBZVM_ERROR_NONE = 0, // No error
        BBZVM_ERROR_INSTR,    // Unknown instruction 
        BBZVM_ERROR_STACK,    // Stack error
        BBZVM_ERROR_LNUM,     // Wrong number of local variables
        BBZVM_ERROR_PC,       // Program counter out of range
        BBZVM_ERROR_FLIST,    // Function call id out of range
        BBZVM_ERROR_TYPE,     // Type mismatch
        BBZVM_ERROR_STRING,   // Unknown string id
        BBZVM_ERROR_SWARM     // Unknown swarm id
    } bbzvm_error;

    /**
     * @brief VM instructions
     */
    typedef enum {
        /**
         * Opcodes without argument
         */
        BBZVM_INSTR_NOP = 0,    // No operation
        BBZVM_INSTR_DONE,       // End of the program
        BBZVM_INSTR_PUSHNIL,    // Push nil onto stack
        BBZVM_INSTR_DUP,        // Duplicate stack top
        BBZVM_INSTR_POP,        // Pop value from stack
        BBZVM_INSTR_RET0,       // Returns from closure call, see bbzvm_ret0()
        BBZVM_INSTR_RET1,       // Returns from closure call, see bbzvm_ret1()
        BBZVM_INSTR_ADD,        // Push stack(#1) + stack(#2), pop operands
        BBZVM_INSTR_SUB,        // Push stack(#1) - stack(#2), pop operands
        BBZVM_INSTR_MUL,        // Push stack(#1) * stack(#2), pop operands
        BBZVM_INSTR_DIV,        // Push stack(#1) / stack(#2), pop operands
        BBZVM_INSTR_MOD,        // Push stack(#1) % stack(#2), pop operands
        BBZVM_INSTR_POW,        // Push stack(#1) ^ stack(#2), pop operands
        BBZVM_INSTR_UNM,        // Push -stack(#1), pop operand
        BBZVM_INSTR_AND,        // Push stack(#1) & stack(#2), pop operands
        BBZVM_INSTR_OR,         // Push stack(#1) | stack(#2), pop operands
        BBZVM_INSTR_NOT,        // Push !stack(#1), pop operand
        BBZVM_INSTR_EQ,         // Push stack(#1) == stack(#2), pop operands
        BBZVM_INSTR_NEQ,        // Push stack(#1) != stack(#2), pop operands
        BBZVM_INSTR_GT,         // Push stack(#1) > stack(#2), pop operands
        BBZVM_INSTR_GTE,        // Push stack(#1) >= stack(#2), pop operands
        BBZVM_INSTR_LT,         // Push stack(#1) < stack(#2), pop operands
        BBZVM_INSTR_LTE,        // Push stack(#1) <= stack(#2), pop operands
        BBZVM_INSTR_GLOAD,      // Push global variable corresponding to string at stack #1, pop operand
        BBZVM_INSTR_GSTORE,     // Store stack-top value into global variable at stack #2, pop operands
        BBZVM_INSTR_PUSHT,      // Push empty table
        BBZVM_INSTR_TPUT,       // Put key (stack(#2)), value (stack #1) in table (stack #3), pop key and value
        BBZVM_INSTR_TGET,       // Push value for key (stack(#1)) in table (stack #2), pop key
        BBZVM_INSTR_CALLC,      // Calls the closure on top of the stack as a normal closure
        BBZVM_INSTR_CALLS,      // Calls the closure on top of the stack as a swarm closure
        /**
         * Opcodes with argument
         */
        /* Float argument */
        BBZVM_INSTR_PUSHF,    // Push float constant onto stack
        /* Integer argument */
        BBZVM_INSTR_PUSHI,    // Push integer constant onto stack
        BBZVM_INSTR_PUSHS,    // Push string constant onto stack
        BBZVM_INSTR_PUSHCN,   // Push native closure onto stack
        BBZVM_INSTR_PUSHCC,   // Push c-function closure onto stack
        BBZVM_INSTR_PUSHL,    // Push native closure lambda onto stack
        BBZVM_INSTR_LLOAD,    // Push local variable at given position
        BBZVM_INSTR_LSTORE,   // Store stack-top value into local variable at given position, pop operand
        BBZVM_INSTR_JUMP,     // Set PC to argument
        BBZVM_INSTR_JUMPZ,    // Set PC to argument if stack top is zero, pop operand
        BBZVM_INSTR_JUMPNZ,   // Set PC to argument if stack top is not zero, pop operand
        BBZVM_INSTR_COUNT     // Used to count how many instructions have been defined
    } bbzvm_instr;

    /**
     * @brief Type for the pointer to a function which fetches bytecode data.
     * @warning The function provider should take endianness
     * into account if copying byte-by-byte.
     * @param[in] offset Bytecode offset for the data to fetch.
     * @param[in] size Size of the data to set. The VM will at most
     * try to read 4 bytes (uint32_t).
     * @return A pointer to the data to the data.
     */
    typedef const uint8_t* (*bbzvm_bcode_fetch_fun)(uint16_t offset, uint8_t size);

    /**
     * @brief The BittyBuzz Virtual Machine.
     * 
     * Responsibilities:
     *      1) Load and run bytecode.
     */
    typedef struct __attribute__((packed)) {
        /** @brief Bytecode fetcher function */
        bbzvm_bcode_fetch_fun bcode_fetch_fun;
        /** @brief Size of the loaded bytecode */
        uint16_t bcode_size;
        /** @brief Program counter */
        int16_t pc;
        /** @brief Current stack content */
        bbzheap_idx_t stack;
        /** @brief Stack list */
        bbzheap_idx_t stacks;
        /* Current local variable table */
        // TODO
        /** @brief Local variable table list */
        bbzheap_idx_t lsymts;
        /** @brief Global symbols */
        bbzheap_idx_t gsyms;
        /* Strings */
        // TODO
        /** @brief Heap content */
        bbzheap_t heap;
        /** @brief Singleton bbznil_t */
        bbzheap_idx_t nil;
        /** @brief Registered functions */
        bbzheap_idx_t flist;
        /* List of known swarms */
        // TODO
        /* List of known swarms */
        // TODO
        /* Swarm members */
        // TODO
        /* Counter for swarm membership broadcasting */
        // TODO
        /* Input message FIFO */
        // TODO
        /* Output message FIFO */
        // TODO
        /* Virtual stigmergy maps */
        // TODO
        /* Neighbor value listeners */
        // TODO
        /** @brief Current VM state */
        bbzvm_state state;
        /** @brief Current VM error */
        bbzvm_error error;
        /* Current VM error message */
        // TODO
        /** @brief Robot id */
        uint16_t robot;
        /* Random number generator state */
        // TODO
        /* Random number generator index */
        // TODO
    } bbzvm_t;

    typedef int (*bbzvm_funp)(bbzvm_t* vm);



    // ======================================
    // =  GENERAL VM FUNCTION DEFNINITIONS  =
    // ======================================



    /**
     * @brief Sets up a VM.
     * @param robot The robot id.
     * @return The VM.
     */
    void bbzvm_construct(bbzvm_t* vm, uint16_t robot);

    /**
     * @brief Destroys the VM.
     * @param vm The VM.
     */
    void bbzvm_destruct(bbzvm_t* vm);

    /**
     * @brief Sets the error state of the VM.
     * @param vm The VM.
     * @param errcode The code of the error.
     * @see bbzvm_error
     */
    void bbzvm_seterror(bbzvm_t* vm, bbzvm_error errcode);

    /**
     * @brief Sets the bytecode function in the VM.
     * @warning The passed buffer should not be deleted until the VM is done with it.
     * @warning The function provider should take endianness
     * into account if copying byte-by-byte.
     * @param[in,out] vm The VM.
     * @param[in] bcode_fetch_fun The function to call to read bytecode data.
     * @param[in] bcode_size The size (in bytes) of the bytecode.
     * @return 0 if everything OK, a non-zero value in case of error
     */
    int bbzvm_set_bcode(bbzvm_t* vm, bbzvm_bcode_fetch_fun bcode_fetch_fun, uint32_t bcode_size);

    /**
     * @brief Processes the input message queue.
     * @param vm The VM.
     */
    void bbzvm_process_inmsgs(bbzvm_t* vm);

    /**
     * @brief Processes the output message queue.
     * @param vm The VM.
     */
    void bbzvm_process_outmsgs(bbzvm_t* vm);

    /**
     * @brief Executes the next step in the bytecode, if possible.
     * @param vm The VM.
     * @return The updated VM state.
     */
    bbzvm_state bbzvm_step(bbzvm_t* vm);

    /**
     * @brief Executes the script up to completion.
     * @param vm The VM.
     * @return The updated VM state.
     */
    bbzvm_state bbzvm_execute_script(bbzvm_t* vm);
    

    // ======================================
    // =         BYTECODE FUNCTIONS         =
    // ======================================


    /**
     * @brief Calls a Buzz closure.
     * It expects the stack to be as follows:
     * #1   arg1
     * #2   arg2
     * ...
     * #N   argN
     * #N+1 closure
     * This function pops all arguments.
     * @param vm The VM.
     * @param argc The number of arguments.
     * @return 0 if everything OK, a non-zero value in case of error
     */
    bbzvm_state bbzvm_closure_call(bbzvm_t* vm, uint32_t argc);

    /**
     * @brief Calls a function defined in Buzz.
     * It expects the stack to be as follows:
     * #1 arg1
     * #2 arg2
     * ...
     * #N argN
     * This function pops all arguments.
     * @param vm The VM.
     * @param fname The function name.
     * @param argc The number of arguments.
     * @return 0 if everything OK, a non-zero value in case of error
     */
    bbzvm_state bbzvm_function_call(bbzvm_t* vm, const char* fname, uint32_t argc);

    /**
     * @brief Registers a function in the VM.
     * @param vm The VM.
     * @param funp The function pointer to register.
     * @return The function id.
     */
    uint32_t bbzvm_function_register(bbzvm_t* vm, bbzvm_funp funp);

    /**
     * @brief Calls a closure.
     * Internally checks whether the operation is valid.
     * This function expects the stack to be as follows:
     * #1   An integer for the number of closure parameters N
     * #2   Closure arg1
     * ...
     * #1+N Closure argN
     * #2+N The closure
     * This function pushes a new stack and a new local variable table filled with the
     * activation record entries and the closure arguments. In addition, it leaves the stack
     * beneath as follows:
     * #1 An integer for the return address
     * @param vm The VM.
     * @param isswrm 0 for a normal closure, 1 for a swarm closure
     * @return The VM state.
     */
    bbzvm_state bbzvm_call(bbzvm_t* vm, int isswrm);

    /**
     * @brief Pops the stack.
     * Internally checks whether the operation is valid.
     * @param vm The VM.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pop(bbzvm_t* vm);

    /**
     * @brief Duplicates the current stack top.
     * @param vm The VM.
     * @return The VM state.
     */
    bbzvm_state bbzvm_dup(bbzvm_t* vm);

    /**
     * @brief Pushes a variable on the stack.
     * @param vm The VM.
     * @param v The variable.
     * @return The VM state.
     */
    bbzvm_state bbzvm_push(bbzvm_t* vm, bbzheap_idx_t v);

    /**
     * @brief Pushes a userdata on the stack.
     * @param vm The VM.
     * @param v The C pointer to the user data.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushu(bbzvm_t* vm, void* v);

    /**
     * @brief Pushes nil on the stack.
     * @param vm The VM.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushnil(bbzvm_t* vm);

    /**
     * @brief Pushes a 32 bit signed int value on the stack.
     * @param vm The VM.
     * @param v The value.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushi(bbzvm_t* vm, int16_t v);

    /**
     * @brief Pushes a float value on the stack.
     * @param vm The VM.
     * @param v The value.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushf(bbzvm_t* vm, float v);

    /**
     * @brief Pushes a native closure on the stack.
     * Internally checks whether the operation is valid.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * @param vm The VM.
     * @param rfrnc The closure reference.
     * @param nat 1 if the closure in native, 0 if not
     * @param v The value.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushc(bbzvm_t* vm, int32_t rfrnc, int32_t nat);

    /**
     * @brief Pushes a string on the stack.
     * @param vm The VM.
     * @param strid The string id.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushs(bbzvm_t* vm, uint16_t strid);

    /**
     * @brief Pushes a lambda native closure on the stack.
     * Internally checks whether the operation is valid.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * @param vm The VM.
     * @param addr The closure address.
     * @return The VM state.
     */
    bbzvm_state bbzvm_pushl(bbzvm_t* vm, int32_t addr);

    /**
     * @brief Stores a (idx,value) pair in a table.
     * Internally checks whether the operation is valid.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * The stack is expected to be as follows:
     * #1 value
     * #2 idx
     * #3 table
     * This operation pops #1 and #2, leaving the table at the stack top.
     * @param vm The VM.
     */
    bbzvm_state bbzvm_tput(bbzvm_t* vm);
    
    /**
     * @brief Fetches a (idx,value) pair from a table.
     * Internally checks whether the operation is valid.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * The stack is expected to be as follows:
     * #1 idx
     * #2 table
     * This operation pops #1 and pushes the value, leaving the table at
     * stack #2. If the element for the given idx is not found, nil is
     * pushed as value.
     * @param vm The VM.
     */
    bbzvm_state bbzvm_tget(bbzvm_t* vm);

    /**
     * @brief Pushes the global variable located at the given stack index.
     * Internally checks whether the operation is valid.
     * @param vm The VM.
     */
    bbzvm_state bbzvm_gload(bbzvm_t* vm);

    /**
     * @brief Stores the object located at the stack top into the a global variable, pops operand.
     * Internally checks whether the operation is valid.
     * @param vm The VM.
     * @param idx The local variable index.
     */
    bbzvm_state bbzvm_gstore(bbzvm_t* vm);

    /**
     * @brief Returns from a closure without setting a return value.
     * Internally checks whether the operation is valid.
     * This function expects at least two stacks to be present. The
     * first stack is popped. The stack beneath, now the top stack, is
     * expected to have at least one element: the return address at
     * #1. The return address is popped and used to update the program
     * counter.
     * @param vm The VM.
     * @return The VM state.
     */
    bbzvm_state bbzvm_ret0(bbzvm_t* vm);

    /**
     * @brief Returns from a closure setting a return value.
     * Internally checks whether the operation is valid.
     * This function expects at least two stacks to be present. The
     * first stack must have at least one element, which is saved as
     * the return value of the call. The stack is then popped. The
     * stack beneath, now the top stack, is expected to have at least
     * one element: the return address at #1. The return address is
     * popped and used to update the program counter. Then, the saved
     * return value is pushed on the stack.
     * @param vm The VM.
     * @return The VM state.
     */
    bbzvm_state bbzvm_ret1(bbzvm_t* vm);



    // ======================================
    // =      MISC FUNCTIONS AND MACROS     =
    // ======================================

    /**
     * @brief Fetches the object at the given index in the VM's heap.
     * @param[in] vm The VM.
     * @param[in] idx Index of the object on the VM's heap.
     * @return A pointer to the fetched object.
     */
    __attribute__((always_inline)) inline
    bbzobj_t* bbzvm_obj_at(const bbzvm_t* vm, bbzheap_idx_t idx) {
        return bbzheap_obj_at(&vm->heap, idx);
    }

    /**
     * Returns the size of the stack.
     * The most recently pushed element in the stack is at size - 1.
     * @param vm The VM.
     * @return The size of the VM's current stack.
     */
     __attribute__((always_inline)) inline
    uint16_t bbzvm_stack_size(bbzvm_t* vm) {
        return bbzdarray_size(&vm->heap, vm->stack);
    }

    /**
     * @brief Returns the heap index of the element at given stack position,
     * where 0 is the top of the stack and >0 goes down the stack.
     * @warning This function performs no sanity check on the passed index.
     * @param[in,out] vm The VM.
     * @param[in] idx The stack index.
     * @return The heap index of the element at given stack index.
     */
    __attribute__((always_inline)) inline
    bbzheap_idx_t bbzvm_stack_at(bbzvm_t* vm, uint16_t idx) {
        bbzheap_idx_t ret = 0x7FFF;
        bbzdarray_get(&vm->heap, vm->stack, bbzvm_stack_size(vm) - idx, &ret);
        return ret;
    }


    /**
     * Checks whether the given stack's size is >= to the passed size.
     * If the size is not valid, it updates the VM state.
     * This macro is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * @param vm The VM.
     * @param size The stack index, where 0 is the stack top and >0 goes down the stack.
     */
    #define bbzvm_stack_assert(vm, size)                                 \
        if (bbzvm_stack_size(vm) < (size)) {                        \
            bbzvm_seterror(vm, BBZVM_ERROR_STACK);                      \
            return (vm)->state;                                         \
        }
    
    /**
     * @brief Checks whether the type at the given stack position is correct.
     * If the type is wrong, it updates the VM state and exits the current function.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or bbzvm_step().
     * @param[in,out] vm The VM data.
     * @param[in] idx The stack index, where 0 is the stack top and >0 goes down the stack.
     * @param[in] tpe The type to check
     */
    #define bbzvm_type_assert(vm, idx, tpe)                             \
        {                                                               \
            bbzobj_t* o = bbzheap_obj_at(&vm->heap,                     \
                                         bbzvm_stack_at(vm, idx));      \
            if(bbztype(*o) != tpe) {                                    \
                bbzvm_seterror(vm, BBZVM_ERROR_TYPE);                   \
                return BBZVM_STATE_ERROR;                               \
            }                                                           \
        }
    
    /*
     * Terminates the current Buzz script.
     * This function is designed to be used within int-returning functions such as
     * BuzzVM hook functions or buzzvm_step().
     * @param[in,out] vm The VM data.
     */
    #define bbzvm_done(vm)                                              \
        (vm)->state = BBZVM_STATE_DONE;                                 \
        return BBZVM_STATE_DONE;


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZVM_H