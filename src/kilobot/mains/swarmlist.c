#include <kilobot/lib/bbzkilobot.h>
#include <bittybuzz/bbzinclude.h>

#define ARRAY_SIZE 10
#define TIMESTAMP_INIT 200
typedef uint8_t lamport_t;

typedef enum {
    MSG_SWARMLIST = 2
} msg_type_t;

typedef struct {
    uint16_t data[ARRAY_SIZE];
    uint16_t length;
} darray_t;

typedef void (*foreach_fun)(darray_t* arr, uint16_t* val);

uint8_t should_send, message_sent, message_rcvd;
message_t msg_rcvd, msg;
darray_t robotIDs, swarmLists, swarmLamports, swarmTimers;
uint16_t swarmsLocal = 0;
lamport_t lamportLocal = 0;

uint8_t led_state;

void setled(uint8_t l) {
    set_color(l);
    led_state = l;
}

ALWAYS_INLINE
uint8_t dnew(darray_t* arr) {arr->length = 0; return 1;}
ALWAYS_INLINE
uint8_t dpush(darray_t* arr, uint16_t val) {if (arr->length < ARRAY_SIZE - 1) {arr->data[arr->length++]=val;return 1;}return 0;}
ALWAYS_INLINE
uint8_t dpop(darray_t* arr) {if (arr->length > 0) {--arr->length;return 1;} return 0;}
ALWAYS_INLINE
uint8_t dset(darray_t* arr, uint16_t pos, uint16_t val) {if (pos < arr->length) {arr->data[pos]=val;return 1;} return 0;}
ALWAYS_INLINE
uint8_t dget(darray_t* arr, uint16_t pos, uint16_t* val) {if (pos < arr->length) {*val = arr->data[pos];return 1;}return 0;}
ALWAYS_INLINE
uint16_t dsize(darray_t* arr) {return arr->length;}
uint16_t dfind(darray_t* arr, uint16_t val) {
    for (uint16_t i = 0; i < arr->length; ++i)
        if (arr->data[i] == val)
            return i;
    return arr->length;
};
void dforeach(darray_t* arr, foreach_fun fun) {
    for (uint16_t i = 0; i < arr->length; ++i) {
        fun(arr, arr->data + i);
    }
}

void display_size(darray_t* darray) {
    uint16_t size = dsize(darray);
    if (size <= 0) {
        setled(0);
    }
    else if (size == 1) {
        setled(RGB(3, 0, 0));
    }
    else if (size == 2) {
        setled(RGB(1, 2, 0));
    }
    else if (size == 3) {
        setled(RGB(0, 3, 0));
    }
    else if (size == 4) {
        setled(RGB(0, 1, 2));
    }
    else {
        setled(RGB(2, 0, 1));
    }
}

void setRobotSwarm(uint16_t rid, uint16_t swarmList, lamport_t lamport) {
    uint16_t pos = dfind(&robotIDs, rid);
    if (pos >= dsize(&robotIDs)) {
        dpush(&robotIDs, rid);
        dpush(&swarmLists, swarmList);
        dpush(&swarmTimers, TIMESTAMP_INIT);
        dpush(&swarmLamports, lamport);
    }
    else {
        dset(&robotIDs, pos, rid);
        dset(&swarmLists, pos, swarmList);
        dset(&swarmTimers, pos, TIMESTAMP_INIT);
        dset(&swarmLamports, pos, lamport);
    }
}

void removeRobotSwarmPos(uint16_t pos) {
    uint16_t size = dsize(&robotIDs);
    if (size == 0) return;
    if (pos < size) {
        uint16_t v = 0;
        dget(&robotIDs, size-1, &v);
        dset(&robotIDs, pos, v);
        dget(&swarmLists, size-1, &v);
        dset(&swarmLists, pos, v);
        dget(&swarmTimers, size-1, &v);
        dset(&swarmTimers, pos, v);
        dget(&swarmLamports, size-1, &v);
        dset(&swarmLamports, pos, v);
        dpop(&robotIDs);
        dpop(&swarmLists);
        dpop(&swarmTimers);
        dpop(&swarmLamports);
    }
}

void removeRobotSwarm(uint16_t rid) {
    uint16_t size = dsize(&robotIDs);
    if (size == 0) return;
    uint16_t pos = dfind(&robotIDs, rid);
    removeRobotSwarmPos(pos);
}

void join(uint8_t swarmID) {
    swarmsLocal |= 1 << (swarmID % 16);
    ++lamportLocal;
}

void leave(uint8_t swarmID) {
    swarmsLocal &= ~(1 << (swarmID % 16));
    ++lamportLocal;
}

message_t* tx_send() {
    if (should_send) {
        should_send = 0;
        return &msg;
    }
    else {
        return 0;
    }
}

void tx_send_success() {
    message_sent = 1;
}

void send_msg(uint16_t rid, uint16_t swarmList, lamport_t lamport) {
    msg.type = MSG_SWARMLIST;
    msg.data[0] = (uint8_t)rid;
    *(uint16_t*)(msg.data + 1) = swarmList;
    msg.data[3] = lamport;
    msg.crc = bbzmessage_crc(&msg);
    should_send = 1;
}

void rx_rcvr(message_t* message, distance_measurement_t* d) {
    uint16_t v;
    if (!message_rcvd) {
        switch (message->type) {
            case MSG_SWARMLIST:
                v = dfind(&robotIDs, message->data[0]);
                if ((!dget(&swarmLamports, v, &v) ||
                        v < message->data[3]) &&
                        (uint16_t) message->data[0] != kilo_uid &&
                        message->crc == bbzmessage_crc(message)) {
                    setRobotSwarm(message->data[0], *(uint16_t *) (message->data + 1), message->data[3]);
                    send_msg(message->data[0], *(uint16_t *) (message->data + 1), message->data[3]);
                    if (should_send && !message_sent) {
                        tx_clock_reset();
                    }
                }
                return;
                //break;
            default:
                break;
        }
    }
    msg_rcvd.type = message->type;
    for (uint8_t i = 0; i < 9; ++i) {
        msg_rcvd.data[i] = message->data[i];
    }
    message_rcvd = 1;
}

uint16_t ticks = 0;
void setup() {
    rand_seed(rand_hard());

    dnew(&robotIDs);
    dnew(&swarmLists);
    dnew(&swarmLamports);
    dnew(&swarmTimers);

    ticks = (rand_soft() >> 3) + 1;

    join(kilo_uid);
    setled(RGB(1,0,2));
    delay(50);

    delay(rand_soft()*2+255);
    setled(RGB(2,0,1));
    delay(50);
}

void timer_updater(darray_t* darray, uint16_t* v) {
    if (*v <= 0) {
        uint16_t pos = dfind(darray, *v);
        if (pos < dsize(darray))
            removeRobotSwarmPos(pos);
    }
    else
        --*v;
}

void update_timers() {
    display_size(&swarmTimers);
    delay(25);
    setled(0);
    dforeach(&swarmTimers, timer_updater);
}

void loop() {
    update_timers();
    setled(RGB(0,0,0));
    if (message_sent) {
        message_sent = 0;
//        setled(RGB(3,1,0));
//        delay(50);
    }
    if (message_rcvd) {
        message_rcvd = 0;
//        setled(RGB(0, 0, 3));
//        delay(50);
    }
    if (!should_send && --ticks <= 0) {
        ticks = 1;//(rand_soft() >> 3) + 1;//
        leave(kilo_uid);
        join(kilo_uid);
        send_msg(kilo_uid, swarmsLocal, lamportLocal);
    }
}

int main() {
    bbzkilo_init();

    kilo_message_tx = tx_send;
    kilo_message_rx = rx_rcvr;
    kilo_message_tx_success = tx_send_success;

    bbzkilo_start(setup, loop);

    return 0;
}