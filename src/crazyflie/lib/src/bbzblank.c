#include "led.h"

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
bbz_init(setup);
/*
      int err = platformInit();
  if (err != 0) {
    // The firmware is running on the wrong hardware. Halt
    while(1);
  }
    
  // Initializes the system onboard CF
  systemLaunch();
    
  // Start the FreeRTOS scheduler
  vTaskStartScheduler();*/
  
  //TODO: Move to platform launch failed
  ledInit();
  ledSet(0, 1);
  ledSet(1, 1);
  
  //Should never reach this point!
  while(1);
// 
  return 0;
}
