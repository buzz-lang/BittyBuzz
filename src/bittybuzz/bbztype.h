#ifndef BBZTYPE
#define BBZTYPE

#include <bittybuzz/bbzfloat.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Object types in KiloBuzz
 */
#define BBZTYPE_NIL      0
#define BBZTYPE_INT      1
#define BBZTYPE_FLOAT    2
#define BBZTYPE_STRING   3
#define BBZTYPE_TABLE    4
#define BBZTYPE_CLOSURE  5
#define BBZTYPE_USERDATA 6
#define BBZTYPE_NCLOSURE 7

/**
 * @brief Nil type
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata; /**< @brief Object metadata. */
} bbznil_t;

/**
 * @brief 16-bit signed integer
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata; /**< @brief Object metadata. */
   int16_t value; /**< @brief Integer's value. */
} bbzint_t;

/**
 * @brief Float
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata; /**< @brief Object metadata. */
   bbzfloat value;
} bbzfloat_t;

/**
 * @brief String
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;  /**< @brief Object metadata. */
   uint16_t value; /**< @brief The string id */
} bbzstring_t;

/**
 * @brief Table
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;  /**< @brief Object metadata. */
   uint16_t value; /**< @brief The index of the first segment in the heap */
} bbztable_t;

/**
 * @brief Dynamic Array
 */
typedef struct __attribute__((packed)) {
   /**
    * @brief Object metadata.
    * @details 3rd bit: 1 if is dynamic array,
    * 2nd bit: 1 if elements need invalidation on array destroy
    */
   uint8_t mdata;
   /**
    * @brief Index of the first segment in the heap.
    */
   uint16_t value;
} bbzdarray_t;

/**
 * @brief Closure
 */
typedef struct __attribute__((packed)) {
   /**
    * @brief Object metadata.
    * @details 7th topmost bit: 'native' flag.
    */
   uint8_t mdata;
   struct {
      /**
       * @brief Location of the closure.
       * @details Jump address (C closure) or function id (native closure).
       */
      int8_t ref;
      /**
       * @brief Position in the heap of the activation record array.
       * @details A value of 0xFF means it uses the default activation record of the VM.
       */
      uint8_t actrec;
   } value;
} bbzclosure_t;

/**
 * @brief User data
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata; /**< @brief Object metadata. */
   void* value;   /**< @brief User value. */
} bbzuserdata_t;

/**
 * @brief A handle for a object
 */
typedef union __attribute__((packed)) {
   struct {
      /**
       * @brief Object metadata.
       * @details 8th,7th,6th bit for type
       * 5th bit for valid in heap
       * 4th bit for garbage collection
       */
      uint8_t mdata; 
   }             o; /**< @brief Generic object */
   bbznil_t      n; /**< @brief Nil object */
   bbzint_t      i; /**< @brief Integer object */
   bbzfloat_t    f; /**< @brief Floating-point object */
   bbzstring_t   s; /**< @brief String object */
   bbztable_t    t; /**< @brief Table object */
   bbzclosure_t  c; /**< @brief Closure object */
   bbzuserdata_t u; /**< @brief Data object */
} bbzobj_t;

/**
 * @brief Compares two objects.
 * @param a The first object to compare.
 * @param b The second object to compare.
 * @return -1 if a<b, 0 if a==b, 1 if a>b
 */
int bbztype_cmp(const bbzobj_t* a,
                const bbzobj_t* b);

/**
 * @brief Returns the type of an object.
 * @param obj The object.
 */
#define bbztype(obj) ((obj).o.mdata >> 5)

/**
 * @brief Casts an object into a different type.
 * The cast is done without checking type compatibility.
 * @param obj  The object.
 * @param type The type.
 */
#define bbztype_cast(obj, type) (obj).o.mdata = (((obj).o.mdata & 0x1F) | (type << 5))

/**
 * @brief Returns 1 if an object is nil, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isnil(obj) (bbztype(obj) == BBZTYPE_NIL)

/**
 * @brief Returns 1 if an object is int, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isint(obj) (bbztype(obj) == BBZTYPE_INT)

/**
 * @brief Returns 1 if an object is float, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isfloat(obj) (bbztype(obj) == BBZTYPE_FLOAT)

/**
 * @brief Returns 1 if an object is string, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isstring(obj) (bbztype(obj) == BBZTYPE_STRING)

/**
 * @brief Returns 1 if an object is table, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_istable(obj) (bbztype(obj) == BBZTYPE_TABLE)

/**
 * @brief Returns 1 if an object is dynamic array, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isdarray(obj) (bbztype(obj) == BBZTYPE_TABLE && ((obj).o.mdata & 0x04))

/**
 * @brief Returns 1 if an object is closure, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isclosure(obj) ((bbztype(obj) & BBZTYPE_CLOSURE) == BBZTYPE_CLOSURE)

/**
 * @brief Returns 1 if an object is userdata, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isuserdata(obj) (bbztype(obj) == BBZTYPE_USERDATA)

/**
 * @brief Returns 1 if a closure is native, 0 otherwise.
 * @param obj The object.
 */
#define bbzclosure_isnative(obj) ((obj).c.mdata & 0x40)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
