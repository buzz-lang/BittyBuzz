#ifndef __MESSAGE_SEND_H__
#define __MESSAGE_SEND_H__

#include "bbzmessage.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern volatile uint8_t tx_mask;

uint8_t bbzmessage_send(const message_t *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif//__MESSAGE_SEND_H__
