/**
 * @file bbzswarm.h
 * @brief Definition of swarms, which are collections of robots.
 */

#ifndef BBZSWARM_H
#define BBZSWARM_H

#include "bbzinclude.h"
#include "bbztable.h"
#include "bbzvm.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Data type for the robot membership data structure.
 */
typedef bbztable_t bbzswarm_members_t;

/**
 * @brief Creates a new swarm membership structure.
 * @return A new swarm membership structure.
 */
bbzswarm_members_t bbzswarm_members_new();

/**
 * @brief Destroys a swarm membership structure.
 * @param[out] m The swarm membership structure.
 */
void bbzswarm_members_destroy(bbzswarm_members_t* m);

/**
 * @brief Adds info on the fact that a robot joined a swarm.
 * @param[in] m The swarm membership structure.
 * @param[in] robot The robot id.
 * @param[in] swarm The swarm id.
 */
void bbzswarm_members_join(bbzswarm_members_t m,
                           uint16_t robot,
                           uint16_t swarm);

/**
 * @brief Adds info on the fact that a robot left a swarm.
 * @param[in] m The swarm membership structure.
 * @param[in] robot The robot id.
 * @param[in] swarm The swarm id.
 */
void bbzswarm_members_leave(bbzswarm_members_t m,
                                   uint16_t robot,
                                   uint16_t swarm);

/**
 * @brief Refreshes the membership information for a robot.
 * The ownership of the passed swarm id list is assumed by
 * this structure. Do not free it.
 * @param[in] m The swarm membership structure.
 * @param[in] robot The robot id.
 */
void bbzswarm_members_refresh(bbzswarm_members_t m,
                              uint16_t robot);

/**
 * @brief Returns 1 if a robot is a member of the given swarm, 0 otherwise.
 * @param[in] m The swarm membership structure.
 * @param[in] robot The robot id.
 * @param[in] swarm The swarm id.
 * @return 1 if a robot is a member of the given swarm, 0 otherwise.
 */
void bbzswarm_members_isrobotin(bbzswarm_members_t m,
                                       uint16_t robot,
                                       uint16_t swarm);

/**
 * @brief Updates the information in the swarm membership structure.
 * @param[in] m The swarm membership structure.
 */
void bbzswarm_members_update(bbzswarm_members_t m);

/**
 * @brief Registers the swarm data into the virtual machine.
 */
void bbzswarm_register();

/**
 * @brief Buzz C closure to create a new swarm object.
 */
void bbzswarm_create();

/**
 * @brief Buzz C closure to return the current swarm id or the parent's.
 */
void bbzswarm_id();

/**
 * @brief Buzz C closure to create a new swarm object as a complementary of another.
 */
void bbzswarm_others();

/**
 * @brief Buzz C closure to join a swarm.
 */
void bbzswarm_join();

/**
 * @brief Buzz C closure to leave a swarm.
 */
void bbzswarm_leave();

/**
 * @brief Buzz C closure to check whether the robot is within a swarm.
 */
void bbzswarm_in();

/**
 * @brief Buzz C closure to execute conditionally add a robot to a swarm.
 */
void bbzswarm_select();

/**
 * @brief Buzz C closure to execute a closure if the robot belong to a swarm.
 */
void bbzswarm_exec();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZSWARM_H