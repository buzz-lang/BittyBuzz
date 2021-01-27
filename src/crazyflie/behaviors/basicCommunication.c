#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "led.h"
#include "motors.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "debug.h"
#include "power_distribution.h"
#include "commander.h"
#include "stabilizer_types.h"

void bbz_led() {
    bbzvm_assert_lnum(2);
#ifndef DEBUG
    uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    uint8_t value = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    DEBUG_PRINT("color: %d", color);
    ledSet(color?LINK_LED:LINK_DOWN_LED, value);
#endif
    bbzvm_ret0();
}

void bbz_takeoff() {
#ifndef DEBUG
    setpoint_t setpoint;
    memset(&setpoint, 0, sizeof(setpoint_t));
    setpoint.mode.x = modeAbs;
    setpoint.mode.y = modeAbs;
    setpoint.mode.z = modeAbs;

    setpoint.position.x = 0;
    setpoint.position.y = 0;
    setpoint.position.z = 2;

    setpoint.mode.yaw = modeAbs;
    setpoint.attitude.yaw = 0;
    commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_EXTRX);
    // uint16_t speed = (uint16_t)(MOTORS_TIM_BEEP_CLK_FREQ / A4)/ 250;
    // for(int i = 1; i <= speed; i += 2) {
    //     motorsBeep(MOTOR_M1, false, G6, i);
    //     motorsBeep(MOTOR_M2, false, G6, i);
    //     motorsBeep(MOTOR_M3, false, G6, i);
    //     motorsBeep(MOTOR_M4, false, G6, i);
    //     vTaskDelay(M2T(100));
    // }
    // motorsBeep(MOTOR_M1, false, G6, speed);
    // motorsBeep(MOTOR_M2, false, G6, speed);
    // motorsBeep(MOTOR_M3, false, G6, speed);
    // motorsBeep(MOTOR_M4, false, G6, speed);
    
    
#endif
    bbzvm_ret0();
}

void bbz_land() {
#ifndef DEBUG
    // uint16_t speed = (uint16_t)(MOTORS_TIM_BEEP_CLK_FREQ / A4)/ 250;
    // for(int i = speed; i >= 0; i -= 2) {
    //     motorsBeep(MOTOR_M1, false, G6, i);
    //     motorsBeep(MOTOR_M2, false, G6, i);
    //     motorsBeep(MOTOR_M3, false, G6, i);
    //     motorsBeep(MOTOR_M4, false, G6, i);
    //     vTaskDelay(M2T(100));
    // }
    // motorsBeep(MOTOR_M1, false, G6, 0);
    // motorsBeep(MOTOR_M2, false, G6, 0);
    // motorsBeep(MOTOR_M3, false, G6, 0);
    // motorsBeep(MOTOR_M4, false, G6, 0);
    setpoint_t setpoint;
    memset(&setpoint, 0, sizeof(setpoint_t));
    setpoint.mode.x = modeAbs;
    setpoint.mode.y = modeAbs;
    setpoint.mode.z = modeAbs;

    setpoint.position.x = 0;
    setpoint.position.y = 0;
    setpoint.position.z = 0;

    setpoint.mode.yaw = modeAbs;
    setpoint.attitude.yaw = 0;
    commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_EXTRX);
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    vTaskDelay(M2T(d));
#endif
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(takeoff), bbz_takeoff);
    bbzvm_function_register(BBZSTRING_ID(land), bbz_land);
}

int main() {
    bbz_init(setup);

    return 0;
}