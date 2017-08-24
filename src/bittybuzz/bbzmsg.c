#include "bbzmsg.h"
#include "bbzutil.h"

#ifndef BBZ_DISABLE_MESSAGES
/****************************************/
/****************************************/

void bbzmsg_serialize_u8(bbzringbuf_t *rb,
                         uint8_t data) {
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = data;
}

/****************************************/
/****************************************/

void bbzmsg_deserialize_u8(uint8_t *data,
                           const bbzringbuf_t *rb,
                           int16_t *pos) {
    if (*pos + sizeof(uint8_t) > bbzringbuf_size(rb)) { *pos = -1; return; }
    *data = *bbzringbuf_at(rb, (uint8_t)*pos);
    *pos += sizeof(uint8_t);
}

/****************************************/
/****************************************/

void bbzmsg_serialize_u16(bbzringbuf_t *rb,
                          uint16_t data) {
    data = htons(data);
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = ((uint8_t*)&data)[0];
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = ((uint8_t*)&data)[1];
}

/****************************************/
/****************************************/

void bbzmsg_deserialize_u16(uint16_t *data,
                            const bbzringbuf_t *rb,
                            int16_t *pos) {
    if (*pos + sizeof(uint16_t) > bbzringbuf_size(rb)) { *pos = -1; return; }
    ((uint8_t*)data)[0] = *bbzringbuf_at(rb, (uint8_t)*pos);
    ((uint8_t*)data)[1] = *bbzringbuf_at(rb, (uint8_t)(*pos + 1));
    *data = ntohs(*data);
    *pos += sizeof(uint16_t);
}

/****************************************/
/****************************************/

void bbzmsg_serialize_obj(bbzringbuf_t *rb, bbzobj_t *obj) {
    bbzmsg_serialize_u8(rb, obj->mdata);
    bbzmsg_serialize_u16(rb, (uint16_t)obj->biggest.value);
}

/****************************************/
/****************************************/

void bbzmsg_deserialize_obj(bbzobj_t *data, bbzringbuf_t *rb, int16_t *pos) {
    bbzmsg_deserialize_u8(&data->mdata, rb, pos);
    if (*pos < 0) return;
    bbzmsg_deserialize_u16((uint16_t*)&data->biggest.value, rb, pos);
}

/****************************************/
/****************************************/

void bbzmsg_sort_priority(bbzringbuf_t* rb) {
    // +=-=-=-= QuickSort =-=-=-=+
    for(uint16_t i = 1; i < bbzringbuf_size(rb); ++i) {
        uint16_t j = i;
        while(j > 0 && ((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j)))->type < ((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j-1)))->type) {
            bbzutil_swapArrays(bbzringbuf_at(rb, (uint8_t) (j)), bbzringbuf_at(rb, (uint8_t) (j - 1)), sizeof(bbzmsg_t));
            --j;
        }
    }
}

/****************************************/
/****************************************/

#ifndef BBZ_DISABLE_NEIGHBORS
void bbzmsg_process_broadcast(bbzmsg_t* msg) {
    // Get the topic
    bbzvm_pushs(msg->bc.topic);
    bbzheap_idx_t topic = bbzvm_stack_at(0);
    bbzvm_pop();
    // Check if the topic has a listener. Break out of the switch if not.
    bbzheap_idx_t l;
    if (!bbztable_get(vm->neighbors.listeners, topic, &l)) return;
    // Call the listener
    bbzvm_pushnil(); // Push self table
    bbzvm_push(l);
    bbzvm_push(topic);
    bbzvm_pushu(0);
    bbzobj_t* x = bbzheap_obj_at(bbzvm_stack_at(0));
    bbztype_copy(msg->bc.value, *x);
    bbzheap_obj_unmake_permanent(*x);
    x->biggest.value = msg->bc.value.biggest.value;
    bbzvm_pushi(msg->bc.rid);
    bbzvm_closure_call(3);
    bbzvm_gc();
}
#endif

/****************************************/
/****************************************/

#ifndef BBZ_DISABLE_VSTIGS
static uint8_t bbzlamport_isnewer(bbzlamport_t lamport, bbzlamport_t old_lamport) {
    // This function uses a circular Lamport model (0 == 255 + 1).
    // A Lamport clock is 'newer' than an old Lamport clock if its value
    // is less than 'LAMPORT_THRESHOLD' ticks ahead of the old clock.
    return (uint8_t)(((lamport - old_lamport) & 0xFF) < BBZLAMPORT_THRESHOLD);/**/
}
void bbzmsg_process_vstig(bbzmsg_t* msg) {
    // Search the key in the vstig
    uint8_t inLocalVStig = 0;
    bbzvstig_elem_t* data;
    bbzheap_idx_t o;
    for (uint16_t i = 0; i < vm->vstig.size; ++i) {
        data = (vm->vstig.data + i);
        if (msg->vs.key == data->key) {
            inLocalVStig = 1;
            if (bbzlamport_isnewer(msg->vs.lamport, data->timestamp)) {
                // Update the value
                data->robot = msg->vs.rid;
                data->key = msg->vs.key;
                bbzheap_obj_makeinvalid(*bbzheap_obj_at(data->value));
                bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
                *bbzheap_obj_at(o) = msg->vs.data;
                bbzheap_obj_makevalid(*bbzheap_obj_at(o));
                bbzheap_obj_unmake_permanent(*bbzheap_obj_at(data->value));
                data->value = o;
                bbzheap_obj_make_permanent(*bbzheap_obj_at(o));
                data->timestamp = msg->vs.lamport;
                // Propagate the value.
                bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, data->robot,
                                             data->key,
                                             data->value, data->timestamp);
            } // The following "else if" is only for VSTIG_QUERY mesages.
            else if (msg->type == BBZMSG_VSTIG_QUERY &&
                     bbzlamport_isnewer(data->timestamp, msg->vs.lamport)) {
                /* Local element is newer */
                /* Append a PUT message to the out message queue */
                bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, vm->robot, msg->vs.key,
                                             data->value, data->timestamp);
            }
            else if (data->timestamp == msg->vs.lamport &&
                     data->robot != msg->vs.rid) {
                // Conflict! Call the onconflict callback closure.
                bbzheap_idx_t tmp = vm->nil;
                // Check if there is a callback closure.
                if (bbztable_get(vm->vstig.hpos, bbzstring_get(__BBZSTRID___INTERNAL_1_DO_NOT_USE__),
                                 &tmp)) {
                    bbzvm_pushnil(); // Push self table
                    bbzvm_push(tmp);
                    bbzvm_pushs(msg->vs.key);
                    // push the local data
                    bbzvm_pusht();
                    bbztable_add_data(__BBZSTRID_robot, bbzint_new(data->robot));
                    bbztable_add_data(__BBZSTRID_data, data->value);
                    bbztable_add_data(__BBZSTRID_timestamp, bbzint_new(data->timestamp));
                    // push the remote data
                    bbzvm_pusht();
                    bbzheap_idx_t rd = bbzvm_stack_at(0);
                    bbztable_add_data(__BBZSTRID_robot, bbzint_new(msg->vs.rid));
                    bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
                    *bbzheap_obj_at(o) = msg->vs.data;
                    bbzheap_obj_makevalid(*bbzheap_obj_at(o));
                    bbztable_add_data(__BBZSTRID_data, o);
                    bbztable_add_data(__BBZSTRID_timestamp, bbzint_new(msg->vs.lamport));
                    bbzvm_closure_call(3);
                    // Update the value with the table returned by the closure.
                    // If error, either no value was returned, or the returned value is of the wrong type.
                    bbzvm_assert_exec(bbztype_istable(*bbzheap_obj_at(bbzvm_stack_at(0))), BBZVM_ERROR_RET);
                    tmp = 0;
                    bbztable_get(bbzvm_stack_at(0), bbzstring_get(__BBZSTRID_robot), &tmp);
                    bbzrobot_id_t oldRID = data->robot;
                    data->robot = tmp ?
                                  (bbzrobot_id_t) bbzheap_obj_at(tmp)->i.value :
                                  data->robot;
                    tmp = vm->nil;
                    bbzheap_obj_makeinvalid(*bbzheap_obj_at(data->value));
                    bbzheap_obj_unmake_permanent(*bbzheap_obj_at(data->value));
                    bbztable_get(bbzvm_stack_at(0), bbzstring_get(__BBZSTRID_data), &tmp);
                    data->value = tmp;
                    bbzheap_obj_make_permanent(*bbzheap_obj_at(tmp));
                    data->timestamp = msg->vs.lamport;
                    // If this is the robot that lost, call the onconflictlost callback closure.
                    if ((bbzrobot_id_t) bbzheap_obj_at(tmp)->i.value != vm->robot &&
                        oldRID == vm->robot) {
                        // Check if there is an onconflictlost callback closure.
                        tmp = vm->nil;
                        if (bbztable_get(vm->vstig.hpos,
                                         bbzstring_get(__BBZSTRID___INTERNAL_2_DO_NOT_USE__), &tmp)) {
                            bbzvm_pushnil(); // Push self table
                            bbzvm_push(tmp);
                            bbzvm_pushs(msg->vs.key);
                            bbzvm_push(rd);
                            bbzvm_closure_call(2);
                        }
                    }
                    // Propagate the winning value.
                    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, data->robot,
                                                 data->key,
                                                 data->value, data->timestamp);
                }
                else {
                    // No conflict manager, use default behavior.
                    if (msg->vs.rid >= data->robot) {
                        data->robot = msg->vs.rid;
                        data->key = msg->vs.key;
                        bbzheap_obj_makeinvalid(*bbzheap_obj_at(data->value));
                        bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
                        *bbzheap_obj_at(o) = msg->vs.data;
                        bbzheap_obj_makevalid(*bbzheap_obj_at(o));
                        bbzheap_obj_unmake_permanent(*bbzheap_obj_at(data->value));
                        data->value = o;
                        bbzheap_obj_make_permanent(*bbzheap_obj_at(o));
                        data->timestamp = msg->vs.lamport;
                    }
                    // Propagate the winning value.
                    bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT, data->robot,
                                                 data->key,
                                                 data->value, data->timestamp);
                }
            }
            break;
        }
    }
    if (!inLocalVStig) {
        data = vm->vstig.data + vm->vstig.size;
        data->robot = msg->vs.rid;
        data->key = msg->vs.key;
        bbzvm_assert_mem_alloc(BBZTYPE_USERDATA, &o);
        *bbzheap_obj_at(o) = msg->vs.data;
        bbzheap_obj_makevalid(*bbzheap_obj_at(o));
        data->value = o;
        bbzheap_obj_make_permanent(*bbzheap_obj_at(o));
        data->timestamp = msg->vs.lamport;
        bbzoutmsg_queue_append_vstig(BBZMSG_VSTIG_PUT,
                                     data->robot,
                                     data->key,
                                     data->value,
                                     data->timestamp);
        ++vm->vstig.size;
    }
}
#endif

/****************************************/
/****************************************/

#ifndef BBZ_DISABLE_SWARMS
void bbzmsg_process_swarm(bbzmsg_t* msg) {
    RM_UNUSED_WARN(msg);
    // TODO
}
#endif

/****************************************/
/****************************************/
#endif // !BBZ_DISABLE_MESSAGES