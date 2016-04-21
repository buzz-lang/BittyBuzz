#ifndef BBZTABLE
#define BBZTABLE

#include <bittybuzz/bbzheap.h>

int bbztable_get(bbzheap_t* h,
                 uint16_t t,
                 uint16_t k,
                 uint16_t* v);

int bbztable_set(bbzheap_t* h,
                 uint16_t t,
                 uint16_t k,
                 uint16_t v);

uint8_t bbztable_size(bbzheap_t* h,
                      uint16_t t);

#endif
