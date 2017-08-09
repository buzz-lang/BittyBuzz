/**
 * @file bbzinmsg.h
 * @brief Definition of the message queues structure for incoming messages.
 */

#ifndef BBZINMSG_H
#define BBZINMSG_H

#include "bbzinclude.h"
#include "bbzringbuf.h"
#include "bbzmsg.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * @brief Type for the input message structure.
 * @note You should not create an instance of this structure manually ;
 * we assume there is only a single instance: vm->inmsgs.
 */
typedef struct PACKED bbzinmsg_queue_t {
#ifndef BBZ_DISABLE_MESSAGES
    bbzringbuf_t queue; /**< @brief Message queue. */
    bbzmsg_t buf[BBZINMSG_QUEUE_CAP+2]; /**< @brief Output message buffer */
#endif
} bbzinmsg_queue_t;

#ifndef BBZ_DISABLE_MESSAGES
/**
 * Appends a message to the queue.
 * @param[in] payload The message payload.
 */
void bbzinmsg_queue_append(bbzmsg_payload_t* payload);

/**
 * @brief Extracts a message from the queue.
 * @note You are in charge of allocating and freeing the payload buffer.
 * @param[out] buf A buffer where to put the deserialized payload of the received message.
 */
bbzmsg_t * bbzinmsg_queue_extract();

/**
 * Create a new message queue.
 */
#define bbzinmsg_queue_construct() bbzringbuf_construct(&vm->inmsgs.queue, (uint8_t*)vm->inmsgs.buf, sizeof(bbzmsg_t), BBZINMSG_QUEUE_CAP+1);

/**
 * Destroys a message queue.
 */
#define bbzinmsg_queue_destruct() bbzringbuf_clear(&vm->inmsgs.queue)

/**
 * Returns the size of a message queue.
 * @return The size of a message queue.
 */
#define bbzinmsg_queue_size() bbzringbuf_size(&vm->inmsgs.queue)

/**
 * Returns <tt>true</tt> if the message queue is empty.
 * @param msgq The message queue.
 * @return <tt>true</tt> if the message queue is empty.
 */
#define bbzinmsg_queue_isempty() bbzringbuf_empty(&vm->inmsgs.queue)

/**
 * Returns the message at the given position in the queue.
 * @param msg The message queue.
 * @param pos The position.
 * @return The message at the given position.
 */
#define bbzinmsg_queue_get(pos) ((bbzmsg_t*)bbzringbuf_at(&vm->inmsgs.queue, pos))
#else
#define bbzinmsg_queue_append(...)
#define bbzinmsg_queue_extract(...) ((bbzmsg_t*)NULL)
#define bbzinmsg_queue_construct(...)
#define bbzinmsg_queue_destruct(...)
#define bbzinmsg_queue_size(...) (0)
#define bbzinmsg_queue_isempty(...) (1)
#define bbzinmsg_queue_get(...) ((bbzmsg_t*)NULL)
#endif // !BBZ_DISABLE_MESSAGES

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h" // Include AFTER bbzinmsg.h because of circular dependencies.

#endif // !BBZINMSG_H