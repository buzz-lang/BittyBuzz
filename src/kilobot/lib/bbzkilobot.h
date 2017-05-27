/**
 * @file bbzkilobot.h
 * @brief Definition of Kilobot's library used in the BittyBuzz's vm
 * implementation for Kilobots.
 */

#ifndef BITTYBUZZ_BBZKILOBOT_H
#define BITTYBUZZ_BBZKILOBOT_H

typedef void (*bbzkilo_setup_func)();

/**
 * @brief Initialize kilobot hardware.
 *
 * TODO Change the list of what this function does.
 * This function initializes all hardware of the kilobots. This includes
 * calibrating the hardware oscillator, setting hardware timers,
 * configuring ports, setting up analog-to-digital converters,
 * registering system interrupts and the initializing the messaging
 * subsystem.
 *
 * It is recommended that you call this function as early as possible
 * inside the `main` function of your program.
 */
void bbzkilo_init();

/**
 * @brief Start bittybuzz event loop.
 *
 * This function receives one parameter, @p setup, which is a function
 * that will be called once to perform any initialization required by
 * the bittybuzz VM, usually C closure registering.
 *
 * Using the overhead controller it is possible to interrupt the event
 * loop to trigger events such as program start, program reset, and
 * program restart.
 *
 * @param setup put your setup code here, to be run only once.
 *
 * @code
 *
 * uint8_t math_sin() {
 *    // ...
 * }
 *
 * void setup() {
 *    bbzkilo_function_register(sin, math_sin);
 * }
 *
 * int main() {
 *   bbzkilo_init();
 *   bbzkilo_start(setup);
 *   return 0;
 * }
 * @endcode
 */
void bbzkilo_start(bbzkilo_setup_func setupFunc);

#endif // !BITTYBUZZ_BBZKILOBOT_H
