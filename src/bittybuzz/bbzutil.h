/**
 * @file bbzutil.h
 * @brief Definition of useful functions that can be used as a shorthand
 * for common operations.
 */

#ifndef BBZUTIL_H
#define BBZUTIL_H

#include "bbzinclude.h"
#include "bbzstrids.h"
#include "bbzvm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Registers a function in a table.
 * @details The stack is expected to have the table as stack top.
 * The table will be left at stack top, allowing you to call
 * bbztable_add_function() multiple times.
 *
 * <b>Example usage:</b>
 *
 * @code
 *
 * bbzvm_pusht(); // Push a new table on the stack
 * bbztable_add_function(BBZSTRING_ID(foo));
 * bbztable_add_function(BBZSTRING_ID(bar));
 * // ...
 * bbzheap_gc(); // Issue periodically if you are experiencing memory shortages
 * // ...
 * bbztable_add_function(BBZSTRING_ID(baz));
 * bbzheap_idx_t my_table = bbzvm_stack_at(0);
 * bbzvm_pop();
 *
 * @endcode
 *
 * @note
 * <ul>
 * <li>If you are using this function many times in a row, it is advised to
 * periodically call the garbage-collector to free some space on the heap.</li>
 * <li>A table can technically contain any object as key, but in the
 * typical case the key is a string. If you need the key to be another
 * type of object (integer, float, another closure, ...), you can do it
 * manually. Inspire yourself from the implementation of this function,
 * available on the GitHub repository of BittyBuzz.</li>
 * </ul>
 *
 * @param[in] strid The string ID of the field in which the function will be
 * stored.
 * @param[in] fun The function to store.
 */
void bbztable_add_function(uint16_t strid, bbzvm_funp fun);

/**
 * @brief Registers some data in a table.
 * @details The stack is expected to have the table as stack top.
 * The table will be left at stack top, allowing you to call
 * bbztable_add_data() multiple times.
 *
 * <b>Example usage:</b>
 *
 * @code
 *
 * bbzvm_pusht(); // Push a new table on the stack
 * bbztable_add_data(BBZSTRING_ID(id), bbzvm_stack_at(1));
 * bbztable_add_data(BBZSTRING_ID(led), bbzvm_stack_at(2));
 * // ...
 * bbzheap_gc(); // Issue periodically if you are experiencing memory shortages
 * // ...
 * bbztable_add_data(BBZSTRING_ID(distance), bbzvm_stack_at(N+1));
 * bbzheap_idx_t my_table = bbzvm_stack_at(0);
 * bbzvm_pop();
 *
 * @endcode
 *
 * @note
 * <ul>
 * <li>If you are using this function many times in a row, it is advised to
 * periodically call the garbage-collector to free some space on the heap.</li>
 * <li>A table can technically contain any object as key, but in the
 * typical case the key is a string. If you need the key to be another
 * type of object (integer, float, another closure, ...), you can do it
 * manually. Inspire yourself from the implementation of this function,
 * available on the GitHub repository of BittyBuzz.</li>
 * </ul>
 *
 * @param[in] strid The string ID of the field in which the data will be
 * stored.
 * @param[in] data The data to store.
 */
void bbztable_add_data(uint16_t strid, bbzheap_idx_t data);

/**
 * @brief Gets a subfield of a table.
 * @details The stack is expected to have the table as stack top.
 * The table will be left at stack top, allowing you to call
 * bbztable_get_subfield() multiple times.
 *
 * <b>Example usage:</b>
 *
 * @code
 *
 * bbzvm_push(my_table);
 * bbzheap_idx_t id = bbztable_get_subfield(BBZSTRING_ID(id));
 * bbzheap_idx_t led = bbztable_get_subfield(BBZSTRING_ID(led));
 * // ...
 * bbzheap_gc(); // Issue periodically if you are experiencing memory shortages
 * // ...
 * bbzheap_idx_t distance = bbztable_get_subfield(BBZSTRING_ID(distance));
 * bbzvm_pop();
 *
 * @endcode
 *
 * @note
 * <ul>
 * <li>If you are using this function many times in a row, it is advised to
 * periodically call the garbage-collector to free some space on the heap.</li>
 * <li>A table can technically contain any object as key, but in the
 * typical case the key is a string. If you need the key to be another
 * type of object (integer, float, another closure, ...), you can do it
 * manually. Inspire yourself from the implementation of this function,
 * available on the GitHub repository of BittyBuzz.</li>
 * </ul>
 *
 * @param[in] strid The string ID of the field we want to get.
 */
bbzheap_idx_t bbztable_get_subfield(uint16_t strid);

/**
 * @todo Move this function away from this header.
 *
 * @brief Swap the <code>size</code> first bytes between <code>arr1</code>
 * and <code>arr2</code>.
 * @note Can be used with 2 structures (pointer) of the same size.
 * @param[in,out] arr1 The first array to swap.
 * @param[in,out] arr2 The second array to swap.
 * @param[in] size The number of bytes to swap in each array.
 */
void bbzutil_swapArrays(uint8_t *arr1, uint8_t *arr2, uint8_t size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZUTIL_H