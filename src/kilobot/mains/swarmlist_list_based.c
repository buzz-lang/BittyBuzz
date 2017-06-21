/**
 * @file swarmlist_list_based.c
 * @brief Test of a "List-based" swarm-list strategy.
 */

#include <kilobot/lib/bbzkilobot.h>
#include <kilobot/lib/bbzmessage_crc.h>

#include "swarmlist_list_based.h"

#include <bittybuzz/bbzTEMP.h>

// ===============================
// =  GENERAL GLOBAL VARIABLES   =
// ===============================

swarmlist_t swarmlist;
uint8_t local_swarm_mask = 0x01;
lamport8_t local_lamport = 0;

// ===============================
// =         TX MESSAGES         =
// ===============================

volatile uint8_t should_send_tx = 0;
message_t msg_tx;
volatile uint8_t msg_tx_sent = 0;

#define msg_tx_busy_wait() while(!msg_tx_sent) { }

message_t* which_msg_tx() {
    if (should_send_tx) {
        return &msg_tx;
    }
    return 0;
}

void msg_tx_success() {
    should_send_tx = 0;
    msg_tx_sent = 1;
}

#define NUM_ENTRIES_PER_SWARM_MSG 3
#define SWARM_ENTRY_SZ 3
void send_next_swarm_chunk() {
    if (swarmlist.size != 0) {
        // Send several swarm messages
        const uint8_t NUM_MSGS = (swarmlist.size / NUM_ENTRIES_PER_SWARM_MSG + 1 >= SWARM_CHUNK_AMOUNT) ?
                                 (SWARM_CHUNK_AMOUNT) :
                                 (swarmlist.size / NUM_ENTRIES_PER_SWARM_MSG + 1);

        for (uint8_t i = 0; i < NUM_MSGS; ++i) {
            // Send a swarm message
            msg_tx.type = SWARM;
            for (uint8_t j = 0; j < NUM_ENTRIES_PER_SWARM_MSG; ++j) {
                // Increment our own Lamport clock so that others are aware
                // that we still exist.
                swarmlist_entry_t* entry = &swarmlist.data[swarmlist.next_to_send];

                // Don't send the info of inactive robots
                uint8_t num_entries_skipped = 0;
                while (!swarmlist_entry_isactive(entry) &&
                        num_entries_skipped < swarmlist.size) {
                    swarmlist_next();
                    entry = &swarmlist.data[swarmlist.next_to_send];

                    // Break infinite loops
                    ++num_entries_skipped;
                }

                // Increment Lamport clock when sending our own info.
                if (entry->robot == kilo_uid) {
                    ++local_lamport;
                    entry->lamport = local_lamport;
                }

                // Append the next entry's data
                msg_tx.data[SWARM_ENTRY_SZ*j]   = entry->robot;
                msg_tx.data[SWARM_ENTRY_SZ*j+1] = entry->swarm_mask;
                msg_tx.data[SWARM_ENTRY_SZ*j+2] = entry->lamport;

                // Go to next robot (if we only have one or two robots, we'll
                // send the same robot info several times, but that's OK).
                swarmlist_next();
            }
            msg_tx.crc = bbzmessage_crc(&msg_tx);
            should_send_tx = 1;
            msg_tx_busy_wait();
        }
    }
}

// ===============================
// =         RX MESSAGES         =
// ===============================

void process_msg_rx(message_t* msg_rx, distance_measurement_t* d) {
    LED(1,1,1);
    switch(msg_rx->type) {
        case SWARM: {
            process_msg_rx_swarm(msg_rx); break;
        }
        default: LED(3,0,3); delay(65535); break;
    }
}

void process_msg_rx_swarm(message_t* msg_rx) {
    for (uint8_t j = 0; j < NUM_ENTRIES_PER_SWARM_MSG; ++j) {
        bbzrobot_id_t robot = msg_rx->data[SWARM_ENTRY_SZ*j];
        // We have the most updated info about ourself ;
        // don't update our info.
        if (robot != kilo_uid) {
            uint8_t swarm_mask  = msg_rx->data[SWARM_ENTRY_SZ*j+1];
            lamport8_t lamport  = msg_rx->data[SWARM_ENTRY_SZ*j+2];
            swarmlist_update(robot, swarm_mask, lamport);
        }
    }
}

// ===============================
// =       OTHER FUNCTIONS       =
// ===============================

void swarmlist_construct() {
    swarmlist.size = 0;
    swarmlist.next_to_send = 0;
}

void swarmlist_update(bbzrobot_id_t robot,
                      uint8_t swarm_mask,
                      lamport8_t lamport) {
    uint8_t sz = swarmlist.size;
    ++swarmlist.size;

    // Does the entry already exist?
    uint8_t pos;
    uint8_t should_update = 1;
    for (pos = 0; pos < sz; ++pos) {
        if (swarmlist.data[pos].robot == robot) {
            // Yes ; it is not a new entry after all.
            --swarmlist.size;
            
            lamport8_t old_lamport = swarmlist.data[pos].lamport;
            if (swarmlist_entry_isactive(&swarmlist.data[pos])) {
                should_update = lamport_isnewer(lamport, old_lamport);
            }
            else {
                should_update = (lamport != old_lamport);
            }

            break;
        }
    }

    if (should_update) {
        swarmlist_entry_t* entry = &swarmlist.data[pos];
        entry->robot      = robot;
        entry->swarm_mask = swarm_mask;
        entry->lamport    = lamport;
        entry->time_to_inactive = SWARMLIST_TICKS_TO_INACTIVE;
    }
}

void swarmlist_tick() {
    for (uint8_t i = 0; i < swarmlist.size; ++i) {
       // Deal with entries in inactive mode
        if (swarmlist_entry_isactive(&swarmlist.data[i])) {
            --swarmlist.data[i].time_to_inactive;

            // if (!swarmlist_entry_isactive(&swarmlist.data[i])) {
            //     swarmlist.data[i].time_to_removal = SWARMLIST_TICKS_TO_REMOVAL;
            // }
        }
        // else {
        //     // Remove old inactive entries
        //     if (swarmlist.data[i].time_to_removal > 0) {
        //         --swarmlist.data[i].time_to_removal;
        //     }
        //     else {
        //         cli(); // No interrups ; don't accept messages during removal.
        //         swarmlist.data[i] = swarmlist.data[swarmlist.size-1];
        //         --swarmlist.size;
        //         sei();
        //     }
        // }
    }
}

uint8_t swarmlist_count() {
    uint8_t count = 0;

    for (uint8_t i = 0; i < swarmlist.size; ++i) {
        if (swarmlist_entry_isactive(&swarmlist.data[i])) {
            ++count;
        }
    }

    return count;
}

uint8_t lamport_isnewer(lamport8_t lamport, lamport8_t old_lamport) {
    // This function uses a circular Lamport model (0 == 255 + 1).
    // A Lamport clock is 'newer' than an old Lamport clock if its value
    // is less than 'LAMPORT_THRESHOLD' ticks ahead of the old clock.

    uint8_t lamport_overflow = (UINT8_MAX - old_lamport < LAMPORT_THRESHOLD);
    if (lamport_overflow) {
        return lamport > old_lamport ||
               lamport <= (uint8_t)(old_lamport + LAMPORT_THRESHOLD);
    }
    else {
        return lamport >  old_lamport &&
               lamport <= old_lamport + LAMPORT_THRESHOLD;
    }
}

// ===============================
// =  SETUP AND LOOP FUNCTIONS   =
// ===============================

void setup() {
    rand_seed(rand_hard());
    swarmlist_construct();
    swarmlist_update(kilo_uid, local_swarm_mask, local_lamport);
}

uint8_t loops_till_tick = 1;
uint16_t loops_till_next_chunk = 1;
void loop() {

    if (loops_till_next_chunk > 0) {
        --loops_till_next_chunk;
    }

    if (loops_till_next_chunk == 0 && !should_send_tx) {
        loops_till_next_chunk = (rand_soft() >> 7) + SWARM_CHUNK_DELAY;
        send_next_swarm_chunk();
    }

    switch(swarmlist_count() % 8) {
        case 0: {
            LED(0,0,0);
            break;
        }
        case 1: {
            LED(1,0,0);
            break;
        }
        case 2: {
            LED(2,1,0);
            break;
        }
        case 3: {
            LED(1,1,0);
            break;
        }
        case 4: {
            LED(0,1,0);
            break;
        }
        case 5: {
            LED(0,2,1);
            break;
        }
        case 6: {
            LED(0,0,1);
            break;
        }
        case 7: {
            LED(1,0,1);
            break;
        }
        default: ;
    }

    --loops_till_tick;
    if (loops_till_tick == 0) {
        loops_till_tick = LOOPS_PER_TICK;
        swarmlist_tick();
    }

    // Let's assume we do something that takes some time.
    delay(50);
}

// ===============================
// =        MAIN FUNCTION        =
// ===============================

int main() {
    bbzkilo_init();

    kilo_message_tx = which_msg_tx;
    kilo_message_tx_success = msg_tx_success;
    kilo_message_rx = process_msg_rx;

    bbzkilo_start(setup, loop);
}