/**
 * @file bbzringbuf.h
 * @brief Definition of BittyBuzz's ring buffer, which is a buffer
 * useful for O(1) push and pop operations on a queue.
 */

#ifndef BBZRINGBUF
#define BBZRINGBUF

#include <bittybuzz/bbztype.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Buffer implemented to seamlessly loop on itself,
 * allowing for O(1) push and pop for queues.
 */
typedef struct __attribute__((packed)) {
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
#define bbzringbuf_clear(rb) (rb).datastart = 0; (rb).dataend = 0;

/**
 * @brief Initializes a new ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] buf The pointer to the linear buffer.
 * @param[in] sz Number of elements of the linear buffer.
 * @param[in] cap The capacity of the ring buffer.
 */
#define bbzringbuf_new(rb, buf, sz, cap) (rb).buffer = buf; (rb).elsize = sz; (rb).capacity = cap; bbzringbuf_clear(rb);

/**
 * @brief Returns the capacity of the ring buffer.
 * The capacity is the maximum number of elements the ring buffer can store.
 * @param[in] rb The ring buffer.
 */
#define bbzringbuf_capacity(rb) ((rb).capacity)

/**
 * @brief Returns the size of the ring buffer.
 * The size is the current number of elements the ring buffer is storing.
 * @param[in] rb The ring buffer.
 */
#define bbzringbuf_size(rb) (                                 \
      ((rb).dataend >= (rb).datastart) ?                      \
      ((rb).dataend - (rb).datastart)                         \
      :                                                       \
      (((rb).capacity) - ((rb).datastart - (rb).dataend - 1)) \
      )

/**
 * @brief Returns 1 if the ring buffer is full, 0 otherwise.
 * @param[in] rb The ring buffer.
 */
#define bbzringbuf_full(rb) ((((rb).dataend + 1) % (rb).capacity) == (rb).datastart)

/**
 * @brief Returns the object at the given index in the ring buffer.
 * The index starts from 0 for the oldest element in the structure, no
 * matter where it is actually located in the internal linear buffer.
 * @param[in] rb The ring buffer.
 * @param[in] idx The index.
 */
#define bbzringbuf_at(rb, idx) ((rb).buffer + (((rb).datastart + (idx)) % (rb).capacity) * (rb).elsize)

/**
 * @brief Returns the object at the given internal index in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param[in] rb The ring buffer.
 * @param[in] idx The internal index.
 */
#define bbzringbuf_rawat(rb, idx) ((rb).buffer + ((idx) % (rb).capacity) * (rb).elsize)

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

#endif
