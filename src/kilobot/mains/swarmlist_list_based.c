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

#define NUM_ENTRIES_PER_MSG 3
void send_next_swarm_chunk() {
    if (swarmlist.size != 0) {
        // Send several swarm messages
        const uint8_t NUM_MSGS = (swarmlist.size / NUM_ENTRIES_PER_MSG + 1 >= SWARM_CHUNK_AMOUNT) ?
                                 (NUM_ENTRIES_PER_MSG * SWARM_CHUNK_AMOUNT) :
                                 (swarmlist.size / NUM_ENTRIES_PER_MSG + 1);

        for (uint8_t i = 0; i < NUM_MSGS; ++i) {
            // Send a swarm message
            msg_tx.type = SWARM;
            for (uint8_t j = 0; j < NUM_ENTRIES_PER_MSG; ++j) {
                // Append the next entry's data
                msg_tx.data[3*j]   = swarmlist.data[swarmlist.next_to_send].robot;
                msg_tx.data[3*j+1] = swarmlist.data[swarmlist.next_to_send].swarm_mask;
                msg_tx.data[3*j+2] = swarmlist.data[swarmlist.next_to_send].lamport;

                // Go to next robot (if we only have one robot, we'll
                // send the same robot info several times, but that's OK).
                ++swarmlist.next_to_send;
                if (swarmlist.next_to_send >= swarmlist.size) {
                    swarmlist.next_to_send = 0;
                }
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
    switch(msg_rx->type) {
        case SWARM: {
            process_msg_rx_swarm(msg_rx); break;
        }
        default: LED(3,0,3); delay(65535); break;
    }
}

void process_msg_rx_swarm(message_t* msg_rx) {
    LED(0,0,0); delay(10);
    for (uint8_t j = 0; j < NUM_ENTRIES_PER_MSG; ++j) {
        bbzrobot_id_t robot = msg_rx->data[3*j];
        uint8_t swarm_mask  = msg_rx->data[3*j+1];
        lamport8_t lamport  = msg_rx->data[3*j+2];
        swarmlist_update(robot, swarm_mask, lamport);
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
    for (pos = 0; pos < sz; ++pos) {
        if (swarmlist.data[pos].robot == robot) {
            // Yes ; it is not a new entry after all.
            --swarmlist.size;
            break;
        }
    }

    // if (lamport_isnewer(lamport, swarmlist.data[pos].lamport)) {
        swarmlist.data[pos].robot      = robot;
        swarmlist.data[pos].swarm_mask = swarm_mask;
        swarmlist.data[pos].lamport    = lamport;
    // }
}

uint8_t swarmlist_size() {
    return swarmlist.size;
}

uint8_t lamport_isnewer(lamport8_t lamport, lamport8_t old_lamport) {
    // This function uses a circular Lamport model (0 comes after 255).
    // A Lamport clock is 'newer' than an old Lamport clock if its value
    // is less than 'LAMPORT_THRESHOLD' ticks ahead of the old clock.

    // Lamport overflow?
    if (UINT8_MAX - old_lamport < LAMPORT_THRESHOLD) {
        // No
        return lamport >  old_lamport &&
               lamport <= old_lamport + LAMPORT_THRESHOLD;
    }
    else {
        // Yes
        return lamport >  old_lamport ||
               lamport <= old_lamport - UINT8_MAX + LAMPORT_THRESHOLD;
               // I guess this would work too:
               // lamport <= (uint8_t)(old_lamport + LAMPORT_THRESHOLD);
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

uint16_t loops_till_next_chunk = 0;
void loop() {

    if (loops_till_next_chunk > 0) {
        --loops_till_next_chunk;
    }

    if (loops_till_next_chunk == 0 && !should_send_tx) {
        LED(3,3,3); delay(10);
        loops_till_next_chunk = (rand_soft() >> 7) + SWARM_CHUNK_DELAY;
        send_next_swarm_chunk();
    }

    switch(swarmlist_size()) {
        case 0: {
            LED(0,0,0);
            break;
        }
        case 1: {
            LED(3,0,0);
            break;
        }
        case 2: {
            LED(2,1,0);
            break;
        }
        case 3: {
            LED(3,3,0);
            break;
        }
        case 4: {
            LED(0,3,0);
            break;
        }
        case 5: {
            LED(0,2,1);
            break;
        }
        case 6: {
            LED(0,0,3);
            break;
        }
        case 7: {
            LED(3,0,3);
            break;
        }
        default: LED(3,3,3); break;
    }

    // Let's assume we do stuff that take some time.
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