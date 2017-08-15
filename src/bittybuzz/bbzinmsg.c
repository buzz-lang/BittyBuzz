#include "bbzinmsg.h"

#ifndef BBZ_DISABLE_MESSAGES
/****************************************/
/****************************************/

void bbzinmsg_queue_append(bbzmsg_payload_t* payload) {
    int16_t pos = 0;
    bbzmsg_t* m = vm->inmsgs.buf+vm->inmsgs.queue.capacity;
    m->base.type = 0;
    bbzmsg_deserialize_u8((uint8_t*)&m->base.type, payload, &pos);
    if (pos < 0) return;
    bbzmsg_deserialize_u16(&m->base.rid, payload, &pos);
    if (pos < 0) return;
    switch(m->base.type) {
        case BBZMSG_BROADCAST:
#ifndef BBZ_DISABLE_NEIGHBORS
            bbzmsg_deserialize_u16(&m->bc.topic, payload, &pos);
            if (pos < 0) return;
            bbzmsg_deserialize_obj(&m->bc.value, payload, &pos);
            bbzheap_obj_makevalid(m->bc.value);
            if (pos < 0) return;
            bbzmsg_deserialize_u8(&m->bc.dist, payload, &pos);
            if (pos < 0) return;
            break;
#else
            return;
#endif
        case BBZMSG_VSTIG_PUT: // fallthrough
        case BBZMSG_VSTIG_QUERY:
#ifndef BBZ_DISABLE_VSTIGS
            bbzmsg_deserialize_u16(&m->vs.key, payload, &pos);
            if (pos < 0) return;
            bbzmsg_deserialize_obj(&m->vs.data, payload, &pos);
            bbzheap_obj_makevalid(m->vs.data);
            if (pos < 0) return;
            bbzmsg_deserialize_u8(&m->vs.lamport, payload, &pos);
            if (pos < 0) return;
            break;
#else
            return;
#endif
        case BBZMSG_SWARM:
#ifndef BBZ_DISABLE_SWARMS
            bbzmsg_deserialize_u16(&m->sw.lamport, payload, &pos);
            if (pos < 0) return;
            bbzmsg_deserialize_u8(&m->sw.swarms, payload, &pos);
            if (pos < 0) return;
            break;
#else
            return;
#endif
        default:
            // Unknown type of message, the message is dropped.
            return;
    }
    // If everything succeed, we push the ring buffer forward.
    if (bbzringbuf_full(&vm->inmsgs.queue)) {
        // If full, replace the message with the lowest priority (the last of the queue) with the new one.
        *((bbzmsg_t*)bbzringbuf_rawat(&vm->inmsgs.queue, vm->inmsgs.queue.dataend - (uint8_t)1 + vm->inmsgs.queue.capacity)) = *m;
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
#endif // !BBZ_DISABLE_MESSAGES