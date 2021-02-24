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

void bbz_takeoff() {
#ifndef DEBUG
    // setpoint_t *setpoint = malloc(sizeof(setpoint_t));
    // memset(setpoint, 0, sizeof(setpoint_t));
    // for (int i =0; i< 60; i++){ //takeoff;
    //     setpoint->mode.z = modeAbs;
    //     setpoint->position.z = 0.4;

    //     setpoint->mode.yaw = modeVelocity;
    //     setpoint->attitudeRate.yaw = 0;

    //     setpoint->mode.x = modeVelocity;
    //     setpoint->mode.y = modeVelocity;
    //     setpoint->velocity.x = 0;
    //     setpoint->velocity.y = 0;

    //     setpoint->velocity_body = true;

    //     commanderSetSetpoint(setpoint, 3);
	// 	vTaskDelay(M2T(100));
	// }
    // memset(setpoint, 0, sizeof(setpoint_t));
    // commanderSetSetpoint(setpoint, 3);
    // uint16_t speed = (uint16_t)(0.6*(1<<16));
    // motorsSetRatio(MOTOR_M1, speed);
    // motorsSetRatio(MOTOR_M2, speed);
    // motorsSetRatio(MOTOR_M3, speed);
    // motorsSetRatio(MOTOR_M4, speed);
#endif
    bbzvm_ret0();
}

void bbz_land() {
#ifndef DEBUG
    // for (int i =0; i< 10; i++){ //takeoff;
    //     setpoint_t *setpoint = malloc(sizeof(setpoint_t));
    //     memset(setpoint, 0, sizeof(setpoint_t));
    //     setpoint->mode.z = modeAbs;
    //     setpoint->position.z = 0.0;

    //     setpoint->mode.x = modeVelocity;   			
    //     setpoint->mode.y = modeVelocity;			
    //     setpoint->velocity.x = 0;					
    //     setpoint->velocity.y = 0;	
    //     setpoint->velocity_body = true;

    //     commanderSetSetpoint(setpoint, 3);
	// 	vTaskDelay(M2T(100));
	// }
    motorsSetRatio(MOTOR_M1, 0);
    motorsSetRatio(MOTOR_M2, 0);
    motorsSetRatio(MOTOR_M3, 0);
    motorsSetRatio(MOTOR_M4, 0);
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
}

int main() {
    bbz_init(setup);

    return 0;
}