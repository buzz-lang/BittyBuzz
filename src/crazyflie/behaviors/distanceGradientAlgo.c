#include "led.h"
#include "motors.h"

// /* ST includes */
// #include "stm32fxxx.h"

#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>

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

void bbz_min() {
    bbzvm_assert_lnum(2);
    bbzvm_lload(bbztype_cmp(bbzheap_obj_at(bbzvm_locals_at(1)),bbzheap_obj_at(bbzvm_locals_at(2)))>0?2:1);
    bbzvm_ret1();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_pusht(); // "math"
    bbztable_add_function(BBZSTRING_ID(min), bbz_min);
    bbzvm_gsym_register(BBZSTRING_ID(math), bbzvm_stack_at(0));
    bbzvm_pop();
}

int main() {
 bbz_init(setup);
    return 0;
}