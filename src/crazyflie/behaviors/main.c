// /* Personal configs */
// #include "FreeRTOSConfig.h"
// 
// /* FreeRtos includes */
// #include "FreeRTOS.h"
// #include "task.h"
// 
// /* Project includes */
// #include "config.h"
// #include "platform.h"
// #include "system.h"
// #include "usec_time.h"

#include "led.h"
#include "motors.h"

// /* ST includes */
// #include "stm32fxxx.h"

#include <bbzsymbols.h>
#include <bittybuzz/bbzutil.h>
#include <bittybuzz/util/bbzstring.h>
#include <bbzcrazyflie.h>

// void bbz_takeoff() {
//     bbzvm_assert_lnum(1);
//     motorsPlayTone(G6, 1000);
//     bbzvm_ret0();
// }

void setup() {
//     bbzvm_function_register(BBZSTRING_ID(takeoff), bbz_takeoff);
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


