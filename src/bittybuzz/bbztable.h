#ifndef BBZTABLE
#define BBZTABLE

#include <bittybuzz/bbzheap.h>

/*
 * Search the value associated with the key k in the table t from the heap h.
 * Set as output the value of v, the found value corresponding to the key k.
 * @param h A pointer to the heap.
 * @param t The position of the table's object in the heap.
 * @param k The key (can be any object).
 * @param v A buffer for the pointer to the value.
 * @return 1 for success, 0 for failure (index not in table)
 */
int bbztable_get(bbzheap_t* h,
                 bbzheap_idx_t t,
                 bbzheap_idx_t k,
                 bbzheap_idx_t* v);

/*
 * Add/Edit the value corresponding to the key k in the table t from the heap h.
 * If the key isn't in the table, it will be added.
 * If the key has a value and the new value v is nil, the key will be removed from the table.
 * @param h A pointer to the heap.
 * @param t The position of the table's object in the heap.
 * @param k The key (can be any object).
 * @param v The value to set.
 * @return 1 for success, 0 for failure (out of memory)
 */
int bbztable_set(bbzheap_t* h,
                 bbzheap_idx_t t,
                 bbzheap_idx_t k,
                 bbzheap_idx_t v);

/*
 * Return the count of valid keys in the table.
 * @param h A pointer to the heap.
 * @param t The position of the table's object in the heap.
 * @return The size of the table.
 */
uint8_t bbztable_size(bbzheap_t* h,
                      bbzheap_idx_t t);

#endif
