#include "led.h"
#include "motors.h"

// /* ST includes */
// #include "stm32fxxx.h"

#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>


void bbz_takeoff() {
    bbzvm_assert_lnum(2);
    uint16_t motorsEnable = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
    if (motorsEnable != 0) {
        motorsPlayTone((uint16_t)C4, (uint16_t)5000);
    }
    else {
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
    }
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(takeoff), bbz_takeoff);
}

int main() 
{    
  bbz_init(setup);
//   bbz_start(setup);
  
  //TODO: Move to platform launch failed
  ledInit();
  ledSet(0, 1);
  ledSet(1, 1);
  
  //Should never reach this point!
//   while(1);
// 
  return 0;
}


