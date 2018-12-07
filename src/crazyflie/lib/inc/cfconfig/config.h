/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * config.h - Main configuration file
 *
 * This file define the default configuration of the copter
 * It contains two types of parameters:
 * - The global parameters are globally defined and independent of any
 *   compilation profile. An example of such define could be some pinout.
 * - The profiled defines, they are parameter that can be specific to each
 *   dev build. The vanilla build is intended to be a "customer" build without
 *   fancy spinning debugging stuff. The developers build are anything the
 *   developer could need to debug and run his code/crazy stuff.
 *
 * The golden rule for the profile is NEVER BREAK ANOTHER PROFILE. When adding a
 * new parameter, one shall take care to modified everything necessary to
 * preserve the behavior of the other profiles.
 *
 * For the flag. T_ means task. H_ means HAL module. U_ would means utils.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "nrf24l01.h"

#include "trace.h"
#include "usec_time.h"

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "__cross_studio_io.h"

/* Exported macro ------------------------------------------------------------*/
#define BIT(x) (1<<(x))
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))


#define PROTOCOL_VERSION 4

#ifdef STM32F4XX
 // #ifndef P_NAME
 //   #define P_NAME "Crazyflie 2.0"
 // #endif
  #define QUAD_FORMATION_X

  #define CONFIG_BLOCK_ADDRESS    (2048 * (64-1))
  #define MCU_ID_ADDRESS          0x1FFF7A10
  #define MCU_FLASH_SIZE_ADDRESS  0x1FFF7A22
  #define FREERTOS_HEAP_SIZE      40000
  #define FREERTOS_MIN_STACK_SIZE 150       // M4-FPU register setup is bigger so stack needs to be bigger. Originally: 150
  #define FREERTOS_MCU_CLOCK_HZ   168000000

  #define configGENERATE_RUN_TIME_STATS 1
  #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() initUsecTimer()
  #define portGET_RUN_TIME_COUNTER_VALUE() usecTimestamp()
#endif

// Task priorities. Higher number higher priority
#define STABILIZER_TASK_PRI     5
#define SENSORS_TASK_PRI        4
#define ADC_TASK_PRI            3
#define FLOW_TASK_PRI           3
#define MULTIRANGER_TASK_PRI    3
#define SYSTEM_TASK_PRI         2
#define CRTP_TX_TASK_PRI        2
#define CRTP_RX_TASK_PRI        2
#define EXTRX_TASK_PRI          2
#define ZRANGER_TASK_PRI        2
#define ZRANGER2_TASK_PRI       2
#define LOG_TASK_PRI            1
#define MEM_TASK_PRI            1
#define PARAM_TASK_PRI          1
#define PROXIMITY_TASK_PRI      0
#define PM_TASK_PRI             0
#define USDLOG_TASK_PRI         1
#define USDWRITE_TASK_PRI       0
#define PCA9685_TASK_PRI        3
#define CMD_HIGH_LEVEL_TASK_PRI 2
#define BBZ_TASK_PRI            1

#define SYSLINK_TASK_PRI        3
#define USBLINK_TASK_PRI        3

// Not compiled
#if 0
  #define INFO_TASK_PRI           2
  #define PID_CTRL_TASK_PRI       2
#endif


// Task names
#define SYSTEM_TASK_NAME        "SYSTEM"
#define ADC_TASK_NAME           "ADC"
#define PM_TASK_NAME            "PWRMGNT"
#define CRTP_TX_TASK_NAME       "CRTP-TX"
#define CRTP_RX_TASK_NAME       "CRTP-RX"
#define CRTP_RXTX_TASK_NAME     "CRTP-RXTX"
#define LOG_TASK_NAME           "LOG"
#define MEM_TASK_NAME           "MEM"
#define PARAM_TASK_NAME         "PARAM"
#define SENSORS_TASK_NAME       "SENSORS"
#define STABILIZER_TASK_NAME    "STABILIZER"
#define NRF24LINK_TASK_NAME     "NRF24LINK"
#define ESKYLINK_TASK_NAME      "ESKYLINK"
#define SYSLINK_TASK_NAME       "SYSLINK"
#define USBLINK_TASK_NAME       "USBLINK"
#define PROXIMITY_TASK_NAME     "PROXIMITY"
#define EXTRX_TASK_NAME         "EXTRX"
#define UART_RX_TASK_NAME       "UART"
#define ZRANGER_TASK_NAME       "ZRANGER"
#define ZRANGER2_TASK_NAME      "ZRANGER2"
#define FLOW_TASK_NAME          "FLOW"
#define USDLOG_TASK_NAME        "USDLOG"
#define USDWRITE_TASK_NAME      "USDWRITE"
#define PCA9685_TASK_NAME       "PCA9685"
#define CMD_HIGH_LEVEL_TASK_NAME "CMDHL"
#define MULTIRANGER_TASK_NAME   "MR"
#define BBZ_TASK_NAME            "BBZ"

//Task stack sizes
#define SYSTEM_TASK_STACKSIZE         (2* configMINIMAL_STACK_SIZE)
#define ADC_TASK_STACKSIZE            configMINIMAL_STACK_SIZE
#define PM_TASK_STACKSIZE             configMINIMAL_STACK_SIZE
#define CRTP_TX_TASK_STACKSIZE        configMINIMAL_STACK_SIZE
#define CRTP_RX_TASK_STACKSIZE        (2* configMINIMAL_STACK_SIZE)
#define CRTP_RXTX_TASK_STACKSIZE      configMINIMAL_STACK_SIZE
#define LOG_TASK_STACKSIZE            configMINIMAL_STACK_SIZE
#define MEM_TASK_STACKSIZE            (2 * configMINIMAL_STACK_SIZE)
#define PARAM_TASK_STACKSIZE          configMINIMAL_STACK_SIZE
#define SENSORS_TASK_STACKSIZE        (2 * configMINIMAL_STACK_SIZE)
#define STABILIZER_TASK_STACKSIZE     (3 * configMINIMAL_STACK_SIZE)
#define NRF24LINK_TASK_STACKSIZE      configMINIMAL_STACK_SIZE
#define ESKYLINK_TASK_STACKSIZE       configMINIMAL_STACK_SIZE
#define SYSLINK_TASK_STACKSIZE        configMINIMAL_STACK_SIZE
#define USBLINK_TASK_STACKSIZE        configMINIMAL_STACK_SIZE
#define PROXIMITY_TASK_STACKSIZE      configMINIMAL_STACK_SIZE
#define EXTRX_TASK_STACKSIZE          configMINIMAL_STACK_SIZE
#define UART_RX_TASK_STACKSIZE        configMINIMAL_STACK_SIZE
#define ZRANGER_TASK_STACKSIZE        (2 * configMINIMAL_STACK_SIZE)
#define ZRANGER2_TASK_STACKSIZE       (2 * configMINIMAL_STACK_SIZE)
#define FLOW_TASK_STACKSIZE           (2 * configMINIMAL_STACK_SIZE)
#define USDLOG_TASK_STACKSIZE         (2 * configMINIMAL_STACK_SIZE)
#define USDWRITE_TASK_STACKSIZE       (2 * configMINIMAL_STACK_SIZE)
#define PCA9685_TASK_STACKSIZE        (2 * configMINIMAL_STACK_SIZE)
#define CMD_HIGH_LEVEL_TASK_STACKSIZE configMINIMAL_STACK_SIZE
#define MULTIRANGER_TASK_STACKSIZE    (2 * configMINIMAL_STACK_SIZE)
#define BBZ_TASK_STACKSIZE            (3 * configMINIMAL_STACK_SIZE)

///////////////////////////////////
//    COMMUNICATION CONSTANTS    //
///////////////////////////////////
#define ROBOT_ID                              ( RID )  //this should be different for each robot
#define RADIO_CHANNEL                         80 //The radio channel. From 0 to 125
#define RADIO_DATARATE                        RADIO_RATE_250K
#define RADIO_ADDRESS                         0xE7E7E7E7E7ULL
#define PAYLOAD_SIZE                          8  //size of the data from master to slave
#define PAYLOAD_MAX_SIZE                      32
#define COMMUNICATION_TIMEOUT                 2000 //in ms

#define RECEIVER_ID                           250

///////// MESSAGE TYPES ///////////
//Operating messages
#define TYPE_UPDATE                           0x03
#define TYPE_STATUS                           0x04
#define TYPE_MOTORS_VELOCITY                  0x05
#define TYPE_ROBOT_POSITION                   0x06
#define TYPE_BBZ_MESSAGE                      0x10


/**
 * \def PROPELLER_BALANCE_TEST_THRESHOLD
 * This is the threshold for a propeller/motor to pass. It calculates the variance of the accelerometer X+Y
 * when the propeller is spinning.
 */
#define PROPELLER_BALANCE_TEST_THRESHOLD  2.5f

/**
 * \def ACTIVATE_AUTO_SHUTDOWN
 * Will automatically shot of system if no radio activity
 */
//#define ACTIVATE_AUTO_SHUTDOWN

/**
 * \def ACTIVATE_STARTUP_SOUND
 * Playes a startup melody using the motors and PWM modulation
 * ACTIVATE_WJ_SOUND for fun. Choose either 1.
 */

// #define ACTIVATE_STARTUP_SOUND
#define ACTIVATE_WJ_SOUND

// Define to force initialization of expansion board drivers. For test-rig and programming.
//#define FORCE_EXP_DETECT

/**
 * \def PRINT_OS_DEBUG_INFO
 * Print with an interval information about freertos mem/stack usage to console.
 */
//#define PRINT_OS_DEBUG_INFO


//Debug defines
//#define BRUSHLESS_MOTORCONTROLLER
//#define ADC_OUTPUT_RAW_DATA
//#define UART_OUTPUT_TRACE_DATA
//#define UART_OUTPUT_RAW_DATA_ONLY
//#define IMU_OUTPUT_RAW_DATA_ON_UART
//#define T_LAUCH_MOTORS
//#define T_LAUCH_MOTOR_TEST
//#define MOTOR_RAMPUP_TEST
/**
 * \def ADC_OUTPUT_RAW_DATA
 * When defined the gyro data will be written to the UART channel.
 * The UART must be configured to run really fast, e.g. in 2Mb/s.
 */
//#define ADC_OUTPUT_RAW_DATA

#if defined(UART_OUTPUT_TRACE_DATA) && defined(ADC_OUTPUT_RAW_DATA)
#  error "Can't define UART_OUTPUT_TRACE_DATA and ADC_OUTPUT_RAW_DATA at the same time"
#endif

#if defined(UART_OUTPUT_TRACE_DATA) || defined(ADC_OUTPUT_RAW_DATA) || defined(IMU_OUTPUT_RAW_DATA_ON_UART)
#define UART_OUTPUT_RAW_DATA_ONLY
#endif

#if defined(UART_OUTPUT_TRACE_DATA) && defined(T_LAUNCH_ACC)
#  error "UART_OUTPUT_TRACE_DATA and T_LAUNCH_ACC doesn't work at the same time yet due to dma sharing..."
#endif

typedef struct {
	uint16_t positionX;
	uint16_t positionY;
	uint8_t colorRed;
	uint8_t colorGreen;
	uint8_t colorBlue;
	uint8_t preferredSpeed;
	int16_t orientation;
	bool isFinalGoal;
	uint8_t empty;
        bool ignoreOrientation;
}PositionControlMessage;

typedef struct
{
  uint16_t x;
  uint16_t y;
  float angle;
  bool finalGoal;
  bool ignoreOrientation;
} Target;

typedef struct
{
  uint16_t x;
  uint16_t y;
} Position;

typedef struct
{
  uint8_t type;
  uint8_t id;
} Header;

typedef struct
{
  Header header;
  uint8_t payload[PAYLOAD_MAX_SIZE-sizeof(Header)];
} Message;

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
    int16_t T;
} ImuRawData;

typedef struct
{
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float T;
} MotionData;

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
} AccelRawData;

typedef struct
{
    int16_t gx;
    int16_t gy;
    int16_t gz;
} GyroRawData;

typedef struct
{
    float w;
    float x;
    float y;
    float z;
} Quaternion;

typedef enum {
	DISCONNECTED 		 = 0x00,
	CHARGING 		 = 0x01,
        CHARGED 		 = 0x02,
} CHARGING_STATE_t;

typedef struct
{
  uint8_t id;
  uint64_t pipeAddress;
} Robot;

#endif /* CONFIG_H_ */
