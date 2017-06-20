/**
 * @file swarm_recursive.c
 * @brief Test of a breadth-first search strategy to know robot
 * count without doing any swarm-list message.
 */

#include <bittybuzz/bbzinclude.h>

#include "swarm_recursive.h"

// ===============================
// =      GLOBAL VARIABLES       =
// ===============================

volatile uint8_t should_send = 0;
volatile uint8_t msg_sent = 0;
message_t msg_tx;

uint8_t got_neigh_broad = 0;
uint8_t got_swarm_num = 0;
uint8_t got_swarm_num_final = 0;
uint8_t got_swarm_num_recur = 0;
uint8_t swarm0_member_count = 0;
uint8_t swarm0_member_count_increment;
bbzrobot_id_t swarm_num_recur_sender;

#define MAX_NEIGHBORS 4
volatile bbzrobot_id_t neighbors[MAX_NEIGHBORS] = {};
volatile uint8_t num_neighbors = 0;
volatile uint8_t curr_neighbor_idx = 0;

#define LED_BLINK(r,g,b) set_color(RGB(r,g,b)); delay(20); set_color(RGB(0,0,0));

// ===============================
// =    FUNCTION DEFINITIONS     =
// ===============================

void setup() {

    if (is_leader()) {
        // swarm0_member_count = is_in_swarm(0) ? 1 : 0;
        // initiate_swarm_msg();
    }
}

void loop() {
    if (!got_neigh_broad) {
        find_neighbors();
        delay(500);
    }
}


uint8_t is_in_swarm(uint8_t swid) {
    switch(swid) {
        case 0: return (kilo_uid % 2);
        default: return 0;
    }
}


message_t* which_msg_tx() {
    if (should_send) {
        return &msg_tx;
    }
    return 0;
}

void msg_tx_success() {
    msg_sent = 1;
    should_send = 0;
}

volatile uint8_t is_msg_tx_sent() {
    volatile uint8_t ret = msg_sent;
    msg_sent = 0;
    return ret;
}

void find_neighbors() {
    send_msg_tx_neigh_broad();
}

void initiate_swarm_msg() {
    send_msg_tx_swarm_num(neighbors[0]);
}

//////////////
// TX MESSAGES
//////////////

#define SENDER      0
#define RECEIVER    1
#define INCREMENT   2

void send_msg_tx(Msg_type type, uint8_t data[], uint8_t sz) {
    msg_tx.type = type;
    for (uint8_t i = 0; i < sz; ++i) {
        msg_tx.data[i] = data[i];
    }
    msg_tx.crc = bbzmessage_crc(&msg_tx);
    should_send = 1;
    while(should_send) { }
    LED_BLINK(3,3,3);
}

void send_msg_tx_neigh_broad() {
    send_msg_tx(MSG_TYPE_NEIGH_BROAD, 0, 0);
}

void send_msg_tx_neigh_broad_res() {
    uint8_t data[] = { kilo_uid };
    send_msg_tx(MSG_TYPE_NEIGH_BROAD_RES, data, 2);
}

void send_msg_tx_swarm_num() {
    uint8_t data[] = { kilo_uid, neighbors[curr_neighbor_idx] };
    send_msg_tx(MSG_TYPE_SWARM_NUM, data, 2);
}

void send_msg_tx_swarm_num_res(bbzrobot_id_t reciever, uint8_t should_increment) {
    uint8_t data[] = { kilo_uid, reciever, should_increment };
    send_msg_tx(MSG_TYPE_SWARM_NUM_RES, data, 3);
}

void send_msg_tx_swarm_num_recur() {
    uint8_t data[] = { kilo_uid, neighbors[curr_neighbor_idx] };
    send_msg_tx(MSG_TYPE_SWARM_NUM_RECUR, data, 2);
}

void send_msg_tx_swarm_num_recur_res() {
    uint8_t data[] = { kilo_uid, swarm_num_recur_sender, swarm0_member_count_increment };
    send_msg_tx(MSG_TYPE_SWARM_NUM_RECUR_RES, data, 3);
}

void send_msg_tx_swarm_num_final() {
    uint8_t data[] = { swarm0_member_count };
    send_msg_tx(MSG_TYPE_SWARM_NUM_FINAL, data, 1);
}

/////////////
// RX MESAGES
/////////////

void process_msg_rx(message_t* msg_rx, distance_measurement_t* d) {
    set_color(RGB(3,3,3));
    switch(msg_rx->type) {
        case MSG_TYPE_NEIGH_BROAD:         process_msg_rx_neigh_broad(msg_rx);         break;
        case MSG_TYPE_NEIGH_BROAD_RES:     process_msg_rx_neigh_broad_res(msg_rx);     break;
        case MSG_TYPE_SWARM_NUM:           process_msg_rx_swarm_num(msg_rx);           break;
        case MSG_TYPE_SWARM_NUM_RES:       process_msg_rx_swarm_num_res(msg_rx);       break;
        case MSG_TYPE_SWARM_NUM_RECUR:     process_msg_rx_swarm_num_recur(msg_rx);     break;
        case MSG_TYPE_SWARM_NUM_RECUR_RES: process_msg_rx_swarm_num_recur_res(msg_rx); break;
        case MSG_TYPE_SWARM_NUM_FINAL:     process_msg_rx_swarm_num_final(msg_rx);     break;
        default: break;
    }
}

void process_msg_rx_neigh_broad(message_t* msg_rx) {
    LED_BLINK(3,0,0);
    got_neigh_broad = 1;
    for (uint8_t i = 0; i < 5; ++i) {
        send_msg_tx_neigh_broad_res();
    }
}

void process_msg_rx_neigh_broad_res(message_t* msg_rx) {
    LED_BLINK(0,3,0);
    for (uint8_t i = 0; i < num_neighbors; ++i) {
        if (msg_rx->data[SENDER] == neighbors[i]) {
            return;
        }
    }
    neighbors[num_neighbors++] = msg_rx->data[SENDER];
}

void process_msg_rx_swarm_num(message_t* msg_rx) {
    LED_BLINK(0,0,3);
    // If the message is for me, process it.
    if (msg_rx->data[RECEIVER] == kilo_uid) {
        uint8_t should_increment = (!got_swarm_num && is_in_swarm(0));
        if (!got_swarm_num) {
            got_swarm_num = 1;
            got_swarm_num_final = 0;
        }
        send_msg_tx_swarm_num_res(msg_rx->data[SENDER], should_increment);
    }
}

void process_msg_rx_swarm_num_res(message_t* msg_rx) {
    LED_BLINK(3,0,3);
    // If the message is for me, process it.
    if (msg_rx->data[RECEIVER] == kilo_uid) {
        uint8_t should_increment = msg_rx->data[INCREMENT];
        if (should_increment) {
            ++swarm0_member_count_increment;
        }

        ++curr_neighbor_idx;
        if (curr_neighbor_idx < num_neighbors) {
            // Send to next neighbor
            send_msg_tx_swarm_num(neighbors[curr_neighbor_idx]);
        }
        else {
            // Begin recursing
            curr_neighbor_idx = 0;
            send_msg_tx_swarm_num_recur();
        }
    }
}

void process_msg_rx_swarm_num_recur(message_t* msg_rx) {
    LED_BLINK(0,3,3);
    // If the message is for me and I haven't done the recursion yet, process it.
    if (msg_rx->data[RECEIVER] == kilo_uid && !got_swarm_num_recur) {
        got_swarm_num_recur = 1;
        swarm_num_recur_sender = msg_rx->data[SENDER];
        initiate_swarm_msg();
    }
}

void process_msg_rx_swarm_num_recur_res(message_t* msg_rx) {
    LED_BLINK(3,3,0);
    // If the message is for me, process it.
    if (msg_rx->data[RECEIVER] == kilo_uid) {
        swarm0_member_count_increment =+ msg_rx->data[INCREMENT];

        ++curr_neighbor_idx;
        if (curr_neighbor_idx < num_neighbors) {
            // Send to next neighbor
            send_msg_tx_swarm_num_recur();
        }
        else {
            // Done. Return result to recurse message sender, or
            // send the final value if we are the leader.
            if (is_leader()) {
                swarm0_member_count += swarm0_member_count_increment;
                send_msg_tx_swarm_num_final();
            }
            else {
                send_msg_tx_swarm_num_recur_res();
            }
        }
    }
}

void process_msg_rx_swarm_num_final(message_t* msg_rx) {
    LED_BLINK(3,3,3);
    if (!got_swarm_num_final) {
        got_swarm_num_final = 1;

        // Reset stuff
        got_swarm_num = 0;
        got_swarm_num_recur = 0;
        swarm0_member_count_increment = 0;
        curr_neighbor_idx = 0;

        // Pass on the message
        send_msg_tx_swarm_num_final();
    }
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

    return 0;
}