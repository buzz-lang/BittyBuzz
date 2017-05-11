#ifndef BBZRINGBUF
#define BBZRINGBUF

#include <bittybuzz/bbztype.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct {
   uint8_t* buffer;    // pointer to linear buffer
   uint8_t  elsize;    // size of individual element
   uint8_t  capacity;  // buffer capacity
   uint8_t  datastart; // data start index
   uint8_t  dataend;   // data end index
} bbzringbuf_t;

/*
 * Erases all the elements in the ring buffer.
 * @param rb The ring buffer.
 */
#define bbzringbuf_clear(rb) (rb).datastart = 0; (rb).dataend = 0;

/*
 * Initializes a new ring buffer.
 * @param rb  The ring buffer.
 * @param buf The pointer to the linear buffer.
 * @param cap The capacity of the ring buffer.
 */
#define bbzringbuf_new(rb, buf, sz, cap) (rb).buffer = buf; (rb).elsize = sz; (rb).capacity = cap; bbzringbuf_clear(rb);

/*
 * Returns the capacity of the ring buffer.
 * The capacity is the maximum number of elements the ring buffer can store.
 * @param rb The ring buffer.
 */
#define bbzringbuf_capacity(rb) ((rb).capacity)

/*
 * Returns the size of the ring buffer.
 * The size is the current number of elements the ring buffer is storing.
 * @param rb The ring buffer.
 */
#define bbzringbuf_size(rb) (                                 \
      ((rb).dataend >= (rb).datastart) ?                      \
      ((rb).dataend - (rb).datastart)                         \
      :                                                       \
      (((rb).capacity) - ((rb).datastart - (rb).dataend - 1)) \
      )

/*
 * Returns 1 if the ring buffer is full, 0 otherwise.
 * @param rb The ring buffer.
 */
#define bbzringbuf_full(rb) ((((rb).dataend + 1) % (rb).capacity) == (rb).datastart)

/*
 * Returns the object at the given index in the ring buffer.
 * The index starts from 0 for the oldest element in the structure, no
 * matter where it is actually located in the internal linear buffer.
 * @param rb The ring buffer.
 * @param idx The index.
 */
#define bbzringbuf_at(rb, idx) ((rb).buffer + (((rb).datastart + (idx)) % (rb).capacity) * (rb).elsize)

/*
 * Returns the object at the given internal index in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param rb The ring buffer.
 * @param idx The internal index.
 */
#define bbzringbuf_rawat(rb, idx) ((rb).buffer + ((idx) % (rb).capacity) * (rb).elsize)

/*
 * Returns the internal index of a newly created slot in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param rb  The ring buffer.
 * @return The internal index of the newly created slot.
 */
uint8_t bbzringbuf_makeslot(bbzringbuf_t* rb);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
