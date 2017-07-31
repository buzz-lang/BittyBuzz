/**
 * @file bbzneighbors.h
 * @brief Definition of the neighbors table, which is a table of all
 * robots a robot can communicate with.
 *
 * @details <h2>Explanation of the implementation:</h2>
 *
 * <h3>'Regular' implementation (non Xtreme):</h3>
 *
 * Both the <code>neighbors</code> table and the neighbor-like tables
 * (such as the one returned by the filter closure) have the same
 * implementation:
 * The table contains a subfield  (string __BBZSTRID_INTERNAL_1_DO_NOT_USE)
 * which itself contains one table for each neighbor.
 *
 * <h3>BBZ_XTREME_MEMORY implementation:</h3>
 *
 * Creating a table for each neighbor robot would be memory-expensive, because
 * a table segment would have to be allocated for each neighbor, even though
 * we only really need 5 bytes of data for each neighbor (robot ID,
 * distance, azimuth, elevation). Instead, the neighbors structure is
 * implemented as a C structure (see #bbzneighbors_elem_t).
 *
 * This, however, poses a problem : the algorithms ('foreach', 'get', etc.)
 * have to be compatible both with the 'neighbors' structure, which has C
 * implementation, and the nieghbor-like structures (such as
 * the table returned by neighbors.filter), which have a Buzz implementation.
 * The (ugly!) solution we have implemented is to keep the heap position
 * of the 'neighbors' table inside the VM, and then vary the algorithm
 * depending on whether we are working on the 'neighbors' table or a
 * neighbor-like table.
 *
 * We could have made two distinct set of algorithms, one for the 'nieghbors'
 * table, and one for the others, but this would have taken more program
 * space.
 *
 * Thus, we end up with the following implementation:
 * <ul>
 * <li> The <code>neighbors</code> table, apart from the closures, contains a
 * count subfield (string __BBZSTRID_INTERNAL_2_DO_NOT_USE). The actual
 * data is placed inside a C structure (a table of #bbzneighbors_elem_t).
 * <li> The neighbor-like tables, apart from the closures, contain a
 * count subfield (string __BBZSTRID_INTERNAL_2_DO_NOT_USE). The actual
 * data is placed directly inside the neighbor-like table, and is a table
 * containing the <code>{distance, azumuth, elevation}</code> subfields.
 * Thus, we <i>could</i> access the distance for a neighbor by doing
 * something like <code>neighborlike[32].distance</code>.
 * </ul>
 */

#ifndef BBZNEIGHBORS_H
#define BBZNEIGHBORS_H

#include "bbzinclude.h"
#include "bbzringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Type for an entry of the neighbors structure.
 */
typedef struct PACKED {
#ifndef BBZ_DISABLE_NEIGHBORS
    bbzrobot_id_t robot; /**< @brief ID of the robot this entry is for. */
    uint8_t distance;    /**< @brief Distance between to the given robot. */
    uint8_t azimuth;     /**< @brief Angle (in rad) on the XY plane. */
    uint8_t elevation;   /**< @brief Angle (in rad) between the XY plane and the robot. */
#endif // !BBZ_DISABLE_NEIGHBORS
} bbzneighbors_elem_t;

/**
 * @brief Type for the neighbors structure.
 * @note You should not create a neighbors table manually ; we assume there
 * is only a single instance: <code>vm->neighbors.hpos</code>.
 */
typedef struct PACKED {
#ifndef BBZ_DISABLE_NEIGHBORS
    bbzheap_idx_t hpos;      /**< @brief Heap's position of the 'neighbors' table. */
    bbzheap_idx_t listeners; /**< @brief Neighbor value listeners. */
#ifdef BBZ_XTREME_MEMORY
    bbzringbuf_t rb;         /**< @brief Data buffer. */
    bbzneighbors_elem_t data[BBZNEIGHBORS_CAP+1]; /**< @brief Neighbor data. */
#else
    uint8_t count;           /**< @brief Current number of neighbors. */
#endif // BBZ_XTREME_MEMORY
#endif // !BBZ_DISABLE_NEIGHBORS
} bbzneighbors_t;

#ifndef BBZ_DISABLE_NEIGHBORS
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
 * @param[in] data The data for that neighbor.
 * @note For some robots, distance, azimuth and/or elevation might be
 * unavailable. Though there is no restriction, it is advised to set the
 * unavailable values to 0.
 * @warning You should not add the data of a robot you already added.
 * @see bbzneighbors_reset()
 */
void bbzneighbors_add(const bbzneighbors_elem_t* data);

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
 * @warning This function may use a lot of memory because a table has
 * to be allocated for <i>each neighbor</i> for which the predicate is true.
 * Set the value to nil once you are done with it.
 * Also prefer more memory-savvy methods, such as bbzneighbors_foreach(),
 * bbzneighbors_reduce() or bbzneighbors_get(), which only allocate a single
 * table.
 */
void bbzneighbors_filter();

/**
 * @brief Buzz C closure which pushes the number of neighbors on the stack.
 */
void bbzneighbors_count();
#else
#define bbzneighbors_register(...)
#define bbzneighbors_reset(...)
#define bbzneighbors_add(...)
void bbzneighbors_dummy();
void bbzneighbors_dummyret();
#define bbzneighbors_broadcast bbzneighbors_dummy
#define bbzneighbors_listen bbzneighbors_dummy
#define bbzneighbors_ignore bbzneighbors_dummy
#define bbzneighbors_get bbzneighbors_dummy
#define bbzneighbors_foreach bbzneighbors_dummy
#define bbzneighbors_map bbzneighbors_dummy
#define bbzneighbors_reduce bbzneighbors_dummy
#define bbzneighbors_filter bbzneighbors_dummy
#define bbzneighbors_count bbzneighbors_dummy
#endif // !BBZ_DISABLE_NEIGHBORS

#include "bbzvm.h" // Include AFTER bbzneighbors.h because of circular dependencies.

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZNEIGHBORS_H