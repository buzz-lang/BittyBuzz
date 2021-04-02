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

#include <string.h>

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
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = 0; i <= 10; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = i/25.0;

        setpoint->mode.yaw = modeVelocity;
        setpoint->attitudeRate.yaw = 0.0;

        setpoint->mode.x = modeVelocity;
        setpoint->mode.y = modeVelocity;
        setpoint->velocity.x = 0.0;
        setpoint->velocity.y = 0.0;
        setpoint->velocity_body = true;

        commanderSetSetpoint(setpoint, 3);

        vTaskDelay(M2T(100));
    }    
    
#endif
    bbzvm_ret0();
}

void bbz_move_to() {
#ifndef DEBUG

    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));
    
    for (int i = 0; i < 20; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = 0.4;

        setpoint->mode.yaw = modeVelocity;
        setpoint->attitudeRate.yaw = 0.0;

        setpoint->mode.x = modeVelocity;
        setpoint->mode.y = modeVelocity;
        setpoint->velocity.x = 0.0;
        setpoint->velocity.y = 0.0;
        setpoint->velocity_body = true;

        commanderSetSetpoint(setpoint, 3);

        vTaskDelay(M2T(100));
    }

    for (int i = 0; i < 50; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = 0.4;

        setpoint->mode.yaw = modeVelocity;
        setpoint->attitudeRate.yaw = 36*2;

        setpoint->mode.x = modeVelocity;
        setpoint->mode.y = modeVelocity;
        setpoint->velocity.x = 0.5;
        setpoint->velocity.y = 0.0;
        setpoint->velocity_body = true;

        commanderSetSetpoint(setpoint, 3);

        vTaskDelay(M2T(100));
    }

    for (int i = 0; i < 10; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = (10.0-i)/25.0;

        setpoint->mode.yaw = modeVelocity;
        setpoint->attitudeRate.yaw = 0;

        setpoint->mode.x = modeVelocity;
        setpoint->mode.y = modeVelocity;
        setpoint->velocity.x = 0.0;
        setpoint->velocity.y = 0.0;
        setpoint->velocity_body = true;

        commanderSetSetpoint(setpoint, 3);

        vTaskDelay(M2T(100));
    }
    
#endif
}

void bbz_land() {
#ifndef DEBUG
    motorsBeep(MOTOR_M1, false, G6, 0);
    motorsBeep(MOTOR_M2, false, G6, 0);
    motorsBeep(MOTOR_M3, false, G6, 0);
    motorsBeep(MOTOR_M4, false, G6, 0);
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
    bbzvm_function_register(BBZSTRING_ID(move_to), bbz_move_to);
}

int main() {
    bbz_init(setup);

    return 0;
}