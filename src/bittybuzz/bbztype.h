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

// ---------------------------------
// - Metadata field-related macros -
// ---------------------------------

// Index of flags and values in the metadata field of an object.

/**
 * @brief Index of the object's type
 */
#define BBZTYPE_TYPEIDX 0
/**
 * @brief Base mask of the object's type. Used in combinasion with #BBZTYPE_TYPEIDX
 */
#define BBZTYPE_BASEMASK 0x07
/**
 * @brief Index of the object's validity flag
 */
#define BBZTYPE_OBJ_VALID_FLAG_IDX 3
/**
 * @brief Index of the object's permanence flag
 */
#define BBZTYPE_OBJ_PERM_FLAG_IDX 4
/**
 * @brief Index of the garbage-collection marking flag
 */
#define BBZTYPE_GCMARK_FLAG_IDX 5
/**
 * @brief Index of the type-dependent flag #1
 */
#define BBZTYPE_TYPEDEP_FLAG1_IDX 6
/**
 * @brief Index of the type-dependent flag #2
 */
#define BBZTYPE_TYPEDEP_FLAG2_IDX 7

/**
 * @brief Mask for the object's type
 */
#define BBZTYPE_MASK ((uint8_t)(BBZTYPE_BASEMASK << BBZTYPE_TYPEIDX))
/**
 * @brief Mask for the closure's native flag
 */
#define BBZCLOSURE_NATIVE_MASK ((uint8_t)(1 << BBZTYPE_TYPEDEP_FLAG1_IDX))
/**
 * @brief Mask for the closure's lambda flag
 */
#define BBZCLOSURE_LAMBDA_MASK ((uint8_t)(1 << BBZTYPE_TYPEDEP_FLAG2_IDX))

/**
 * @brief Mask for the dynamic array's flag
 */
#define BBZTABLE_DARRAY_MASK ((uint8_t)(1 << BBZTYPE_TYPEDEP_FLAG1_IDX))

/**
 * @brief Mask for the object's heap "validity" flag (if it is used or not)
 */
#define BBZHEAP_OBJ_MASK_VALID ((uint8_t)(1 << BBZTYPE_OBJ_VALID_FLAG_IDX))
/**
 * @brief Mask for the object's permanence flag (if it is always in use or not)
 */
#define BBZHEAP_MASK_PERMANENT ((uint8_t)(1 << BBZTYPE_OBJ_PERM_FLAG_IDX))
/**
 * @brief Mask for garbage-collector's marking flag (if an object should be kept valid or not)
 */
#define BBZHEAP_MASK_GCMARK ((uint8_t)(1 << BBZTYPE_GCMARK_FLAG_IDX))

/**
 * @brief Object types in BittyBuzz
 */
typedef enum bbztype_t {
    BBZTYPE_NIL = 0,    /**< @brief Nil type. */
    BBZTYPE_INT,        /**< @brief Integer type. */
    BBZTYPE_FLOAT,      /**< @brief Float type. */
    BBZTYPE_STRING,     /**< @brief String type. */
    BBZTYPE_TABLE,      /**< @brief Table type. */
    BBZTYPE_CLOSURE,    /**< @brief Closure type. */
    BBZTYPE_USERDATA,   /**< @brief User-data type. */
} bbztype_t;

/**
 * @brief Nil type
 */
typedef struct PACKED bbznil_t {
    uint8_t mdata; /**< @brief Object metadata. */
} bbznil_t;

/**
 * @brief 16-bit signed integer
 */
typedef struct PACKED bbzint_t {
    uint8_t mdata; /**< @brief Object metadata. */
    int16_t value; /**< @brief Integer's value. */
} bbzint_t;

/**
 * @brief Float type
 */
typedef struct PACKED bbzfloat_t {
    uint8_t mdata;  /**< @brief Object metadata. */
    bbzfloat value; /**< @brief Float object's value. */
} bbzfloat_t;

/**
 * @brief String type
 */
typedef struct PACKED bbzstring_t {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The string id */
} bbzstring_t;

/**
 * @brief Table type
 */
typedef struct PACKED bbztable_t {
    uint8_t mdata;  /**< @brief Object metadata. */
    uint16_t value; /**< @brief The index of the first segment in the heap */
} bbztable_t;

/**
 * @brief Dynamic Array type
 */
typedef struct PACKED bbzdarray_t {
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
typedef struct PACKED bbzclosure_t {
    /**
     * @brief Object metadata.
     * @details 2nd bit: 'native' flag.<br/>
     *          1st bit: 'lambda' flag.
     */
    uint8_t mdata;
    void (*value)(); /**< @brief Closure object's value. */
} bbzclosure_t;

/**
 * @brief A lambda closure object's value type
 */
typedef struct PACKED bbzlclosure_value_t {
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
} bbzlclosure_value_t;

/**
 * @brief Lambda closure type
 */
typedef struct PACKED bbzlclosure_t {
    /**
     * @brief Object metadata.
     * @details 2nd bit: 'native' flag.<br/>
     *          1st bit: 'lambda' flag.
     */
    uint8_t mdata;
    bbzlclosure_value_t value; /**< @brief Closure object's value. */
} bbzlclosure_t;

/**
 * @brief User data type
 */
typedef struct PACKED bbzuserdata_t {
    uint8_t mdata; /**< @brief Object metadata. */
    uintptr_t value;   /**< @brief User value. */
} bbzuserdata_t;

/**
 * @brief Structure with fields common to all objects.
 */
typedef struct PACKED bbzobj_base_t {
    /**
     * @brief Object metadata.
     * @details 8th bit for type dependent usage ('is_lambda' flag for closures)<br/>
     * 7th bit for type dependent usage ('is_darray' flag for tables; 'is_native' flag for closures)<br/>
     * 6th bit for garbage collection marking<br/>
     * 5th bit for permanence flag (if set, the object will not be garbage collected)<br/>
     * 4th bit for validity in heap<br/>
     * 3rd,2nd,1st bit for type
     */
    uint8_t mdata;
} bbzobj_base_t;

/**
 * @brief A handle for a object
 */
typedef union PACKED bbzobj_t {
    uint8_t       mdata; /**< @brief Object metadata @see bbzobj_base_t::mdata */
    bbzobj_base_t o; /**< @brief Generic object */
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
#define bbztype(obj) (((obj).mdata & BBZTYPE_MASK) >> BBZTYPE_TYPEIDX)

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
#define bbztype_cast(obj, type) {(obj).mdata = (((obj).mdata & ~BBZTYPE_MASK) | ((type) << BBZTYPE_TYPEIDX));}

/**
 * @brief Copy the type from the source object to the destination object
 * @param[in]   src  The source object from which the type will be copied.
 * @param[out]  dest The destination object to which the type will be copied.
 */
#define bbztype_copy(src, dest) {(dest).mdata = ((dest).mdata & ~BBZTYPE_MASK) | ((src).mdata & BBZTYPE_MASK);}

/**
 * @brief Returns 1 if an object is of the specified type, 0 otherwise.
 * @param[in] obj  The object.
 * @param[in] type The type to assert.
 * @return 1 if an object is of the specified type, 0 otherwise.
 */
#define bbztype_is(obj, type) (((obj).mdata & BBZTYPE_MASK) == ((type) << BBZTYPE_TYPEIDX))

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
/**
 * @brief Set the "native" flag of a closure.
 * @param[in,out] obj The closure object to make native.
 */
#define bbzclosure_make_native(obj) do{(obj).mdata |= BBZCLOSURE_NATIVE_MASK;}while(0)
/**
 * @brief Unset the "native" flag of a closure.
 * @param[in,out] obj The closure object to unmake native.
 */
#define bbzclosure_unmake_native(obj) do{(obj).mdata &= ~BBZCLOSURE_NATIVE_MASK;}while(0)

/**
 * @brief Returns non-0 if a closure is a lambda closure, 0 otherwise.
 * @param[in] obj The object.
 */
#define bbztype_isclosurelambda(obj) ((obj).mdata & BBZCLOSURE_LAMBDA_MASK)
/**
 * @brief Set the "lambda" flag of a closure.
 * @param[in,out] obj The closure object to make lambda.
 */
#define bbzclosure_make_lambda(obj) do{(obj).mdata |= BBZCLOSURE_LAMBDA_MASK;}while(0)
/**
 * @brief Unset the "lambda" flag of a closure.
 * @param[in,out] obj The closure object to unmake lambda.
 */
#define bbzclosure_unmake_lambda(obj) do{(obj).mdata &= ~BBZCLOSURE_LAMBDA_MASK;}while(0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !BBZTYPE_H
