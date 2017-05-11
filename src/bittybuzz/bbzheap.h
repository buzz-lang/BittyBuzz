#ifndef BBZHEAP
#define BBZHEAP

/*
 * The BittyBuzz heap is a static buffer of uint8_t, whose size is
 * decided by the developer.
 * The heap contains instances of bbzobj_t variables.
 *
 * Non-structured types such as nil, int, float, and string are stored
 * directly. Structured types such as tables and closures require more
 * information.
 *
 * The idea is that the structure is filled from left to right (i.e.,
 * increasing index values) with bbzobj_t structures. For
 * non-structured types, this is it.
 *
 * Tables are stored as follows:
 * 1. A bbzobj_t is stored left-to-right;
 * 2. The bbzobj_t refers to a data segment, which is stored
 *    right-to-left.
 * 3. Each segment is composed of 5 (key,value) elements, each
 *    composed of a (bbzobj_t,bbzobj_t) pair. Each segment also has a
 *    2-byte field which contains flags and a pointer to the next
 *    segment, if any.
 *
 * TODO: closures
 */

#include <bittybuzz/bbztype.h>
#include <bittybuzz/config.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * A table segment.
 */
typedef struct __attribute__((packed)) {
   uint16_t keys[BBZHEAP_ELEMS_PER_TSEG];   /* 16th bit: valid; other bits: obj index */
   uint16_t values[BBZHEAP_ELEMS_PER_TSEG]; /* 16th bit: valid; other bits: obj index */
   uint16_t mdata; /* 16th     bit : valid
                      15th-1st bits: next segment index (0x7FFF means no next) */
} bbzheap_tseg_t;

/*
 * An array segment.
 */
typedef struct __attribute__((packed)) {
   uint16_t values[2*BBZHEAP_ELEMS_PER_TSEG]; /* 16th bit: valid; other bits: obj index */
   uint16_t mdata; /* 16th     bit : valid
                      15th-1st bits: next segment index (0x7FFF means no next) */
} bbzheap_aseg_t;

/*
 * The heap structure.
 */
typedef struct __attribute__((packed)) {
   uint8_t* rtobj;             /* pointer to after the rightmost object in heap, not necessarly valid */
   uint8_t* ltseg;             /* pointer to the leftmost table segment in heap, not necessarly valid */
   uint8_t data[BBZHEAP_SIZE]; /* data buffer */
} bbzheap_t;

/**
 * @brief Type for a heap index.
 * This can be considered to be a custom pointer
 * to a heap-allocated element.
 */
typedef uint16_t bbzheap_idx_t;

/*
 * Clears the heap.
 * Sets the entire heap to zero.
 * @param h The heap.
 */
void bbzheap_clear(bbzheap_t* h);

/*
 * Allocates space for an object on the heap.
 * Sets as output the value of o, a buffer for the index of the allocated object.
 * The value of o is not checked for NULL, so make sure it's a valid pointer.
 * @param h The heap.
 * @param t The type of the object.
 * @param o A buffer for the index of the allocated object.
 * @return 1 for success, 0 for failure (out of memory)
 */
int bbzheap_obj_alloc(bbzheap_t* h,
                      int t,
                      bbzheap_idx_t* o);

/*
 * Returns a pointer located at position i within the heap.
 * @param h The heap.
 * @param i The position.
 * @return A pointer to the object.
 */
#define bbzheap_obj_at(h, i) ((bbzobj_t*)((h)->data) + (i))

/*
 * Returns non-zero if the given object is valid (i.e., in use).
 * @param x The object.
 * @return non-zero if the given object is valid (i.e., in use).
 */
#define bbzheap_obj_isvalid(x) ((x).o.mdata & 0x10)

/**
 *  @brief Copy the value of an object to an other object.
 *  @param [in] h A pointer to the heap.
 *  @param [in] iSrc The position of the source object.
 *  @param [in] iDest The position of the destination object.
 */
#define bbzheap_obj_copy(h, iSrc, iDest) (*bbzheap_obj_at(h, iDest)) = (*bbzheap_obj_at(h, iSrc))

/*
 * Allocates space for a table segment on the heap.
 * Sets as output the value of s, the index of the allocated segment.
 * @param h The heap.
 * @param s A buffer for the pointer to the allocated segment.
 * @return 1 for success, 0 for failure (out of memory)
 */
int bbzheap_tseg_alloc(bbzheap_t* h,
                       bbzheap_idx_t* s);

#define NO_NEXT 0x7FFF
#define MASK_NEXT 0x7FFF
#define MASK_VALID_SEG 0x8000
#define MASK_VALID_SEG_ELEM 0x8000

/*
 * Returns a table segment located at position i within the heap.
 * @param h The heap.
 * @param i The position.
 * @return A pointer to the table segment.
 */
#define bbzheap_tseg_at(h, i) ((bbzheap_tseg_t*)((h)->data + BBZHEAP_SIZE) - ((i)+1))

/*
 * Returns the next table segment linked to the given one.
 * @param s The table segment.
 * @return The index of the next segment.
 */
#define bbzheap_tseg_next_get(s) ((s)->mdata & MASK_NEXT)

/*
 * Sets the next table segment linked to the given one.
 * @param s The table segment.
 * @param n The index of the next segment.
 */
#define bbzheap_tseg_next_set(s, n) (s)->mdata = ((s)->mdata & ~MASK_NEXT) | ((n) & MASK_NEXT)

/*
 * Returns 1 if the given table segment has a valid next, 0 otherwise.
 * @param s The table segment.
 * @return 1 if the given table segment has a valid next, 0 otherwise.
 */
#define bbzheap_tseg_hasnext(s) (((s)->mdata & MASK_NEXT) != NO_NEXT)

/*
 * Returns non-zero if the given segment is valid (in use).
 * @param s The table segment.
 * @return non-zero if the given segment is valid (in use).
 */
#define bbzheap_tseg_isvalid(s) ((s).mdata & MASK_VALID_SEG)

/*
 * Returns non-zero if the given segment element (key or value) is valid (in use).
 * @param e The table segment element.
 * @return non-zero if the given segment element (key or value) is valid (in use).
 */
#define bbzheap_tseg_elem_isvalid(e) ((e) & MASK_VALID_SEG_ELEM)

/*
 * Returns the value of the given table segment element.
 * @param e The table segment element.
 */
#define bbzheap_tseg_elem_get(e) ((e) & ~MASK_VALID_SEG_ELEM)

/*
 * Sets the value of the given table segment element, and validates the element.
 * @param e The table segment element.
 * @param x The value.
 */
#define bbzheap_tseg_elem_set(e, x) (e) = ((x) & ~MASK_VALID_SEG_ELEM) | MASK_VALID_SEG_ELEM

/**
 * @brief Allocates space for an array segment on the heap.
 * Sets as output the value of s, the index of the allocated segment.
 * @param h The heap.
 * @param s A buffer for the pointer to the allocated segment.
 * @return 1 for success, 0 for failure (out of memory)
 */
#define bbzheap_aseg_alloc(h, s) bbzheap_tseg_alloc(h, s)

/**
 * @brief Returns an array segment located at position i within the heap.
 * @param h The heap.
 * @param i The position.
 * @return A pointer to the array segment.
 */
#define bbzheap_aseg_at(h, i) ((bbzheap_aseg_t*)((h)->data + BBZHEAP_SIZE) - ((i)+1))

/**
 * @brief Returns the next array segment linked to the given one.
 * @param s The array segment.
 * @return The index of the next segment.
 */
#define bbzheap_aseg_next_get(s) bbzheap_tseg_next_get(s)

/**
 * @brief Sets the next array segment linked to the given one.
 * @param s The array segment.
 * @param n The index of the next segment.
 */
#define bbzheap_aseg_next_set(s, n) bbzheap_tseg_next_set(s, n)

/**
 * @brief Returns 1 if the given array segment has a valid next, 0 otherwise.
 * @param s The array segment.
 * @return 1 if the given array segment has a valid next, 0 otherwise.
 */
#define bbzheap_aseg_hasnext(s) bbzheap_tseg_hasnext(s)

/**
 * @brief Returns non-zero if the given segment is valid (in use).
 * @param s The array segment.
 * @return non-zero if the given segment is valid (in use).
 */
#define bbzheap_aseg_isvalid(s) bbzheap_tseg_isvalid(s)

/**
 * @brief Returns non-zero if the given segment element is valid (in use).
 * @param e The array segment element.
 * @return non-zero if the given segment element is valid (in use).
 */
#define bbzheap_aseg_elem_isvalid(e) bbzheap_tseg_elem_isvalid(e)

/**
 * @brief Returns the value of the given array segment element.
 * @param e The array segment element.
 */
#define bbzheap_aseg_elem_get(e) bbzheap_tseg_elem_get(e)

/**
 * @brief Sets the value of the given array segment element, and validates the element.
 * @param e The array segment element.
 * @param x The value.
 */
#define bbzheap_aseg_elem_set(e, x) bbzheap_tseg_elem_set(e, x)

/*
 * Performs garbage collection on the heap.
 * @param h The heap.
 * @param st The stack.
 * @param sz The stack size (number of elements in the stack).
 */
void bbzheap_gc(bbzheap_t* h,
                bbzheap_idx_t* st,
                int sz);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
