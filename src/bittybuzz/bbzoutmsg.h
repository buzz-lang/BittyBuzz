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
 * @brief Type for the output message structure.
 * @note You should not create an instance of this structure manually ;
 * we assume there is only a single instance: vm->outmsgs.
 */
typedef struct PACKED {
    bbzringbuf_t queues[BBZMSG_TYPE_COUNT]; /**< @brief Message queues. One for each queue type. */
} bbzoutmsg_queue_t;

/**
 * @brief Constructs the VM's outgoing message queue.
 */
void bbzoutmsg_construct();

/**
 * Appends a new broadcast message to be sent.
 * @param topic The topic on which to send (a string object).
 * @param value The value.
 */
void bbzoutmsg_append_broadcast(bbzheap_idx_t topic, bbzheap_idx_t value);

// Other message types are TODO

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h" // Include AFTER bbzoutmsg.h because of circular dependencies.

#endif // !BBZOUTMSG_H