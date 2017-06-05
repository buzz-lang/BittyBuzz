/**
 * @file bbzmsg.h
 * @brief Definition of message types between robots.
 */

#ifndef BBZMSG_H
#define BBZMSG_H

#include "bbzinclude.h"
#include "bbzdarray.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Buzz message type.
 * @details The types are ordered by decreasing priority
 */
typedef enum {
    BBZMSG_BROADCAST = 0, /**< @brief Neighbor broadcast */
    BBZMSG_SWARM_LIST,    /**< @brief Swarm listing */
    BBZMSG_VSTIG_PUT,     /**< @brief Virtual stigmergy PUT */
    BBZMSG_VSTIG_QUERY,   /**< @brief Virtual stigmergy QUERY */
    BBZMSG_SWARM_JOIN,    /**< @brief Swarm joining */
    BBZMSG_SWARM_LEAVE,   /**< @brief Swarm leaving */
    BBZMSG_TYPE_COUNT     /**< @brief How many message types have been defined */
} bbzmsg_payload_type_t;

/**
 * @brief Data of a message.
 */
typedef bbzheap_idx_t bbzmsg_payload_t;

/**
 * @brief Serializes a 8-bit unsigned integer.
 * @details The data is appended to the given buffer. The buffer is treated as a
 * dynamic array of uint8_t.
 * @param[in] buf The buffer where the serialized data is appended.
 * @param[in] data The data to serialize.
 */
void bbzmsg_serialize_u8(bbzheap_idx_t buf,
                         uint8_t data);

/**
 * @brief Deserializes a 8-bit unsigned integer.
 * @details The data is read from the given buffer starting at the given position.
 * The buffer is treated as a dynamic array of uint8_t.
 * @param[out] data The deserialized data of the element.
 * @param[in] buf The buffer where the serialized data is stored.
 * @param[in] pos The position at which the data starts.
 * @return The new position in the buffer, of -1 in case of error.
 */
int64_t bbzmsg_deserialize_u8(uint8_t* data,
                              bbzheap_idx_t buf,
                              uint32_t pos);

/**
 * @brief Serializes a 16-bit unsigned integer.
 * @details The data is appended to the given buffer. The buffer is treated as a
 * dynamic array of uint8_t.
 * @param[in] buf The buffer where the serialized data is appended.
 * @param[in] data The data to serialize.
 */
void bbzmsg_serialize_u16(bbzheap_idx_t buf,
                          uint16_t data);

/**
 * @brief Deserializes a 16-bit unsigned integer.
 * @details The data is read from the given buffer starting at the given position.
 * The buffer is treated as a dynamic array of uint8_t.
 * @param[out] data The deserialized data of the element.
 * @param[in] buf The buffer where the serialized data is stored.
 * @param[in] pos The position at which the data starts.
 * @return The new position in the buffer, of -1 in case of error.
 */
int64_t bbzmsg_deserialize_u16(uint16_t* data,
                               bbzheap_idx_t buf,
                               uint32_t pos);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZMSG_H