/**
 * @file bbzdarray.h
 * @brief Definition of dynamic arrays, which are not a feature of Buzz but,
 * rather, a means of implementation.
 */

#ifndef BBZDARRAY_H
#define BBZDARRAY_H

#include "bbzincludes.h"
#include "bbzheap.h"
#include "bbztype.h"

#ifndef NULL
#define NULL 0
#endif

#define bbzdarray_mark_cloned(d) (d)->mdata |= 0x02
#define bbzdarray_unmark_cloned(d) (d)->mdata &= ~0x02
#define bbzdarray_iscloned(d) ((d)->mdata & 0x02)

#define bbzdarray_mark_swarm(d) (d)->mdata |= 0x01
#define bbzdarray_unmark_swarm(d) (d)->mdata &= ~0x01
#define bbzdarray_isswarm(d) ((d)->mdata & 0x01)

#ifdef __cplusplus
extern "C" {
#endif

    /**
     *  @brief Function pointer for an element-wise function:
     *
     *  void f(bbzheap_t* h, bbzheap_idx_t darray, uint16_t pos, void* params)
     *
     *  This function pointer is used to destroy elements by
     *  bbzdarray_destroy() and in methods such as
     *  bbzdarray_foreach().
     */
    typedef void (*bbzdarray_elem_funp)(bbzheap_idx_t darray, uint16_t pos, void* params);

    /**
     *  @brief Function pointer to compare bbzdarray elements.
     *
     *  int8_t f(const bbzobj_t* a, const bbzobj_t* b)
     *
     *  The function must return:
     *  <0 if *a <  *b
     *   0 if *a == *b
     *  >0 if *a >  *b
     */
    typedef int8_t (*bbzdarray_elem_cmpp)(const bbzobj_t* a, const bbzobj_t* b);

    /**
     *  @brief Allocate a dynamic array.
     *  @param [out] d A buffer for position of the dynamic array's object in
     *                 the heap.
     *  @return 1 for success, 0 for failure (out of memory).
     */
    uint8_t bbzdarray_new(bbzheap_idx_t* d);

    /**
     *  @brief Destroys a dynamic array. Invalidates all its array segment.
     *  @param[in] d The dynamic array.
     */
    void bbzdarray_destroy(bbzheap_idx_t d);

    /**
     *  @brief Searchs the value associated with the key k in the dynamic array
     *         d from the heap h. Set as output the value of v, the found value
     *         corresponding to the key k.
     *  @param[in] d The position of the dynamic array's object in the heap.
     *  @param[in] idx The index of the element to get.
     *  @param[out] v A buffer for the pointer to the value.
     *  @return 1 for success, 0 for failure (index not in table)
     */
    uint8_t bbzdarray_get(bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t* v);

    /**
     *  @brief Changes the value corresponding to the key k in the array d
     *         from the heap h.
     *         The key has to be in the range of the array's size.
     *  @param[in] d The position of the dynamic array's object in the heap.
     *  @param[in] idx The index of the element to get.
     *  @param[in] v The value to set.
     *  @return 1 for success, 0 for failure (index out of range)
     */
    uint8_t bbzdarray_set(bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t v);

    /**
     *  @brief Remove the value at the end of the array d.
     *  @param [in] d The position of the dynamic array's object in the heap.
     *  @return 1 for success, 0 for failure (empty array)
     */
    uint8_t bbzdarray_pop(bbzheap_idx_t d);

    /**
     *  @brief Insert a value v at the end of the array d.
     *  @param [in] d The position of the dynamic array's object in the heap.
     *  @param [in] v The position in the heap of the object to push. 
     *  @return 1 for success, 0 for failure (out of memory)
     */
    uint8_t bbzdarray_push(bbzheap_idx_t d,
                           bbzheap_idx_t v);

    /**
     *  @brief Return the count of sequential valid values in the table.
     *  @param[in] d The position of the dynamic array's object in the heap.
     *  @return The size of the dynamic array.
     */
    uint16_t bbzdarray_size(bbzheap_idx_t d);
    
    /**
     *  @brief Creates a new dynamic array from the given dynamic array.
     *  @param[in] d The dynamic array.
     *  @param[out] newd A buffer for the new dynamic array.
     *  @return 1 for success, 0 for failure (out of memory)
     */
    uint8_t bbzdarray_clone(bbzheap_idx_t d,
                        bbzheap_idx_t* newd);

    /**
     *  @brief Erases all the elements of the dynamic array.
     *  @param[in] d The dynamic array.
     */
    void bbzdarray_clear(bbzheap_idx_t d);
    
    /**
     *  @brief Applies a function to each element of the dynamic array.
     *  @param[in] d The dynamic array.
     *  @param[in] fun The function to apply to each element.
     *  @param[in,out] params A data structure to pass along.
     */
    void bbzdarray_foreach(bbzheap_idx_t d,
                           bbzdarray_elem_funp fun,
                           void* params);
    
    /**
     *  @brief Finds the position of an element.
     *         If the element is not found, the returned position
     *         is equal to the darray size.
     *  @param[in] d The dynamic array.
     *  @param[in] cmp The element comparison function.
     *  @param[in] data The position in the heap of the element to find.
     *  @return The position of the found element, or the size of the array if
     *          not found.
     */
    uint16_t bbzdarray_find(bbzheap_idx_t d,
                            bbzdarray_elem_cmpp cmp,
                            bbzheap_idx_t data);

    /**
     * @brief Allocates space for a lambda closure on the heap.
     * Sets as output the value of l, a buffer for the index of the allocated closure.
     * The value of l is not checked for NULL, so make sure it's a valid pointer.
     * @param[in] d Dynamic array from which we will copy data.
     * @param[out] l A buffer for the index of the allocated closure.
     * @return 1 for success, 0 for failure (out of memory)
     */
    uint8_t bbzdarray_lambda_alloc(bbzheap_idx_t d, uint8_t* l);

#ifdef __cplusplus
}
#endif

/**
 *  @brief Returns the last element in the dynamic array.
 *  @param[in] d The dynamic array.
 *  @param[out] v A buffer for the pointer to the value.
 *  @return The last element in the dynamic array.
 */
#define bbzdarray_last(d, v) bbzdarray_get(d, bbzdarray_size(d)-1, v)

/**
 *  @brief Returns <tt>true</tt> if the dynamic array is empty.
 *  @param[in] d The dynamic array.
 *  @return <tt>true</tt> if the dynamic array is empty.
 */
#define bbzdarray_isempty(d) (bbzdarray_size(d) == 0)

#endif // !BBZDARRAY_H
