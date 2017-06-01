#ifndef __MESSAGE_SEND_H__
#define __MESSAGE_SEND_H__

#include "bbzmessage.h"

extern volatile uint8_t tx_mask;
uint8_t bbzmessage_send(const message_t *);

#endif//__MESSAGE_SEND_H__
