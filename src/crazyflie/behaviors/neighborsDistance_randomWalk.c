#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "led.h"
#include "motors.h"


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
//     const uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
//     delay(d);
#endif
    bbzvm_ret0();
}

void bbz_min() {
    bbzvm_assert_lnum(2);
    bbzvm_lload(bbztype_cmp(bbzheap_obj_at(bbzvm_locals_at(1)),bbzheap_obj_at(bbzvm_locals_at(2)))>0?2:1);
    bbzvm_ret1();
}

void bbz_rand() {
    bbzvm_assert_lnum(0);
//     bbzvm_pushi(((rand_soft() & 0x7F) << 8) | rand_soft());
    bbzvm_ret1();
}
static uint8_t motor_state = 0;

void bbz_forward() {
    if (motor_state != 0) {
//         spinup_motors();
//         set_motors(MAX_SPEED, MAX_SPEED);
        motor_state = 0;
    }
    bbzvm_ret0();
}

void bbz_right() {
    if (motor_state != 1) {
//         spinup_motors();
//         set_motors(-MAX_SPEED/2, MAX_SPEED/2);
        motor_state = 1;
    }
    bbzvm_ret0();
}

void bbz_left() {
    if (motor_state != 2) {
//         spinup_motors();
//         set_motors(MAX_SPEED/2, -MAX_SPEED/2);
        motor_state = 2;
    }
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(rand), bbz_rand);
    bbzvm_function_register(BBZSTRING_ID(forward), bbz_forward);
    bbzvm_function_register(BBZSTRING_ID(right), bbz_right);
    bbzvm_function_register(BBZSTRING_ID(left), bbz_left);
//     rand_seed(rand_soft());
//     spinup_motors();
//     set_motors(20, 20);
}

int main() {
    bbz_init(setup);
    
    return 0;
}