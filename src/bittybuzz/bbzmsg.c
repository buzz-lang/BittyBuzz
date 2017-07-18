#include "bbzmsg.h"

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

void swapArrays(uint8_t *arr1, uint8_t *arr2, uint8_t size) {
    for (uint8_t i=0;i<size;++i) {
        arr1[i] ^= arr2[i];
        arr2[i] ^= arr1[i];
        arr1[i] ^= arr2[i];
    }
}

void bbzmsg_sort_priority(bbzringbuf_t* rb) {
    // +=-=-=-= QuickSort=-=-=-=+

    // The last space is never used. So we use it as a temporary variable.
//    bbzmsg_t* tmp = ((bbzmsg_t*)bbzringbuf_at(rb, rb->dataend + rb->capacity));
    for(uint16_t i = 1; i < bbzringbuf_size(rb); ++i) {
//        *tmp = *(bbzmsg_t*)bbzringbuf_at(rb, (uint8_t)i);
        uint16_t j = i;
        while(j > 0 && /*/ tmp->type /*/((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j)))->type/**/ < ((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j-1)))->type) {/**/
            swapArrays(bbzringbuf_at(rb,(uint8_t)(j)),bbzringbuf_at(rb,(uint8_t)(j-1)),sizeof(bbzmsg_t));/*/
            *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j))) = *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j-1)));/**/
            --j;
        }
//        *((bbzmsg_t*)bbzringbuf_at(rb,(uint8_t)(j))) = *tmp;
    }
}

/****************************************/
/****************************************/
