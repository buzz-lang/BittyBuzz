/**
 * @file bbzswarm.h
 * @brief Definition of swarms, which are collections of robots.
 * @details <h2>Explanation of the implementation:</h2>
 *
 * The swarm membership information for a robot (the "swarmlist") is
 * contained in a bitfield. See #bbzswarmlist_t for details.
 *
 * <h3>With swarmlist broadcasts:</h3>
 *
 * This configuration is required by the <code>neighbors.kin</code> and
 * <code>neighbors.nonkin</code> closures. However, these closures are
 * currently not implemented, thus <b>this configuration is currently not
 * supported</b> (but has been partly implemented). However, for future work
 * and for reference, here are some possible designs that were thought
 * of until it was realized that broadcasting swarmlists is not necessary:
 *
 * <h4>C implementation (BBZ_XTREME_MEMORY?)</h4>
 *
 * The swarmlists data would be stored in a C array. Each element would
 * contain the robot's ID (2B) and its swarmlist (1B), totalling for
 * 3B per entry.
 *
 * This had the advantage of taking very little RAM, but the number
 * of entries has a hard limit.
 *
 * <h4>Dynamic implementation (NON-BBZ_XTREME_MEMORY?)</h4>
 *
 * The swarmlists data would be stored directly inside the <code>swarm</code>
 * table, or in a subtable of it, like in the non-xtreme implementation
 * of the <code>neighbors</code> structure. The key of each entry
 * would be the robot's ID and the data would be the swarmlist of
 * that robot. This requires 10B in the heap for each entry:
 *
 * <table>
 * <tr><th>Object-side of the heap</th>       <td>robot ID object (3B)</td>   <td>swarmlist obj (3B)</td></tr>
 * <tr><th>Table segment-side of the heap</th><td>robot ID reference (2B)</td><td>swarmlist reference (2B)</td></tr>
 * </table>
 *
 * Normally, the table segments are supposed to contain a reference
 * (#bbzheap_idx_t) to the key and the value, as the above table shows.
 * However, assuming no user will try to access the fields, we could
 * "cheat" this down to 4B:
 *
 * <table>
 * <tr><th>Table segment-side of the heap</th><td>robot ID (2B)</td><td>swarmlist (2B)</td></tr>
 * </table>
 *
 * Beware that only the 15 least least significant bits of the
 * robot's ID as well as the swarmlist could be used at that point.
 *
 * This method has the advantage of being dynamic, meaning the data table
 * will become larger when we need more entries and might shrink when
 * we release entries. Also, we could be able to increase the swarmlist to
 * 2B at this point without any extra heap usage, since heap-allocated
 * objects always have 2B of payload data anyway. However, it takes more
 * RAM space compared to the C implementation, and the last cheat we
 * propose kinda breaks the engineering.
 *
 * <h4>Note</h4>
 *
 * The 'regular' Buzz also has an 'age' field to tell since how long
 * we haven't received any update for the swarmlist of a neighbor robot.
 * It is used to remove entries that we haven't had an update for for a
 * while. It would be possible to use this inside BittyBuzz too,
 * however we can work without it. This could be done by synchronizing
 * the <code>neighbors</code> structure with the <code>swarm</code>
 * structure in such a way that removing the swarmlist entry for
 * <code>swarm</code> is done at the same time as we remove the neighbor
 * for <code>neighbors</code>. The downside at that point is that we would
 * always need the 'neighbors' structure to be enabled when we use swarm
 * broadcasts.
 *
 *
 * <h3>Without swarmlist broadcasts</h3>
 *
 * Since we don't share swarmlists, we only have our own swarmlist,
 * which is a 1B value. It is available under
 * <code>vm->swarm.my_swarmlist</code>, however users are expected not to
 * use this value, but use bbzswarm_isrobotin() instead.
 */

#ifndef BBZSWARM_H
#define BBZSWARM_H

#include "bbzinclude.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Data type for the robot membership data structure.
 */
typedef struct PACKED bbzswarm_t {
#ifndef BBZ_DISABLE_SWARMS
    bbzheap_idx_t hpos;          /**< @brief Heap's position of the 'swarm' table. */
    bbzheap_idx_t swarmstack;    /**< @brief The stack of swarm IDs that we push to/pop from when we call/return from the 'exec' function. */
#ifdef BBZ_DISABLE_SWARMLIST_BROADCASTS
    bbzswarmlist_t my_swarmlist; /**< @brief Current robot's swarmlist */
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS
#endif // !BBZ_DISABLE_SWARMS
} bbzswarm_t;

#ifndef BBZ_DISABLE_SWARMS

/**
 * @brief Registers and constructs the VM's swarm membership structure.
 */
void bbzswarm_register();

/**
 * @brief Adds info on the fact that a robot joined a swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise #BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID. In non-broadcasting configuration,
 * this value must be equal to the current robot's ID (<code>vm->robot</code>)
 * otherwise #BBZVM_ERROR_OUTOFRANGE is set.
 * @param[in] swarm The swarm ID.
 * @return The swarm list of the robot we added to the swarm.
 */
bbzswarmlist_t bbzswarm_addmember(bbzrobot_id_t robot,
                                  bbzswarm_id_t swarm);

/**
 * @brief Adds info on the fact that a robot left a swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise #BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID. In non-broadcasting configuration,
 * this value must be equal to the current robot's ID (<code>vm->robot</code>)
 * otherwise #BBZVM_ERROR_OUTOFRANGE is set.
 * @param[in] swarm The swarm ID.
 * @return The swarm list of the robot we removed from the swarm.
 */
bbzswarmlist_t bbzswarm_rmmember(bbzrobot_id_t robot,
                                 bbzswarm_id_t swarm);

/**
 * @brief Refreshes the membership information for a robot.
 * @param[in] robot The robot ID. In non-broadcasting configuration,
 * this value must be equal to the current robot's ID (<code>vm->robot</code>)
 * otherwise #BBZVM_ERROR_OUTOFRANGE is set.
 * @param[in] swarmlist Bitfield representing the swarms the robot is a
 * member of.
 */
void bbzswarm_refresh(bbzrobot_id_t robot,
                      bbzswarmlist_t swarmlist);

/**
 * @brief Determines whether a robot is a member of the given swarm.
 * @note The swarm's ID must be between 0 and 7, otherwise #BBZVM_ERROR_SWARM is set.
 * @param[in] robot The robot ID. In non-broadcasting configuration,
 * this value must be equal to the current robot's ID (<code>vm->robot</code>)
 * otherwise #BBZVM_ERROR_OUTOFRANGE is set.
 * #BBZVM_ERROR_OUTOFRANGE is set.
 * @param[in] swarm The swarm ID.
 * @return Nonzero if a robot is a member of the given swarm, 0 otherwise.
 */
uint8_t bbzswarm_isrobotin(bbzrobot_id_t robot,
                           bbzswarm_id_t swarm);

#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
/**
 * @brief Completly forgets a robot's swarmlist.
 */
void bbzswarm_rmentry(bbzrobot_id_t robot);
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

// ======================================
// =        BUZZ SWARM CLOSURES         =
// ======================================

// ------------------------------
// - 'swarm' structure closures -
// ------------------------------

/**
 * @brief Buzz C closure which creates a new swarm object.
 * @details This closure expects one parameter: the swarm's ID.
 * @note The swarm's ID must be between 0 and 7, otherwise
 * #BBZVM_ERROR_SWARM is set.
 */
void bbzswarm_create();

#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
/**
 * @brief Buzz C closure which creates a subswarm structure as the
 * intersection of two other subswarm structures.
 * @details This closure expects one parameter: the swarm's ID.
 * @note
 * <ul>
 * <li>The swarm's ID must be between 0 and 7, otherwise
 * #BBZVM_ERROR_SWARM is set.</li>
 * <li>Due to memory shortages, we disable this feature unless
 * swarmlist broadcasts are enabled.
 * With swarmlists broadcasts disabled, this feature can be
 * reproduced with:</li>
 * @code
 * s0 = swarm.create(0)
 *
 * # ...
 *
 * s1 = swarm.create(1)
 *
 * # ...
 *
 * s2 = swarm.create(2)
 * if (s0.in() and s1.in()) {
 *     s2.join()
 * }
 * # We now have s2 = swarm.intersection(2, s0, s1)
 * @endcode
 * </ul>
 */
void bbzswarm_intersection();

/**
 * @brief Buzz C closure which creates a subswarm structure as the
 * union of two other subswarm structures.
 * @details This closure expects three parameters: the ID of the swarm to
 * create, and the two swarms to unionize.
 * @note
 * <ul>
 * <li>The swarm's ID must be between 0 and 7, otherwise
 * #BBZVM_ERROR_SWARM is set.</li>
 * <li>Due to memory shortages, we disable this feature unless
 * swarmlist broadcasts are enabled.
 * With swarmlists broadcasts disabled, this feature can be
 * reproduced with:</li>
 * @code
 * s0 = swarm.create(0)
 *
 * # ...
 *
 * s1 = swarm.create(1)
 *
 * # ...
 *
 * s2 = swarm.create(2)
 * if (s0.in() or s1.in()) {
 *     s2.join()
 * }
 * # We now have s2 = s1.union(2, s0, s1)
 * @endcode
 * </ul>
 */
void bbzswarm_union();

/**
 * @brief Buzz C closure which creates a subswarm structure as the
 * set difference of two other subswarm structures.
 * @details This closure expects three parameters: the ID of the swarm to
 * create, the swarm that the robots must members of and the swarm
 * that the robot, must not be a member of.
 * @note
 * <ul>
 * <li>The swarm's ID must be between 0 and 7, otherwise
 * #BBZVM_ERROR_SWARM is set.</li>
 * <li>Due to memory shortages, we disable this feature unless
 * swarmlist broadcasts are enabled.
 * With swarmlists broadcasts disabled, this feature can be
 * reproduced with:</li>
 * @code
 * s0 = swarm.create(0)
 *
 * # ...
 *
 * s1 = swarm.create(1)
 *
 * # ...
 *
 * s2 = swarm.create(2)
 * if (s0.in() and (not s1.in())) {
 *     s2.join()
 * }
 * # We now have s2 = s1.difference(2, s0, s1)
 * @endcode
 * </ul>
 */
void bbzswarm_difference();
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

/**
 * @brief Buzz C closure which returns the swarmstack top swarm id or
 * the parent's.
 */
void bbzswarm_id();


// ------------------------------
// -     subswarm closures      -
// ------------------------------

#ifndef BBZ_DISABLE_SWARMLIST_BROADCASTS
/**
 * @brief Buzz C closure which creates a subswarm structure as the
 * complement of another subswarm structure.
 * @note Due to memory shortages, we disable this feature unless
 * swarmlist broadcasts are enabled.
 * With swarmlists broadcasts disabled, this feature can be
 * reproduced with:
 * @code
 * s0 = swarm.create(0)
 *
 * # ...
 *
 * s1 = swarm.create(1)
 * if (s0.in()) {
 *     s1.join()
 * }
 * # We now have s0 = s1.others()
 * @endcode
 */
void bbzswarm_others();
#endif // !BBZ_DISABLE_SWARMLIST_BROADCASTS

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
 * @brief Buzz C closure which executes a closure if the robot belongs to
 * a swarm.
 */
void bbzswarm_exec();

#else // !BBZ_DISABLE_SWARMS
#define bbzswarm_register(...)
#define bbzswarm_addmember(...)
#define bbzswarm_rmmember(...)
#define bbzswarm_refresh(...)
#define bbzswarm_isrobotin(...)
#define bbzswarm_rmentry(...)
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