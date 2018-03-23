/**
  ******************************************************************************
  * @file    bbzzooids.h 
  * @author  Emir Khaled Belhaddad
  * @version V0.0.1
  * @date    7-Feburary-2018
  * @brief   Header for bbzzooids.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BBZZOOIDS_H
#define __BBZZOOIDS_H

/* Includes ------------------------------------------------------------------*/
#include "functions.h"
#include "position_control.h"
#include <bittybuzz/bbzvm.h>
#include <bittybuzz/util/bbzstring.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
const uint8_t* bbzzooids_bcodeFetcher(bbzpc_t offset, uint8_t size);
void bbz_init(void);
void bbz_start(void (*setup)(void));
void bbz_err_receiver(bbzvm_error errcode);
void set_color(uint8_t rgb);
uint8_t rand_soft();
void rand_seed(uint8_t s);
void set_motors(uint8_t m1, uint8_t m2);
void spinup_motors();

#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* __BBZZOOIDS_H */

/*********************************END OF FILE**********************************/
