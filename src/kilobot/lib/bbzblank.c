#include <avr/pgmspace.h>
#include "bbzkilobot.h"

#define TBL_SZ 79

PROGMEM const uint8_t bcode[] = {(uint8_t)(_BBZSTRID_COUNT_),(uint8_t)((_BBZSTRID_COUNT_)>>8),0,1};
PROGMEM const uint16_t bcode_size = sizeof(bcode);

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
}

void loop() {
    bbzvm_gc();
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
    bbzvm_ret0();
}

void setup() {
    bbzvm_set_error_receiver(err_receiver);
    bbzvm_function_register(__BBZSTRID_step, loop);
}

int main() {
    bbzkilo_init();
    bbzkilo_start(setup);

    return 0;
}
