/**
 * @file bbztable.h
 * @brief Definition of BittyBuzz's table, a key-value structure implemented
 * as a dynamically-allocated array.
 */

#ifndef BBZTABLE
#define BBZTABLE

#include <bittybuzz/bbzheap.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * @brief Search the value associated with the key k in the table t from the heap.
 * Set as output the value of v, the found value corresponding to the key k.
 * @param[in] t The position of the table's object in the heap.
 * @param[in] k The key (can be any object).
 * @param[out] v A buffer for the pointer to the value.
 * @return 1 for success, 0 for failure (index not in table)
 */
uint8_t bbztable_get(bbzheap_idx_t t,
                 bbzheap_idx_t k,
                 bbzheap_idx_t* v);

/**
 * @brief Add/Edit the value corresponding to the key k in the table t from the heap h.
 * If the key isn't in the table, it will be added.
 * If the key has a value and the new value v is nil, the key will be removed from the table.
 * @param[in] t The position of the table's object in the heap.
 * @param[in] k The key (can be any object).
 * @param[in] v The value to set.
 * @return 1 for success, 0 for failure (out of memory)
 */
uint8_t bbztable_set(bbzheap_idx_t t,
                 bbzheap_idx_t k,
                 bbzheap_idx_t v);

/**
 * @brief Return the count of valid keys in the table.
 * @param[in] t The position of the table's object in the heap.
 * @return The size of the table.
 */
uint8_t bbztable_size(bbzheap_idx_t t);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
