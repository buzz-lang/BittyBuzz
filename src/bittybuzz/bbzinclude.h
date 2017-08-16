/**
 * @file bbzinclude.h
 * @brief Definition of various symbols. Header should be included by all files.
 */

#ifndef BBZINCLUDES_H
#define BBZINCLUDES_H

#include <inttypes.h>

#include "bittybuzz/config.h"
#include "bittybuzz/bbzenums.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//typedef uint16_t bbzuint_t;
//typedef int16_t bbzint_t;

/**
 * @brief Type for a heap index.
 * This can be considered to be a custom pointer to a heap-allocated element.
 * @details Only the 15 LSBs can be used out of the 16 bits that
 * a heap index has, meaning the heap's design can contain up to to
 * 32,768 objects.
 */
typedef uint16_t bbzheap_idx_t;

/**
 * @brief Type for the ID of a robot.
 */
typedef uint16_t bbzrobot_id_t;

/**
 * @brief Type for the ID of a swarm.
 * @details Valid swarm IDs are within range
 * [<code>0</code>, <code>8*sizeof(bbzswarmlist_t)</code>[.
 * Thus, swarm IDs greater than, or equal to,
 * <code>8*sizeof(bbzswarmlist_t)</code> are invalid and BBZVM_ERROR_SWARM
 * will be set if such an ID is passed to any swarm function.
 */
typedef uint8_t bbzswarm_id_t;

/**
 * @brief Type for the swarm list of a robot, that is, the list of swarms
 * that the robot is a member of.
 * @details This is a bitfield where the i-th bit represents whether
 * the robot is a member of the i-th swarm.<br/>
 * Thus, bit 0 -> swarm 0, bit 1 -> swarm 1, ...<br/>
 * For example, 0b01000001 means that the robot is a member of swarms 6 and 0.
 */
typedef uint8_t bbzswarmlist_t;

/**
 * @brief Type for lamport clocks.
 */
typedef uint16_t bbzlamport_t;

/**
 * @brief Type for the program counter.
 */
typedef uint16_t bbzpc_t;

/**
 * @brief NULL pointer.
 */
#ifndef NULL
#define NULL ((void*)0)
#endif // !NULL

/**
 * @brief Specifies that a function should always be inlined.
 */
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

/**
 * @brief Specifies that a structure should not contain padding bytes.
 */
#define PACKED __attribute__((packed))

/**
 * @brief Specifies that a function should not perform extra
 * computation before and after the call.
 */
#if defined(BBZCROSSCOMPILING) || defined(DOXYGEN)
#define NAKED __attribute__((naked))
#else // defined(BBZCROSSCOMPILING) || defined(DOXYGEN)
#define NAKED
#endif // defined(BBZCROSSCOMPILING) || defined(DOXYGEN)

/**
 * @brief Removes warnings about unused variables/parameters.
 * @param[in] var The unused variable.
 */
#define RM_UNUSED_WARN(var) ((void)(var))
/**
 * @brief Removes warnings about unused return values.
 * @param[in] exp The expression returning a useless value.
 */
#define RM_UNUSED_RETVAL_WARN(exp) if(exp)do{}while(0)

/**
 * @brief Swaps the value of two (2) integers-like variables.
 * @param[in,out] a the first variable to swap.
 * @param[in,out] b the second variable to swap.
 */
#define SWAP(a, b) {(a)^=(b);(b)^=(a);(a)^=(b);}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZINCLUDES_H
