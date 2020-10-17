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
    BBZVM_ERROR_NONE = 0,   /**< @brief No error */ // =0
    BBZVM_ERROR_INSTR,      /**< @brief Unknown instruction */ // =1
    BBZVM_ERROR_STACK,      /**< @brief Stack error */ // =2
    BBZVM_ERROR_LNUM,       /**< @brief Wrong number of local variables */ // =3
    BBZVM_ERROR_PC,         /**< @brief Program counter out of range */ // =4
    BBZVM_ERROR_FLIST,      /**< @brief Function call id out of range */ // =5
    BBZVM_ERROR_TYPE,       /**< @brief Type mismatch */ // =6
    BBZVM_ERROR_OUTOFRANGE, /**< @brief Out-of-bounds argument (e.g., trying <code>swarm.id()</code> while the swarmstack is empty) */ // =7
    BBZVM_ERROR_NOTIMPL,    /**< @brief Feature not yet implemented */ // =8
    BBZVM_ERROR_RET,        /**< @brief Non-returning function when returning was expected, or vice versa. */ // =9
    BBZVM_ERROR_STRING,     /**< @brief Unknown string id */ // =10
    BBZVM_ERROR_SWARM,      /**< @brief Unknown swarm id */ // =11
    BBZVM_ERROR_VSTIG,      /**< @brief Too many vstig entries */ // =12
    BBZVM_ERROR_MEM,        /**< @brief Out of memory */ // =13
    BBZVM_ERROR_MATH,       /**< @brief Math error */ // =14
    BBZVM_ERROR_COUNT       /**< @brief Number of errors defined by BittyBuzz. */
} bbzvm_error;

/**
 * @brief VM instructions.
 */
typedef enum bbzvm_instr {
    /*
     * Opcodes without argument
     */
    BBZVM_INSTR_NOP = 0, /**< @brief No operation */ // =0
    BBZVM_INSTR_DONE,    /**< @brief End of the program */ // =1
    BBZVM_INSTR_PUSHNIL, /**< @brief Push nil onto stack */ // =2
    BBZVM_INSTR_DUP,     /**< @brief Duplicate stack top */ // =3
    BBZVM_INSTR_POP,     /**< @brief Pop value from stack */ // =4
    BBZVM_INSTR_RET0,    /**< @brief Returns from closure call @see bbzvm_ret0() */ // =5
    BBZVM_INSTR_RET1,    /**< @brief Returns from closure call @see bbzvm_ret1() */ // =6
    BBZVM_INSTR_ADD,     /**< @brief Push stack(#0) + stack(#1), pop operands*/ // =7
    BBZVM_INSTR_SUB,     /**< @brief Push stack(#0) - stack(#1), pop operands*/ // =8
    BBZVM_INSTR_MUL,     /**< @brief Push stack(#0) * stack(#1), pop operands*/ // =9
    BBZVM_INSTR_DIV,     /**< @brief Push stack(#0) / stack(#1), pop operands*/ // =10
    BBZVM_INSTR_MOD,     /**< @brief Push stack(#0) % stack(#1), pop operands*/ // =11
    BBZVM_INSTR_POW,     /**< @brief Push stack(#0) ^ stack(#1), pop operands*/ // =12
    BBZVM_INSTR_UNM,     /**< @brief Push -stack(#0), pop operand */ // =13
    BBZVM_INSTR_LAND,    /**< @brief Push stack(#0) & stack(#1), pop operands*/ // =14
    BBZVM_INSTR_LOR,     /**< @brief Push stack(#0) | stack(#1), pop operands*/ // =15
    BBZVM_INSTR_LNOT,    /**< @brief Push !stack(#0), pop operand */ // =16
    BBZVM_INSTR_BAND,    /**< @brief Push stack(#0) & stack(#1), pop operands*/ // =17
    BBZVM_INSTR_BOR,     /**< @brief Push stack(#0) | stack(#1), pop operands*/ // =18
    BBZVM_INSTR_BNOT,    /**< @brief Push !stack(#0), pop operand */ // =19
    BBZVM_INSTR_LSHIFT,  /**< @brief Push stack(#0) << stack(#1), pop operands*/ // =20
    BBZVM_INSTR_RSHIFT,  /**< @brief Push stack(#0) >> stack(#1), pop operands*/ // =21
    BBZVM_INSTR_EQ,      /**< @brief Push stack(#0) == stack(#1), pop operands*/ // =22
    BBZVM_INSTR_NEQ,     /**< @brief Push stack(#0) != stack(#1), pop operands*/ // =23
    BBZVM_INSTR_GT,      /**< @brief Push stack(#0) > stack(#1), pop operands*/ // =24
    BBZVM_INSTR_GTE,     /**< @brief Push stack(#0) >= stack(#1), pop operands*/ // =25
    BBZVM_INSTR_LT,      /**< @brief Push stack(#0) < stack(#1), pop operands*/ // =26
    BBZVM_INSTR_LTE,     /**< @brief @brief/ Push stack(#0) <= stack(#1), pop operands */ // =27
    BBZVM_INSTR_GLOAD,   /**< @brief Push global variable corresponding to string at stack #0, pop operand */ // =28
    BBZVM_INSTR_GSTORE,  /**< @brief Store value at stack #0 into global variable at stack #1, pop operands */ // =29
    BBZVM_INSTR_PUSHT,   /**< @brief Push empty table */ // =30
    BBZVM_INSTR_TPUT,    /**< @brief Put key (stack(#1)), value (stack #0) in table (stack #2), pop operands */ // =31
    BBZVM_INSTR_TGET,    /**< @brief Push value for key (stack(#0)) in table (stack #1), pop key */ // =32
    BBZVM_INSTR_CALLC,   /**< @brief Calls the closure on top of the stack as a normal closure */ // =233
    BBZVM_INSTR_CALLS,   /**< @brief Calls the closure on top of the stack as a swarm closure */ // =34
    /*
     * Opcodes with argument
     */
    /* Float argument */
    BBZVM_INSTR_PUSHF,   /**< @brief Push float constant onto stack */ // =35
    /* Integer argument */
    BBZVM_INSTR_PUSHI,   /**< @brief Push integer constant onto stack */ // =36
    BBZVM_INSTR_PUSHS,   /**< @brief Push string constant onto stack */ // =37
    BBZVM_INSTR_PUSHCN,  /**< @brief Push native closure onto stack */ // =38
    BBZVM_INSTR_PUSHCC,  /**< @brief Push c-function closure onto stack */ // =39
    BBZVM_INSTR_PUSHL,   /**< @brief Push native closure lambda onto stack */ // =40
    BBZVM_INSTR_LLOAD,   /**< @brief Push local variable at given position */ // =41
    BBZVM_INSTR_LSTORE,  /**< @brief Store stack-top value into local variable at given position, pop operand */ // =42
    BBZVM_INSTR_LREMOVE,  /**< @brief Remove the last <argument> local variables */ // =43
    BBZVM_INSTR_JUMP,    /**< @brief Set PC to argument */ // =44
    BBZVM_INSTR_JUMPZ,   /**< @brief Set PC to argument if stack top is zero, pop operand */ // =45
    BBZVM_INSTR_JUMPNZ,  /**< @brief Set PC to argument if stack top is not zero, pop operand */ // =46
    BBZVM_INSTR_COUNT    /**< @brief Used to count how many instructions have been defined */ // =47
} bbzvm_instr;

/**
 * @brief Buzz message type.
 * @details The types are ordered by decreasing priority.
 */
typedef enum bbzmsg_payload_type_t {
    BBZMSG_BROADCAST = 0, /**< @brief Neighbor broadcast */
    BBZMSG_VSTIG_PUT,     /**< @brief Virtual stigmergy PUT */
    BBZMSG_VSTIG_QUERY,   /**< @brief Virtual stigmergy QUERY */
    BBZMSG_SWARM,         /**< @brief Swarm listing */
    BBZMSG_TYPE_COUNT     /**< @brief How many message types have been defined */
} bbzmsg_payload_type_t;

#endif // !BBZENUMS_H
