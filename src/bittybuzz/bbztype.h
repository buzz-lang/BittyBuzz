/**
 * @file bbztype.h
 * @brief Definition of BittyBuzz's main types.
 */

#ifndef BBZTYPE_H
#define BBZTYPE_H

#include "bbzinclude.h"
#include "bbzfloat.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*
 * Object types in BittyBuzz
 */
#define BBZTYPE_NIL      (uint8_t)0 /**< @brief Nil type. */
#define BBZTYPE_INT      (uint8_t)1 /**< @brief Integer type. */
#define BBZTYPE_FLOAT    (uint8_t)2 /**< @brief Float type. */
#define BBZTYPE_STRING   (uint8_t)3 /**< @brief String type. */
#define BBZTYPE_TABLE    (uint8_t)4 /**< @brief Table type. */
#define BBZTYPE_CLOSURE  (uint8_t)5 /**< @brief Closure type. */
#define BBZTYPE_USERDATA (uint8_t)6 /**< @brief User-data type. */
#define BBZTYPE_NCLOSURE (uint8_t)7 /**< @brief Native closure type. */

/**
 * @brief Nil type
 */
typedef struct PACKED {
    uint8_t mdata; /**< @brief Object metadata. */
} bbznil_t;

/**
 * @brief 16-bit signed integer
 */
typedef struct PACKED {
    uint8_t mdata; /**< @brief Object metadata. */
    int16_t value; /**< @brief Integer's value. */
} bbzint_t;

/**
 * @brief Float
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    bbzfloat value; /**< @brief Float object's value. */
} bbzfloat_t;

/**
 * @brief String
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The string id */
} bbzstring_t;

/**
 * @brief Table
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The index of the first segment in the heap */
} bbztable_t;

/**
 * @brief Dynamic Array
 */
typedef struct PACKED {
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
typedef struct PACKED {
    /**
     * @brief Object metadata.
     * @details 7th topmost bit: 'native' flag.
     *          3rd topmost bit: 'lambda' flag.
     */
    uint8_t mdata;
    void* value; /**< @brief Closure object's value. */
} bbzclosure_t;

/**
 * @brief Lambda Closure
 */
typedef struct PACKED {
    /**
     * @brief Object metadata.
     * @details 7th topmost bit: 'native' flag.
     *          3rd topmost bit: 'lambda' flag.
     */
    uint8_t mdata;
    struct {
        /**
         * @brief Location of the closure.
         * @details Jump address (C closure) or function id (native closure).
         */
        uint8_t ref;
        /**
         * @brief Position in the heap of the activation record array.
         * @details A value of 0xFF means it uses the default activation record of the VM.
         */
        uint8_t actrec;
    } value; /**< @brief Closure object's value. */
} bbzlclosure_t;

/**
 * @brief User data
 */
typedef struct PACKED {
    uint8_t mdata; /**< @brief Object metadata. */
    void* value;   /**< @brief User value. */
} bbzuserdata_t;

/**
 * @brief A handle for a object
 */
typedef union PACKED {
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
    bbzlclosure_t l; /**< @brief Lambda closure object */
    bbzuserdata_t u; /**< @brief Data object */
} bbzobj_t;

/**
 * @brief Compares two objects.
 * @param[in] a The first object to compare.
 * @param[in] b The second object to compare.
 * @return <0 if a<b, 0 if a==b, >0 if a>b
 */
int8_t bbztype_cmp(const bbzobj_t* a,
                   const bbzobj_t* b);

/**
 * @brief Returns the type of an object.
 * @param[in] obj The object.
 */
#define bbztype(obj) ((obj).o.mdata >> 5)

/**
 * @brief Casts an object into a different type.
 * The cast is done without checking type compatibility.
 * @param[in,out] obj  The object.
 * @param[in] type The type.
 */
#define bbztype_cast(obj, type) (obj).o.mdata = (((obj).o.mdata & 0x1F) | (type << 5))

/**
 * @brief Returns 1 if an object is nil, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isnil(obj) (bbztype(obj) == BBZTYPE_NIL)

/**
 * @brief Returns 1 if an object is int, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isint(obj) (bbztype(obj) == BBZTYPE_INT)

/**
 * @brief Returns 1 if an object is float, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isfloat(obj) (bbztype(obj) == BBZTYPE_FLOAT)

/**
 * @brief Returns 1 if an object is string, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isstring(obj) (bbztype(obj) == BBZTYPE_STRING)

/**
 * @brief Returns 1 if an object is table, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_istable(obj) (bbztype(obj) == BBZTYPE_TABLE)

/**
 * @brief Returns 1 if an object is dynamic array, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isdarray(obj) (bbztype(obj) == BBZTYPE_TABLE && ((obj).o.mdata & 0x04))

/**
 * @brief Returns 1 if an object is closure, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosure(obj) ((bbztype(obj) & BBZTYPE_CLOSURE) == BBZTYPE_CLOSURE)

/**
 * @brief Returns 1 if an object is userdata, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isuserdata(obj) (bbztype(obj) == BBZTYPE_USERDATA)

/**
 * @brief Returns 1 if a closure is native, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosurenative(obj) ((obj).c.mdata & 0x40)

/**
 * @brief Returns 1 if a closure is a lambda closure, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosurelambda(obj) ((obj).c.mdata & 0x04)

#define bbzclosure_make_lambda(obj) ((obj).l.mdata |= 0x04)
#define bbzclosure_unmake_lambda(obj) ((obj).l.mdata &= ~0x04)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZTYPE_H
