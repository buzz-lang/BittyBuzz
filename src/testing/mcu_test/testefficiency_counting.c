#include <bittybuzz/bbzvm.h>
#include <avr/pgmspace.h>

#include "ressources/testefficiency_counting.h"

uint8_t bcode_buf[4];

const uint8_t* bcode_fetcher(int16_t offset, uint8_t size) {
    if (offset + size <= vm->bcode_size) {
        for (uint8_t i = 0; i < size; ++i) {
            uint16_t base_addr = (uint16_t)&bcode + offset;
            bcode_buf[i] = pgm_read_byte(base_addr + i);
        }
    }
    else {
        // Error. Clear the bytecode buffer.
        *(uint32_t*)bcode_buf = 0;
    }
    return bcode_buf;
}

void err_reciever(bbzvm_error errcode) {
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

bbzvm_state bbz_led() {
    bbzvm_lload(1);
    int8_t color = (int8_t)bbzvm_obj_at(bbzvm_stack_at(0))->i.value;
    set_led(color);
    return bbzvm_ret0();
}


bbzheap_idx_t led_c;

void setup() {

    set_led(M);
    set_led(M);

    bbzvm_construct(0);
    bbzvm_set_error_notifier(&err_reciever);
    bbzvm_set_bcode(&bcode_fetcher, bcode_size);

    led_c = bbzvm_function_register(0, &bbz_led);

    bbzvm_execute_script();

    bbzvm_destruct();
}

void loop() {

    // bbzvm_push(led_c);
    // bbzvm_pushi(B);
    // bbzvm_pushi(1);
    // bbzvm_callc();

    // bbzvm_push(led_c);
    // bbzvm_pushi(G);
    // bbzvm_pushi(1);
    // bbzvm_callc();

    // bbzvm_gc();
}

bbzvm_t vmObj;
bbzvm_t* vm = (bbzvm_t*)0x0118; // FIXME Yay for unreliable hard-coded values!

int main() {

    setup();
    while (1) {
        loop();
    }

    return 0;
}