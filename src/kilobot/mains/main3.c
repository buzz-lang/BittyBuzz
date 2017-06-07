#include <kilobot/lib/bbzkilobot.h>
#include <kilobot/lib/bbzmessage.h>

#ifndef NULL
#define NULL (void*)0
#endif

typedef enum {
    MSG_PING = 2,
    MSG_PONG,
    MSG_DATA,
    MSG_RCVD
} msg_type_t;

typedef enum {
    STATE_INIT = 0,
    STATE_PENDING,
    STATE_DONE
} state_t;

state_t state;
message_t msg;
message_t msgbuf;

uint8_t sent_message = 0;
uint8_t should_send = 1;
uint8_t rcvd_message = 0;
message_t rcvd_data;

// message transmission callback
message_t* tx_message() {
    if (should_send) {
        return &msg;
    }
    else {
        return NULL;
    }
}
// successful transmission callback
void tx_message_success() {
    sent_message = 1;
    if (state == STATE_DONE) {
        should_send = 0;
    }
    else {
        msg = msgbuf;
    }
}

// receive message callback
void rx_message(message_t *msg2, distance_measurement_t *d) {
    rcvd_message = 1;
    rcvd_data.type = msg2->type;
    for (uint8_t i = 0; i < 9; ++i)
        rcvd_data.data[i] = msg2->data[i];
    if (rcvd_data.type == MSG_PING/* || rcvd_data.type == MSG_PONG*/) {
        msgbuf = msg;
        msg.type = MSG_PONG;
        msg.data[0] = msg2->data[0];
        msg.crc = bbzmessage_crc(&msg);
//        rcvd_message = 0;
        should_send = 1;
    }
    if (rcvd_data.type == MSG_DATA) {
        msgbuf = msg;
        msg.type = MSG_RCVD;
        msg.data[0] = msg2->data[0];
        msg.crc = bbzmessage_crc(&msg);
//        rcvd_message = 0;
        should_send = 1;
    }
    if (state == STATE_PENDING) {
        if (rcvd_data.type == MSG_PONG || rcvd_data.type == MSG_RCVD) {
            if (rcvd_data.data[0] == kilo_uid) {
                state = STATE_DONE;
                should_send = 0;
            }
            rcvd_message = 0;
        }
    }
}

void setup() {
    state = STATE_INIT;
    rcvd_message = 0;
    should_send = 0;
    sent_message = 0;

    /*if (kilo_uid == 39) {
        should_send = 0;
        state = STATE_DONE;
    }*/
}

void loop () {
    set_color(RGB(0,0,0));
    switch(state) {
        case STATE_INIT:
            set_color(RGB(2,0,1));
            if (kilo_uid == 14) {
                msg.type = MSG_DATA;
                delay(300);
            }
            else
                msg.type = MSG_PING;
            msg.data[0] = kilo_uid;
            msg.crc = bbzmessage_crc(&msg);
            msgbuf = msg;
            rcvd_message = 0;
            should_send = 1;
            state = STATE_PENDING;
            break;
        case STATE_PENDING:
            set_color(RGB(0,0,3));
            break;
        case STATE_DONE:
            set_color(RGB(0,3,0));
//            if (kilo_uid != 14)
//                delay(200);
            if (!should_send &&/* kilo_uid % 2 ==*/ 0) {
                msg.type = MSG_DATA;
                msg.data[0] = kilo_uid;//RGB(3,3,3);
                msg.crc = bbzmessage_crc(&msg);
                msgbuf = msg;
                rcvd_message = 0;
                should_send = 1;
                state = STATE_PENDING;
            }
            break;
        default:
            break;
    }
    if (sent_message) {
        sent_message = 0;
        set_color(RGB(0,1,2));
        delay(25);
    }
    if (rcvd_message) {
        rcvd_message = 0;
        set_color(rcvd_data.data[0]);
        delay(50);
    }

//    delay(50);
}

int main() {
    bbzkilo_init();
    kilo_message_tx = tx_message;
    kilo_message_rx = rx_message;
    kilo_message_tx_success = tx_message_success;
    bbzkilo_start(setup, loop);

    return 0;
}