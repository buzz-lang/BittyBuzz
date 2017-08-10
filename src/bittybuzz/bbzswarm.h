/**
 * @file bbzswarm.h
 * @brief Definition of swarms, which are collections of robots.
 */

#ifndef BBZSWARM_H
#define BBZSWARM_H

#include "bbzinclude.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Maximum age (in steps) for swarm membership to be remembered.
 */
#define MEMBERSHIP_AGE_MAX 50

/**
 * @brief Data type for the robot membership data structure.
 */
typedef struct PACKED bbzswarm_t {
#ifndef BBZ_DISABLE_SWARMS
    bbzheap_idx_t hpos;       /**< @brief Heap's position of the 'swarm' table. */
    bbzheap_idx_t swarmstack; /**< @brief The stack of swarm IDs that we push to/pop from when we call/return from the 'exec' function. */
#endif // !BBZ_DISABLE_SWARMS
} bbzswarm_t;

#ifndef BBZ_DISABLE_SWARMS

/**
 * @brief Registers and constructs the VM's swarm membership structure.
 */
void bbzswarm_register();

/**
 * @brief Adds info on the fact that a robot joined a swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID.
 * @param[in] swarm The swarm ID.
 */
void bbzswarm_addmember(bbzrobot_id_t robot,
                        bbzswarm_id_t swarm);

/**
 * @brief Adds info on the fact that a robot left a swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID.
 * @param[in] swarm The swarm ID.
 */
void bbzswarm_rmmember(bbzrobot_id_t robot,
                       bbzswarm_id_t swarm);

/**
 * @brief Refreshes the membership information for a robot.
 * @param[in] robot The robot ID.
 * @param[in] swarmlist Bitfield where the i-th bit represents whether
 * the robot is a member of the i-th swarm.
 * Thus, bit 0 -> swarm 0, bit 1 -> swarm 1, ...
 */
void bbzswarm_refresh(bbzrobot_id_t robot,
                      bbzswarmlist_t swarmlist);

/**
 * @brief Determines whether a robot is a member of the given swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID.
 * @param[in] swarm The swarm ID.
 * @return Nonzero if a robot is a member of the given swarm, 0 otherwise.
 */
uint8_t bbzswarm_isrobotin(bbzrobot_id_t robot,
                           bbzswarm_id_t swarm);

/**
 * @brief Updates the information in the swarm membership structure.
 */
void bbzswarm_update();

// ======================================
// =        BUZZ SWARM CLOSURES         =
// ======================================

// ------------------------------
// - 'swarm' structure closures -
// ------------------------------

/**
 * @brief Buzz C closure which creates a new swarm object.
 * @details This closure expects one parameter: the swarm's ID.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 */
void bbzswarm_create();

/**
 * @brief Buzz C closure which creates
 * @details This closure expects one parameter: the swarm's ID.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 */
void bbzswarm_intersection();

/**
 * @brief Buzz C closure which
 * @details This closure expects three parameters: the ID of the swarm to
 * create, and the two swarms to intersect.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 */
void bbzswarm_union();

/**
 * @brief Buzz C closure which
 * @details This closure expects one parameter: the swarm's ID.
 * @note The swarm's ID must be between 0 and 7, otherwise BBZVM_ERROR_SWARM is set.
 */
void bbzswarm_difference();

/**
 * @brief Buzz C closure which returns the swarmstack top swarm id or the parent's.
 */
void bbzswarm_id();


// ------------------------------
// -     subswarm closures      -
// ------------------------------

/**
 * @brief Buzz C closure which creates a new swarm object as the complementary of another.
 */
void bbzswarm_others();

/**
 * @brief Buzz C closure which makes the current robot join a swarm.
 */
void bbzswarm_join();

/**
 * @brief Buzz C closure which makes the current robot leave a swarm.
 */
void bbzswarm_leave();

/**
 * @brief Buzz C closure which checks whether the robot is within a swarm.
 */
void bbzswarm_in();

/**
 * @brief Buzz C closure which conditionally adds a robot to a swarm.
 */
void bbzswarm_select();

/**
 * @brief Buzz C closure which executes a closure if the robot belongs to a swarm.
 */
void bbzswarm_exec();

#else // !BBZ_DISABLE_SWARMS
#define bbzswarm_register()
#define bbzswarm_addmember(...)
#define bbzswarm_rmmember(...)
#define bbzswarm_refresh(...)
#define bbzswarm_isrobotin(...)
#define bbzswarm_update()
void bbzswarm_dummy();
void bbzswarm_dummyret();
#define bbzswarm_create()       bbzswarm_dummyret()
#define bbzswarm_intersection() bbzswarm_dummyret()
#define bbzswarm_union()        bbzswarm_dummyret()
#define bbzswarm_difference()   bbzswarm_dummyret()
#define bbzswarm_id()           bbzswarm_dummyret()
#define bbzswarm_others()       bbzswarm_dummyret()
#define bbzswarm_join()         bbzswarm_dummy()
#define bbzswarm_leave()        bbzswarm_dummy()
#define bbzswarm_in()           bbzswarm_dummyret()
#define bbzswarm_select()       bbzswarm_dummy()
#define bbzswarm_exec()         bbzswarm_dummy()
#endif // !BBZ_DISABLE_SWARMS

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h"

#endif // !BBZSWARM_H