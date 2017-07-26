#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzheap_idx_t t = bbzvm_stack_at(0);
    bbzvm_pushs(strid);
    bbzvm_pushcc(fun);
    bbzvm_tput();
    bbzvm_push(t);
}

/****************************************/
/****************************************/

void bbztable_add_data(uint16_t strid, bbzheap_idx_t data) {
    bbzheap_idx_t t = bbzvm_stack_at(0);
    bbzvm_pushs(strid);
    bbzvm_push(data);
    bbzvm_tput();
    bbzvm_push(t);
}

/****************************************/
/****************************************/

void bbzutil_swapArrays(uint8_t *arr1, uint8_t *arr2, uint8_t size) {
    for (uint8_t i=0;i<size;++i) {
        arr1[i] ^= arr2[i];
        arr2[i] ^= arr1[i];
        arr1[i] ^= arr2[i];
    }
}