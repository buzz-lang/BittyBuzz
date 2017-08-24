#include <bittybuzz/bbzringbuf.h>

#define TEST_MODULE bbzringbuf
#define NUM_TEST_CASES 9
#include "testingconfig.h"

TEST(rb_construct) {
    uint16_t buf[42];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t*)buf, sizeof(uint16_t), 42);
    ASSERT_EQUAL((uintptr_t)rb.buffer, (uintptr_t)(uint8_t*)buf);
    ASSERT_EQUAL(rb.capacity, 42);
    ASSERT_EQUAL(rb.datastart, 0);
    ASSERT_EQUAL(rb.dataend, 0);
    ASSERT_EQUAL(rb.elsize, sizeof(uint16_t));
}

TEST(rb_capacity) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t*)buf, sizeof(uint16_t), 12);
    ASSERT_EQUAL(bbzringbuf_capacity(&rb), 12);
}

TEST(rb_size) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t*)buf, sizeof(uint16_t), 12);

    buf[1] = 0x2345; // Not necessary
    buf[2] = 0x6789; // Not necessary
    rb.datastart = 1;
    rb.dataend = 0;
    ASSERT_EQUAL(bbzringbuf_size(&rb), 11);
}

TEST(rb_full) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t*)buf, sizeof(uint16_t), 12);

    rb.datastart = 3;
    rb.dataend = 0;
    ASSERT_EQUAL(bbzringbuf_full(&rb), 0);
    rb.datastart = 3;
    rb.dataend = 2;
    ASSERT_EQUAL(bbzringbuf_full(&rb), 1);
    rb.datastart = 3;
    rb.dataend = 3;
    ASSERT_EQUAL(bbzringbuf_full(&rb), 0);
}

TEST(rb_at) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t *) buf, sizeof(uint16_t), 12);

    rb.datastart = 2;
    rb.dataend = 3;
    buf[2] = 0x2345;
    ASSERT_EQUAL( (uintptr_t)bbzringbuf_at(&rb, 0), (uintptr_t)&buf[2]);
    ASSERT_EQUAL(*(uint16_t*)bbzringbuf_at(&rb, 0), 0x2345);
    ASSERT_EQUAL( (uintptr_t)bbzringbuf_at(&rb, rb.capacity), (uintptr_t)&buf[2]);
    ASSERT_EQUAL(*(uint16_t*)bbzringbuf_at(&rb, rb.capacity), 0x2345);
}

TEST(rb_rawat) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t *) buf, sizeof(uint16_t), 12);

    rb.datastart = 2;
    rb.dataend = 3;
    buf[2] = 0x2345;
    ASSERT_EQUAL( (uintptr_t)bbzringbuf_rawat(&rb, 2), (uintptr_t)&buf[2]);
    ASSERT_EQUAL(*(uint16_t*)bbzringbuf_rawat(&rb, 2), 0x2345);
    ASSERT_EQUAL( (uintptr_t)bbzringbuf_rawat(&rb, 2+rb.capacity), (uintptr_t)&buf[2]);
    ASSERT_EQUAL(*(uint16_t*)bbzringbuf_rawat(&rb, 2+rb.capacity), 0x2345);
}

TEST(rb_empty) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t *) buf, sizeof(uint16_t), 12);

    rb.datastart = 3;
    rb.dataend = 3;
    ASSERT_EQUAL(bbzringbuf_empty(&rb), 1);
    rb.datastart = 3;
    rb.dataend = 4;
    ASSERT_EQUAL(bbzringbuf_empty(&rb), 0);
}

TEST(rb_pop) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t *) buf, sizeof(uint16_t), 12);

    rb.datastart = 2;
    rb.dataend = 4;
    buf[2] = 0x2345;
    buf[3] = 0x6789;
    ASSERT_EQUAL(bbzringbuf_pop(&rb), 1);
    ASSERT_EQUAL(rb.datastart, 3);
    ASSERT_EQUAL(rb.dataend, 4);
    ASSERT_EQUAL(bbzringbuf_pop(&rb), 1);
    ASSERT_EQUAL(rb.datastart, 4);
    ASSERT_EQUAL(rb.dataend, 4);
}

TEST(rb_makeslot) {
    uint16_t buf[12];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, (uint8_t*)buf, sizeof(uint16_t), 12);

    rb.datastart = 2;
    rb.dataend = 0;
    ASSERT_EQUAL(bbzringbuf_makeslot(&rb), 0);
    ASSERT_EQUAL(rb.datastart, 2);
    ASSERT_EQUAL(rb.dataend, 1);

    ASSERT_EQUAL(bbzringbuf_makeslot(&rb), 1);
    ASSERT_EQUAL(rb.datastart, 3);
    ASSERT_EQUAL(rb.dataend, 2);
}

TEST_LIST {
    ADD_TEST(rb_construct);
    ADD_TEST(rb_capacity);
    ADD_TEST(rb_size);
    ADD_TEST(rb_full);
    ADD_TEST(rb_at);
    ADD_TEST(rb_rawat);
    ADD_TEST(rb_empty);
    ADD_TEST(rb_pop);
    ADD_TEST(rb_makeslot);
}
