#include "led.h"
#include "motors.h"

// /* ST includes */
// #include "stm32fxxx.h"

#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>

#define TBL_SZ 79

__attribute__((section(".bcode.data")))
const uint8_t bcode[] = {(uint8_t)(_BBZSTRID_COUNT_),(uint8_t)((_BBZSTRID_COUNT_)>>8),0,1};
const uint16_t bcode_size = sizeof(bcode);

void loop() {
    bbzvm_gc();
    ledSet(LINK_LED, 1);
    ledSet(LINK_DOWN_LED, 1);
    ledSet(LINK_LED, 0);
    ledSet(LINK_DOWN_LED, 0);
    ledSet(LINK_LED, 1);
    ledSet(LINK_DOWN_LED, 1);
    ledSet(LINK_LED, 0);
    ledSet(LINK_DOWN_LED, 0);
    ledSet(LINK_LED, 1);
    ledSet(LINK_DOWN_LED, 1);
    ledSet(LINK_LED, 0);
    ledSet(LINK_DOWN_LED, 0);
    ledSet(LINK_LED, 1);
    ledSet(LINK_DOWN_LED, 1);
    ledSet(LINK_LED, 0);
    ledSet(LINK_DOWN_LED, 0);
    ledSet(LINK_LED, 1);
    ledSet(LINK_DOWN_LED, 1);
    ledSet(LINK_LED, 0);
    ledSet(LINK_DOWN_LED, 0);
    bbzvm_ret0();
}

void setup() {
    ledInit();
    bbzvm_function_register(__BBZSTRID_step, loop);
}

int main() {
  bbz_init(setup);
//   bbz_start(setup);
  
  //TODO: Move to platform launch failed
  ledInit();
  ledSet(0, 1);
  ledSet(1, 1);
  
  //Should never reach this point!
  while(1);
// 
  return 0;
}
