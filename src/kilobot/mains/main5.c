#include <bbzkilobot.h>
#include <bbzkilobot_include.h>
#include <bittybuzz/bbzutil.h>

void ___led(uint8_t x) {set_color(x); delay(50); set_color(0); delay(100);}

void err_receiver(bbzvm_error errcode) {
    set_motors(0,0);
    uint8_t i;
    for (i = 4; i; --i) {
        ___led(RGB(1,2,0));
    }
    delay(300);
#if 1
    for (i = 4; i; --i) {
        delay(700);
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
//    ___led(RGB(2,2,2));
//    ___led(RGB(2,2,2));
}

void bbz_led() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    const uint8_t color = (uint8_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    set_color((uint8_t)RGB(color&1?3:0, color&2?3:0, color&4?3:0));
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    const uint16_t d = (uint16_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    delay(d);
#endif
    bbzvm_ret0();
}

void bbz_min() {
    bbzvm_assert_lnum(2);
    bbzvm_lload(bbztype_cmp(bbzheap_obj_at(bbzvm_lsym_at(1)),bbzheap_obj_at(bbzvm_lsym_at(2)))>0?2:1);
    bbzvm_ret1();
}

void bbz_show_dist() {
    bbzvm_assert_lnum(1);
    const int16_t dist = bbzheap_obj_at(bbzvm_lsym_at(1))->i.value;
    if (dist > 500) {
        set_color(RGB(2,0,2));
    }
    else if (dist > 400) {
        set_color(RGB(0,0,2));
    }
    else if (dist > 300) {
        set_color(RGB(0,2,1));
    }
    else if (dist > 200) {
        set_color(RGB(0,3,0));
    }
    else if (dist > 100) {
        set_color(RGB(1,2,0));
    }
    else if (dist > -1) {
        set_color(RGB(2,0,0));
    }
    else {
        set_color(0);
    }
    bbzvm_ret0();
}

#define funreg_PASTER(NAME) bbzvm_function_register(BBZSTRING_ID(NAME), bbz_ ## NAME);
#define funreg(NAME) funreg_PASTER(NAME)
void setup() {
    bbzvm_set_error_receiver(err_receiver);
    funreg(led);
    funreg(delay);
//    funreg(show_dist);
    bbzvm_pusht(); // "math"
    bbztable_add_function(BBZSTRING_ID(min), bbz_min);
    bbzvm_gsym_register(BBZSTRING_ID(math), bbzvm_stack_at(0));
    bbzvm_pop();
}

int main() {
    bbzkilo_init();
    bbzkilo_start(setup);
    return 0;
}