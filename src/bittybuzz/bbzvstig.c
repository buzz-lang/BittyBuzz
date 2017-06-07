#include "bbzvstig.h"
#include "bbzstrids.h"

void add_function(uint16_t symid, bbzvm_funp fun) {
    bbzvm_dup();
    bbzvm_pushs(symid);
    bbzheap_idx_t cc = bbzvm_function_register(-1, fun);
    bbzvm_push(cc);
    bbzvm_tput();
}

void add_data(uint16_t symid, bbzheap_idx_t data) {
    bbzvm_dup();
    bbzvm_pushs(symid);
    bbzvm_push(data);
    bbzvm_tput();
}

/****************************************/
/****************************************/

void bbzvstig_register() {
    // Create the 'stigmergy' table and set its 'create' field.
    bbzvm_pusht();
    add_function(__BBZSTRID_CREATE, bbzvstig_create);

    // 'stigmergy' table is now stack top. Register it.
    bbzvm_pushs(__BBZSTRID_STIGMERGY);
    bbzvm_gstore();
}

/****************************************/
/****************************************/

void bbzvstig_create() {
    bbzvm_lload(1);

    // Empty the vstig.
    vm->vstig.size = 0;

    // Create a table, and register some fields in it.
    bbzvm_pusht();
    // TODO When creating a second vstig table, we might want the first one
    //      to have its id changed too.
    add_data(__BBZSTRID_ID, bbzvm_stack_at(0));
    add_function(__BBZSTRID_PUT,  bbzvstig_put);
    add_function(__BBZSTRID_GET,  bbzvstig_get);
    add_function(__BBZSTRID_SIZE, bbzvstig_size);


    // Table is now stack top. Return it.
    return bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzvstig_get() {
    bbzvm_lload(1);
    bbzheap_idx_t key = bbzvm_stack_at(0);

    // TODO Communicate with other robots

    // Find the 'key' entry.
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        if (bbztype_cmp(
                bbzheap_obj_at(vm->vstig.data[i].key),
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Get it and exit.
            bbzvm_push(vm->vstig.data[i].value);
            return bbzvm_ret1();
        }
    }

    // Entry not found. Push nil instead.
    bbzvm_pushnil();

    return bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzvstig_put() {
    bbzvm_lload(1);
    bbzvm_lload(2);
    bbzheap_idx_t key   = bbzvm_stack_at(0);
    bbzheap_idx_t value = bbzvm_stack_at(1);

    // TODO Communicate with other robots

    // Find the 'key' entry.
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        if (bbztype_cmp(
                bbzheap_obj_at(vm->vstig.data[i].key),
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Set it and exit.
            vm->vstig.data[i].value = value;
            return bbzvm_ret0();
        }
    }

    // No such entry found ; create it if we have enough space.
    if (vm->vstig.size < BBZVSTIG_CAP) {
        vm->vstig.data[vm->vstig.size].key   = key;
        vm->vstig.data[vm->vstig.size].value = value;
        ++vm->vstig.size;
        // TODO Timestamp & robot ID
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_VSTIG);
    }

    return bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzvstig_size() {
    bbzvm_pushi(vm->vstig.size);
    return bbzvm_ret1();
}