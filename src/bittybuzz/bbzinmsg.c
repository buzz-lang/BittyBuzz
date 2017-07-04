#include "bbzinmsg.h"

/****************************************/
/****************************************/

void bbzinmsg_queue_append(bbzmsg_payload_t* payload) {
    int16_t pos = 0;
    bbzmsg_t* m = &vm->inmsgs.buf[vm->inmsgs.queue.capacity];
    pos = bbzmsg_deserialize_u8(&m->type, payload, (uint16_t)pos);
    if (pos < 0) return;
    switch(m->type) {
        case BBZMSG_BROADCAST:
            pos = bbzmsg_deserialize_u16(&m->bc.rid, payload, (uint16_t)pos);
            if (pos < 0) return;
            pos = bbzmsg_deserialize_u16(&m->bc.topic, payload, (uint16_t)pos);
            if (pos < 0) return;
            bbzvm_assert_exec(bbzheap_obj_alloc(BBZTYPE_USERDATA, &m->bc.value), BBZVM_ERROR_MEM);
            pos = bbzmsg_deserialize_obj(bbzheap_obj_at(m->bc.value), payload, (uint16_t)pos);
            if (pos < 0) return;
            break;
        case BBZMSG_VSTIG_PUT: // fallthrough
        case BBZMSG_VSTIG_QUERY:
            pos = bbzmsg_deserialize_u16(&m->vs.rid, payload, (uint16_t)pos);
            if (pos < 0) return;
            pos = bbzmsg_deserialize_u16(&m->vs.key, payload, (uint16_t)pos);
            if (pos < 0) return;
            bbzvm_assert_exec(bbzheap_obj_alloc(BBZTYPE_USERDATA, &m->vs.data), BBZVM_ERROR_MEM);
            pos = bbzmsg_deserialize_obj(bbzheap_obj_at(m->vs.data), payload, (uint16_t)pos);
            if (pos < 0) return;
            pos = bbzmsg_deserialize_u8(&m->vs.lamport, payload, (uint16_t)pos);
            if (pos < 0) return;
            break;
        case BBZMSG_SWARM_CHUNK:
            pos = bbzmsg_deserialize_u16(&m->sw.rid, payload, (uint16_t)pos);
            if (pos < 0) return;
            pos = bbzmsg_deserialize_u16(&m->sw.lamport, payload, (uint16_t)pos);
            if (pos < 0) return;
            pos = bbzmsg_deserialize_u8(&m->sw.swarms, payload, (uint16_t)pos);
            if (pos < 0) return;
            break;
        default:
            // Unknown type of message, the message is dropped.
            return;
    }
    // If everything succeed, we push the ring buffer forward.
    if (bbzringbuf_full(&vm->inmsgs.queue)) {
        // If full, replace the message with the lowest priority (the last of the queue) with the new one.
        *((bbzmsg_t*)bbzringbuf_at(&vm->inmsgs.queue, vm->inmsgs.queue.dataend - (uint8_t)1 + vm->inmsgs.queue.capacity)) = *m;
    }
    else {
        // If not full, push the message at the end of the queue.
        *((bbzmsg_t*)bbzringbuf_rawat(&vm->inmsgs.queue, bbzringbuf_makeslot(&vm->inmsgs.queue))) = *m;
    }
    // TODO Do we need to sort the incomming list?
    bbzmsg_sort_priority(&vm->inmsgs.queue);
}

/****************************************/
/****************************************/

bbzmsg_t * bbzinmsg_queue_extract() {
    bbzmsg_t* ret = &vm->inmsgs.buf[vm->inmsgs.queue.capacity];
    *ret = *bbzinmsg_queue_get(0);
    bbzringbuf_pop(&vm->inmsgs.queue);
#ifdef BBZMSG_POP_NEEDS_SORT
    bbzmsg_sort_priority(&vm->inmsgs.queue);
#endif
    return ret;
}

/****************************************/
/****************************************/
