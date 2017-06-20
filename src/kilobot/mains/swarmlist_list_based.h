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

/**
 * The number of robots in the experiment.
 */
#define ROBOT_COUNT 9

/**
 * The number of loop() calls between the emission of swarm chunks.
 */
#define SWARM_CHUNK_DELAY 5

/**
 * The number of swarm messages to send per swarm chunk.
 */
#define SWARM_CHUNK_AMOUNT 1

/**
 * The maximum number of ticks a Lamport clock sould be above an old
 * Lamport clock so that the new Lamport clock is considered as 'new'.
 */
#define LAMPORT_THRESHOLD 50

#define LED(r,g,b) set_color(RGB(r,g,b))

/**
 * The type of a message.
 */
typedef enum {
    SWARM ///< Swarm message.
} msg_type_t;

/**
 * Processes a swarm message.
 */
void process_msg_rx_swarm(message_t* msg_rx);

/**
 * Type for an 8-bit Lamport clock.
 */
typedef uint8_t lamport8_t;

/**
 * Entry of the swarm list.
 */
typedef struct PACKED {
    bbzrobot_id_t robot; ///< Robot ID this entry is for.
    uint8_t swarm_mask;  ///< Swarms this robot is a member of (1 bit for each swarm).
    lamport8_t lamport;   ///< Time at which the entry was last updated (Lamport clock).
} swarmlist_entry_t;

/**
 * The data that we know about other robots.
 * @note We assume there is only one instance of this structure.
 */
typedef struct PACKED {
    swarmlist_entry_t data[ROBOT_COUNT]; ///< The data of the swarm list.
    uint8_t size; ///< Number of entries.
    uint8_t next_to_send; ///< The index of the next entry to send via a swarm chunk.
} swarmlist_t;

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
 * Determines the size of the swarmlist.
 */
uint8_t swarmlist_size();

/**
 * Broadcasts a swarm data chunk.
 * @details A "swarm data chunk" is a set of swarm entry messages.
 * @see SWARM_CHUNK_DELAY
 * @see SWARM_CHUNK_AMOUNT
 */
void send_next_swarm_chunk();

/**
 * Decides whether a lamport is newer than another one.
 * @note If the two Lamport clocks are equal, the Lamport is not
 * considered as new.
 */
uint8_t lamport_isnewer(lamport8_t lamport, lamport8_t old_lamport);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !SWARMLIST_LIST_BASED_H