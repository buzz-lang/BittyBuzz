#include <bittybuzz/bbzneighbors.h>

#define NUM_TEST_CASES 11
#define TEST_MODULE neighbors
#include "testingconfig.h"

bbzvm_t vmObj;

TEST(nadd) {
    vm = &vmObj;
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
#ifndef BBZ_XTREME_MEMORY
    ASSERT_EQUAL(vm->neighbors.count, 1);
#else
    ASSERT_EQUAL(bbzringbuf_size(&vm->neighbors.rb), 1);
#endif

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(broadcast) {
    bbzvm_construct(0);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
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

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
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

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
    bbzvm_pushs(__BBZSTRID_listen);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_pushcc(listen_fun);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
    bbzvm_pushs(__BBZSTRID_ignore);
    bbzvm_tget();
    bbzvm_pushs(0);
    bbzvm_closure_call(1);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    bbzvm_pushs(0);
    bbzheap_idx_t topic = bbzvm_stack_at(0);
    bbzvm_pop();
    bbzheap_idx_t dummy;
    ASSERT(!bbztable_get(vm->neighbors.listeners, topic, &dummy));

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(get) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup();
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

    bbzvm_assert_type(bbzvm_locals_at(1), BBZTYPE_INT);
    bbzvm_assert_type(bbzvm_locals_at(2), BBZTYPE_TABLE);
    bbzheap_idx_t dist_idx;
    if (bbztable_get(bbzvm_locals_at(2), bbzstring_get(__BBZSTRID_distance), &dist_idx)) {
#ifndef BBZ_NEIGHBORS_USE_FLOATS
        bbzvm_assert_type(dist_idx, BBZTYPE_INT);
        bbzvm_assert_exec(bbzheap_obj_at(dist_idx)->i.value > 0)
#else // !BBZ_NEIGHBORS_USE_FLOATS
        bbzvm_assert_type(dist_idx, BBZTYPE_FLOAT);
        bbzvm_assert_exec(bbzfloat_tofloat(bbzheap_obj_at(dist_idx)->f.value) > 0.f, BBZVM_ERROR_MATH);
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    }
    if (bbztable_get(bbzvm_locals_at(2), bbzstring_get(__BBZSTRID_azimuth), &dist_idx)) {
#ifndef BBZ_NEIGHBORS_USE_FLOATS
        bbzvm_assert_type(dist_idx, BBZTYPE_INT);
        bbzvm_assert_exec(bbzheap_obj_at(dist_idx)->i.value == 0)
#else // !BBZ_NEIGHBORS_USE_FLOATS
        bbzvm_assert_type(dist_idx, BBZTYPE_FLOAT);
        bbzvm_assert_exec(bbzfloat_tofloat(bbzheap_obj_at(dist_idx)->f.value) == 0.f, BBZVM_ERROR_MATH);
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    }

    bbzvm_ret0();
}

TEST(foreach) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
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
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
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

    bbzheap_obj_at(bbzvm_locals_at(3))->i.value += 1;

    bbzvm_lload(3);
    bbzvm_ret1();
}

TEST(reduce) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup();
    bbzvm_pushs(__BBZSTRID_reduce);
    bbzvm_tget();
    bbzvm_pushcc(reduce_fun);
    bbzvm_pushi(0);
    bbzheap_idx_t ret = bbzvm_stack_at(0);
    bbzvm_closure_call(2);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzheap_obj_at(ret)->i.value, 2);

    bbzvm_gc();
    bbzvm_destruct();
}

void filter_fun() {
    bbzvm_assert_lnum(2);

    // ...
    bbzvm_pushi((int16_t) (bbzheap_obj_at(bbzvm_locals_at(1))->i.value % 2));

    bbzvm_ret1();
}

TEST(filter) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup(); // Push self table
    bbzvm_pushs(__BBZSTRID_map);
    bbzvm_tget();
    bbzvm_pushcc(filter_fun);
    bbzvm_closure_call(1);
    ASSERT(vm->state != BBZVM_STATE_ERROR);

    bbzvm_gc();
    bbzvm_destruct();
}

TEST(count) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem2);

    bbzvm_push(vm->neighbors.hpos);
    bbzvm_dup();
    bbzvm_pushs(__BBZSTRID_count);
    bbzvm_tget();
    bbzvm_closure_call(0);
    REQUIRE(vm->state != BBZVM_STATE_ERROR);
    ASSERT_EQUAL(bbzheap_obj_at(bbzvm_stack_at(0))->i.value, 2);

    bbzvm_gc();
    bbzvm_destruct();
}

#ifndef BBZ_XTREME_MEMORY
#define data_gc_count vm->neighbors.count
#else // !BBZ_XTREME_MEMORY
#define data_gc_count bbzringbuf_size(&vm->neighbors.rb)
#endif // !BBZ_XTREME_MEMORY

TEST(data_gc) {
    bbzvm_construct(0);

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=64,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem2 = {.robot=2,.distance=bbzfloat_fromint(64),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem2);
    REQUIRE(data_gc_count == 1);

    vm->neighbors.clear_counter = BBZNEIGHBORS_MARK_TIME-1;

#ifndef BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=127,.azimuth=0,.elevation=0};
    bbzneighbors_elem_t elem3 = {.robot=3,.distance=100,.azimuth=0,.elevation=0};
#else // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_elem_t elem = {.robot=1,.distance=bbzfloat_fromint(127),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
    bbzneighbors_elem_t elem3 = {.robot=3,.distance=bbzfloat_fromint(100),.azimuth=bbzfloat_fromint(0),.elevation=bbzfloat_fromint(0)};
#endif // !BBZ_NEIGHBORS_USE_FLOATS
    bbzneighbors_add(&elem);
    bbzneighbors_add(&elem3);
    REQUIRE(data_gc_count == 3);

    bbzneighbors_data_gc();
    ASSERT_EQUAL(data_gc_count, 2);
}

#undef data_gc_count

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
    ADD_TEST(data_gc);
}
