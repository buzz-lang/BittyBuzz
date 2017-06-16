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

#endif // !BBZUTIL_H