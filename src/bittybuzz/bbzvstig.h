/**
 * @file bbzvstig.h
 * @brief Definition of BittyBuzz's Virtual Stigmergy, a structure of data
 * shared accross a swarm of robots inspired from nest-building instects'
 * stigmergies.
 * @warning Due to low hardware ressources, we only allow a single
 * stigmergy. Its ID must be 1.
 */

#ifndef BBZVSTIG
#define BBZVSTIG

#include "bbzincludes.h"
#include "bbzheap.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Virtual stigmergy element.
 */
typedef struct __attribute__((packed)) {
   bbzheap_idx_t key;      /**< @brief Element's key. */
   bbzheap_idx_t value;    /**< @brief Element's current value. */
   uint8_t timestamp; /**< @brief Timestamp (Lamport clock) of last update of the value. */
   uint8_t robot;     /**< @brief Robot ID. */
} bbzvstig_elem_t;

/**
 * @brief Virtual stigmergy.
 */
typedef struct __attribute__((packed)) {
   bbzvstig_elem_t* data; /**< @brief Stigmergy elements. */
   uint8_t capacity;      /**< @brief Allocated stigmergy element capacity. */
   uint8_t size;          /**< @brief Number of stigmergy elements. */
} bbzvstig_t;

/**
 * @brief Creates a virtual stigmergy structure.
 * @param[in,out] vs The virtual stigmergy structure.
 * @param[in] buf The linear buffer associated to this structure.
 * @param[in] cap The maximum number of elements in the structure.
 */
#define bbzvstig_new(vs, buf, cap) (vs).data = buf; (vs).capacity = cap; (vs).size = 0;

/**
 * @brief Returns the capacity of the virtual stigmergy structure.
 * @param[in] vs The virtual stigmergy structure.
 * @return The capacity of the virtual stigmergy structure.
 */
#define bbzvstig_capacity(vs) ((vs).capacity)

/**
 * @brief Returns the size of the virtual stigmergy structure.
 * @param[in] vs The virtual stigmergy structure.
 * @return The size of the virtual stigmergy structure.
 */
#define bbzvstig_size(vs) ((vs).size)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
