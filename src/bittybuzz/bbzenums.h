/**
 * @file bbzenums.h
 * @brief Definition of various enums.
 */

#ifndef BBZENUMS_H
#define BBZENUMS_H

/**
 * @brief VM states
 */
typedef enum bbzvm_state {
    BBZVM_STATE_NOCODE = 0, /**< @brief No code loaded */
    BBZVM_STATE_READY,      /**< @brief Ready to execute next instruction */
    BBZVM_STATE_STOPPED,    /**< @brief Stopped (Paused) @details May be used for debugging purpose. */
    BBZVM_STATE_DONE,       /**< @brief Program finished */
    BBZVM_STATE_ERROR,      /**< @brief Error occurred */
    BBZVM_STATE_COUNT,      /**< @brief The number of states in the enum. */
} bbzvm_state;


/**
 * @brief VM error codes
 */
typedef enum bbzvm_error {
    BBZVM_ERROR_NONE = 0,   /**< @brief No error @note =0 */
    BBZVM_ERROR_INSTR,      /**< @brief Unknown instruction @note =1 */
    BBZVM_ERROR_STACK,      /**< @brief Stack error @note =2 */
    BBZVM_ERROR_LNUM,       /**< @brief Wrong number of local variables @note =3 */
    BBZVM_ERROR_PC,         /**< @brief Program counter out of range @note =4 */
    BBZVM_ERROR_FLIST,      /**< @brief Function call id out of range @note =5 */
    BBZVM_ERROR_TYPE,       /**< @brief Type mismatch @note =6 */
    BBZVM_ERROR_OUTOFRANGE, /**< @brief Out-of-bounds argument (e.g. Trying <code>swarm.id()</code> while the swarmstack is empty) @note =7 */
    BBZVM_ERROR_RET,        /**< @brief Non-returning function when returning was expected, or vice versa. @note =8 */
    BBZVM_ERROR_STRING,     /**< @brief Unknown string id @note =9 */
    BBZVM_ERROR_SWARM,      /**< @brief Unknown swarm id @note =10 */
    BBZVM_ERROR_VSTIG,      /**< @brief Too many vstig entries. @note =11 */
    BBZVM_ERROR_MEM,        /**< @brief Out of memory @note =12 */
    BBZVM_ERROR_MATH,       /**< @brief Math error @note =13 */
    BBZVM_ERROR_COUNT       /**< @brief Number of errors defined by BittyBuzz. */
} bbzvm_error;

/**
 * @brief VM instructions.
 */
typedef enum bbzvm_instr {
    /*
     * Opcodes without argument
     */
    BBZVM_INSTR_NOP = 0, /**< @brief No operation */
    BBZVM_INSTR_DONE,    /**< @brief End of the program */
    BBZVM_INSTR_PUSHNIL, /**< @brief Push nil onto stack */
    BBZVM_INSTR_DUP,     /**< @brief Duplicate stack top */
    BBZVM_INSTR_POP,     /**< @brief Pop value from stack */
    BBZVM_INSTR_RET0,    /**< @brief Returns from closure call, see bbzvm_ret0() */
    BBZVM_INSTR_RET1,    /**< @brief Returns from closure call, see bbzvm_ret1() */
    BBZVM_INSTR_ADD,     /**< @brief Push stack(#0) + stack(#1), pop operands */
    BBZVM_INSTR_SUB,     /**< @brief Push stack(#0) - stack(#1), pop operands */
    BBZVM_INSTR_MUL,     /**< @brief Push stack(#0) * stack(#1), pop operands */
    BBZVM_INSTR_DIV,     /**< @brief Push stack(#0) / stack(#1), pop operands */
    BBZVM_INSTR_MOD,     /**< @brief Push stack(#0) % stack(#1), pop operands */
    BBZVM_INSTR_POW,     /**< @brief Push stack(#0) ^ stack(#1), pop operands */
    BBZVM_INSTR_UNM,     /**< @brief Push -stack(#0), pop operand */
    BBZVM_INSTR_AND,     /**< @brief Push stack(#0) & stack(#1), pop operands */
    BBZVM_INSTR_OR,      /**< @brief Push stack(#0) | stack(#1), pop operands */
    BBZVM_INSTR_NOT,     /**< @brief Push !stack(#0), pop operand */
    BBZVM_INSTR_EQ,      /**< @brief Push stack(#0) == stack(#1), pop operands */
    BBZVM_INSTR_NEQ,     /**< @brief Push stack(#0) != stack(#1), pop operands */
    BBZVM_INSTR_GT,      /**< @brief Push stack(#0) > stack(#1), pop operands */
    BBZVM_INSTR_GTE,     /**< @brief Push stack(#0) >= stack(#1), pop operands */
    BBZVM_INSTR_LT,      /**< @brief Push stack(#0) < stack(#1), pop operands */
    BBZVM_INSTR_LTE,     /**< @brief @brief/ Push stack(#0) <= stack(#1), pop operands */
    BBZVM_INSTR_GLOAD,   /**< @brief Push global variable corresponding to string at stack #0, pop operand */
    BBZVM_INSTR_GSTORE,  /**< @brief Store value at stack #0 into global variable at stack #1, pop operands */
    BBZVM_INSTR_PUSHT,   /**< @brief Push empty table */
    BBZVM_INSTR_TPUT,    /**< @brief Put key (stack(#1)), value (stack #0) in table (stack #2), pop operands */
    BBZVM_INSTR_TGET,    /**< @brief Push value for key (stack(#0)) in table (stack #1), pop key */
    BBZVM_INSTR_CALLC,   /**< @brief Calls the closure on top of the stack as a normal closure */
    BBZVM_INSTR_CALLS,   /**< @brief Calls the closure on top of the stack as a swarm closure */
    /*
     * Opcodes with argument
     */
    /* Float argument */
    BBZVM_INSTR_PUSHF,   /**< @brief Push float constant onto stack */
    /* Integer argument */
    BBZVM_INSTR_PUSHI,   /**< @brief Push integer constant onto stack */
    BBZVM_INSTR_PUSHS,   /**< @brief Push string constant onto stack */
    BBZVM_INSTR_PUSHCN,  /**< @brief Push native closure onto stack */
    BBZVM_INSTR_PUSHCC,  /**< @brief Push c-function closure onto stack */
    BBZVM_INSTR_PUSHL,   /**< @brief Push native closure lambda onto stack */
    BBZVM_INSTR_LLOAD,   /**< @brief Push local variable at given position */
    BBZVM_INSTR_LSTORE,  /**< @brief Store stack-top value into local variable at given position, pop operand */
    BBZVM_INSTR_JUMP,    /**< @brief Set PC to argument */
    BBZVM_INSTR_JUMPZ,   /**< @brief Set PC to argument if stack top is zero, pop operand */
    BBZVM_INSTR_JUMPNZ,  /**< @brief Set PC to argument if stack top is not zero, pop operand */
    BBZVM_INSTR_COUNT    /**< @brief Used to count how many instructions have been defined */
} bbzvm_instr;

/**
 * @brief Buzz message type.
 * @details The types are ordered by decreasing priority.
 */
typedef enum bbzmsg_payload_type_t {
    BBZMSG_BROADCAST = 0, /**< @brief Neighbor broadcast */
    BBZMSG_VSTIG_PUT,     /**< @brief Virtual stigmergy PUT */
    BBZMSG_VSTIG_QUERY,   /**< @brief Virtual stigmergy QUERY */
    BBZMSG_SWARM_CHUNK,   /**< @brief Swarm listing */
    BBZMSG_TYPE_COUNT     /**< @brief How many message types have been defined */
} bbzmsg_payload_type_t;

#endif // !BBZENUMS_H