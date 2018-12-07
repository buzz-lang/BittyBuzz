#include "led.h"
#include "motors.h"

// /* ST includes */
// #include "stm32fxxx.h"

#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bittybuzz/bbzvm.h>
#include <bbzcrazyflie.h>

void bbz_led() {
    bbzvm_assert_lnum(1);
// #ifndef DEBUG
// //     uint8_t color = (uint8_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
// //     set_color(RGB(color&1?3:0, color&2?3:0, color&4?3:0));
// #endif
    bbzvm_ret0();
}
// 
void bbz_delay() {
    bbzvm_assert_lnum(1);
// #ifndef DEBUG
// //     uint16_t d = (uint16_t)bbzheap_obj_at(bbzvm_locals_at(1))->i.value;
// //     delay(d);
// #endif
    bbzvm_ret0();
}

void setup() {
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
}

int main() 
{
  bbz_init(setup);
//   bbz_start(setup);
//   int err = platformInit();
//   if (err != 0) {
//     // The firmware is running on the wrong hardware. Halt
//     while(1);
//   }
//   // Initializes the system onboard CF
//   systemLaunch();
//   
//   // Start the FreeRTOS scheduler
//   vTaskStartScheduler();
//   
  //TODO: Move to platform launch failed
  ledInit();
  ledSet(0, 1);
  ledSet(1, 1);
  
  //Should never reach this point!
  while(1);
// 
  return 0;
}
