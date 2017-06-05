/**
 * @file bbzvstig.h
 * @brief Definition of BittyBuzz's Virtual Stigmergy, a structure of data
 * shared accross a swarm of robots inspired from nest-building instects'
 * stigmergies.
 * @warning Due to low hardware ressources, we only allow a single
 * stigmergy. Its ID must be 1.
 */

#ifndef BBZVSTIG_H
#define BBZVSTIG_H

#include "bbzinclude.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Virtual stigmergy element.
 */
typedef struct __attribute__((packed)) {
    bbzheap_idx_t key;   /**< @brief Element's key. */
    bbzheap_idx_t value; /**< @brief Element's current value. */
    uint8_t timestamp;   /**< @brief Timestamp (Lamport clock) of last update of the value. */
    bbzrobot_id_t robot;   /**< @brief Robot ID. */
} bbzvstig_elem_t;

/**
 * @brief Virtual stigmergy.
 * @note You should not create a stigmergy manually ; we assume there
 * is only a single instance: vm->vstig.
 */
typedef struct __attribute__((packed)) {
    uint8_t size;           /**< @brief Number of stigmergy elements. */
} bbzvstig_t;

/**
 * @brief Add an element to the virtual stigmergy.
 * @param[in] elem The object to add.
 */
void bbvstig_serialize(bbzheap_idx_t elem);

/**
 * @brief Remove an element from the virtual stigmergy.
 * @return The deserialized element.
 */
bbzheap_idx_t bbzvstig_deserialize();


// ======================================
// =        BUZZ VSTIG CLOSURES         =
// ======================================

/**
 * @brief Buzz C closure which creates the VM's virtual stigmergy structure.
 * @param[in] buf The linear buffer associated to this structure.
 * @param[in] cap The maximum number of elements in the structure.
 */
#define bbzvstig_construct(buf, cap) vm->vstig.data = buf; vm->vstig.capacity = cap; vm->vstig.size = 0;

/**
 * @brief Buzz C closure which destroys the VM's virtual stigmergy structure.
 */
#define bbzvstig_destruct() 

/**
 * @brief Buzz C closure which returns the capacity of the virtual stigmergy structure.
 * @return The capacity of the virtual stigmergy structure.
 */
#define bbzvstig_capacity() vm->vstig.capacity

/**
 * @brief Buzz C closure which returns the size of the virtual stigmergy structure.
 * @return The size of the virtual stigmergy structure.
 */
#define bbzvstig_size() vm->vstig.size

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h"

#endif // !BBZVSTIG_H