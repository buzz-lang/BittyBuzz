#ifndef BBZOBJRINGBUF
#define BBZOBJRINGBUF

#include <bittybuzz/bbzringbuf.h>

typedef bbzringbuf_t bbzobjringbuf_t;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Erases all the elements in the ring buffer.
 * @param rb The ring buffer.
 */
#define bbzobjringbuf_clear(rb) bbzringbuf_clear(rb)

/**
 * @brief Initializes a new ring buffer.
 * @param rb  The ring buffer.
 * @param buf The pointer to the linear buffer.
 * @param cap The capacity of the ring buffer.
 */
#define bbzobjringbuf_new(rb, buf, cap) bbzringbuf_new(rb, (uint8_t*)buf, sizeof(bbzobj_t), cap)

/**
 * @brief Returns the capacity of the ring buffer.
 * The capacity is the maximum number of elements the ring buffer can store.
 * @param rb The ring buffer.
 */
#define bbzobjringbuf_capacity(rb) bbzringbuf_capacity(rb)

/**
 * @brief Returns the size of the ring buffer.
 * The size is the current number of elements the ring buffer is storing.
 * @param rb The ring buffer.
 */
#define bbzobjringbuf_size(rb) bbzringbuf_size(rb)

/**
 * @brief Returns 1 if the ring buffer is full, 0 otherwise.
 * @param rb The ring buffer.
 */
#define bbzobjringbuf_full(rb) bbzringbuf_full(rb)

/**
 * @brief Returns the object at the given index in the ring buffer.
 * The index starts from 0 for the oldest element in the structure, no
 * matter where it is actually located in the internal linear buffer.
 * @param rb  The ring buffer.
 * @param idx The index.
 */
#define bbzobjringbuf_at(rb, idx) (*(bbzobj_t*)bbzringbuf_at(rb, idx))

/**
 * @brief Returns the object at the given internal index in the ring buffer.
 * The internal index corresponds to the index in the internal linear buffer.
 * @param rb The ring buffer.
 * @param idx The internal index.
 */
#define bbzobjringbuf_rawat(rb, idx) (*(bbzobj_t*)((rb).buffer + ((idx) % (rb).capacity) * (rb).elsize))

/**
 * @brief Returns the index of a newly created slot in the ring buffer.
 * This function is used internally by the bbzringbuf_append_*() functions.
 * @param rb  The ring buffer.
 * @return The index of the newly created slot.
 */
#define bbzobjringbuf_makeslot(rb) bbzringbuf_makeslot(rb)

/**
 * @brief Appends a nil object to the ring buffer.
 * @param rb  The ring buffer.
 */
#define bbzobjringbuf_append_nil(rb) { uint8_t __bbzslot = bbzobjringbuf_makeslot(&(rb)); bbztype_cast(bbzobjringbuf_rawat(rb, __bbzslot), BBZTYPE_NIL); }

/**
 * @brief Appends an int object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_int(rb, val) { uint8_t __bbzslot = bbzringbuf_makeslot(&(rb)); bbztype_cast(bbzobjringbuf_rawat(rb, __bbzslot), BBZTYPE_INT); bbzobjringbuf_rawat(rb, __bbzslot).i.value = val; }

/**
 * @brief Appends a float object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_float(rb, val)

/**
 * @brief Appends a string object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_string(rb, val)

/**
 * @brief Appends a table object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_table(rb, val)

/**
 * @brief Appends a closure object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_closure(rb, val)

/**
 * @brief Appends a userdata object to the ring buffer.
 * @param rb  The ring buffer.
 * @param val The value.
 */
#define bbzobjringbuf_append_userdata(rb, val)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
