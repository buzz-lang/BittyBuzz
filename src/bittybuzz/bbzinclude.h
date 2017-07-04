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

/**
 * @brief Type for the ID of a robot.
 */
typedef uint16_t bbzrobot_id_t;

/**
 * @brief Type for a heap index.
 * This can be considered to be a custom pointer
 * to a heap-allocated element.
 */
typedef uint16_t bbzheap_idx_t;

/**
 * @brief Type for the swarms list.
 */
typedef uint8_t bbzswarmlist_t;

/**
 * @brief Type for lamport clocks.
 */
typedef uint16_t bbzlamport_t;

/**
 * @brief Specifies that a function should always be inlined.
 */
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

/**
 * @brief Specifies that a structure should not contain padding bytes.
 */
#define PACKED __attribute__((packed))

#if defined(BBZCROSSCOMPILING) || defined(DOXYGEN)
/**
 * @brief Specifies that a function should not perform extra
 * computation before and after the call.
 */
#define NAKED __attribute__((naked))
#else // defined(BBZCROSSCOMPILING) || defined(DOXYGEN)
#define NAKED
#endif // defined(BBZCROSSCOMPILING) || defined(DOXYGEN)

#define RM_UNUSED_WARN(var) (void*)var
#define RM_UNUSED_RETVAL_WARN(exp) if(exp)do{}while(0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZINCLUDES_H
