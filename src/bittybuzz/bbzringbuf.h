/**
 * @file bbzringbuf.h
 * @brief Definition of BittyBuzz's ring buffer, which is a buffer
 * useful for O(1) push and pop operations on a queue.
 */

#ifndef BBZRINGBUF_H
#define BBZRINGBUF_H

#include "bbzinclude.h"
#include "bbztype.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Buffer implemented to seamlessly loop on itself,
 * allowing for O(1) push and pop for queues.
 */
typedef struct PACKED {
    uint8_t* buffer;    /**< @brief Pointer to linear buffer */
    uint8_t  elsize;    /**< @brief Size of individual element */
    uint8_t  capacity;  /**< @brief Buffer capacity */
    uint8_t  datastart; /**< @brief Data start index */
    uint8_t  dataend;   /**< @brief Data end index */
} bbzringbuf_t;

/**
 * @brief Erases all the elements in the ring buffer.
 * @param[in,out] rb The ring buffer.
 */
ALWAYS_INLINE
void bbzringbuf_clear(bbzringbuf_t* rb) { rb->datastart = 0; rb->dataend = 0; }

/**
 * @brief Initializes a new ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] buf The pointer to the linear buffer.
 * @param[in] elsz Size (in bytes) of an inidivdual element.
 * @param[in] cap The capacity (noumber of elements it can contains) of the ring buffer.
 */
//ALWAYS_INLINE
void bbzringbuf_construct(bbzringbuf_t *rb, uint8_t *buf, uint8_t elsz, uint8_t cap);// { rb->buffer = buf; rb->elsize = elsz; rb->capacity = cap; bbzringbuf_clear(rb); }

/**
 * @brief Returns the capacity of the ring buffer.
 * The capacity is the maximum number of elements the ring buffer can store.
 * @param[in] rb The ring buffer.
 * @return The capacity of the ring buffer.
 */
ALWAYS_INLINE
uint8_t bbzringbuf_capacity(const bbzringbuf_t* rb) { return rb->capacity; }

/**
 * @brief Returns the size of the ring buffer.
 * The size is the current number of elements the ring buffer is storing.
 * @param[in] rb The ring buffer.
 * @return The number of elements inside the ring buffer.
 */
//ALWAYS_INLINE
uint8_t bbzringbuf_size(const bbzringbuf_t* rb)/**/;/*/ {
    return ((rb->dataend >=  rb->datastart) ?
           (rb->dataend  -  rb->datastart) :
           (rb->capacity - (rb->datastart - rb->dataend))); // (rb->capacity - (rb->datastart - rb->dataend - (uint8_t)1)));
}/**/


/**
 * @brief Checks whether the ring buffer is full.
 * @param[in] rb The ring buffer.
 * @return 1 if the buffer is full, 0 otherwise
 */
//ALWAYS_INLINE
uint8_t bbzringbuf_full(const bbzringbuf_t* rb)/**/;/*/ { return (uint8_t) (((rb->dataend + 1) % rb->capacity) == rb->datastart); }/**/

/**
 * @brief Returns the object at the given index in the ring buffer.
 * The index starts from 0 for the oldest element in the structure, no
 * matter where it is actually located in the internal linear buffer.
 * @param[in] rb The ring buffer.
 * @param[in] idx The index.
 * @return A pointer to the object starting at given index.
 */
//ALWAYS_INLINE
uint8_t* bbzringbuf_at(const bbzringbuf_t* rb, uint8_t idx)/**/;/*/ { return rb->buffer + ((rb->datastart + idx) % rb->capacity) * rb->elsize; }/**/

/**
 * @brief Returns the object at the given internal index in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param[in] rb The ring buffer.
 * @param[in] idx The internal index.
 * @return A pointer to the object starting at given internal index.
 */
//ALWAYS_INLINE
uint8_t* bbzringbuf_rawat(const bbzringbuf_t* rb, uint8_t idx)/**/;/*/ { return (rb->buffer + (idx % rb->capacity) * rb->elsize); }/**/

/**
 * @brief Returns whether the buffer is empty or not.
 * @param rb The ring buffer.
 * @return Whether the buffer is empty or not.
 */
//ALWAYS_INLINE
uint8_t bbzringbuf_empty(bbzringbuf_t *rb)/**/;/*/ { return (uint8_t)(rb->datastart == rb->dataend); }/**/

/**
 * @brief Pops the first element in the list, if any.
 * @param rb The ring buffer.
 * @return 1 if the pop was sucessful, 0 if the buffer was already empty
 */
//ALWAYS_INLINE
uint8_t bbzringbuf_pop(bbzringbuf_t* rb)/**/;/*/ { if (bbzringbuf_empty(rb)) return 0; rb->datastart = (rb->datastart + (uint8_t)1) % rb->capacity; return 1; }/**/

/**
 * @brief Returns the internal index of a newly created slot in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param[in,out] rb  The ring buffer.
 * @return The internal index of the newly created slot.
 */
uint8_t bbzringbuf_makeslot(bbzringbuf_t* rb);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZRINGBUF_H
