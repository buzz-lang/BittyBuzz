#include "bbzvstig.h"
#include "bbzutil.h"
#include "bbzvm.h"
#include "bbztype.h"

#ifndef BBZ_DISABLE_VSTIGS

#define BBZVSTIG_ONCONFLICT_FIELD __BBZSTRID___INTERNAL_1_DO_NOT_USE__
#define BBZVSTIG_ONCONFLICTLOST_FIELD __BBZSTRID___INTERNAL_2_DO_NOT_USE__

void bbzvstig_register() {
    bbzvm_pushs(__BBZSTRID_stigmergy);

    bbzvstig_construct();
    // Create the 'stigmergy' table and set its 'create' field.
    bbzvm_pusht();
    bbztable_add_function(__BBZSTRID_create, bbzvstig_create);

    // Construct the 'stigmergy' structure.
    vm->vstig.hpos = bbzvm_stack_at(0);
    bbzheap_obj_make_permanent(*bbzheap_obj_at(vm->vstig.hpos));

    // String 'stigmergy' is stack-top, and table is now stack #1. Register it.
    bbzvm_gstore();
    bbzvm_gc();
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
    bbztable_add_data(__BBZSTRID_id, bbzvm_locals_at(1));
    bbztable_add_function(__BBZSTRID_put,  bbzvstig_put);
    bbztable_add_function(__BBZSTRID_get,  bbzvstig_get);
    bbzvm_gc();
    bbztable_add_function(__BBZSTRID_size, bbzvstig_size);
    bbztable_add_function(__BBZSTRID_onconflict, bbzvstig_onconflict);
    bbzvm_gc();
    bbztable_add_function(__BBZSTRID_onconflictlost, bbzvstig_onconflictlost);

    // Table is now stack top. Return it.
    bbzvm_ret1();

//    bbzvm_gc();
}

/****************************************/
/****************************************/

void bbzvstig_onconflict() {
    bbzvm_assert_lnum(1);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_gc();
    bbztable_add_data(BBZVSTIG_ONCONFLICT_FIELD, bbzvm_locals_at(1));
    bbzvm_gc();

    bbzvm_ret0();
    bbzvm_gc();
}

/****************************************/
/****************************************/

void bbzvstig_onconflictlost() {
    bbzvm_assert_lnum(1);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_gc();
    bbztable_add_data(BBZVSTIG_ONCONFLICTLOST_FIELD, bbzvm_locals_at(1));
    bbzvm_gc();

    bbzvm_ret0();
    bbzvm_gc();
}

/****************************************/
/****************************************/

void bbzvstig_get() {
    bbzvm_assert_lnum(1);

    // Get args
    bbzheap_idx_t key = bbzvm_locals_at(1);

    bbzvm_gc();

    // Find the 'key' entry.
    bbzobj_t tmp;
    bbztype_cast(tmp, BBZTYPE_STRING);
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        tmp.s.value = vm->vstig.data[i].key;
        if (bbztype_cmp(
                &tmp,
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Get it and exit.
            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_QUERY,
                                         vm->vstig.data[i].robot,
                                         vm->vstig.data[i].key,
                                         vm->vstig.data[i].value,
                                         vm->vstig.data[i].timestamp);
            bbzvm_push(vm->vstig.data[i].value);
            bbzvm_ret1();
            bbzvm_gc();
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
    bbzvm_gc();
}

/****************************************/
/****************************************/

void bbzvstig_put() {
    bbzvm_assert_lnum(2);

    // Get args
    bbzheap_idx_t key   = bbzvm_locals_at(1);
    bbzheap_idx_t value = bbzvm_locals_at(2);
    // BittyBuzz's virtual stigmertgie cannot handle composite types.
    bbzvm_assert_exec(!bbztype_istable(*bbzheap_obj_at(value)), BBZVM_ERROR_TYPE);

    bbzvm_gc();

    // Find the 'key' entry.
    bbzobj_t tmp;
    bbztype_cast(tmp, BBZTYPE_STRING);
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        tmp.s.value = vm->vstig.data[i].key;
        if (bbztype_cmp(
                &tmp,
                bbzheap_obj_at(key)) == 0) {
            // Entry found. Set it and exit.
            vm->vstig.data[i].robot = vm->robot;
            bbzheap_obj_unmake_permanent(*bbzheap_obj_at(vm->vstig.data[i].value));
            vm->vstig.data[i].value = value;
            bbzheap_obj_make_permanent(*bbzheap_obj_at(value));
            bbzvm_gc();
            ++vm->vstig.data[i].timestamp;
            bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                         vm->vstig.data[i].robot,
                                         vm->vstig.data[i].key,
                                         vm->vstig.data[i].value,
                                         vm->vstig.data[i].timestamp);
            bbzvm_ret0();
            bbzvm_gc();
            return;
        }
    }

    // No such entry found ; create it if we have enough space.
    if (vm->vstig.size < BBZVSTIG_CAP) {
        vm->vstig.data[vm->vstig.size].robot = vm->robot;
        vm->vstig.data[vm->vstig.size].key   = bbzheap_obj_at(key)->s.value;
        bbzheap_obj_make_permanent(*bbzheap_obj_at(key));
        vm->vstig.data[vm->vstig.size].value = value;
        bbzheap_obj_make_permanent(*bbzheap_obj_at(value));
        vm->vstig.data[vm->vstig.size].timestamp = 1;
        bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                     vm->vstig.data[vm->vstig.size].robot,
                                     vm->vstig.data[vm->vstig.size].key,
                                     vm->vstig.data[vm->vstig.size].value,
                                     vm->vstig.data[vm->vstig.size].timestamp);
        ++vm->vstig.size;
    }
    else {
        bbzvm_seterror(BBZVM_ERROR_VSTIG);
    }

    bbzvm_ret0();
    bbzvm_gc();
}

/****************************************/
/****************************************/

void bbzvstig_size() {
    bbzvm_assert_lnum(0);
    bbzvm_pushi(vm->vstig.size);
    bbzvm_ret1();
}
#endif // !BBZ_DISABLE_VSTIGS

void bbzvstig_dummy() {bbzvm_ret0();};