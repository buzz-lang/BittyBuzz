#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzheap_idx_t t = bbzvm_stack_at(0); // Keep track of the table
    bbzvm_pushs(strid); // Push string key
    bbzvm_pushcc(fun);  // Push C closure
    bbzvm_tput();       // Store in table, popping table, key and data
    bbzvm_push(t);      // Restore the table on the stack
}

/****************************************/
/****************************************/

void bbztable_add_data(uint16_t strid, bbzheap_idx_t data) {
    bbzheap_idx_t t = bbzvm_stack_at(0); // Keep track of the table
    bbzvm_pushs(strid); // Push string key
    bbzvm_push(data);   // Push data
    bbzvm_tput();       // Store in table, popping table, key and data
    bbzvm_push(t);      // Restore the table on the stack
}

/****************************************/
/****************************************/

bbzheap_idx_t bbztable_get_subfield(uint16_t strid) {
    bbzheap_idx_t ret = vm->nil;
    bbztable_get(bbzvm_stack_at(0), bbzstring_get(strid), &ret);
    /*
     * Here's an equivalent implementation that uses more stack:
     *
     * bbzheap_idx_t t = bbzvm_stack_at(0); // Keep track of the table
     * bbzvm_pushs(strid); // Push string key
     * bbzvm_tget();       // Get subfield of table
     * bbzheap_idx_t ret = bbzvm_stack_at(0);
     * bbzvm_pop();        // Pop subfield
     * bbzvm_push(t);      // Restore the table on the stack
     */
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