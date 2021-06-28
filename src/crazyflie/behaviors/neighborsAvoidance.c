#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "motors.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "stabilizer_types.h"
#include "commander.h"
#include "string.h"
#include "debug.h"
#include "log.h"
#include "crtp_commander_high_level.h"
#include "param.h"

void bbz_takeoff() {
#ifndef DEBUG
    bbzvm_assert_lnum(2);
    float height = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    int nb_of_steps = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;

    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = 0; i < nb_of_steps; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = ((float)i * height) / (float)nb_of_steps;

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
    free(setpoint);
#endif
    bbzvm_ret0();
}

void bbz_hover() {
#ifndef DEBUG
    bbzvm_assert_lnum(1);
    float height = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = 0; i < 5; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = height;

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
    free(setpoint);    
#endif
    bbzvm_ret0();
}

void bbz_spin() {
#ifndef DEBUG
    bbzvm_assert_lnum(2);
    float curHeight = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    int yaw = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = 0; i < 15; i++) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = curHeight;

        setpoint->mode.yaw = modeVelocity;
        setpoint->attitudeRate.yaw = -yaw;

        setpoint->mode.x = modeVelocity;
        setpoint->mode.y = modeVelocity;
        setpoint->velocity.x = 0.0;
        setpoint->velocity.y = 0.0;
        setpoint->velocity_body = true;

        commanderSetSetpoint(setpoint, 3);

        vTaskDelay(M2T(100));
    }
    free(setpoint);
#endif
    bbzvm_ret0();
}

void bbz_land() {
#ifndef DEBUG
    bbzvm_assert_lnum(2);
    float initHeight = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    int nb_of_steps = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = nb_of_steps; i >= 0; i--) {

        setpoint-> mode.z = modeAbs;
        setpoint->position.z = ((float)i * initHeight) / (float)nb_of_steps;

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
    motorsBeep(MOTOR_M1, false, G6, 0);
    motorsBeep(MOTOR_M2, false, G6, 0);
    motorsBeep(MOTOR_M3, false, G6, 0);
    motorsBeep(MOTOR_M4, false, G6, 0);
    free(setpoint);
#endif
    bbzvm_ret0();
}

void bbz_goTo() {
#ifndef DEBUG
    bbzvm_assert_lnum(3);
    float x = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    float y = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value / 1000.0;
    float z = (int)bbzheap_obj_at(bbzvm_locals_at(3))->i.value / 1000.0;
    
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    for (int i = 0; i < 30; i++) {
 
        setpoint->mode.x = modeAbs;
        setpoint->mode.y = modeAbs;
        setpoint->mode.z = modeAbs;
        setpoint->position.x = x;
        setpoint->position.y = y;
        setpoint->position.z = z;
 
        setpoint->mode.yaw = modeAbs;
        setpoint->attitude.yaw = 0.0;
 
        commanderSetSetpoint(setpoint, 3);
 
        vTaskDelay(M2T(100));
    }
    free(setpoint);
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
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(takeoff), bbz_takeoff);
    bbzvm_function_register(BBZSTRING_ID(land), bbz_land);
    bbzvm_function_register(BBZSTRING_ID(spin), bbz_spin);
    bbzvm_function_register(BBZSTRING_ID(hover), bbz_hover);
    bbzvm_function_register(BBZSTRING_ID(goTo), bbz_goTo);
}

int main() {
    bbz_init(setup);

    return 0;
}