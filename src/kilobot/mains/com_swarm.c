#include "com_swarm.h"


volatile uint8_t should_send = 0;
message_t msg_tx;
volatile uint8_t sent_msg = 0;

volatile State state;
volatile uint16_t ids[3];
volatile uint8_t num_ids = 0;

message_t* my_msg_tx() {
    if (should_send) {
        return &msg_tx;
    }
    return 0;
}

void my_tx_msg_success() {
     sent_msg = 1;
}

void my_msg_rx(message_t* msg_rx, distance_measurement_t* d) {
    set_color(RGB(0,3,0));
    delay(500);
    set_color(RGB(0,0,0));
    switch(msg_rx->data[0]) {
        case 0: {
            set_color(RGB(3,0,0));
            delay(300);
            set_color(RGB(0,0,0));
            msg_tx.data[0] = MSG_ANS;
            msg_tx.data[1] = kilo_uid;
            should_send = 1;
            while (!sent_msg) {}
            should_send = 0;
            break;
        }
        case 1: {
            if (state == STATE_NEIGHBOR_QUERY) {
                set_color(RGB(0,3,0));
                delay(300);
                set_color(RGB(0,0,0));
                ids[num_ids++] = msg_rx->data[1];
            }
            break;
        }
        default: break;
    }
}

void find_neighbors() {
    msg_tx.data[0] = MSG_NEIGHBORS;
    set_color(RGB(3,0,0));
    should_send = 1;
    while (!sent_msg) { }
    sent_msg = 0;
    set_color(RGB(0,0,0));
    state = STATE_NEIGHBOR_QUERY;
    delay(500);
    state = STATE_DONE;
}


void setup() {
    state = STATE_INIT;
    msg_tx.type = NORMAL;
    msg_tx.crc = bbzmessage_crc(&msg_tx);
}

void loop() {
    find_neighbors();
    set_color(RGB(3,3,3));
    while (1) { }
}

int main() {
    bbzkilo_init();
    kilo_message_tx = my_msg_tx;
    kilo_message_tx_success = my_tx_msg_success;
    kilo_message_rx = my_msg_rx;
    msg_tx.data[0] = 1;
    bbzkilo_start(setup, loop);

    return 0;
}