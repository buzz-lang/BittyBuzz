#ifndef BITTYBUZZ_MESSAGES_H
#define BITTYBUZZ_MESSAGES_H

#include <stdint.h>
#include <bittybuzz/bbzinclude.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum {
    NORMAL = 0,
    GPS,
    BBZMSG,
    BOOT = 0x80,
    BOOTPGM_PAGE,
    BOOTPGM_SIZE,
    RESET,
    SLEEP,
    WAKEUP,
    CHARGE,
    VOLTAGE,
    RUN,
    READUID,
    CALIB,
} message_type_t;

/**
 * @brief Message structure.
 *
 * A message structure is 12 bytes in length and is composed of three
 * parts: the payload (9 bytes), the message type (1 byte), and a CRC (2
 * bytes).
 *
 * @note When preparing a message for transmission, at a minimum you
 * must specify the type (use a value between 0 and 127 for user
 * messages) and the CRC (use the message_crc() function for this
 * purpose).
 *
 * @see message_crc, kilo_message_rx, kilo_message_tx,
 * kilo_message_tx_success
 */
typedef struct PACKED message_t {
    uint8_t data[9]; ///< message payload.
    uint8_t type;    ///< message type.
    uint16_t crc;    ///< message crc.
} message_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BITTYBUZZ_MESSAGES_H
