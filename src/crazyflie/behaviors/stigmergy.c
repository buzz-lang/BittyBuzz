#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "led.h"
#include "motors.h"

// extern Target currentGoal;
// extern Motor motorValues;
// extern uint8_t currentGoal_reached;

void bbz_led() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    ledSet(color&1?LINK_LED:LINK_DOWN_LED, 1);
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
//     uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
//     delay(d);
#endif
    bbzvm_ret0();
}

// void bbz_goto() {
//     bbzvm_assert_lnum(2);
//     int16_t x = bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
//     int16_t y = bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
// //     currentGoal.x = x;
// //     currentGoal.y = y;
//     currentGoal_reached = false;
// //     positionControl(currentGoal.x, currentGoal.y, currentGoal.angle, &motorValues, &currentGoal_reached, true, currentGoal.finalGoal, currentGoal.ignoreOrientation);
//     bbzvm_pushi(currentGoal_reached);
//     bbzvm_ret1();
// }

#define registerFunc(name) bbzvm_function_register(BBZSTRING_ID(name), bbz_##name)

void setup() {
    registerFunc(led);
    registerFunc(delay);
//     registerFunc(goto);
}

int main() {
    bbz_init(setup);

    return 0;
}