
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "config.h" 
#include "utilities.h"
#include "stm32f0xx_lp_modes.h"
#include "led.h"
#include "motors.h"
#include "radio.h"
#include "sensors.h"
#include "position_control.h"
#include "memcpy_fast.h"
#include "qfplib.h"

typedef void (*message_rx_t)(Message *, uint16_t distance, int16_t azimuth);
typedef Message *(*message_tx_t)(void);
typedef void (*message_tx_success_t)(void);

extern volatile message_tx_t         message_tx;
extern volatile message_tx_success_t message_tx_success;
extern volatile message_rx_t         message_rx;

void initRobot();
void updateRobot();
void sleep();
void delay(uint16_t ms);
void checkRadio();
void handleIncomingRadioMessage();
void handleOutgoingRadioMessage();
void prepareMessageToSend(Position *position, float *orientation, uint8_t *touch, bool *reached, uint16_t batteryLevel);
void checkTouch();
void switchToChargingMode();

#ifdef __cplusplus
}
#endif

#endif /* __FUNCTIONS_H */