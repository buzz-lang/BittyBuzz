
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SENSORS_H
#define __SENSORS_H

#ifdef __cplusplus
 extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "config.h"
#include "utilities.h"
#include "qt1070.h"
#include "lsm6ds3.h"
#include "MadgwickAHRS.h"
#include "RingBuffer.h"

bool initSensors();
void initPhotoDiodes();
void updateOrientation();
void initBatteryMonitor();
void readPhotoDiode(uint8_t photodiodeId);
bool calculatePhotodiodePosition(uint8_t photodiodeId);
bool updateRobotPosition();
Position* getRobotPosition();
float* getRobotAngle();
uint16_t getBatteryLevel();
void startReadBatteryLevel();

bool isBlinded();


#ifdef __cplusplus
}
#endif

#endif /* __USB_H */
