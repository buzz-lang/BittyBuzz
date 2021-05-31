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
int done = 0;
void bbz_led() {
    bbzvm_assert_lnum(2);
#ifndef DEBUG
    uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    uint8_t value = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(2))->i.value;
    ledSet(color?LINK_LED:LINK_DOWN_LED, value);
#endif
    bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    vTaskDelay(M2T(d));
#endif
    bbzvm_ret0();
}

void bbz_print() {
    bbzvm_assert_lnum(1);
#ifndef DEBUG
    uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    if (t == NULL) {
        t = xTaskGetTickCount();
    } else if(d == 42 && done == 0) {
        t = xTaskGetTickCount() - t;
        DEBUG_PRINT("CONSENSUS: %d\n", t);
        done = 1;
    } else if (done == 1) {
        DEBUG_PRINT("%d", t);
    }
    DEBUG_PRINT("PRINT: %d\n", d);
#endif
    bbzvm_ret0();
}

void setup() {
    // bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    // bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(show), bbz_print);
}

int main() {
    bbz_init(setup);

    return 0;
}