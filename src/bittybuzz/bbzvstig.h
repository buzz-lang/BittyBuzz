#ifndef BBZVSTIG
#define BBZVSTIG

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct __attribute__((packed)) {
   bbzobj_t key;
   bbzobj_t value;
   uint8_t timestamp;
   uint8_t robot;
} bbzvstig_elem_t;

typedef struct __attribute__((packed)) {
   bbzvstig_elem_t* data;
   uint8_t capacity;
   uint8_t size;
} bbzvstig_t;

/**
 * @brief Creates a virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @param buf The linear buffer associated to this structure.
 * @param cap The maximum number of elements in the structure.
 */
#define bbzvstig_new(vs, buf, cap) (vs).data = buf; (vs).capacity = cap; (vs).size = 0;

/**
 * @brief Returns the capacity of the virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @return The capacity of the virtual stigmergy structure.
 */
#define bbzvstig_capacity(vs) ((vs).capacity)

/**
 * @brief Returns the size of the virtual stigmergy structure.
 * @param vs The virtual stigmergy structure.
 * @return The size of the virtual stigmergy structure.
 */
#define bbzvstig_size(vs) ((vs).size)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
