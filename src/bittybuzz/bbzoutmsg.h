/**
 * @file bbzoutmsg.h
 * @brief Definition of the message queues structure for outgoing messages.
 */

#ifndef BBZOUTMSG_H
#define BBZOUTMSG_H

#include "bbzinclude.h"
#include "bbzmsg.h"
#include "bbzringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief 
 * @note You should not create a stigmergy manually ; we assume there
 * is only a single instance: vm->outmsgs.
 */
typedef struct PACKED {
    bbzringbuf_t queues[BBZMSG_TYPE_COUNT]; /**< @brief Message queues. One for each queue type. */
} bbzoutmsg_queue_t;

/**
 * @brief Constructs the VM's outgoing message queue.
 */
void bbzoutmsg_construct();

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h" // Include AFTER bbzoutmsg.h because of circular dependencies.

#endif // !BBZOUTMSG_H