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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bittybuzz/bbzfloat.h>
#include "crtp_commander_high_level.h"

uint16_t idRange = 0;
uint16_t range = 0;

float VELOCITY = 0.5f;
uint16_t PROXIMITY_LIMIT = 300;
float TAKEOFF_HEIGHT = 0;

void bbz_get_up() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "up");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_get_right() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "right");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_get_left() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "left");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_get_front() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "front");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_get_back() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "back");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_get_z() {
    bbzvm_assert_lnum(0);
    idRange = logGetVarId("range", "zrange");
    range = logGetUint(idRange);
    bbzvm_pushi(range);
    bbzvm_ret1();
}

void bbz_print() {
    bbzvm_assert_lnum(1);
    int val = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    DEBUG_PRINT("%d\n", val);
    bbzvm_ret0();
}

void bbz_takeoff() {
    bbzvm_assert_lnum(1);
    TAKEOFF_HEIGHT = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    setpoint-> mode.z = modeAbs;
    setpoint->position.z = TAKEOFF_HEIGHT;

    setpoint->mode.yaw = modeVelocity;
    setpoint->attitudeRate.yaw = 0.0;

    setpoint->mode.x = modeVelocity;
    setpoint->mode.y = modeVelocity;
    setpoint->velocity.x = 0.0;
    setpoint->velocity.y = 0.0;
    setpoint->velocity_body = true;

    commanderSetSetpoint(setpoint, 3);

    free(setpoint);
    bbzvm_ret0();
}

void bbz_land() {
    bbzvm_assert_lnum(0);
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    setpoint-> mode.z = modeAbs;
    setpoint->position.z = 0.0;

    setpoint->mode.yaw = modeVelocity;
    setpoint->attitudeRate.yaw = 0;

    setpoint->mode.x = modeVelocity;
    setpoint->mode.y = modeVelocity;
    setpoint->velocity.x = 0.0;
    setpoint->velocity.y = 0.0;
    setpoint->velocity_body = true;

    commanderSetSetpoint(setpoint, 3);

    motorsBeep(MOTOR_M1, false, G6, 0);
    motorsBeep(MOTOR_M2, false, G6, 0);
    motorsBeep(MOTOR_M3, false, G6, 0);
    motorsBeep(MOTOR_M4, false, G6, 0);
    free(setpoint);
    bbzvm_ret0();
}

void bbz_move_toward() {
    bbzvm_assert_lnum(2);
    float vx = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    float vy = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value / 1000.0;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

    setpoint-> mode.z = modeAbs;
    setpoint->position.z = TAKEOFF_HEIGHT;

    setpoint->mode.yaw = modeVelocity;
    setpoint->attitudeRate.yaw = 0.0;

    setpoint->mode.x = modeVelocity;
    setpoint->mode.y = modeVelocity;
    setpoint->velocity.x = vx;
    setpoint->velocity.y = vy;
    setpoint->velocity_body = true;

    commanderSetSetpoint(setpoint, 3);

    free(setpoint);
    bbzvm_ret0();
}

void bbz_hover() {
#ifndef DEBUG
    bbzvm_assert_lnum(1);
    float height = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));

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

    free(setpoint);    
#endif
    bbzvm_ret0();
}
void bbz_rand() {
    bbzvm_assert_lnum(0);
    int r = (int)((float)rand() * ((float)300 / (float)RAND_MAX)) - 150;
    bbzvm_pushi(r);
    bbzvm_ret1();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(rand_bbz), bbz_rand);
    bbzvm_function_register(BBZSTRING_ID(get_up), bbz_get_up);
    bbzvm_function_register(BBZSTRING_ID(get_left), bbz_get_left);
    bbzvm_function_register(BBZSTRING_ID(get_right), bbz_get_right);
    bbzvm_function_register(BBZSTRING_ID(get_front), bbz_get_front);
    bbzvm_function_register(BBZSTRING_ID(get_back), bbz_get_back);
    bbzvm_function_register(BBZSTRING_ID(get_z), bbz_get_z);
    bbzvm_function_register(BBZSTRING_ID(print), bbz_print);
    bbzvm_function_register(BBZSTRING_ID(takeoff), bbz_takeoff);
    bbzvm_function_register(BBZSTRING_ID(land), bbz_land);
    bbzvm_function_register(BBZSTRING_ID(move_toward), bbz_move_toward);
    
    bbzvm_function_register(BBZSTRING_ID(hover), bbz_hover);
}

int main() {
    srand(time(NULL));
    bbz_init(setup);

    return 0;
}