/**
 * @file bbzinclude.h
 * @brief Defines various symbols. Header should be included by all files.
 */

#ifndef BBZINCLUDES_H
#define BBZINCLUDES_H

#include <inttypes.h>

#include "bittybuzz/config.h"

#ifdef __cplusplus
extern "C" {
#endif


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
 * @brief Specifies that a function should always be inlined.
 */
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

#ifdef __cplusplus
}
#endif

#endif // !BBZINCLUDES_H
