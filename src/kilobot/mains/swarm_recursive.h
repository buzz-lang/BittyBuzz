#include <bbzkilobot.h>

#ifndef SWARM_RECURSIVE_H
#define SWARM_RECURSIVE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum {
    MSG_TYPE_NEIGH_BROAD = 2,     /**< @brief Trying to enstablish who my neighbors are. */
    MSG_TYPE_NEIGH_BROAD_RES,     /**< @brief I'm a neighbor of you. */
    MSG_TYPE_SWARM_NUM,           /**< @brief Increment the swarm member number, if you haven't already. */
    MSG_TYPE_SWARM_NUM_RES,       /**< @brief Done incrementing the swarm member number. */
    MSG_TYPE_SWARM_NUM_RECUR,     /**< @brief Recurse the swarm number message to your neighbors. */
    MSG_TYPE_SWARM_NUM_RECUR_RES, /**< @brief Finished my swarm recursion. */
    MSG_TYPE_SWARM_NUM_FINAL      /**< @brief Here is the final value of the swarm count. Pass it on. */
} Msg_type;

/**
 * @brief ID of the leader robot which initiates the swarm message.
 * @details We need to pick a leader. In order not to make a leader-choosing
 * algorithm, let's assume the leader has been chosen as the robot whose ID
 * is LEADER_ID.
 */
#define LEADER_ID 23

void setup();
void loop();

static inline uint8_t is_leader() { return kilo_uid == LEADER_ID; }
uint8_t is_in_swarm(uint8_t swid);

message_t* which_msg_tx();
void msg_tx_success();
volatile uint8_t is_msg_tx_sent();
void send_msg_tx(Msg_type type, uint8_t data[], uint8_t sz);
void send_msg_tx_neigh_broad();
void send_msg_tx_neigh_broad_res();
void send_msg_tx_swarm_num();
void send_msg_tx_swarm_num_res(bbzrobot_id_t reciever, uint8_t should_increment);
void send_msg_tx_swarm_num_recur();
void send_msg_tx_swarm_num_recur_res();
void send_msg_tx_swarm_num_final();

void find_neighbors();
void initiate_swarm_msg();
void process_msg_rx(message_t* msg_rx, distance_measurement_t* d);
void process_msg_rx_neigh_broad(message_t* msg_rx);
void process_msg_rx_neigh_broad_res(message_t* msg_rx);
void process_msg_rx_swarm_num(message_t* msg_rx);
void process_msg_rx_swarm_num_res(message_t* msg_rx);
void process_msg_rx_swarm_num_recur(message_t* msg_rx);
void process_msg_rx_swarm_num_recur_res(message_t* msg_rx);
void process_msg_rx_swarm_num_final(message_t* msg_rx);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !SWARM_RECURSIVE_H