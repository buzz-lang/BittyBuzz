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

/*
 * Nil type
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
} bbznil_t;

/*
 * 16-bit signed integer
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
   int16_t value;
} bbzint_t;

/*
 * Float
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
   bbzfloat value;
} bbzfloat_t;

/*
 * String
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
   uint16_t value; /* The string id */
} bbzstring_t;

/*
 * Table
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
   uint16_t value; /* The index of the first segment in the heap */
} bbztable_t;

/*
 * Dynamic Array
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;  /* 3rd bit: 1 if is dynamic array,
                      2nd bit: 1 if elements need invalidation on array destroy */
   uint16_t value; /* The index of the first segment in the heap */
} bbzdarray_t;

/*
 * Closure
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata; /* contains 'native' flag as 7th topmost bit */
   struct {
      int8_t ref; /* jump address or function id */
      uint8_t actrec; /* position in the heap of the activation record array.
                         A value of 0xFF means it uses the default activation record of the VM */
   } value;
} bbzclosure_t;

/*
 * User data
 */
typedef struct __attribute__((packed)) {
   uint8_t mdata;
   void* value;
} bbzuserdata_t;

/*
 * A handle for a object
 */
typedef union __attribute__((packed)) {
   struct {
      uint8_t mdata; /* object metadata
                        8th,7th,6th bit for type
                        5th         bit for valid in heap
                        4th         bit for garbage collection */
   }             o;  /* as a generic object */
   bbznil_t      n;  /* as nil */
   bbzint_t      i;  /* as integer */
   bbzfloat_t    f;  /* as floating-point */
   bbzstring_t   s;  /* as string */
   bbztable_t    t;  /* as table */
   bbzclosure_t  c;  /* as closure */
   bbzuserdata_t u;  /* as user data */
} bbzobj_t;

/*
 * Compares two objects.
 * @param a The first object to compare.
 * @param b The second object to compare.
 * @return -1 if a<b, 0 if a==b, 1 if a>b
 */
int bbztype_cmp(const bbzobj_t* a,
                const bbzobj_t* b);

/*
 * Returns the type of an object.
 * @param obj The object.
 */
#define bbztype(obj) ((obj).o.mdata >> 5)

/*
 * Casts an object into a different type.
 * The cast is done without checking type compatibility.
 * @param obj  The object.
 * @param type The type.
 */
#define bbztype_cast(obj, type) (obj).o.mdata = (((obj).o.mdata & 0x1F) | (type << 5))

/*
 * Returns 1 if an object is nil, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isnil(obj) (bbztype(obj) == BBZTYPE_NIL)

/*
 * Returns 1 if an object is int, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isint(obj) (bbztype(obj) == BBZTYPE_INT)

/*
 * Returns 1 if an object is float, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isfloat(obj) (bbztype(obj) == BBZTYPE_FLOAT)

/*
 * Returns 1 if an object is string, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isstring(obj) (bbztype(obj) == BBZTYPE_STRING)

/*
 * Returns 1 if an object is table, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_istable(obj) (bbztype(obj) == BBZTYPE_TABLE)

/*
 * Returns 1 if an object is dynamic array, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isdarray(obj) (bbztype(obj) == BBZTYPE_TABLE && ((obj).o.mdata & 0x04))

/*
 * Returns 1 if an object is closure, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isclosure(obj) ((bbztype(obj) & BBZTYPE_CLOSURE) == BBZTYPE_CLOSURE)

/*
 * Returns 1 if an object is userdata, 0 otherwise.
 * @param obj The object.
 */
#define bbztype_isuserdata(obj) (bbztype(obj) == BBZTYPE_USERDATA)

/*
 * Returns 1 if a closure is native, 0 otherwise.
 * @param obj The object.
 */
#define bbzclosure_isnative(obj) ((obj).c.mdata & 0x40)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
