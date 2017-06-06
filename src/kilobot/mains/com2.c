#include <bbzkilobot.h>
#include <bbzkiloreg.h>
#include <bbzmessage_crc.h>

message_t msg;
message_t* my_msg_tx() {
    return &msg;
}

uint8_t sent_msg = 0;
void my_tx_msg_success() {
     sent_msg = 1;
}

void setup() {
    msg.type = NORMAL;
    msg.crc = bbzmessage_crc(&msg);
}

void loop () {
    if (sent_msg) {
        sent_msg = 0;
        set_color(RGB(3,0,0));
        delay(100);
        set_color(RGB(0,0,0));
    }
}

int main() {
    bbzkilo_init();
    kilo_message_tx = my_msg_tx;
    kilo_message_tx_success = my_tx_msg_success;
    msg.data[0] = 1;
    bbzkilo_start(setup, loop);

    return 0;
}