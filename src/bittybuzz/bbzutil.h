/**
 * @file bbzutil.h
 * @brief Definition of functions that help registering values in the VM.
 */

#ifndef BBZUTIL_H
#define BBZUTIL_H

#include "bbzinclude.h"
#include "bbzstrids.h"
#include "bbzvm.h"

/**
 * @brief Registers a function in a table.
 * @details The stack is expected to have the table as stack top.
 * @param[in] strid The string ID of the field in which the function will be
 * stored.
 * @param[in] fun The function to store.
 */
void bbztable_add_function(uint16_t strid, bbzvm_funp fun);

/**
 * @brief Registers some data in a table.
 * @details The stack is expected to have the table as stack top.
 * @param[in] strid The string ID of the field in which the data will be
 * stored.
 * @param[in] data The data to store.
 */
void bbztable_add_data(uint16_t strid, bbzheap_idx_t data);

/**
 * @brief Swap the <code>size</code> first bytes between <code>arr1</code> and <code>arr2</code>.
 * @note Can be used with 2 structures (pointer) of the same size.
 * @param[in,out] arr1 The first array to swap.
 * @param[in,out] arr2 The second array to swap.
 * @param[in] size The number of bytes to swap in each array.
 */
void bbzutil_swapArrays(uint8_t *arr1, uint8_t *arr2, uint8_t size);

#endif // !BBZUTIL_H