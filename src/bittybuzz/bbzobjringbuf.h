/**
 * @file bbzobjringbuf.h
 * @brief Definition of BittyBuzz's object ring buffer, which is a buffer
 * useful for O(1) push and pop operations on a queue.
 */

#ifndef BBZOBJRINGBUF_H
#define BBZOBJRINGBUF_H

#include "bbzinclude.h"
#include "bbzringbuf.h"
#include "bbzheap.h"
#include "bbzvm.h"

/**
 * @brief Buffer implemented to seamlessly loop on itself,
 * allowing for O(1) push and pop for queues.
 */
typedef bbzringbuf_t bbzobjringbuf_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Erases all the elements in the ring buffer.
 * @param[in,out] rb The ring buffer.
 */
ALWAYS_INLINE
void bbzobjringbuf_clear(bbzobjringbuf_t* rb) { bbzringbuf_clear(rb); }

/**
 * @brief Initializes a new ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] buf The pointer to the linear buffer.
 * @param[in] cap The capacity (number of object it can contains) of the ring buffer.
 */
ALWAYS_INLINE
void bbzobjringbuf_construct(bbzobjringbuf_t* rb, bbzobj_t* buf, uint8_t cap) { bbzringbuf_construct(rb, (uint8_t*)buf, sizeof(bbzheap_idx_t), cap); }

/**
 * @brief Returns the capacity of the ring buffer.
 * The capacity is the maximum number of elements the ring buffer can store.
 * @param[in] rb The ring buffer.
 */
ALWAYS_INLINE
uint8_t bbzobjringbuf_capacity(const bbzobjringbuf_t* rb) { return bbzringbuf_capacity(rb); }

/**
 * @brief Returns the size of the ring buffer.
 * The size is the current number of elements the ring buffer is storing.
 * @param[in] rb The ring buffer.
 * @return The number of elements inside the ring buffer.
 */
ALWAYS_INLINE
uint8_t bbzobjringbuf_size(const bbzobjringbuf_t* rb) { return bbzringbuf_size(rb); }

/**
 * @brief Checks whether the ring buffer is full.
 * @param[in] rb The ring buffer.
 * @return 1 if the buffer is full, 0 otherwise.
 */
ALWAYS_INLINE
uint8_t bbzobjringbuf_full(const bbzobjringbuf_t* rb) { return bbzringbuf_full(rb); }

/**
 * @brief Returns the object at the given index in the ring buffer.
 * The index starts from 0 for the oldest element in the structure, no
 * matter where it is actually located in the internal linear buffer.
 * @param[in] rb  The ring buffer.
 * @param[in] idx The index.
 * @return A pointer to the object starting at given index.
 */
ALWAYS_INLINE
bbzobj_t* bbzobjringbuf_at(const bbzobjringbuf_t* rb, uint8_t idx) { return (bbzobj_t*)bbzringbuf_at(rb, idx); }

/**
 * @brief Returns the object at the given internal index in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param[in] rb The ring buffer.
 * @param[in] idx The internal index.
 * @return A pointer to the object starting at given internal index.
 */
ALWAYS_INLINE
bbzobj_t* bbzobjringbuf_rawat(const bbzobjringbuf_t* rb, uint8_t idx) { return (bbzobj_t*)bbzringbuf_rawat(rb, idx); }

/**
 * @brief Returns the index of a newly created slot in the ring buffer.
 * This function is used internally by the bbzringbuf_append_*() functions.
 * @param[in,out] rb  The ring buffer.
 * @return The index of the newly created slot.
 */
ALWAYS_INLINE
uint8_t bbzobjringbuf_makeslot(bbzobjringbuf_t* rb) { return bbzringbuf_makeslot(rb); }

/**
 * @brief Appends a nil object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_nil(bbzobjringbuf_t* rb) { uint8_t slot = bbzobjringbuf_makeslot(rb); bbztype_cast(*bbzobjringbuf_rawat(rb, slot), BBZTYPE_NIL); *bbzobjringbuf_rawat(rb, slot) = *bbzheap_obj_at(vm->nil); }

/**
 * @brief Appends an int object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_int(bbzobjringbuf_t *rb, int16_t val) { uint8_t slot = bbzringbuf_makeslot(rb); bbztype_cast(*bbzobjringbuf_rawat(rb, slot), BBZTYPE_INT); bbzobjringbuf_rawat(rb, slot)->i.value = val; }

/**
 * @brief Appends a float object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_float(bbzobjringbuf_t* rb, bbzheap_idx_t val) { uint8_t slot = bbzringbuf_makeslot(rb); /* TODO */ }

/**
 * @brief Appends a string object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_string(bbzobjringbuf_t* rb, bbzheap_idx_t val) { uint8_t slot = bbzringbuf_makeslot(rb); /* TODO */ }

/**
 * @brief Appends a table object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_table(bbzobjringbuf_t* rb, bbzheap_idx_t val) { uint8_t slot = bbzringbuf_makeslot(rb); /* TODO */ }

/**
 * @brief Appends a closure object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_closure(bbzobjringbuf_t* rb, bbzheap_idx_t val) { uint8_t slot = bbzringbuf_makeslot(rb); /* TODO */ }

/**
 * @brief Appends a userdata object to the ring buffer.
 * @param[in,out] rb  The ring buffer.
 * @param[in] val The value.
 */
ALWAYS_INLINE
void bbzobjringbuf_append_userdata(bbzobjringbuf_t* rb, bbzheap_idx_t val) { uint8_t slot = bbzringbuf_makeslot(rb); /* TODO */ }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZOBJRINGBUF_H
