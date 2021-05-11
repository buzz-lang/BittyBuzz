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

uint16_t idUp = 0;
uint16_t idLeft = 0; 
uint16_t idRight = 0; 
uint16_t idFront = 0; 
uint16_t idBack = 0; 

uint16_t up = 0;  
uint16_t left = 0; 
uint16_t right = 0; 
uint16_t front = 0; 
uint16_t back = 0;

float VELOCITY = 0.5f;
uint16_t PROXIMITY_LIMIT = 300;

void handleObstacles(setpoint_t *setpoint) {
    up = logGetUint(idUp);
    left = logGetUint(idLeft);
    right = logGetUint(idRight);
    front = logGetUint(idFront);
    back = logGetUint(idBack);

    if (back <= PROXIMITY_LIMIT)
        setpoint->velocity.x += VELOCITY;
    if (front <= PROXIMITY_LIMIT)
        setpoint->velocity.x -= VELOCITY;
    if (left <= PROXIMITY_LIMIT)
        setpoint->velocity.y -= VELOCITY;
    if (right <= PROXIMITY_LIMIT)
        setpoint->velocity.y += VELOCITY;
}

void bbz_takeoff() {
#ifndef DEBUG
    bbzvm_assert_lnum(2);
    float height = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value / 1000.0;
    int nb_of_steps = (int)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    setpoint_t *setpoint = malloc(sizeof(sizeof(setpoint_t)));
    memset(setpoint, 0, sizeof(setpoint_t));
    
    idUp = logGetVarId("range", "up");
    idLeft = logGetVarId("range", "left");
    idRight = logGetVarId("range", "right");
    idFront = logGetVarId("range", "front");
    idBack = logGetVarId("range", "back");

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
        handleObstacles(setpoint);
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

        handleObstacles(setpoint);
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
}

int main() {
    bbz_init(setup);

    return 0;
}