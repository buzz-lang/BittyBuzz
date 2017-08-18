#include <bittybuzz/bbzvstig.h>

#define TEST_MODULE bbzvstig
#define NUM_TEST_CASES 4
#include "testingconfig.h"

bbzvm_t vmObj;
bbzvm_t* vm = &vmObj;

uint8_t buf[4] = {0,0,0,0};
const uint8_t* bcodefetcher(bbzpc_t offset, uint8_t size) {
    RM_UNUSED_WARN(size);
    return buf + offset;
}

uint8_t createWorks = 0;
TEST(vstig_create) {
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_pushs(__BBZSTRID_create);
    bbzvm_tget();
    bbzvm_pushi(0);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT(vm->vstig.hpos > vm->nil);
    bbzvm_pushs(__BBZSTRID_put);
    bbzheap_idx_t dummy = 0;
    REQUIRE(bbztable_get(bbzvm_stack_at(1),bbzvm_stack_at(0),&dummy));
    createWorks = 1;

    bbzvm_gc();
    bbzvm_destruct();
}

uint8_t putWorks = 0;
TEST(vstig_put) {
    REQUIRE(createWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_pushs(__BBZSTRID_create);
    bbzvm_tget();
    bbzvm_pushi(0);
    bbzvm_closure_call(1);

    bbzvm_pushs(__BBZSTRID_put);
    bbzvm_tget();
    bbzvm_pushs(__BBZSTRID_data);
    bbzvm_pushi(42);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(vm->vstig.size, 1);
    ASSERT_EQUAL(vm->vstig.data[0].key, __BBZSTRID_data);
    ASSERT_EQUAL(bbzheap_obj_at(vm->vstig.data[0].value)->i.value, 42);
    putWorks = 1;

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(vstig_get) {
    REQUIRE(createWorks);
    REQUIRE(putWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_pushs(__BBZSTRID_create);
    bbzvm_tget();
    bbzvm_pushi(0);
    bbzvm_closure_call(1);
    bbzheap_idx_t vs = bbzvm_stack_at(0);

    bbzvm_pushs(__BBZSTRID_put);
    bbzvm_tget();
    bbzvm_pushs(__BBZSTRID_data);
    bbzvm_pushi(42);
    bbzvm_closure_call(2);

    bbzvm_push(vs);
    bbzvm_pushs(__BBZSTRID_get);
    bbzvm_tget();
    bbzvm_pushs(__BBZSTRID_data);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(0))->i.value, 42);

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(vstig_size) {
    REQUIRE(createWorks);
    REQUIRE(putWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->vstig.hpos);
    bbzvm_pushs(__BBZSTRID_create);
    bbzvm_tget();
    bbzvm_pushi(0);
    bbzvm_closure_call(1);
    bbzvm_dup(); // Keep on the stack for future use

    bbzvm_pushs(__BBZSTRID_put);
    bbzvm_tget();
    bbzvm_pushs(__BBZSTRID_data);
    bbzvm_pushi(42);
    bbzvm_closure_call(2);

    // We use the duplicated table here
    bbzvm_pushs(__BBZSTRID_size);
    bbzvm_tget();
    bbzvm_closure_call(0);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(0))->i.value, 1);

    bbzvm_gc();
    bbzvm_destruct();
}


TEST_LIST {
    ADD_TEST(vstig_create);
    ADD_TEST(vstig_put);
    ADD_TEST(vstig_get);
    ADD_TEST(vstig_size);
}