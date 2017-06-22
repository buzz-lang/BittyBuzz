#include <bittybuzz/bbzdarray.h>

#define NUM_TEST_CASES 10
#define TEST_MODULE darray
#include "testingconfig.h"

TEST(da_new) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);
    ASSERT_EQUAL(bbzdarray_size(darray), 0);
    ASSERT(bbzheap_aseg_hasnext(&bbzheap_obj_at(darray)->t));
}

TEST(da_push) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o;
    bbzheap_obj_alloc(BBZTYPE_INT, &o);
    bbzint_t* io = (bbzint_t*)bbzheap_obj_at(o);
    io->value = 10;
    bbzdarray_push(darray, o);
    ASSERT_EQUAL(bbzdarray_size(darray), 1);
    ASSERT_EQUAL(bbzheap_aseg_elem_get(bbzheap_aseg_at(bbzheap_obj_at(darray)->t.value)->values[0]), o);
}

TEST(da_find) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);
    uint16_t o;
    bbzheap_obj_alloc(BBZTYPE_INT, &o);
    bbzint_t* io = (bbzint_t*)bbzheap_obj_at(o);
    io->value = 10;
    bbzdarray_push(darray, o);
    REQUIRE(bbzdarray_size(darray) == 1);

    ASSERT_EQUAL(bbzdarray_find(darray, bbztype_cmp, o), 0);
}

TEST(da_set) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);
    uint16_t o;
    bbzheap_obj_alloc(BBZTYPE_INT, &o);
    bbzint_t* io = (bbzint_t*)bbzheap_obj_at(o);
    io->value = 10;
    bbzdarray_push(darray, o);
    REQUIRE(bbzdarray_size(darray) == 1);

    uint16_t o2;
    bbzheap_obj_alloc(BBZTYPE_INT, &o2);
    bbzint_t* io2 = (bbzint_t*)bbzheap_obj_at(o2);
    io2->value = 255;
    bbzdarray_set(darray, 0, o2);
    REQUIRE(bbzdarray_size(darray) == 1);
    ASSERT_EQUAL(bbzheap_aseg_elem_get(bbzheap_aseg_at(bbzheap_obj_at(darray)->t.value)->values[0]), o2);
    ASSERT_EQUAL(bbzheap_obj_at(bbzheap_aseg_elem_get(bbzheap_aseg_at(bbzheap_obj_at(darray)->t.value)->values[0]))->i.value, bbzheap_obj_at(o2)->i.value);

    io2->value = 15;
    ASSERT_EQUAL(bbzheap_obj_at(bbzheap_aseg_elem_get(bbzheap_aseg_at(bbzheap_obj_at(darray)->t.value)->values[0]))->i.value, 15);
}

TEST(da_push15x) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 15; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    ASSERT_EQUAL(bbzdarray_size(darray), 15);
}

TEST(da_pop7x) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 15; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    REQUIRE(bbzdarray_size(darray) == 15);

    for (int i = 0; i < 7; ++i) {
        bbzdarray_pop(darray);
    }
    ASSERT_EQUAL(bbzdarray_size(darray), 8);

    uint16_t stack[] = {darray};
    bbzheap_gc(stack, 1);
}

TEST(da_clear) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 15; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    REQUIRE(bbzdarray_size(darray) == 15);

    bbzdarray_clear(darray);
    ASSERT_EQUAL(bbzdarray_size(darray), 0);

    uint16_t stack[] = {darray};
    bbzheap_gc(stack, 1);
}

TEST(da_clone) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 22; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    REQUIRE(bbzdarray_size(darray) == 22);

    uint16_t darray2 = darray;
    bbzdarray_clone(darray, &darray2);
    ASSERT_EQUAL(bbzdarray_size(darray2), 22);
    bbzheap_idx_t o1, o2;
    for (uint16_t i = 0; i < bbzdarray_size(darray); ++i) {
        bbzdarray_get(darray, i, &o1);
        REQUIRE(bbzdarray_get(darray2, i, &o2));
        ASSERT_EQUAL(o1, o2);
    }
}

void foreach(bbzheap_idx_t darray, bbzheap_idx_t pos, void* params) {
    ((bbzint_t*)bbzheap_obj_at(pos))->value += 20;
}

TEST(da_foreach) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 22; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    REQUIRE(bbzdarray_size(darray) == 22);

    bbzdarray_foreach(darray, foreach, NULL);
    bbzheap_idx_t o1;
    for (uint16_t i = 0; i < bbzdarray_size(darray); ++i) {
        bbzdarray_get(darray, i, &o1);
        ASSERT_EQUAL(bbzheap_obj_at(o1)->i.value, i+20);
    }
}

TEST(da_destroy) {
    bbzvm_t vmObj;
    vm = &vmObj;
    bbzheap_clear();

    uint16_t darray;
    bbzdarray_new(&darray);

    uint16_t o3;
    bbzint_t* io3;
    for (uint16_t i = 0; i < 22; ++i) {
        bbzheap_obj_alloc(BBZTYPE_INT, &o3);
        io3 = (bbzint_t*)bbzheap_obj_at(o3);
        io3->value = i;
        bbzdarray_push(darray, o3);
    }
    REQUIRE(bbzdarray_size(darray) == 22);

    bbzdarray_destroy(darray);
    ASSERT(!bbzheap_obj_isvalid(*bbzheap_obj_at(darray)));
}

TEST_LIST {
    ADD_TEST(da_new);
    ADD_TEST(da_push);
    ADD_TEST(da_find);
    ADD_TEST(da_set);
    ADD_TEST(da_push15x);
    ADD_TEST(da_pop7x);
    ADD_TEST(da_clear);
    ADD_TEST(da_clone);
    ADD_TEST(da_foreach);
    ADD_TEST(da_destroy);
};