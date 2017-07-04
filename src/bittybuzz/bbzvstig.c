#include "bbzvstig.h"
#include "bbzutil.h"

void bbzvstig_register() {
    bbzvm_pushs(__BBZSTRID_stigmergy);

    // Create the 'stigmergy' table and set its 'create' field.
    bbzvm_pusht();
    bbztable_add_function(__BBZSTRID_create, bbzvstig_create);

    // Construct the 'stigmergy' structure.
    vm->vstig.hpos = bbzvm_stack_at(0);
    bbzvstig_construct();

    // String 'stigmergy' is stack-top, and table is now stack #1. Register it.
    bbzvm_gstore();
}

/****************************************/
/****************************************/

void bbzvstig_create() {
    bbzvm_assert_lnum(1);

    // Empty the vstig.
    vm->vstig.size = 0;

    // Create a table, and register some fields in it.
    bbzvm_pusht();
    // TODO When creating a second vstig table, we might want the first one
    //      to have its id changed too.
    bbztable_add_data(__BBZSTRID_id, bbzvm_lsym_at(1));
    bbztable_add_function(__BBZSTRID_put,  bbzvstig_put);
    bbztable_add_function(__BBZSTRID_get,  bbzvstig_get);
    bbztable_add_function(__BBZSTRID_size, bbzvstig_size);


    // Table is now stack top. Return it.
    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzvstig_get() {
    bbzvm_assert_lnum(1);

    // Get args
    bbzheap_idx_t key = bbzvm_lsym_at(1);

    // TODO Communicate with other robots

    // Find the 'key' entry.
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        if (bbztype_cmp(
                bbzheap_obj_at(vm->vstig.data[i].key),
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Get it and exit.
            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_QUERY,
                                         vm->vstig.data[i].robot,
                                         bbzheap_obj_at(key)->s.value,
                                         vm->vstig.data[i].value,
                                         vm->vstig.data[i].timestamp);
            bbzvm_push(vm->vstig.data[i].value);
            bbzvm_ret1();
            return;
        }
    }

    // Entry not found. Push nil instead.
    bbzvm_pushnil();
    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_QUERY,
                                 vm->robot,
                                 bbzheap_obj_at(key)->s.value,
                                 vm->nil,
                                 0);

    bbzvm_ret1();
}

/****************************************/
/****************************************/

void bbzvstig_put() {
    bbzvm_assert_lnum(2);

    // Get args
    bbzheap_idx_t key   = bbzvm_lsym_at(1);
    bbzheap_idx_t value = bbzvm_lsym_at(2);

    // TODO Communicate with other robots

    // Find the 'key' entry.
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        if (bbztype_cmp(
                bbzheap_obj_at(vm->vstig.data[i].key),
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Set it and exit.
            vm->vstig.data[i].robot = vm->robot;
            vm->vstig.data[i].value = value;
            ++vm->vstig.data[i].timestamp;
            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                         vm->vstig.data[i].robot,
                                         bbzheap_obj_at(key)->s.value,
                                         vm->vstig.data[i].value,
                                         vm->vstig.data[i].timestamp);
            bbzvm_ret0();
            return;
        }
    }

    // No such entry found ; create it if we have enough space.
    if (vm->vstig.size < BBZVSTIG_CAP) {
        vm->vstig.data[vm->vstig.size].robot = vm->robot;
        vm->vstig.data[vm->vstig.size].key   = key;
        vm->vstig.data[vm->vstig.size].value = value;
        vm->vstig.data[vm->vstig.size].timestamp = 1;
        bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                     vm->vstig.data[vm->vstig.size].robot,
                                     bbzheap_obj_at(key)->s.value,
                                     vm->vstig.data[vm->vstig.size].value,
                                     vm->vstig.data[vm->vstig.size].timestamp);
        ++vm->vstig.size;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_VSTIG);
    }

    bbzvm_ret0();
}

/****************************************/
/****************************************/

void bbzvstig_size() {
    bbzvm_assert_lnum(0);
    bbzvm_pushi(vm->vstig.size);
    bbzvm_ret1();
}