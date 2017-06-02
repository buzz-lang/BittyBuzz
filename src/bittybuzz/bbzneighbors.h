/**
 * @file bbzneighbors.h
 * @brief Definition of the neighbors table, which is a table of all
 * robots a robot can communicate with.
 */

#ifndef BBZNEIGHBORS_H
#define BBZNEIGHBORS_H

#include "bbzincludes.h"
#include "bbzvm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Clears the neighbor structure.
 * Add new neighbor data with bbzneighbor_add().
 * @return The updated VM state.
 * @see bbzneighbors_add()
 */
extern bbzvm_state bbzneighbors_reset();

/**
 * @brief Adds a neighbor to the neighbor data structure.
 * @param[in] robot The id of the robot.
 * @param[in] distance The distance between to the given robot.
 * @param[in] azimuth The angle (in rad) on the XY plane.
 * @param[in] elevation The angle (in rad) between the XY plane and the robot.
 * @return The updated VM state.
 * @see bbzneighbors_reset()
 */
extern bbzvm_state bbzneighbors_add(uint16_t robot,
                                    float distance,
                                    float azimuth,
                                    float elevation);

/**
 * @brief Broadcasts a value across the neighbors.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_broadcast();

/**
 * @brief Installs a listener for a value across the neighbors.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_listen();

/**
 * @brief Removes a listener for a value across the neighbors.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_ignore();

/**
 * @brief Pushes a table of robots belonging to the same swarm as the current robot.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_kin();

/**
 * @brief Pushes a table of robots not belonging to the same swarm as the current robot.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_nonkin();

/**
 * @brief Pushes a table containing (robot id, data) onto the stack.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_get();

/**
 * @brief Calls a closure for each neighbor.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_foreach();

/**
 * @brief Makes a new neighbor structure in which each element is transformed by the passed closure.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_map();

/**
 * @brief Performs a left fold/accumulation/reduction operation on the neighbors.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_reduce();

/**
 * @brief Filters the neighbors according to a predicate.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_filter();

/**
 * @brief Pushes the number of neighbors on the stack.
 * @return The updated VM state.
 */
extern bbzvm_state bbzneighbors_count();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZNEIGHBORS_H