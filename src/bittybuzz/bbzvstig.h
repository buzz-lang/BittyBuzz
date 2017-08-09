/**
 * @file bbzvstig.h
 * @brief Definition of BittyBuzz's Virtual Stigmergy, a structure of data
 * shared accross a swarm of robots inspired from nest-building instects'
 * stigmergies.
 * @warning Due to low hardware resources, we only allow a single
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
typedef struct PACKED bbzvstig_elem_t {
#ifndef BBZ_DISABLE_VSTIGS
    uint16_t key;   /**< @brief Element's key. */
    bbzheap_idx_t value; /**< @brief Element's current value. */
    uint8_t timestamp;   /**< @brief Timestamp (Lamport clock) of last update of the value. */
    bbzrobot_id_t robot; /**< @brief Robot ID. */
#endif
} bbzvstig_elem_t;

/**
 * @brief Virtual stigmergy.
 * @note You should not create a stigmergy manually ; we assume there
 * is only a single instance: <code>vm->vstig.hpos</code>.
 */
typedef struct PACKED bbzvstig_t {
#ifndef BBZ_DISABLE_VSTIGS
    bbzvstig_elem_t data[BBZVSTIG_CAP]; /**< @brief Data of the stigmergy. */
    uint8_t size;       /**< @brief Number of stigmergy elements. */
    bbzheap_idx_t hpos; /**< @brief Heap's position of the 'stigmergy' table. */
#endif
} bbzvstig_t;

#ifndef BBZ_DISABLE_VSTIGS
/**
 * @brief Creates the VM's virtual stigmergy structure.
 */
#define bbzvstig_construct() do{vm->vstig.size = 0;}while(0)

/**
 * @brief Registers the 'stigmergy' table, as well as its methods, in the VM.
 */
void bbzvstig_register();


// ======================================
// =        BUZZ VSTIG CLOSURES         =
// ======================================

/**
 * @brief Buzz C closure which creates a stigmergy.
 * @note For reasons of compatibility with Buzz, BittyBuzz also
 * expects the stigmergy ID to be passed to this function, although
 * it is ignored.
 */
void bbzvstig_create();

/**
 * @brief Buzz C closure which set the onconflict callback closure.
 */
void bbzvstig_onconflict();

/**
 * @brief Buzz C closure which set the onconflictlost callback closure.
 */
void bbzvstig_onconflictlost();

/**
 * @brief Buzz C closure which gets a value from the stigmergy.
 * @details One parameter is expected on the stack: the key.
 */
void bbzvstig_get();

/**
 * @brief Buzz C closure which sets a value in the stigmergy.
 * @details Two parameters are expected on the stack: the key and the value.
 */
void bbzvstig_put();

/**
 * @brief Buzz C closure which returns the size of the virtual
 * stigmergy structure.
 */
void bbzvstig_size();
#else
#define bbzvstig_construct(...)
#define bbzvstig_register(...)
void bbzvstig_dummy();
#define bbzvstig_create bbzvstig_dummy
#define bbzvstig_onconflict bbzvstig_dummy
#define bbzvstig_onconflictlost bbzvstig_dummy
#define bbzvstig_get bbzvstig_dummy
#define bbzvstig_put bbzvstig_dummy
#define bbzvstig_size bbzvstig_dummy
#endif // !BBZ_DISABLE_VSTIGS

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h" // Include AFTER bbzvstig.h because of circular dependencies.

#endif // !BBZVSTIG_H