/**
  ******************************************************************************
  * @file    bbzcrazyflie.h
  * @author  Tan Wei Jun (WAYNE)
  * @version V0.0.1
  * @date
  * @brief   Header for bbzcrazyflie.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BBZCRAZYFLIE_H
#define __BBZCRAZYFLIE_H

/* Includes ------------------------------------------------------------------*/
// #include "functions.h"
// #include "position_control.h"
#include <bittybuzz/bbzvm.h>
#include <bittybuzz/util/bbzstring.h>
#include "memcpy_fast.h"
#include "qfplib.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
typedef void (*message_rx_t)(Message *, uint16_t distance, int16_t azimuth);
typedef Message *(*message_tx_t)(void);
typedef void (*message_tx_success_t)(void);

extern volatile message_tx_t         message_tx;
extern volatile message_tx_success_t message_tx_success;
extern volatile message_rx_t         message_rx;

const uint8_t* bbzcrazyflie_bcodeFetcher(bbzpc_t offset, uint8_t size);
void bbz_init();
void bbz_start(void (*setup)(void));
void bbz_err_receiver(bbzvm_error errcode);
// void set_color(uint8_t rgb);
uint8_t rand_soft();
void rand_seed(uint8_t s);
// void set_motors(int8_t m1, int8_t m2);
// void spinup_motors();
void takeoff();
void delay(uint16_t ms);
void delayMicroseconds(uint32_t micros);
uint64_t getMicroSeconds();

void setRobotId(uint8_t _id);
uint8_t getRobotId();

// void handleIncomingRadioMessage();
// void handleOutgoingRadioMessage();

void bbz_createPosObject();
void bbz_updatePosObject();


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* __BBZCRAZYFLIE_H */

/*********************************END OF FILE**********************************/
