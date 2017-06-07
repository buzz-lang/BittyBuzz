/**
 * @file bbzneighbors.h
 * @brief Definition of the neighbors table, which is a table of all
 * robots a robot can communicate with.
 */

#ifndef BBZNEIGHBORS_H
#define BBZNEIGHBORS_H

#include "bbzinclude.h"
#include "bbzvm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Clears the neighbor structure.
 * Add new neighbor data with bbzneighbor_add().
 * @see bbzneighbors_add()
 */
void bbzneighbors_reset();

/**
 * @brief Adds a neighbor to the neighbor data structure.
 * @param[in] robot The id of the robot.
 * @param[in] distance The distance between to the given robot.
 * @param[in] azimuth The angle (in rad) on the XY plane.
 * @param[in] elevation The angle (in rad) between the XY plane and the robot.
 * @see bbzneighbors_reset()
 */
void bbzneighbors_add(uint16_t robot,
                             uint8_t distance,
                             uint8_t azimuth,
                             uint8_t elevation);

/**
 * @brief Broadcasts a value across the neighbors.
 */
void bbzneighbors_broadcast();

/**
 * @brief Installs a listener for a value across the neighbors.
 */
void bbzneighbors_listen();

/**
 * @brief Removes a listener for a value across the neighbors.
 */
void bbzneighbors_ignore();

/**
 * @brief Pushes a table of robots belonging to the same swarm as the current robot.
 */
void bbzneighbors_kin();

/**
 * @brief Pushes a table of robots not belonging to the same swarm as the current robot.
 */
void bbzneighbors_nonkin();

/**
 * @brief Pushes a table containing (robot id, data) onto the stack.
 */
void bbzneighbors_get();

/**
 * @brief Calls a closure for each neighbor.
 */
void bbzneighbors_foreach();

/**
 * @brief Makes a new neighbor structure in which each element is transformed by the passed closure.
 */
void bbzneighbors_map();

/**
 * @brief Performs a left fold/accumulation/reduction operation on the neighbors.
 */
void bbzneighbors_reduce();

/**
 * @brief Filters the neighbors according to a predicate.
 */
void bbzneighbors_filter();

/**
 * @brief Pushes the number of neighbors on the stack.
 */
void bbzneighbors_count();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZNEIGHBORS_H