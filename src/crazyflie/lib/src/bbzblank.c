// #include <functions.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bbzcrazyflie.h>
// #include "led.h"
#include "debug.h"

#define DEBUG_MODULE "MAIN"

#define TBL_SZ 79

__attribute__((section(".bcode.data")))
const uint8_t bcode[] = {(uint8_t)(_BBZSTRID_COUNT_),(uint8_t)((_BBZSTRID_COUNT_)>>8),0,1};
const uint16_t bcode_size = sizeof(bcode);

void loop() {
    bbzvm_gc();
//     setColor(red);
//     delay(75);
//     setColor(pink);
//     delay(75);
//     setColor(purple);
//     delay(75);
//     setColor(blue);
//     delay(75);
//     setColor(cyan);
//     delay(75);
//     setColor(light_blue);
//     delay(75);
//     setColor(green);
//     delay(75);
//     setColor(yellow);
//     delay(75);
//     setColor(orange);
//     delay(75);
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(__BBZSTRID_step, loop);
}

int main() {
    DEBUG_PRINT("Initializing system...\n");
    bbz_init();
    bbz_start(setup);

    return 0;
}
