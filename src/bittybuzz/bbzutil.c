#include "bbzutil.h"

void bbztable_add_function(uint16_t strid, bbzvm_funp fun) {
    bbzheap_idx_t t = bbzvm_stack_at(0);
    bbzvm_pushs(strid);
    bbzvm_pushcc(fun);
    bbzvm_tput();

    // Table popped ; push it again.
    bbzvm_push(t);
}

/****************************************/
/****************************************/

void bbztable_add_data(uint16_t strid, bbzheap_idx_t data) {
    bbzheap_idx_t t = bbzvm_stack_at(0);
    bbzvm_pushs(strid);
    bbzvm_push(data);
    bbzvm_tput();

    // Table popped ; push it again.
    bbzvm_push(t);
}