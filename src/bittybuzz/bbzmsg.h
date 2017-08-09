/**
 * @file bbzmsg.h
 * @brief Definition of message types between robots.
 */

#ifndef BBZMSG_H
#define BBZMSG_H

#include "bbzinclude.h"
#include "bbzringbuf.h"

#ifdef BBZ_XTREME_MEMORY
#define htons(x) (x)
#define ntohs(x) (x)
#else
#include <netinet/in.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct PACKED bbzmsg_base_t {
    bbzmsg_payload_type_t type;
    bbzrobot_id_t rid;
} bbzmsg_base_t;

/**
 * @brief Broadcast message data
 */
typedef struct PACKED bbzmsg_broadcast_t {
#ifndef BBZ_DISABLE_NEIGHBORS
    bbzmsg_payload_type_t type;
    bbzrobot_id_t rid;
    uint16_t topic; /**< @brief The topic of the broadcast. @note A string ID */
    bbzobj_t value; /**< @brief The broadcasted value. */
#endif
} bbzmsg_broadcast_t;

/**
 * @brief Swarm message data
 */
typedef struct PACKED bbzmsg_swarm_t {
#ifndef BBZ_DISABLE_SWARMS
    bbzmsg_payload_type_t type;
    bbzrobot_id_t rid;
    bbzlamport_t lamport;
    bbzswarmlist_t swarms;
#endif
} bbzmsg_swarm_t;

/**
 * @brief Virtual stigmergy message data
 */
typedef struct PACKED bbzmsg_vstig_t {
#ifndef BBZ_DISABLE_VSTIGS
    bbzmsg_payload_type_t type;
    bbzrobot_id_t rid;
    uint8_t lamport;
    uint16_t key;
    bbzobj_t data;
#endif
} bbzmsg_vstig_t;

/**
 * @brief Generic message data
 */
typedef union PACKED bbzmsg_t {
#ifndef BBZ_DISABLE_MESSAGES
    bbzmsg_payload_type_t type;
    bbzmsg_base_t base;
    bbzmsg_broadcast_t bc;
    bbzmsg_swarm_t sw;
    bbzmsg_vstig_t vs;
#endif
} bbzmsg_t;

/**
 * @brief Data of a message.
 */
typedef bbzringbuf_t bbzmsg_payload_t;

#ifndef BBZ_DISABLE_MESSAGES
/**
 * @brief Serializes a 8-bit unsigned integer.
 * @details The data is appended to the given buffer.
 * @param[in,out] rb The buffer where the serialized data is appended.
 * @param[in] data The data to serialize.
 */
void bbzmsg_serialize_u8(bbzringbuf_t *rb,
                         uint8_t data);

/**
 * @brief Deserializes a 8-bit unsigned integer.
 * @details The data is read from the given buffer starting at the given
 * position.
 * @param[out] data The deserialized data of the element.
 * @param[in] rb The buffer where the serialized data is stored.
 * @param[in] pos The position at which the data starts.
 */
void bbzmsg_deserialize_u8(uint8_t *data,
                           const bbzringbuf_t *rb,
                           int16_t *pos);

/**
 * @brief Serializes a 16-bit unsigned integer.
 * @details The data is appended to the given buffer.
 * @param[in,out] rb The buffer where the serialized data is appended.
 * @param[in] data The data to serialize.
 */
void bbzmsg_serialize_u16(bbzringbuf_t *rb,
                          uint16_t data);

/**
 * @brief Deserializes a 16-bit unsigned integer.
 * @details The data is read from the given buffer starting at the given
 * position.
 * @param[out] data The deserialized data of the element.
 * @param[in] rb The buffer where the serialized data is stored.
 * @param[in] pos The position at which the data starts.
 */
void bbzmsg_deserialize_u16(uint16_t *data,
                            const bbzringbuf_t *rb,
                            int16_t *pos);

/**
 * @brief Serializes a BittyBuzz's object.
 * @details The data is appended to the given buffer.
 * @param[in,out] rb The buffer where the serialized data is appended.
 * @param[in] obj The object to serialize.
 */
void bbzmsg_serialize_obj(bbzringbuf_t *rb, bbzobj_t *obj);

/**
 * @brief Serializes a BittyBuzz's object.
 * @details The data is read from the given buffer starting at the given
 * position.
 * @param[out] data The deserialized data of the element.
 * @param[in] rb The buffer where the serialized data is stored.
 * @param[in] pos The position at which the data starts.
 */
void bbzmsg_deserialize_obj(bbzobj_t *data, bbzringbuf_t *rb, int16_t *pos);

#ifndef BBZ_DISABLE_NEIGHBORS
void bbzmsg_process_broadcast(bbzmsg_t* msg);
#endif

#ifndef BBZ_DISABLE_VSTIGS
void bbzmsg_process_vstig(bbzmsg_t* msg);
#endif

#ifndef BBZ_DISABLE_SWARMS
void bbzmsg_process_swarm(bbzmsg_t* msg);
#endif

// +=-=-=-=-=-=-=-=-=-=-=-=-=-=+
// | Message utility functions |
// +=-=-=-=-=-=-=-=-=-=-=-=-=-=+

/**
 * @brief Applies a sorting algorithm to the given ring buffer.
 * @note It currently uses quicksort, but it could change in the futur.
 *       It also modifies the unused space at the end of the ring buffer.
 * @param[in,out] rb The ring buffer which we want to sort.
 */
void bbzmsg_sort_priority(bbzringbuf_t* rb);
#else // !BBZ_DISABLE_MESSAGES
#define bbzmsg_serialize_u8(...)
#define bbzmsg_deserialize_u8(...)
#define bbzmsg_serialize_u16(...)
#define bbzmsg_deserialize_u16(...)
#define bbzmsg_serialize_obj(...)
#define bbzmsg_deserialize_obj(...)
#define bbzmsg_sort_priority(...)
#endif // !BBZ_DISABLE_MESSAGES

#if defined(BBZ_DISABLE_NEIGHBORS) || defined(BBZ_DISABLE_MESSAGES)
#define bbzmsg_process_broadcast(...)
#endif
#if defined(BBZ_DISABLE_VSTIGS) || defined(BBZ_DISABLE_MESSAGES)
#define bbzmsg_process_vstig(...)
#endif
#if defined(BBZ_DISABLE_SWARMS) || defined(BBZ_DISABLE_MESSAGES)
#define bbzmsg_process_swarm(...)
#endif

/*
 * Uncomment this line if the sorting algorithm above needs
 * to be called when an element is removed from the ring buffer.
 */
//#define BBZMSG_POP_NEEDS_SORT

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZMSG_H