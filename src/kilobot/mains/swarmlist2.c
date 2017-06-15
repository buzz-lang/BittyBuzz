#include <kilobot/lib/bbzkilobot.h>

#define ARRAY_SIZE 16
#define TIMESTAMP_INIT 500
#define LAMPORTCLOCK_THRESHOLD 40
#define MSG_BUFFER_SIZE 20
#define INC_SIZE 3
typedef uint8_t lamport_t;

typedef enum {
    MSG_SWARMLIST = 2,
    MSG_SWARMPING
} msg_type_t;

typedef struct {
    uint16_t data[ARRAY_SIZE];
    uint16_t length;
} darray_t;

typedef struct {
    message_t data[MSG_BUFFER_SIZE];
    uint16_t start;
    uint16_t length;
} msgdarray_t;

typedef void (*foreach_fun)(darray_t* arr, uint16_t* val, void* params);

uint8_t should_send, message_sent, message_rcvd;
message_t msg_rcvd, msg;
msgdarray_t msgBuf;
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
    for (uint16_t i = 0; i < arr->length; ++i) {
        if (arr->data[i] == val) {
            return i;
        }
    }
    return arr->length;
};
void dforeach(darray_t* arr, foreach_fun fun, void* params) {
    for (uint16_t i = 0; i < arr->length; ++i) {
        fun(arr, arr->data + i, params);
    }
}

ALWAYS_INLINE
uint16_t mdsize(msgdarray_t* md) {return md->length;}
ALWAYS_INLINE
uint8_t mdpush(msgdarray_t* md, message_t* val) {if(md->length>=MSG_BUFFER_SIZE-1)return 0;md->data[(md->start+(md->length++))%MSG_BUFFER_SIZE]=*val;return 1;}
ALWAYS_INLINE
uint8_t mdpop(msgdarray_t* md) {if(md->length==0)return 0;--md->length;++md->start;md->start%=MSG_BUFFER_SIZE;return 1;}
/**
 * @brief Returns the next value of the ringbuffer.
 * @returns The next value of the ring buffer.
 */
ALWAYS_INLINE
uint8_t mdnext(msgdarray_t* md, message_t* buf) {if(md->length==0)return 0;*buf=md->data[md->start];return 1;}
ALWAYS_INLINE
uint8_t mdpushnext(msgdarray_t* md, message_t* val) {if(md->length>=MSG_BUFFER_SIZE-1)return 0;md->data[(--md->start)%MSG_BUFFER_SIZE]=*val;++md->length;return 1;}

void display_size(uint16_t size) {
    if (size <= 0 + INC_SIZE) {
        setled(0);
    }
    else if (size == 1 + INC_SIZE) {
        setled(RGB(3, 0, 0));
    }
    else if (size == 2 + INC_SIZE) {
        setled(RGB(3, 1, 0));
    }
    else if (size == 3 + INC_SIZE) {
        setled(RGB(1, 2, 0));
    }
    else if (size == 4 + INC_SIZE) {
        setled(RGB(0, 3, 0));
    }
    else if (size == 5 + INC_SIZE) {
        setled(RGB(0, 2, 1));
    }
    else if (size == 6 + INC_SIZE) {
        setled(RGB(0, 0, 3));
    }
    else {
        setled(RGB(2, 0, 1));
    }
}

void valid_counter(darray_t* darray, uint16_t* v, void* count) {
    *(uint16_t*)count += (uint16_t)!!*v;
}

uint16_t dvalidcount() {
    uint16_t count = 0;
    dforeach(&swarmTimers, valid_counter, &count);
    return count;
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
//        dset(&swarmTimers, pos, TIMESTAMP_INIT);
        dset(&swarmLamports, pos, lamport);
    }
}

void removeRobotSwarmPos(uint16_t pos) {
    uint16_t size = dsize(&robotIDs);
    if (size == 0) return;
    if (pos < size) {
//        uint16_t v = 0;
//        dget(&robotIDs, size-1, &v);
//        dset(&robotIDs, pos, v);
//        dget(&swarmLists, size-1, &v);
//        dset(&swarmLists, pos, v);
//        dget(&swarmTimers, size-1, &v);
//        dset(&swarmTimers, pos, v);
//        dget(&swarmLamports, size-1, &v);
//        dset(&swarmLamports, pos, v);
//        dpop(&robotIDs);
//        dpop(&swarmLists);
//        dpop(&swarmTimers);
//        dpop(&swarmLamports);
        dset(&swarmTimers, pos, 0);
    }
}

void join(uint8_t swarmID) {
    swarmsLocal |= 1 << (swarmID % 16);
    ++lamportLocal;
    setRobotSwarm(kilo_uid, swarmsLocal, lamportLocal);
}

void leave(uint8_t swarmID) {
    swarmsLocal &= ~(1 << (swarmID % 16));
    ++lamportLocal;
    setRobotSwarm(kilo_uid, swarmsLocal, lamportLocal);
}

/*/message_t* tx_send() {
    if (should_send) {
        should_send = 0;
        return &msg;
    }
    else {
        return 0;
    }
}/*/
message_t* tx_send() {
    if (should_send) {
        should_send = 0;
        if (mdsize(&msgBuf) > 1) {
            should_send = 1;
            tx_clock_reset();
        }
        if (mdnext(&msgBuf, &msg)) {
            mdpop(&msgBuf);
            return &msg;
        }
    }
    return 0;
}//*/

void tx_send_success() {
    message_sent = 1;
}

void send_ping() {
    msg.type = MSG_SWARMPING;
    msg.data[0] = (uint8_t)kilo_uid;
    *(uint16_t*)(msg.data + 1) = swarmsLocal;
    msg.data[3] = lamportLocal;
    msg.data[4] = (uint8_t)0;
    msg.crc = bbzmessage_crc(&msg);
    mdpush(&msgBuf,&msg);
    should_send = 1;
}

/*/void send_msg(uint16_t rid, uint16_t swarmList, lamport_t lamport) {
    msg.type = MSG_SWARMLIST;
    msg.data[0] = (uint8_t)rid;
    *(uint16_t*)(msg.data + 1) = swarmList;
    msg.data[3] = lamport;
    msg.crc = bbzmessage_crc(&msg);
    should_send = 1;
}/*/
void send_msg(uint16_t rid, uint16_t swarmList, lamport_t lamport) {
    msg.type = MSG_SWARMLIST;
    msg.data[0] = (uint8_t)rid;
    *(uint16_t*)(msg.data + 1) = swarmList;
    msg.data[3] = lamport;
    msg.data[4] = (uint8_t)0;
    msg.crc = bbzmessage_crc(&msg);
    mdpush(&msgBuf,&msg);
    should_send = 1;
}//*/

void send_msg2(uint16_t rid, uint16_t swarmList, lamport_t lamport,
               uint16_t rid2, uint16_t swarmList2, lamport_t lamport2) {
    msg.type = MSG_SWARMLIST;
    msg.data[0] = (uint8_t)rid;
    *(uint16_t*)(msg.data + 1) = swarmList;
    msg.data[3] = lamport;
    msg.data[4] = MSG_SWARMLIST;
    msg.data[5] = (uint8_t)rid2;
    *(uint16_t*)(msg.data + 6) = swarmList2;
    msg.data[7] = lamport2;
    msg.crc = bbzmessage_crc(&msg);
    mdpush(&msgBuf,&msg);
    should_send = 1;
}

/*/void rx_rcvr(message_t* message, distance_measurement_t* d) {
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
}/*/
void rx_rcvr(message_t* message, distance_measurement_t* d) {
    uint16_t v, u;
    if (!message_rcvd) {
        switch (message->type) {
            case MSG_SWARMLIST:
                v = dfind(&robotIDs, message->data[0]);
                if ((!dget(&swarmLamports, v, &v) ||
                        (message->data[3] + 256 - v)%256 > LAMPORTCLOCK_THRESHOLD)/* &&
                    (uint16_t) message->data[0] != kilo_uid*/) {
                    setRobotSwarm(message->data[0], *(uint16_t *) (message->data + 1), message->data[3]);
                }

                if (message->data[4] == MSG_SWARMLIST) {
                    if ((!dget(&swarmLamports, v, &v) ||
                            (message->data[8] + 256 - v)%256 > LAMPORTCLOCK_THRESHOLD)/* &&
                        (uint16_t) message->data[5] != kilo_uid*/) {
                        setRobotSwarm(message->data[5], *(uint16_t *) (message->data + 6), message->data[8]);
                    }
                }
//                return;
                break;
            case MSG_SWARMPING:
                u = dfind(&robotIDs, message->data[0]);
                if ((!dget(&swarmLamports, u, &v) ||
                        (message->data[3] + 256 - v)%256 > LAMPORTCLOCK_THRESHOLD) &&
                        (uint16_t) message->data[5] != kilo_uid) {
                    setRobotSwarm(message->data[0], *(uint16_t *) (message->data + 1), message->data[3]);mdnext(&msgBuf, &msg);
                    mdpushnext(&msgBuf, message);
                    should_send = 1;
                    tx_clock_reset();
                }
                /*if (dget(&swarmLamports, u, &v)) */dset(&swarmTimers, u, TIMESTAMP_INIT);
//                uint8_t prevled = led_state;
//                setled(RGB(1,1,1));
//                delay(25);
//                setled(prevled);
//                return;
                break;
            default:
                break;
        }
    }
    msg_rcvd.type = message->type;
    for (uint8_t i = 0; i < 9; ++i) {
        msg_rcvd.data[i] = message->data[i];
    }
    message_rcvd = 1;
}//*/

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

    spinup_motors();
    set_motors(0,0);
}

void timer_updater(darray_t* darray, uint16_t* v, void* params) {
    uint16_t pos = dfind(darray, *v);
    if (*v <= 0) {
        if (pos < dsize(darray))
            removeRobotSwarmPos(pos);
    }
    else {
        --*v;
    }
}

void update_timers() {
    display_size(dvalidcount());
    delay(25);
    setled(0);
    dforeach(&swarmTimers, timer_updater, 0);
}

void swarm_sendmsg(darray_t* darray, uint16_t* val, void* params) {
    uint16_t v = *val, u;
    uint16_t pos = dfind(darray, *val);
    if (pos < dsize(darray) && dget(&swarmTimers, pos, &u) && u) {
        if (pos % 2 == 0) {
            msg.type = MSG_SWARMLIST;
            msg.data[0] = v;
            dget(&swarmLists, pos, &v);
            *(uint16_t*)(msg.data + 1) = v;
            dget(&swarmLamports, pos, &v);
            msg.data[3] = v;
            msg.data[4] = 0;
            if (pos+1 == dsize(darray)) {
                //msg.crc = bbzmessage_crc(&msg);
                send_msg(msg.data[0], *(uint16_t*)(msg.data+1), msg.data[3]);
            }
        }
        else {
            msg.data[4] = MSG_SWARMLIST;
            msg.data[5] = v;
            dget(&swarmLists, pos, &v);
            *(uint16_t*)(msg.data + 6) = v;
            dget(&swarmLists, pos, &v);
            msg.data[8] = v;
            //msg.crc = bbzmessage_crc(&msg);
            send_msg2(msg.data[0], *(uint16_t*)(msg.data+1), msg.data[3],
                      msg.data[5], *(uint16_t*)(msg.data+6), msg.data[8]);
        }
    }
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
        setled(RGB(0, 0, 0));
        delay(10);
    }
//    if (dsize(&robotIDs) < 3)
//        set_motors(0,kilo_turn_right);
//    else
//        set_motors(kilo_straight_left, kilo_straight_right);
    if (--ticks <= 0) {
        ticks = ((rand_soft() >> 4) + 4) * 3;//5;//
        leave(kilo_uid);
        join(kilo_uid);
        if (!should_send/* && (kilo_uid == 14 || kilo_uid == 50 || kilo_uid == 54)*/) {
            /*/send_msg(kilo_uid, swarmsLocal, lamportLocal);/*/
            send_ping();
            dforeach(&robotIDs, swarm_sendmsg, 0);//*/
        }
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