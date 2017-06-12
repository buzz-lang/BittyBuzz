#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzvm_dup();
    bbzvm_pushs(strid);
    bbzheap_idx_t cc = bbzvm_function_register(-1, fun);
    bbzvm_push(cc);
    bbzvm_tput();
}

/****************************************/
/****************************************/

void bbztable_add_data(uint16_t strid, bbzheap_idx_t data) {
    bbzvm_dup();
    bbzvm_pushs(strid);
    bbzvm_push(data);
    bbzvm_tput();
}