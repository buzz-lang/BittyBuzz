#ifndef BBZDARRAY
#define BBZDARRAY

#include <bittybuzz/bbzheap.h>
#include <bittybuzz/bbztype.h>

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /**
     *  @brief Function pointer for an element-wise function:
     *
     *  void f(bbzheap_t* h, uint16_t darray, uint16_t pos, void* params)
     *
     *  This function pointer is used to destroy elements by
     *  bbzdarray_destroy() and in methods such as
     *  bbzdarray_foreach().
     */
    typedef void (*bbzdarray_elem_funp)(bbzheap_t* h, uint16_t darray, uint16_t pos, void* params);

    /**
     *  @brief Function pointer to compare buzzdarray elements.
     *
     *  int f(const bbzobj_t* a, const bbzobj_t* b)
     *
     *  The function must return:
     *  -1 if *a < *b
     *  0  if *a == *b
     *  1  if *a > *b
     */
    typedef int (*bbzdarray_elem_cmpp)(const bbzobj_t* a, const bbzobj_t* b);

    /**
     *  @brief Allocate a dynamic array.
     *  @param [in]  h A pointer to the heap.
     *  @param [out] d A buffer for position of the dynamic array's object in
     *                 the heap.
     *  @return 1 for success, 0 for failure (out of memory).
     */
    int bbzdarray_new(bbzheap_t* h,
                       bbzheap_idx_t* d);

    /**
     *  @brief Destroys a dynamic array. Invalidates all its array segment.
     *  @param h A pointer to the heap.
     *  @param d The dynamic array.
     */
    void bbzdarray_destroy(bbzheap_t* h,
                           bbzheap_idx_t d);

    /**
     *  @brief Searchs the value associated with the key k in the dynamic array
     *         d from the heap h. Set as output the value of v, the found value
     *         corresponding to the key k.
     *  @param h A pointer to the heap.
     *  @param d The position of the dynamic array's object in the heap.
     *  @param idx The index of the element to get.
     *  @param v A buffer for the pointer to the value.
     *  @return 1 for success, 0 for failure (index not in table)
     */
    int bbzdarray_get(bbzheap_t* h,
                      bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t* v);

    /**
     *  @brief Changes the value corresponding to the key k in the array d
     *         from the heap h.
     *         The key has to be in the range of the array's size.
     *  @param h A pointer to the heap.
     *  @param d The position of the dynamic array's object in the heap.
     *  @param idx The index of the element to get.
     *  @param v The value to set.
     *  @return 1 for success, 0 for failure (index out of range)
     */
    int bbzdarray_set(bbzheap_t* h,
                      bbzheap_idx_t d,
                      uint16_t idx,
                      bbzheap_idx_t v);

    /**
     *  @brief Remove the value at the end of the array d.
     *  @param [in] h A pointer to the heap.
     *  @param [in] d The position of the dynamic array's object in the heap.
     *  @return 1 for success, 0 for failure (empty array)
     */
    int bbzdarray_pop(bbzheap_t* h,
                      bbzheap_idx_t d);

    /**
     *  @brief Insert a value v at the end of the array d.
     *  @param [in] h A pointer to the heap.
     *  @param [in] d The position of the dynamic array's object in the heap.
     *  @param [in] v The position in the heap of the object to push. 
     *  @return 1 for success, 0 for failure (out of memory)
     */
    int bbzdarray_push(bbzheap_t* h,
                       bbzheap_idx_t d,
                       bbzheap_idx_t v);

    /**
     *  @brief Return the count of sequential valid values in the table.
     *  @param h A pointer to the heap.
     *  @param d The position of the dynamic array's object in the heap.
     *  @return The size of the dynamic array.
     */
    uint16_t bbzdarray_size(bbzheap_t* h,
                           bbzheap_idx_t d);
    
    /**
     *  @brief Creates a new dynamic array from the given dynamic array.
     *  @param h A pointer to the heap.
     *  @param d The dynamic array.
     *  @param newd A buffer for the new dynamic array.
     *  @return 1 for success, 0 for failure (out of memory)
     */
    int bbzdarray_clone(bbzheap_t* h,
                        bbzheap_idx_t d,
                        bbzheap_idx_t* newd);

    /**
     *  @brief Erases all the elements of the dynamic array.
     *  @param h A pointer to the heap.
     *  @param d The dynamic array.
     */
    void bbzdarray_clear(bbzheap_t* h,
                         bbzheap_idx_t d);
    
    /**
     *  @brief Applies a function to each element of the dynamic array.
     *  @param h A pointer to the heap.
     *  @param d The dynamic array.
     *  @param fun The function to apply to each element.
     *  @param params A data structure to pass along.
     */
    void bbzdarray_foreach(bbzheap_t* h,
                           bbzheap_idx_t d,
                           bbzdarray_elem_funp fun,
                           void* params);
    
    /**
     *  @brief Finds the position of an element.
     *         If the element is not found, the returned position
     *         is equal to the darray size.
     *  @param h A pointer to the heap.
     *  @param d The dynamic array.
     *  @param cmp The element comparison function.
     *  @param data The position in the heap of the element to find.
     *  @return The position of the found element, or the size of the array if
     *          not found.
     */
    uint16_t bbzdarray_find(bbzheap_t* h,
                            bbzheap_idx_t d,
                            bbzdarray_elem_cmpp cmp,
                            bbzheap_idx_t data);

#ifdef __cplusplus
}
#endif

/**
 *  @brief Returns the last element in the dynamic array.
 *  @param h A pointer to the heap.
 *  @param d The dynamic array.
 *  @param v A buffer for the pointer to the value. 
 *  @return The last element in the dynamic array.
 */
#define bbzdarray_last(h, d, v) bbzdarray_get(h, d, bbzdarray_size(h, d)-1, v)

/**
 *  @brief Returns <tt>true</tt> if the dynamic array is empty.
 *  @param h A pointer to the heap.
 *  @param d The dynamic array.
 *  @return <tt>true</tt> if the dynamic array is empty.
 */
#define bbzdarray_isempty(h, d) (bbzdarray_size(h, d) == 0)

#endif
