#include <bbzkilobot.h>
#include <bbzkilobot_include.h>

void bbz_led() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    const uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_lsym_at(1))->i.value;
    set_color((uint8_t)RGB(color&1?3:0, color&2?3:0, color&4?3:0));
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    const uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_lsym_at(1))->i.value;
    delay(d);
#endif
    bbzvm_ret0();
}

void bbz_min() {
    bbzvm_assert_lnum(2);
    bbzvm_lload(bbztype_cmp(bbzheap_obj_at(bbzvm_lsym_at(1)),bbzheap_obj_at(bbzvm_lsym_at(2)))>0?2:1);
    bbzvm_ret1();
}

void bbz_rand() {
    bbzvm_assert_lnum(0);
    bbzvm_pushi(((rand_soft() & 0x7F) << 8) | rand_soft());
    bbzvm_ret1();
}
static uint8_t motor_state = 0;

void bbz_forward() {
    if (motor_state != 0) {
        spinup_motors();
        set_motors(kilo_straight_left, kilo_straight_right);
        motor_state = 0;
    }
    bbzvm_ret0();
}

void bbz_right() {
    if (motor_state != 1) {
        spinup_motors();
        set_motors(0, kilo_turn_right);
        motor_state = 1;
    }
    bbzvm_ret0();
}

void bbz_left() {
    if (motor_state != 2) {
        spinup_motors();
        set_motors(kilo_turn_left, 0);
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
    rand_seed(rand_hard());
    spinup_motors();
    set_motors(kilo_straight_left, kilo_straight_right);
}

int main() {
    bbzkilo_init();
    bbzkilo_start(setup);
    return 0;
}