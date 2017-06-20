#include <bittybuzz/bbzneighbors.h>

#define NUM_TEST_CASES 10
#define TEST_MODULE neighbors
#include "testingconfig.h"

bbzvm_t vmObj;
bbzvm_t* vm = &vmObj;

uint8_t buf[4] = {0,0,0,0};
const uint8_t* bcodefetcher(int16_t offset, uint8_t size) {
    return buf + offset;
}

uint8_t addWorks = 0;

TEST(nadd) {
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(vm->neighbors.count, 1);
    addWorks = (uint8_t)(vm->neighbors.count == 1);

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(broadcast) {
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_broadcast);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_pushi(42);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

void listen_fun() {
    bbzvm_assert_lnum(3);

    // TODO

    bbzvm_ret0();
}

TEST(listen) {
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_listen);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_pushcc(listen_fun);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(ignore) {
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_listen);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_pushcc(listen_fun);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_ignore);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    bbzheap_idx_t dummy;
    ASSERT(!bbztable_get(vm->neighbors.listeners, 0, &dummy));

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(get) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_get);
    bbzvm_tget();
    bbzvm_pushi(1);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT(bbzvm_stack_at(0) != vm->nil);

    bbzvm_gc();
    bbzvm_destruct();
}

void foreach_fun() {
    bbzvm_assert_lnum(2);

    // TODO

    bbzvm_ret0();
}

TEST(foreach) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_foreach);
    bbzvm_tget();
    bbzvm_pushcc(foreach_fun);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

void map_fun() {
    bbzvm_assert_lnum(2);

    // ...
    bbzvm_lload(1);

    bbzvm_ret1();
}

TEST(map) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_map);
    bbzvm_tget();
    bbzvm_pushcc(map_fun);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

void reduce_fun() {
    bbzvm_assert_lnum(3);

    bbzvm_obj_at(bbzvm_lsym_at(3))->i.value += 1;

    bbzvm_lload(3);
    bbzvm_ret1();
}

TEST(reduce) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_reduce);
    bbzvm_tget();
    bbzvm_pushcc(reduce_fun);
    bbzvm_pushi(0);
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzvm_obj_at(ret)->i.value, 2);

    bbzvm_gc();
    bbzvm_destruct();
}

void filter_fun() {
    bbzvm_assert_lnum(2);

    // ...
    bbzvm_pushi((int16_t) (bbzvm_obj_at(bbzvm_lsym_at(1))->i.value % 2));

    bbzvm_ret1();
}

TEST(filter) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_map);
    bbzvm_tget();
    bbzvm_pushcc(filter_fun);
    bbzvm_closure_call(1);
    ASSERT(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(count) {
    REQUIRE(addWorks);
    bbzvm_construct(0);
    bbzvm_set_bcode(bcodefetcher, 4);

    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem);
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_pushs(__BBZSTRID_map);
    bbzvm_tget();
    bbzvm_closure_call(0);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzvm_obj_at(bbzvm_stack_at(0))->i.value, 2);

    bbzvm_gc();
    bbzvm_destruct();
}

//------------------------
//------------------------

TEST_LIST {
    ADD_TEST(nadd);
    ADD_TEST(broadcast);
    ADD_TEST(listen);
    ADD_TEST(ignore);
    ADD_TEST(get);
    ADD_TEST(foreach);
    ADD_TEST(map);
    ADD_TEST(reduce);
    ADD_TEST(filter);
    ADD_TEST(count);
}