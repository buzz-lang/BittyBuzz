/**
 * @file bbzincludes.h
 * @brief Defines various symbols to be included by all files.
 */

#ifndef BBZINCLUDES_H
#define BBZINCLUDES_H

#include "bittybuzz/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Specifies that a function should always be inlined.
 */
#define ALWAYS_INLINE __attribute__((always_inline)) static inline

#ifdef __cplusplus
}
#endif

#endif // !BBZINCLUDES_H
