#include <avr/pgmspace.h>
#include "bbzkilobot.h"

#define TBL_SZ 79

PROGMEM const uint8_t bcode[] = {(uint8_t)(_BBZSTRID_COUNT_),(uint8_t)((_BBZSTRID_COUNT_)>>8),0,1};
PROGMEM const uint16_t bcode_size = sizeof(bcode);

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
    bbzvm_function_register(__BBZSTRID_step, loop);
}

int main() {
    bbzkilo_init();
    bbzkilo_start(setup);

    return 0;
}
