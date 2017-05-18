#ifndef BBZVSTIG
#define BBZVSTIG

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief Virtual stigmergy element.
 */
typedef struct __attribute__((packed)) {
   bbzobj_t key;      /**< @brief Element's key. */
   bbzobj_t value;    /**< @brief Element's current value. */
   uint8_t timestamp; /**< @brief Timestamp (Lamport clock) since last update of the value. */
   uint8_t robot;     /**< @brief Robot ID. */
} bbzvstig_elem_t;

/**
 * @brief Virtual stigmergy.
 */
typedef struct __attribute__((packed)) {
   bbzvstig_elem_t* data; /**< @brief Stigmergy elements. */
   uint8_t capacity;      /**< @brief Allocated stigmergy element capacity. */
   uint8_t size;          /**< @brief Number of stigmergy elements. */
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
