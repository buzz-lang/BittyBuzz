#include <bittybuzz/bbzmsg.h>
#include <bittybuzz/bbzoutmsg.h>

#define NUM_TEST_CASES 8
#define TEST_MODULE messages
#include "testingconfig.h"

bbzvm_t vmObj;

#if !defined(BBZ_DISABLE_SWARMS) && !defined(BBZ_DISABLE_NEIGHBORS) && !defined(BBZ_DISABLE_VSTIGS) && !defined(BBZ_DISABLE_MESSAGES)
TEST(m_serialize8) {
    uint8_t buf[4];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, sizeof(uint8_t), 4);

    uint8_t x = 0x56;
    bbzmsg_serialize_u8(&rb, x);
    ASSERT_EQUAL(bbzringbuf_size(&rb), 1);
    ASSERT_EQUAL(*bbzringbuf_at(&rb, 0), x);
}

TEST(m_deserialize8) {
    uint8_t buf[4];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, sizeof(uint8_t), 4);
    *bbzringbuf_rawat(&rb, bbzringbuf_makeslot(&rb)) = 0x56;
    int16_t pos = 0;

    uint8_t x;
    bbzmsg_deserialize_u8(&x, &rb, &pos);
    ASSERT_EQUAL(pos, 1);
    ASSERT_EQUAL(x, 0x56);
}

TEST(m_serialize16) {
    uint8_t buf [4];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, sizeof(uint8_t), 4);

    uint16_t x = 0x2345;
    bbzmsg_serialize_u16(&rb, x);
    ASSERT_EQUAL(bbzringbuf_size(&rb), 2);
    ASSERT_EQUAL(*bbzringbuf_at(&rb, 0), (uint8_t)htons(x));
    ASSERT_EQUAL(*bbzringbuf_at(&rb, 1), (uint8_t)(htons(x)>>8));
}

TEST(m_deserialize16) {
    uint8_t buf [4];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, sizeof(uint8_t), 4);
    *bbzringbuf_rawat(&rb, bbzringbuf_makeslot(&rb)) = (uint8_t)htons(0x3456);
    *bbzringbuf_rawat(&rb, bbzringbuf_makeslot(&rb)) = (uint8_t)(htons(0x3456)>>8);

    int16_t pos = 0;
    uint16_t x;
    bbzmsg_deserialize_u16(&x, &rb, &pos);
    ASSERT_EQUAL(pos, 2);
    ASSERT_EQUAL(x, 0x3456);
}

TEST(m_out_append) {
    vm = &vmObj;
    bbzvm_construct(42);

    // Setup

    bbzheap_idx_t val, val2;
    REQUIRE(bbzheap_obj_alloc(BBZTYPE_INT, &val));
    REQUIRE(bbzheap_obj_alloc(BBZTYPE_INT, &val2));
    bbzheap_obj_at(val)->i.value = 0x2345; // Value not necessary, but useful for debugging
    bbzheap_obj_at(val2)->i.value = 0x6789; // Value not necessary, but useful for debugging

    // The actual tests

    bbzoutmsg_queue_append_swarm_chunk(21, 0x42, 2);
    ASSERT_EQUAL(bbzoutmsg_queue_size(), 1);
    ASSERT_EQUAL((vm->outmsgs.buf)->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((vm->outmsgs.buf)->sw.rid, 21);
    ASSERT_EQUAL((vm->outmsgs.buf)->sw.swarms, 0x42);
    ASSERT_EQUAL((vm->outmsgs.buf)->sw.lamport, 2);

    bbzoutmsg_queue_append_broadcast(bbzstring_get(__BBZSTRID_id), val);
    ASSERT_EQUAL(bbzoutmsg_queue_size(), 2);
    ASSERT_EQUAL((vm->outmsgs.buf)->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((vm->outmsgs.buf)->bc.rid, 42);
    ASSERT_EQUAL((vm->outmsgs.buf)->bc.topic, __BBZSTRID_id);
    ASSERT_EQUAL((vm->outmsgs.buf)->bc.value.u.mdata, bbzheap_obj_at(val)->u.mdata);
    ASSERT_EQUAL((vm->outmsgs.buf)->bc.value.u.value, bbzheap_obj_at(val)->u.value);

    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, 42, __BBZSTRID_put, val, 1);
    ASSERT_EQUAL(bbzoutmsg_queue_size(), 3);
    ASSERT_EQUAL((vm->outmsgs.buf)->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->type, BBZMSG_VSTIG_PUT);
    ASSERT_EQUAL((&vm->outmsgs.buf[2])->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->vs.rid, 42);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->vs.key, __BBZSTRID_put);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->vs.data.u.mdata, bbzheap_obj_at(val)->u.mdata);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->vs.data.u.value, bbzheap_obj_at(val)->u.value);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->vs.lamport, 1);

    bbzoutmsg_queue_append_broadcast(bbzstring_get(__BBZSTRID_count), val2);
    ASSERT_EQUAL(bbzoutmsg_queue_size(), 4);
    ASSERT_EQUAL((vm->outmsgs.buf)->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->outmsgs.buf[2])->type, BBZMSG_VSTIG_PUT);
    ASSERT_EQUAL((&vm->outmsgs.buf[3])->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->bc.rid, 42);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->bc.topic, __BBZSTRID_count);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->bc.value.u.mdata, bbzheap_obj_at(val2)->u.mdata);
    ASSERT_EQUAL((&vm->outmsgs.buf[1])->bc.value.u.value, bbzheap_obj_at(val2)->u.value);
}

TEST(m_out_queue_first) {
    vm = &vmObj;
    bbzvm_construct(42);

    uint8_t buf[9];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, 1, 9);

    bbzheap_idx_t val;
    REQUIRE(bbzheap_obj_alloc(BBZTYPE_INT, &val));
    bbzheap_obj_at(val)->i.value = 0x6789; // Value not necessary, but useful for debugging
    bbzoutmsg_queue_append_broadcast(bbzstring_get(__BBZSTRID_count), val);

    bbzoutmsg_queue_first(&rb);
    ASSERT_EQUAL(*buf, BBZMSG_BROADCAST);
    ASSERT_EQUAL(*(uint16_t*)(buf+1), htons(42));
    ASSERT_EQUAL(*(uint16_t*)(buf+3), htons(__BBZSTRID_count));
    ASSERT_EQUAL(((bbzobj_t*)(buf+5))->mdata, bbzheap_obj_at(val)->mdata);
    ASSERT_EQUAL((uint8_t)((bbzobj_t*)(buf+5))->u.value, (uint8_t)htons((uint16_t)bbzheap_obj_at(val)->u.value));
    ASSERT_EQUAL((uint8_t)(((bbzobj_t*)(buf+5))->u.value >> 8), (uint8_t)(htons((uint16_t)bbzheap_obj_at(val)->u.value) >> 8));
    bbzobj_t obj;
    int16_t pos = 5;
    bbzmsg_deserialize_obj(&obj, &rb, &pos);
    ASSERT_EQUAL(pos, 8);
    ASSERT_EQUAL(obj.mdata, bbzheap_obj_at(val)->mdata);
    ASSERT_EQUAL((int16_t)obj.u.value, (int16_t)bbzheap_obj_at(val)->u.value);

    REQUIRE(bbzoutmsg_queue_size() == 1);
    bbzoutmsg_queue_next();
    ASSERT_EQUAL(bbzoutmsg_queue_size(), 0);
}

TEST(m_in_append) {

    vm = &vmObj;
    bbzvm_construct(42);

    // Setup
    bbzobj_t obj1, obj2;
    bbztype_cast(obj1, BBZTYPE_INT);
    obj1.i.value = 0x2345;
    bbztype_cast(obj2, BBZTYPE_INT);
    obj2.i.value = 0x6789;

    uint8_t buf1[10], buf2[10], buf3[10];
    bbzmsg_payload_t payload1, payload2, payload3;
    bbzringbuf_construct(&payload1, buf1, 1, 10);
    bbzringbuf_construct(&payload2, buf2, 1, 10);
    bbzringbuf_construct(&payload3, buf3, 1, 10);

    bbzmsg_serialize_u8 (&payload1, BBZMSG_SWARM_CHUNK);
    bbzmsg_serialize_u16(&payload1, 21);
    bbzmsg_serialize_u16(&payload1, 2);
    bbzmsg_serialize_u8 (&payload1, 0x42);

    bbzmsg_serialize_u8 (&payload2, BBZMSG_BROADCAST);
    bbzmsg_serialize_u16(&payload2, 42);
    bbzmsg_serialize_u16(&payload2, __BBZSTRID_id);
    bbzmsg_serialize_obj(&payload2, &obj1);

    bbzmsg_serialize_u8 (&payload3, BBZMSG_VSTIG_PUT);
    bbzmsg_serialize_u16(&payload3, 42);
    bbzmsg_serialize_u16(&payload3, __BBZSTRID_put);
    bbzmsg_serialize_obj(&payload3, &obj2);
    bbzmsg_serialize_u8 (&payload3, 1);

    // The actual tests

    // We presume that the messages are sorted when appended.

    bbzinmsg_queue_append(&payload1);
    ASSERT_EQUAL(bbzinmsg_queue_size(), 1);
    ASSERT_EQUAL((vm->inmsgs.buf)->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((vm->inmsgs.buf)->sw.rid, 21);
    ASSERT_EQUAL((vm->inmsgs.buf)->sw.lamport, 2);
    ASSERT_EQUAL((vm->inmsgs.buf)->sw.swarms, 0x42);

    bbzinmsg_queue_append(&payload2);
    ASSERT_EQUAL(bbzinmsg_queue_size(), 2);
    ASSERT_EQUAL((vm->inmsgs.buf)->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((vm->inmsgs.buf)->bc.rid, 42);
    ASSERT_EQUAL((vm->inmsgs.buf)->bc.topic, __BBZSTRID_id);
    ASSERT_EQUAL((uint8_t)((vm->inmsgs.buf)->bc.value.mdata & ~BBZHEAP_OBJ_MASK_VALID), (uint8_t)(obj1.mdata & ~BBZHEAP_OBJ_MASK_VALID));
    ASSERT_EQUAL((vm->inmsgs.buf)->bc.value.i.value, obj1.i.value);

    bbzinmsg_queue_append(&payload3);
    ASSERT_EQUAL(bbzinmsg_queue_size(), 3);
    ASSERT_EQUAL((vm->inmsgs.buf)->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->type, BBZMSG_VSTIG_PUT);
    ASSERT_EQUAL((&vm->inmsgs.buf[2])->type, BBZMSG_SWARM_CHUNK);
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->vs.rid, 42);
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->vs.key, __BBZSTRID_put);
    ASSERT_EQUAL((uint8_t)((&vm->inmsgs.buf[1])->vs.data.mdata & ~BBZHEAP_OBJ_MASK_VALID), (uint8_t)(obj2.mdata & ~BBZHEAP_OBJ_MASK_VALID));
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->vs.data.i.value, obj2.i.value);
    ASSERT_EQUAL((&vm->inmsgs.buf[1])->vs.lamport, 1);
}

TEST(m_in_queue_first) {
    vm = &vmObj;
    bbzvm_construct(42);

    bbzobj_t obj1;
    bbztype_cast(obj1, BBZTYPE_INT);
    obj1.i.value = 0x2345;

    uint8_t buf[9];
    bbzringbuf_t rb;
    bbzringbuf_construct(&rb, buf, 1, 9);

    uint8_t buf1[10];
    bbzmsg_payload_t payload1;
    bbzringbuf_construct(&payload1, buf1, 1, 10);
    bbzmsg_serialize_u8 (&payload1, BBZMSG_BROADCAST);
    bbzmsg_serialize_u16(&payload1, 42);
    bbzmsg_serialize_u16(&payload1, __BBZSTRID_count);
    bbzmsg_serialize_obj(&payload1, &obj1);
    bbzinmsg_queue_append(&payload1);

    bbzmsg_t* msg = bbzinmsg_queue_extract();
    ASSERT_EQUAL(msg->type, BBZMSG_BROADCAST);
    ASSERT_EQUAL(msg->bc.rid, 42);
    ASSERT_EQUAL(msg->bc.topic, __BBZSTRID_count);
    ASSERT_EQUAL((uint8_t)(msg->bc.value.mdata & ~BBZHEAP_OBJ_MASK_VALID), (uint8_t)(obj1.mdata & ~BBZHEAP_OBJ_MASK_VALID));
    ASSERT_EQUAL((uint16_t)msg->bc.value.u.value, (uint16_t)obj1.u.value);
}
#endif // !BBZ_DISABLE_SWARMS && !BBZ_DISABLE_NEIGHBORS && !BBZ_DISABLE_VSTIGS && !BBZ_DISABLE_MESSAGES

TEST_LIST {
#if !defined(BBZ_DISABLE_SWARMS) && !defined(BBZ_DISABLE_NEIGHBORS) && !defined(BBZ_DISABLE_VSTIGS) && !defined(BBZ_DISABLE_MESSAGES)
    ADD_TEST(m_serialize8);
    ADD_TEST(m_deserialize8);
    ADD_TEST(m_serialize16);
    ADD_TEST(m_deserialize16);
    ADD_TEST(m_out_append);
    ADD_TEST(m_out_queue_first);
    ADD_TEST(m_in_append);
    ADD_TEST(m_in_queue_first);
#endif // !BBZ_DISABLE_SWARMS && !BBZ_DISABLE_NEIGHBORS && !BBZ_DISABLE_VSTIGS && !BBZ_DISABLE_MESSAGES
}