#include "bbzmsg.h"

/****************************************/
/****************************************/

void bbzmsg_serialize_u8(bbzringbuf_t *rb,
                         uint8_t data) {
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = data;
}

/****************************************/
/****************************************/

int16_t bbzmsg_deserialize_u8(uint8_t *data,
                              const bbzringbuf_t *rb,
                              uint16_t pos) {
    if (pos + sizeof(uint8_t) > bbzringbuf_size(rb)) return -1;
    *data = *bbzringbuf_at(rb, (uint8_t)pos);
    return pos + sizeof(uint8_t);
}

/****************************************/
/****************************************/

void bbzmsg_serialize_u16(bbzringbuf_t *rb,
                          uint16_t data) {
    data = htons(data);
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = (uint8_t)data;
    *bbzringbuf_rawat(rb, bbzringbuf_makeslot(rb)) = (uint8_t)(data >> 8);
}

/****************************************/
/****************************************/

int16_t bbzmsg_deserialize_u16(uint16_t *data,
                               const bbzringbuf_t *rb,
                               uint16_t pos) {
    if (pos + sizeof(uint16_t) > bbzringbuf_size(rb)) return -1;
    *data = *bbzringbuf_at(rb, (uint8_t)pos) +
            (*bbzringbuf_at(rb, (uint8_t)(pos + 1)) << 8);
    *data = ntohs(*data);
    return pos + sizeof(uint16_t);
}

/****************************************/
/****************************************/

void bbzmsg_serialize_obj(bbzringbuf_t *rb, bbzobj_t *obj) {
    bbzmsg_serialize_u8(rb, obj->mdata);
    bbzmsg_serialize_u16(rb, (uint16_t)obj->u.value);
}

/****************************************/
/****************************************/

int16_t bbzmsg_deserialize_obj(bbzobj_t *data, bbzringbuf_t *rb, uint16_t pos) {
    // spos => Signed pos
    int16_t spos = pos;
    spos = bbzmsg_deserialize_u8(&data->mdata, rb, (uint16_t)spos);
    if (spos < 0) return spos;
    return bbzmsg_deserialize_u16((uint16_t*)&data->biggest.value, rb, (uint16_t)spos);
}

/****************************************/
/****************************************/

void bbzmsg_sort_priority(bbzringbuf_t* rb) {
    // +=-=-=-= QuickSort=-=-=-=+

    // The last space is never used. So we use it as a temporary variable.
    bbzmsg_t* tmp = ((bbzmsg_t*)bbzringbuf_at(rb, rb->dataend + rb->capacity));
    for(uint16_t i = 1; i < bbzringbuf_size(rb); ++i) {
        *tmp = *(bbzmsg_t*)bbzringbuf_at(rb, (uint8_t)i);
        uint16_t j = i;
        while(j > 0 && tmp->type < ((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j-1)))->type) {
            *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j))) = *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j-1)));
            --j;
        }
        *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j))) = *tmp;
    }
}

/****************************************/
/****************************************/
