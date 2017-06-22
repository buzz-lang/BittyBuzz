#include <avr/pgmspace.h>

#include <bittybuzz/bbzvm.h>
#include <bittybuzz/bbztype.h>
#include <bittybuzz/util/bbzstring.h>

#include "../lib/bbzkilobot.h"

bbzvm_t* vm;

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

void err_receiver(bbzvm_error errcode) {
    set_led(M); set_led(R); set_led(M);
    _delay_ms(1000.0);
    switch(errcode) {
        case BBZVM_ERROR_INSTR:  set_led(R);         break;
        case BBZVM_ERROR_STACK:  set_led(G); if (bbzvm_stack_size() >= BBZSTACK_SIZE) { set_led(R); } else if (bbzvm_stack_size() <= 0) { set_led(C); } else if (bbzvm_stack_size() + 5 >= BBZSTACK_SIZE) { set_led(Y); } break;
        case BBZVM_ERROR_LNUM:   set_led(B);         break;
        case BBZVM_ERROR_PC:     set_led(C);         break;
        case BBZVM_ERROR_FLIST:  set_led(Y);         break;
        case BBZVM_ERROR_TYPE:   set_led(R); set_led(G); break;
        case BBZVM_ERROR_STRING: set_led(G); set_led(B); break;
        case BBZVM_ERROR_SWARM:  set_led(B); set_led(C); break;
        case BBZVM_ERROR_MEM:    set_led(C); set_led(Y); break;
        default: set_led(M); set_led(M); break;
    }
    set_led(W);
    set_led(W);
    set_led(W);
}

void bbz_led() {
    bbzvm_assert_lnum(1);
    uint8_t color = (uint8_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    //set_led(color);
    set_color(RGB(color&1?3:0, color&2?3:0, color&4?3:0));
    //bin_count(color, 1);
    return bbzvm_ret0();
}

void bbz_delay() {
    bbzvm_assert_lnum(1);
    uint16_t d = (uint16_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value;
    delay(d);
    return bbzvm_ret0();
}

void bbz_setmotor() {
    bbzvm_assert_lnum(2);
    spinup_motors();
    set_motors((uint8_t)bbzvm_obj_at(bbzvm_lsym_at(1))->i.value, (uint8_t)bbzvm_obj_at(bbzvm_lsym_at(2))->i.value);
    return bbzvm_ret0();
}

void setup() {
    bbzvm_construct(kilo_uid);
    bbzvm_set_bcode(bcodeFetcher, bcode_size);
    bbzvm_set_error_receiver(err_receiver);
    bbzvm_function_register(BBZSTRING_ID(led), bbz_led);
    bbzvm_function_register(BBZSTRING_ID(delay), bbz_delay);
    bbzvm_function_register(BBZSTRING_ID(set_motor), bbz_setmotor);
}

void loop () {
    bbzvm_step();
    if (vm->state == BBZVM_STATE_DONE) {
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
    else if (vm->state == BBZVM_STATE_ERROR) {
        set_color(RGB(3,1,0));
    }
}

int main() {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzkilo_init();
    bbzkilo_start(setup, loop);

    return 0;
}