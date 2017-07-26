/**
 * @file bbzheap.h
 * @brief Definition of BittyBuzz's heap, which allows for allocation-like
 * operations.
 */

#ifndef BBZHEAP_H
#define BBZHEAP_H

#include "bbzinclude.h"
#include "bbztype.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define BBZ_DFLT_ACTREC ((uint8_t)0xFF)

/**
 * @brief A table segment.
 */
typedef struct PACKED {
    /**
      * @brief Table keys.
      * @details 16th bit: valid; other bits: obj index
      */
    bbzheap_idx_t keys[BBZHEAP_ELEMS_PER_TSEG];
    /**
      * @brief Table values.
      * @details 16th bit: valid; other bits: obj index.
      */
    bbzheap_idx_t values[BBZHEAP_ELEMS_PER_TSEG];
    /**
      * @brief Segment metadata.
      * @details 16th bit : valid
      * 15th-1st bits: next segment index (0x7FFF means no next)
      */
    uint16_t mdata;
} bbzheap_tseg_t;

/**
 * @brief An array segment.
 */
typedef struct PACKED {
    /**
      * @brief Array values.
      * @details 16th bit: valid; other bits: obj index.
      */
    bbzheap_idx_t values[2*BBZHEAP_ELEMS_PER_TSEG];
    /**
      * @brief Segment metadata.
      * @details 16th bit : valid
      * 15th-1st bits: next segment index (0x7FFF means no next)
      */
    uint16_t mdata;
} bbzheap_aseg_t;

/**
 * @brief The heap structure.
 *
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
 * 3. Each segment is composed of BBZHEAP_ELEMS_PER_TSEG (key,value) elements,
 *    each composed of a (bbzobj_t,bbzobj_t) pair. Each segment also has a
 *    2-byte field which contains flags and a pointer to the next
 *    segment, if any.
 */
typedef struct PACKED {
    uint8_t* rtobj;             /**< @brief Pointer to after the rightmost object in heap, not necessarly valid */
    uint8_t* ltseg;             /**< @brief Pointer to the leftmost table segment in heap, not necessarly valid */
    uint8_t data[BBZHEAP_SIZE]; /**< @brief Data buffer */
} bbzheap_t;

#ifdef DEBUG
void bbzheap_print();
#endif

/**
 * @brief Clears the heap.
 * Sets the entire heap to zero.
 */
void bbzheap_clear();

/**
 * @brief Allocates space for an object on the heap.
 * Sets as output the value of o, a buffer for the index of the allocated object.
 * The value of o is not checked for NULL, so make sure it's a valid pointer.
 * @param[in] t The type of the object.
 * @param[out] o A buffer for the index of the allocated object.
 * @return 1 for success, 0 for failure (out of memory)
 */
uint8_t bbzheap_obj_alloc(uint8_t t,
                          bbzheap_idx_t* o);

#define BBZHEAP_MASK_OBJ_VALID (uint8_t)0x10
#define BBZHEAP_MASK_PERMANENT (uint8_t)0x01

/**
 * @brief Returns a pointer located at position i within the heap.
 * @param[in] i The position (a bbzheap_idx_t).
 * @return A pointer to the object.
 */
#define bbzheap_obj_at(i) ((bbzobj_t*)vm->heap.data + i)

/**
 * @brief Returns non-zero if the given object is valid (i.e., in use).
 * @param[in] x The object.
 * @return non-zero if the given object is valid (i.e., in use).
 */
#define bbzheap_obj_isvalid(x) ((x).mdata & (uint8_t)BBZHEAP_MASK_OBJ_VALID)

/**
 *  @brief Copy the value of an object to an other object.
 *  @param [in] iSrc The position of the source object.
 *  @param [in] iDest The position of the destination object.
 */
#define bbzheap_obj_copy(iSrc, iDest) do{(*bbzheap_obj_at(iDest)) = (*bbzheap_obj_at(iSrc));}while(0)

/**
 * @brief Check if an object is permanent (should never be garbage collected).
 * @param[in] x The index of the object to check the permanence.
 * @return non-zero if the object is permanent.
 */
#define bbzheap_obj_ispermanent(x) ((x).mdata&BBZHEAP_MASK_PERMANENT)

/**
 * @brief Make an object permanent.
 * @param[in,out] x The object to make permanent.
 */
#define bbzheap_obj_make_permanent(x) do{(x).mdata|=BBZHEAP_MASK_PERMANENT;}while(0)

#define bbzheap_obj_remove_permanence(x) do{(x).mdata&=~BBZHEAP_MASK_PERMANENT;}while(0)

/**
 * @brief Allocates space for a table segment on the heap.
 * Sets as output the value of s, the index of the allocated segment.
 * @param[out] s A buffer for the pointer to the allocated segment.
 * @return 1 for success, 0 for failure (out of memory)
 */
uint8_t bbzheap_tseg_alloc(bbzheap_idx_t* s);

/**
 * Next segment index when the segment doesn't have any next.
 */
#define NO_NEXT (uint16_t)0x3FFF

/**
 * Mask for the next segment index.
 * @details This mask applies to the segment's metadata.
 */
#define MASK_NEXT (uint16_t)0x3FFF

/**
 * Mask for whether a segment is valid.
 * @details This mask applies to the segment's metadata.
 */
#define MASK_VALID_SEG (uint16_t)0x8000

/**
 * Mask for whether a segment is garbage-collection exempt.
 * @details This mask applies to the segment's metadata.
 */
#define TSEG_MASK_GCMARK (uint16_t)0x4000

/**
 * Mask for whether a segment element is valid.
 * @details This mask applies to the element itself.
 */
#define MASK_VALID_SEG_ELEM (uint16_t)0x8000

/**
 * @brief Returns a table segment located at position i within the heap.
 * @param[in] i The position.
 * @return A pointer to the table segment.
 */
#define bbzheap_tseg_at(i) ((bbzheap_tseg_t*)(vm->heap.data + BBZHEAP_SIZE) - ((i)+1))

/**
 * @brief Returns the next table segment linked to the given one.
 * @param[in] s The table segment.
 * @return The index of the next segment.
 */
#define bbzheap_tseg_next_get(s) ((s)->mdata & MASK_NEXT)

/**
 * @brief Sets the next table segment linked to the given one.
 * @param[in,out] s The table segment.
 * @param[in] n The index of the next segment.
 */
#define bbzheap_tseg_next_set(s, n) (s)->mdata = ((s)->mdata & ~MASK_NEXT) | ((n) & MASK_NEXT)

/**
 * @brief Returns 1 if the given table segment has a valid next, 0 otherwise.
 * @param[in] s The table segment.
 * @return 1 if the given table segment has a valid next, 0 otherwise.
 */
#define bbzheap_tseg_hasnext(s) (((s)->mdata & MASK_NEXT) != NO_NEXT)

/**
 * @brief Returns non-zero if the given segment is valid (in use).
 * @param[in] s The table segment.
 * @return non-zero if the given segment is valid (in use).
 */
#define bbzheap_tseg_isvalid(s) ((s).mdata & MASK_VALID_SEG)

/**
 * @brief Returns non-zero if the given segment element (key or value) is valid (in use).
 * @param[in] e The table segment element.
 * @return non-zero if the given segment element (key or value) is valid (in use).
 */
#define bbzheap_tseg_elem_isvalid(e) ((e) & MASK_VALID_SEG_ELEM)

/**
 * @brief Returns the value of the given table segment element.
 * @param[in] e The table segment element.
 */
#define bbzheap_tseg_elem_get(e) ((e) & ~MASK_VALID_SEG_ELEM)

/**
 * @brief Sets the value of the given table segment element, and validates the element.
 * @param[in,out] e The table segment element.
 * @param[in] x The value.
 */
#define bbzheap_tseg_elem_set(e, x) (e) = ((x) & ~MASK_VALID_SEG_ELEM) | MASK_VALID_SEG_ELEM

/**
 * @brief Allocates space for an array segment on the heap.
 * Sets as output the value of s, the index of the allocated segment.
 * @param[out] s A buffer for the pointer to the allocated segment.
 * @return 1 for success, 0 for failure (out of memory)
 */
#define bbzheap_aseg_alloc(s) bbzheap_tseg_alloc(s)

/**
 * @brief Returns an array segment located at position i within the heap.
 * @param[in] i The position.
 * @return A pointer to the array segment.
 */
#define bbzheap_aseg_at(i) ((bbzheap_aseg_t*)(vm->heap.data + BBZHEAP_SIZE) - ((i)+1))

/**
 * @brief Returns the next array segment linked to the given one.
 * @param[in] s The array segment.
 * @return The index of the next segment.
 */
#define bbzheap_aseg_next_get(s) bbzheap_tseg_next_get(s)

/**
 * @brief Sets the next array segment linked to the given one.
 * @param[out] s The array segment.
 * @param[in] n The index of the next segment.
 */
#define bbzheap_aseg_next_set(s, n) bbzheap_tseg_next_set(s, n)

/**
 * @brief Returns 1 if the given array segment has a valid next, 0 otherwise.
 * @param[in] s The array segment.
 * @return 1 if the given array segment has a valid next, 0 otherwise.
 */
#define bbzheap_aseg_hasnext(s) bbzheap_tseg_hasnext(s)

/**
 * @brief Returns non-zero if the given segment is valid (in use).
 * @param[in] s The array segment.
 * @return non-zero if the given segment is valid (in use).
 */
#define bbzheap_aseg_isvalid(s) bbzheap_tseg_isvalid(s)

/**
 * @brief Returns non-zero if the given segment element is valid (in use).
 * @param[in] e The array segment element.
 * @return non-zero if the given segment element is valid (in use).
 */
#define bbzheap_aseg_elem_isvalid(e) bbzheap_tseg_elem_isvalid(e)

/**
 * @brief Returns the value of the given array segment element.
 * @param[in] e The array segment element.
 */
#define bbzheap_aseg_elem_get(e) bbzheap_tseg_elem_get(e)

/**
 * @brief Sets the value of the given array segment element, and validates the element.
 * @param[in,out] e The array segment element.
 * @param[in] x The value.
 */
#define bbzheap_aseg_elem_set(e, x) bbzheap_tseg_elem_set(e, x)

/**
 * Performs garbage collection on the heap.
 * @param[in,out] st The stack.
 * @param[in] sz The stack size (number of elements in the stack).
 */
void bbzheap_gc(bbzheap_idx_t* st,
                uint16_t sz);

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks an object as currently in use, i.e., "allocated".
 * @param[in,out] obj The object to mark.
 */
#define obj_makevalid(obj)   (obj).mdata |= BBZHEAP_MASK_OBJ_VALID

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks an object as no longer in use, i.e., "not allocated".
 * @param[in,out] obj The object to mark.
 */
#define obj_makeinvalid(obj) (obj).mdata &= ~BBZHEAP_MASK_OBJ_VALID

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks a segment as currently in use, i.e., "allocated".
 * @param[in,out] s The segment to mark.
 */
#define tseg_makevalid(s) (s).mdata = (uint16_t)0xFFFF // Make the segment valid AND reset next to -1

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks a segment as no longer in use, i.e., "not allocated".
 * @param[in,out] s The segment to mark.
 */
#define tseg_makeinvalid(s) (s).mdata &= ~MASK_VALID_SEG


#ifndef BBZCROSSCOMPILING
#include <stdio.h>
/**
 * @brief Prints the heap's contents for debugging.
 */
void bbzheap_print();
#else // !BBZCROSSCOMPILING
#define bbzheap_print()
#endif // !BBZCROSSCOMPILING


/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks a segment as not garbage-collectable.
 * @param[in,out] s The segment to mark.
 */
#define gc_tseg_mark(s) (s).mdata |= TSEG_MASK_GCMARK

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Marks a segment as not garbage-collectable.
 * @param[in,out] s The segment to mark.
 */
#define gc_tseg_unmark(s) (s).mdata &= ~TSEG_MASK_GCMARK

/**
 * @brief <b>For the VM's internal use only</b>.
 *
 * Returns non-zero if given segment has a GC mark.
 * @param[in,out] s The segment to check.
 * @return non-zero if given segment has a GC mark.
 */
#define gc_tseg_hasmark(s) ((s).mdata & TSEG_MASK_GCMARK)

#ifdef __cplusplus
}
#endif // __cplusplus

#include "bbzvm.h"

#endif // !BBZHEAP_H
