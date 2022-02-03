#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSConfig.h"
#include "debug.h"

TickType_t t = NULL;
void bbz_led() {
    bbzvm_assert_lnum(2);
#ifndef DEBUG
    uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    uint8_t value = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    DEBUG_PRINT("color: %d\n", color);
    ledSet(color?LINK_LED:LINK_DOWN_LED, value);
#endif
    bbzvm_ret0();
}

void bbz_print() {
    bbzvm_assert_lnum(1);
    int val = (int)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    DEBUG_PRINT("WInner: %d\n", val);
    bbzvm_ret0();
}

void bbz_show() {
#ifndef DEBUG
    uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    if (t == NULL) {
        DEBUG_PRINT("ERROR\n");
    } else if (d == 1) {
        t = xTaskGetTickCount() - t;
        DEBUG_PRINT("CONSENSUS: %d\n", t);
    } else {
        DEBUG_PRINT("CONSENSUS: %d\n", t);
    }
    DEBUG_PRINT("SHOW\n");
#endif
    bbzvm_ret0();
}

void bbz_go() {
    // if (t == NULL) {
    DEBUG_PRINT("START\n");
    t = xTaskGetTickCount();
    // }
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(print), bbz_print);
    bbzvm_function_register(BBZSTRING_ID(show), bbz_show);
    bbzvm_function_register(BBZSTRING_ID(go), bbz_go);
}

int main() {
    bbz_init(setup);

    return 0;
}