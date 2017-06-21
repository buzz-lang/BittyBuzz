/**
 * @file swarmlist_list_based.h
 * @brief Test of a "List-based" swarm-list strategy.
 */

#ifndef SWARMLIST_LIST_BASED_H
#define SWARMLIST_LIST_BASED_H

#include <bittybuzz/bbzinclude.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// ===============================
// =     CONFIGURABLE VALUES     =
// ===============================

/**
 * The maximum number of robots a robot can remember swarm data about.
 * If memory is not an issue but safety is, this value can be set
 * to the number of robots in the experiment.
 */
#define ROBOT_SWARMLIST_CAP 60

/**
 * The number of loop() calls between the emission of swarm chunks.
 */
#define SWARM_CHUNK_DELAY 5

/**
 * The maximum number of swarm messages to send per swarm chunk.
 * We send less messages if we don't have enough data to send.
 */
#define SWARM_CHUNK_AMOUNT 3

/**
 * The number of swarmlist ticks before a swarm entry
 * is considered inactive.
 * @note <b>Max value: 255</b>
 */
#define SWARMLIST_TICKS_TO_INACTIVE 100

/**
 * Whether to remove old entries of the swarmlist structure.
 */
#define SWARMLIST_REMOVE_OLD_ENTRIES

/**
 * The number of swarmlist ticks after a swarm entry is inactive
 * before it is completely forgotten. This is only considered if
 * SWARMLIST_REMOVE_OLD_ENTRIES is defined.
 * @note <b>Max value: 255</b>
 * @see SWARMLIST_REMOVE_OLD_ENTRIES
 */
#define SWARMLIST_TICKS_TO_REMOVAL 100

/**
 * The number of loop()s before each swarmlist tick.
 * 1 means "tick every loop". This value has the effect of a
 * multiplier to SWARMLIST_TICKS_TO_INACTIVE and SWARMLIST_TICKS_TO_REMOVAL.
 * @see SWARMLIST_TICKS_TO_INACTIVE
 * @see SWARMLIST_TICKS_TO_REMOVAL
 */
#define LOOPS_PER_TICK 1

/**
 * The maximum number of ticks a Lamport clock sould be above an old
 * Lamport clock so that the new Lamport clock is considered as 'new'.
 */
#define LAMPORT_THRESHOLD 50


// ===============================
// =      SWARMLIST ENTRIES      =
// ===============================

/**
 * The type of a message.
 */
typedef enum {
    SWARM ///< Swarm message.
} msg_type_t;

/**
 * Type for an 8-bit Lamport clock.
 */
typedef uint8_t lamport8_t;


// ===============================
// =      SWARMLIST ENTRIES      =
// ===============================

/**
 * Entry of the swarm list.
 */
typedef struct PACKED {
    uint8_t robot;            ///< Robot ID this entry is for.
    uint8_t swarm_mask;       ///< Swarms this robot is a member of (1 bit for each swarm).
    lamport8_t lamport;       ///< Time at which the entry was last updated (Lamport clock).
    uint8_t time_to_inactive; ///< Number of swarmlist ticks until we consider this robot to be inactive.
#ifdef SWARMLIST_REMOVE_OLD_ENTRIES
    uint8_t time_to_removal;  ///< Number swarmlist ticks until this robot is forgotten. Only used when the robot is inactive.
#endif // SWARMLIST_REMOVE_OLD_ENTRIES
} swarmlist_entry_t;

/**
 * Determines whether a swarmlist entry is active.
 */
ALWAYS_INLINE
uint8_t swarmlist_entry_isactive(const swarmlist_entry_t* e) {
    return e->time_to_inactive > 0 || e->robot == kilo_uid;
}

#ifdef SWARMLIST_REMOVE_OLD_ENTRIES
/**
 * Determines whether a swarmlist entry should be removed.
 */
ALWAYS_INLINE
uint8_t swarmlist_entry_shouldremove(const swarmlist_entry_t* e) {
    return !swarmlist_entry_isactive(e) && e->time_to_removal == 0;
}
#endif // SWARMLIST_REMOVE_OLD_ENTRIES


// ===============================
// =     SWARMLIST STRUCTURE     =
// ===============================

/**
 * The data that we know about other robots.
 * @note We assume there is only one instance of this structure.
 */
typedef struct PACKED {
    swarmlist_entry_t data[ROBOT_SWARMLIST_CAP]; ///< The data of the swarm list.
    uint8_t size;  ///< Number of entries.
    uint8_t next_to_send; ///< The index of the next entry to send via a swarm chunk.
} swarmlist_t;

/**
 * Swarmlist single instance.
 */
extern swarmlist_t swarmlist;

/**
 * Constructs the swarmlist.
 */
void swarmlist_construct();

/**
 * Updates/creates an entry in the swarm list.
 */
void swarmlist_update(bbzrobot_id_t robot,
                      uint8_t swarm_mask,
                      lamport8_t lamport);

/**
 * Passes on to the next swarm entry.
 */
ALWAYS_INLINE
void swarmlist_next() {
    ++swarmlist.next_to_send;
    if (swarmlist.next_to_send >= swarmlist.size)
        swarmlist.next_to_send = 0;
}

/**
 * Determines the size of the swarmlist.
 */
ALWAYS_INLINE
uint8_t swarmlist_size() { return swarmlist.size; }

/**
 * Determines the number of active entries in the swarmlist.
 */
uint8_t swarmlist_count();

/**
 * Removes 1 from all timers. If a timer is at 0, remove the entry.
 */
void swarmlist_tick();

/**
 * Broadcasts a swarm data chunk.
 * @details A "swarm data chunk" is a set of swarm entry messages.
 * @see SWARM_CHUNK_DELAY
 * @see SWARM_CHUNK_AMOUNT
 */
void send_next_swarm_chunk();


// ===============================
// =       OTHER FUNCTIONS       =
// ===============================

/**
 * Processes a swarm message.
 */
void process_msg_rx_swarm(message_t* msg_rx);

/**
 * Decides whether a lamport is newer than another one.
 * @note If the two Lamport clocks are equal, the Lamport is not
 * considered as new.
 */
uint8_t lamport_isnewer(lamport8_t lamport, lamport8_t old_lamport);

/**
 * Sugar for setting the LED color.
 */
#define LED(r,g,b) set_color(RGB(r,g,b))


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !SWARMLIST_LIST_BASED_H