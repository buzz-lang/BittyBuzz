#include "bbzringbuf.h"

/****************************************/
/****************************************/

void bbzringbuf_construct(bbzringbuf_t *rb, uint8_t *buf, uint8_t elsz, uint8_t cap) { rb->buffer = buf; rb->elsize = elsz; rb->capacity = cap; bbzringbuf_clear(rb); }

/****************************************/
/****************************************/

uint8_t bbzringbuf_size(const bbzringbuf_t* rb) {
    return ((rb->dataend >=  rb->datastart) ?
            (rb->dataend  -  rb->datastart) :
            (rb->capacity - (rb->datastart - rb->dataend/* - (uint8_t)1*/)));
}

/****************************************/
/****************************************/

uint8_t bbzringbuf_full(const bbzringbuf_t* rb) { return (uint8_t) (((rb->dataend + 1) % rb->capacity) == rb->datastart); }

/****************************************/
/****************************************/

uint8_t* bbzringbuf_at(const bbzringbuf_t* rb, uint8_t idx) { return rb->buffer + ((rb->datastart + idx) % rb->capacity) * rb->elsize; }

/****************************************/
/****************************************/

uint8_t* bbzringbuf_rawat(const bbzringbuf_t* rb, uint8_t idx) { return (rb->buffer + (idx % rb->capacity) * rb->elsize); }

/****************************************/
/****************************************/

uint8_t bbzringbuf_empty(bbzringbuf_t *rb) { return (uint8_t)(rb->datastart == rb->dataend); }

/****************************************/
/****************************************/

uint8_t bbzringbuf_pop(bbzringbuf_t* rb) { if (bbzringbuf_empty(rb)) return 0; rb->datastart = (rb->datastart + (uint8_t)1) % rb->capacity; return 1; }

/****************************************/
/****************************************/

uint8_t bbzringbuf_makeslot(bbzringbuf_t* rb) {
    uint8_t slot = rb->dataend;
    if(!bbzringbuf_full(rb)) {
        rb->dataend = (rb->dataend + 1) % rb->capacity;
    }
    else {
        rb->dataend = rb->datastart;
        rb->datastart = (rb->datastart + 1) % rb->capacity;
    }
    return slot;
}

/****************************************/
/****************************************/
