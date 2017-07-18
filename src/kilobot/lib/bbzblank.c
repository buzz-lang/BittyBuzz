#include <avr/pgmspace.h>

#include <bittybuzz/bbzvm.h>
#include <bittybuzz/util/bbzstring.h>

#include "bbzkilobot.h"

#define TBL_SZ 79

uint8_t buf[4];
const uint8_t* bcodeFetcher(int16_t offset, uint8_t size) {
    switch(size) {
        case 1:
            *((uint8_t*)buf) = pgm_read_byte((uint16_t)&bcode + sizeof(*bcode)*offset);
            break;
        case 2:
            *((uint16_t*)buf) = pgm_read_word((uint16_t)&bcode + sizeof(*bcode)*offset);
            break;
        case 4:
            *((uint32_t*)buf) = pgm_read_dword((uint16_t)&bcode + sizeof(*bcode)*offset);
            break;
        default:
            break;
    }
    return buf;
}

void bbzvm_set_color() {
    bbzvm_assert_lnum(3);
    set_color(RGB(bbzvm_lsym_at(1),bbzvm_lsym_at(2),bbzvm_lsym_at(3)));
    return bbzvm_ret0();
}

void led() {
    bbzvm_assert_lnum(1);
    uint8_t color = bbzvm_lsym_at(1);
    set_color(RGB(color&1?3:0, color&2?3:0, color&4?3:0));
    return bbzvm_ret0();
}

void setup() {
    bbzvm_construct(kilo_uid);
    bbzvm_set_bcode(bcodeFetcher, bcode_size);
    bbzvm_set_error_receiver(NULL);
    bbzvm_function_register(BBZSTRING_ID(led), led);
    //bbzvm_execute_script();
}

void loop() {
    set_color(RGB(3,0,0));
    delay(75);
    set_color(RGB(2,0,1));
    delay(75);
    set_color(RGB(1,0,2));
    delay(75);
    set_color(RGB(0,0,3));
    delay(75);
    set_color(RGB(0,1,2));
    delay(75);
    set_color(RGB(0,2,1));
    delay(75);
    set_color(RGB(0,3,0));
    delay(75);
    set_color(RGB(1,2,0));
    delay(75);
    set_color(RGB(2,1,0));
    delay(75);


}

int main() {
    bbzvm_t vmObj;
    vm = &vmObj;

    bbzkilo_init();
    bbzkilo_start(setup);

    return 0;
}
