#include "bbzringbuf.h"

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
