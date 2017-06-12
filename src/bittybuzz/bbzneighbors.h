/**
 * @file bbzneighbors.h
 * @brief Definition of the neighbors table, which is a table of all
 * robots a robot can communicate with.
 */

#ifndef BBZNEIGHBORS_H
#define BBZNEIGHBORS_H

#include "bbzinclude.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Type for an entry of the neighbors structure.
 */
typedef struct PACKED {
    bbzrobot_id_t robot; /**< @brief ID of the robot this entry is for. */
    uint8_t distance;    /**< @brief Distance between to the given robot. */
    uint8_t azimuth;     /**< @brief Angle (in rad) on the XY plane. */
    uint8_t elevation;   /**< @brief Angle (in rad) between the XY plane and the robot. */
} bbzneighbors_elem_t;

typedef struct PACKED {
    bbzneighbors_elem_t data[BBZNEIGHBORS_CAP]; /**< @brief Neighbor data. */
    uint8_t size; /**< @brief Current number of neighbors. */
} bbzneighbors_t;

/**
 * @brief Constructs the VM's neighbor structure.
 */
void bbzneighbors_construct();

/**
 * @brief Registers the 'neighbors' table into the VM.
 */
void bbzneighbors_register();

/**
 * @brief Clears the neighbor structure.
 * @details Add new neighbor data with bbzneighbor_add().
 * @see bbzneighbors_add()
 */
void bbzneighbors_reset();

/**
 * @brief Adds a neighbor to the neighbor data structure.
 * @param[in] robot The id of the robot.
 * @param[in] distance The distance between to the given robot.
 * @param[in] azimuth The angle (in rad) on the XY plane.
 * @param[in] elevation The angle (in rad) between the XY plane and the robot.
 * @note For some robots, distance, azimuth and/or elevation might be
 * unavailable, in which case you must set the unavailable values to 0.
 * @see bbzneighbors_reset()
 */
void bbzneighbors_add(uint16_t robot,
                      uint8_t distance,
                      uint8_t azimuth,
                      uint8_t elevation);

// ======================================
// =       BUZZ NEIGHBOR CLOSURES       =
// ======================================

/**
 * @brief Buzz C closure which broadcasts a value across the neighbors.
 */
void bbzneighbors_broadcast();

/**
 * @brief Buzz C closure which installs a listener for a value across
 * the neighbors.
 */
void bbzneighbors_listen();

/**
 * @brief Buzz C closure which removes a listener for a value across the
 * neighbors.
 */
void bbzneighbors_ignore();

/**
 * @brief Buzz C closure which pushes a table of robots belonging to the same
 * swarm as the current robot.
 */
void bbzneighbors_kin();

/**
 * @brief Buzz C closure which pushes a table of robots not belonging to the
 * same swarm as the current robot.
 */
void bbzneighbors_nonkin();

/**
 * @brief Buzz C closure which pushes a table containing (robot id, data) onto
 * the stack.
 */
void bbzneighbors_get();

/**
 * @brief Buzz C closure which calls a closure for each neighbor.
 */
void bbzneighbors_foreach();

/**
 * @brief Buzz C closure which makes a new neighbor structure in which each
 * element is transformed by the passed closure.
 */
void bbzneighbors_map();

/**
 * @brief Buzz C closure which performs a left fold/accumulation/reduction
 * operation on the neighbors.
 */
void bbzneighbors_reduce();

/**
 * @brief Buzz C closure which filters the neighbors according to a predicate.
 */
void bbzneighbors_filter();

/**
 * @brief Buzz C closure which pushes the number of neighbors on the stack.
 */
void bbzneighbors_count();

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h" // Include AFTER bbzneighbors.h because of circular dependencies.

#endif // !BBZNEIGHBORS_H