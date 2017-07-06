#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzvm_dup();
    bbzvm_pushs(strid);
    bbzvm_pushcc(fun);
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