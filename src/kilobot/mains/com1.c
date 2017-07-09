/**
 * @file com1.c
 * @brief Test of recieving messages.
 */

#include <bbzkilobot.h>
#include <bbzkilobot_include.h>

uint8_t rcvd_msg;
void my_msg_rx(message_t* msg, distance_measurement_t* d) {
    rcvd_msg = 1;
}

void loop();

void setup() {
    bbzvm_function_register(BBZSTRING_ID(loop), loop);
}

void loop () {
    if (rcvd_msg == 1) {
        rcvd_msg = 0;
        set_color(RGB(0,3,0));
        delay(100);
    }
    else {
        set_color(RGB(0,0,0));
    }
    bbzvm_ret0();
}

int main() {
    bbzkilo_init();
    kilo_message_rx = my_msg_rx;
    bbzkilo_start(setup);

    return 0;
}