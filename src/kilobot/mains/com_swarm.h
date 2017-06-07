#ifndef COM_SWARM_H
#define COM_SWARM_H

#include <kilobot/lib/bbzkilobot.h>
#include <kilobot/lib/bbzkiloreg.h>
#include <kilobot/lib/bbzmessage_crc.h>

enum Message_Types {
    MSG_NEIGHBORS = 0,  // Can anyone (else) hear me?
    MSG_PING,           // X, are you there?
    MSG_ANS,            // I'm here!
    MSG_SWARM           // Those are the swarm members I know.
};

typedef enum {
    STATE_INIT = 0,
    STATE_NEIGHBOR_QUERY,
    STATE_DONE
} State;

message_t* my_msg_tx();
void my_tx_msg_success();
void my_msg_rx(message_t* msg_rx, distance_measurement_t* d);

void find_neighbors();
void setup();
void loop();

#endif // !COM_SWARM_H