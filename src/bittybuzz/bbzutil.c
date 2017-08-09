#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzvm_dup();        // Duplicate table
    bbzvm_pushs(strid); // Push string key
    bbzvm_pushcc(fun);  // Push C closure
    bbzvm_tput();       // Store in table, popping table, key and data
}

/****************************************/
/****************************************/

void bbztable_add_data(uint16_t strid, bbzheap_idx_t data) {
    bbzvm_dup();        // Duplicate table
    bbzvm_pushs(strid); // Push string key
    bbzvm_push(data);   // Push data
    bbzvm_tput();       // Store in table, popping table, key and data
}

/****************************************/
/****************************************/

bbzheap_idx_t bbztable_get_subfield(uint16_t strid) {
    bbzvm_dup();        // Duplicate table
    bbzvm_pushs(strid); // Push string key
    bbzvm_tget();       // Get subfield of table
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_pop();        // Pop subfield
    return ret;
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