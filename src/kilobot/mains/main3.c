#include <avr/pgmspace.h>

#include <bbzkilobot.h>
#include <bbzkilobot_include.h>

void ___led(uint8_t x) {set_color(x); delay(10); set_color(0); delay(90);}

void err_receiver(bbzvm_error errcode) {
    uint8_t i;
    for (i = 4; i; --i) {
        ___led(RGB(1,2,0));
    }
    delay(300);
#if 1
    for (i = 4; i; --i) {
        delay(800);
        switch(errcode) {
            case BBZVM_ERROR_INSTR:  ___led(RGB(2,0,0)); ___led(RGB(2,0,0)); break;
            case BBZVM_ERROR_STACK:  ___led(RGB(1,2,0)); if (bbzvm_stack_size() >= BBZSTACK_SIZE) { ___led(RGB(0,3,0)); } else if (bbzvm_stack_size() <= 0) { ___led(RGB(2,0,0)); } else { ___led(RGB(1,2,0)); } break;
            case BBZVM_ERROR_LNUM:   ___led(RGB(3,1,0)); ___led(RGB(3,1,0)); break;
            case BBZVM_ERROR_PC:     ___led(RGB(0,3,0)); ___led(RGB(0,3,0)); break;
            case BBZVM_ERROR_FLIST:  ___led(RGB(0,3,0)); ___led(RGB(2,0,0)); break;
            case BBZVM_ERROR_TYPE:   ___led(RGB(0,3,0)); ___led(RGB(1,2,0)); break;
            case BBZVM_ERROR_RET:    ___led(RGB(0,3,0)); ___led(RGB(0,0,2)); break;
            case BBZVM_ERROR_STRING: ___led(RGB(0,2,1)); ___led(RGB(0,2,1)); break;
            case BBZVM_ERROR_SWARM:  ___led(RGB(0,2,1)); ___led(RGB(2,0,0)); break;
            case BBZVM_ERROR_VSTIG:  ___led(RGB(0,2,1)); ___led(RGB(1,2,0)); break;
            case BBZVM_ERROR_MEM:    ___led(RGB(0,2,1)); ___led(RGB(0,0,2)); break;
            case BBZVM_ERROR_MATH:   ___led(RGB(0,0,2)); ___led(RGB(0,0,2)); break;
            default: ___led(RGB(2,0,2)); ___led(RGB(2,0,2)); break;
        }
    }
#endif
    ___led(RGB(2,2,2));
    ___led(RGB(2,2,2));
    ___led(RGB(2,2,2));
    ___led(RGB(2,2,2));
}

void bbz_led() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint8_t color = (uint8_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    //set_led(color);
    set_color(RGB(color&1?3:0, color&2?3:0, color&4?3:0));
    //bin_count(color, 1);
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint16_t d = (uint16_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    delay(d);
#endif
    bbzvm_ret0();
}

void bbz_setmotor() {
    bbzvm_assert_lnum(2);
#ifndef DEBUG
    spinup_motors();
    set_motors((uint8_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value, (uint8_t)bbzvm_obj_at(bbzvm_lsym_at(2))->i.value);
#endif
    bbzvm_ret0();
}

void bbz_rand() {
    bbzvm_pushi(((uint16_t)rand_soft() << 8) | rand_soft());
    bbzvm_ret1();
}

void setup() {
    bbzvm_set_error_receiver(err_receiver);
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(set_motor), bbz_setmotor);
//    bbzvm_function_register(BBZSTRING_ID(rand), bbz_rand);
    set_color(RGB(3,0,0));
    delay(75);
    set_color(RGB(2,0,1));
    delay(75);
    set_color(RGB(1,0,2));
    delay(75);
    set_color(RGB(0,0,3));
    delay(75);
    set_color(RGB(0,1,2));
    delay(75);
    set_color(RGB(0,2,1));
    delay(75);
    set_color(RGB(0,3,0));
    delay(75);
    set_color(RGB(1,2,0));
    delay(75);
    set_color(RGB(2,1,0));
    delay(75);
    rand_seed(rand_hard());
//    delay((rand_soft()>>1) +1);
}

int main() {
    bbzkilo_init();
//    bbzvm_set_error_receiver(err_receiver);
    bbzkilo_start(setup);

    return 0;
}