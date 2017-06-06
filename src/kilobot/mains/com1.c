#include <bbzkilobot.h>
#include <bbzkiloreg.h>

uint8_t rcvd_msg;
void my_msg_rx(message_t* msg, distance_measurement_t* d) {
    rcvd_msg = 1;
}

void setup() { }

void loop () {
    if (rcvd_msg == 1) {
        rcvd_msg = 0;
        set_color(RGB(0,3,0));
        delay(100);
    }
    else {
        set_color(RGB(0,0,0));
    }
}

int main() {
    bbzkilo_init();
    kilo_message_rx = my_msg_rx;
    bbzkilo_start(setup, loop);

    return 0;
}