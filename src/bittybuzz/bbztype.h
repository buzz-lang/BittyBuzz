/**
 * @file bbztype.h
 * @brief Definition of BittyBuzz's main types.
 */

#ifndef BBZTYPE_H
#define BBZTYPE_H

#include "bbzinclude.h"
#include "bbzfloat.h"
#include "bbzringbuf.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define BBZTYPE_TYPEDEP_FLAG1 1
#define BBZTYPE_TYPEDEP_FLAG2 0

#define BBZCLOSURE_NATIVE_MASK (1 << BBZTYPE_TYPEDEP_FLAG1)
#define BBZCLOSURE_LAMBDA_MASK (1 << BBZTYPE_TYPEDEP_FLAG2)

#define BBZTABLE_DARRAY_MASK (1 << BBZTYPE_TYPEDEP_FLAG1)
#define BBZTABLE_DARRAY_SWARM_MASK (1 << BBZTYPE_TYPEDEP_FLAG2)

/**
 * @brief Object types in BittyBuzz
 */
typedef enum bbztype_t {
    BBZTYPE_NIL = 0,    /**< @def a @brief Nil type. */
    BBZTYPE_INT,        /**< @def Integer type. */
    BBZTYPE_FLOAT,      /**< @def Float type. */
    BBZTYPE_STRING,     /**< @def String type. */
    BBZTYPE_TABLE,      /**< @def Table type. */
    BBZTYPE_CLOSURE,    /**< @def Closure type. */
    BBZTYPE_USERDATA,   /**< @def User-data type. */
} bbztype_t;
//#define BBZTYPE_NIL      ((uint8_t)0) /**< @def a @brief Nil type. */
//#define BBZTYPE_INT      ((uint8_t)1) /**< @def Integer type. */
//#define BBZTYPE_FLOAT    ((uint8_t)2) /**< @def Float type. */
//#define BBZTYPE_STRING   ((uint8_t)3) /**< @def String type. */
//#define BBZTYPE_TABLE    ((uint8_t)4) /**< @def Table type. */
//#define BBZTYPE_CLOSURE  ((uint8_t)5) /**< @def Closure type. */
//#define BBZTYPE_USERDATA ((uint8_t)6) /**< @def User-data type. */

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
 * @brief Float type
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    bbzfloat value; /**< @brief Float object's value. */
} bbzfloat_t;

/**
 * @brief String type
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The string id */
} bbzstring_t;

/**
 * @brief Table type
 */
typedef struct PACKED {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The index of the first segment in the heap */
} bbztable_t;

/**
 * @brief Dynamic Array type
 */
typedef struct PACKED {
    /**
     * @brief Object metadata.
     * @details 2nd bit: 1 if is dynamic array
     */
    uint8_t mdata;
    /**
     * @brief Index of the first segment in the heap.
     */
    uint16_t value;
} bbzdarray_t;

/**
 * @brief Closure type
 */
typedef struct PACKED {
    /**
     * @brief Object metadata.
     * @details 7th topmost bit: 'native' flag.
     *          2nd topmost bit: 'lambda' flag.
     */
    uint8_t mdata;
    void (*value)(); /**< @brief Closure object's value. */
} bbzclosure_t;

/**
 * @brief Lambda closure type
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
 * @brief User data type
 */
typedef struct PACKED {
    uint8_t mdata; /**< @brief Object metadata. */
    uintptr_t value;   /**< @brief User value. */
} bbzuserdata_t;

/**
 * @brief A handle for a object
 */
typedef union PACKED {
    uint8_t mdata;
    struct {
        /**
         * @brief Object metadata.
         * @details 8th,7th,6th bit for type
         * 5th bit for valid in heap
         * 4th bit for garbage collection
         * 3rd bit for type dependent usage ('is_darray' flag for tables; 'is_lambda' flag for closures)
         * 2nd bit for type dependent usage ('is_swarm' flag for darrays)
         * 1st bit for permanence flag (if set, the object will not be garbage collected).
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

    bbzuserdata_t biggest; /**< @brief The object that takes the most space */
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
#define bbztype(obj) ((obj).mdata >> 5)

/**
 * @brief Determines whether an object should be considered as true.
 * @param[in] o The object to verify.
 * @return 1 if the object can be considered true, otherwise 0.
 */
ALWAYS_INLINE
uint8_t bbztype_tobool(const bbzobj_t* o) {
    switch(bbztype(*o)) {
        case BBZTYPE_INT: return o->i.value != 0;
        default: return 0;
    }
}

/**
 * @brief Casts an object into a different type.
 * The cast is done without checking type compatibility.
 * @param[in,out] obj  The object.
 * @param[in] type The type.
 */
#define bbztype_cast(obj, type) {(obj).mdata = (((obj).mdata & ~(0x07 << 5)) | ((type) << 5));}

/**
 * @brief Copy the type from the source object to the destination object
 * @param[in]   src  The source object from which the type will be copied.
 * @param[out]  dest The destination object to which the type will be copied.
 */
#define bbztype_copy(src, dest) {(dest).mdata = ((dest).mdata & ~(0x07 << 5)) | ((src).mdata & (0x07 << 5));}

/**
 * @brief Returns 1 if an object is of the specified type, 0 otherwise.
 * @param[in] obj  The object.
 * @param[in] type The type to assert.
 * @return 1 if an object is of the specified type, 0 otherwise.
 */
#define bbztype_is(obj, type) (((obj).mdata & (0x07 << 5)) == (type) << 5)

/**
 * @brief Returns 1 if an object is nil, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isnil(obj) bbztype_is(obj, BBZTYPE_NIL)

/**
 * @brief Returns 1 if an object is int, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isint(obj) bbztype_is(obj, BBZTYPE_INT)

/**
 * @brief Returns 1 if an object is float, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isfloat(obj) bbztype_is(obj, BBZTYPE_FLOAT)

/**
 * @brief Returns 1 if an object is string, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isstring(obj) bbztype_is(obj, BBZTYPE_STRING)

/**
 * @brief Returns 1 if an object is table, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_istable(obj) bbztype_is(obj, BBZTYPE_TABLE)

/**
 * @brief Returns 1 if an object is dynamic array, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isdarray(obj) (bbztype_istable(obj) && ((obj).mdata & BBZTABLE_DARRAY_MASK))

/**
 * @brief Returns 1 if an object is closure, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosure(obj) bbztype_is(obj, BBZTYPE_CLOSURE)

/**
 * @brief Returns 1 if an object is userdata, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isuserdata(obj) bbztype_is(obj, BBZTYPE_USERDATA)

/**
 * @brief Returns non-0 if a closure is native, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosurenative(obj) ((obj).mdata & BBZCLOSURE_NATIVE_MASK)

#define bbzclosure_make_native(obj) do{(obj).mdata |= BBZCLOSURE_NATIVE_MASK;}while(0)
#define bbzclosure_unmake_native(obj) do{(obj).mdata &= ~BBZCLOSURE_NATIVE_MASK;}while(0)

/**
 * @brief Returns non-0 if a closure is a lambda closure, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosurelambda(obj) ((obj).mdata & BBZCLOSURE_LAMBDA_MASK)

#define bbzclosure_make_lambda(obj) do{(obj).mdata |= BBZCLOSURE_LAMBDA_MASK;}while(0)
#define bbzclosure_unmake_lambda(obj) do{(obj).mdata &= ~BBZCLOSURE_LAMBDA_MASK;}while(0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZTYPE_H
