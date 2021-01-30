#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "motors.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"

void bbz_takeoff() {
#ifndef DEBUG
    uint16_t speed = (uint16_t)(0.6*(1<<16));
    motorsSetRatio(MOTOR_M1, speed);
    motorsSetRatio(MOTOR_M2, speed);
    motorsSetRatio(MOTOR_M3, speed);
    motorsSetRatio(MOTOR_M4, speed);
#endif
    bbzvm_ret0();
}

void bbz_land() {
#ifndef DEBUG
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