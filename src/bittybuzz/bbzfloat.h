#ifndef BBZFLOAT
#define BBZFLOAT

#include <stdint.h>

/*
 * 16-bit floating-point type
 * https://en.wikipedia.org/wiki/Half-precision_floating-point_format
 * http://half.sourceforge.net/index.html
 */
typedef uint16_t bbzfloat;

/*
 * Makes a bbzfloat out of a signed 16-bit integer.
 * @param i The signed 16-bit value.
 * @return The bbzfloat.
 */
bbzfloat bbzfloat_fromint(int16_t i);

/*
 * Makes a bbzfloat out of a 32-bit floating-point value.
 * @param f The 32-bit floating-point value.
 * @return The bbzfloat.
 */
bbzfloat bbzfloat_fromfloat(float f);

/*
 * Makes a 32-bit floating-point value out of a bbzfloat.
 * @param The bbzfloat value.
 * @return The 32-bit floating-point value.
 */
float bbzfloat_tofloat(bbzfloat x);

/*
 * Calculates a binary arithmetic operation on two bbzfloats (x+y).
 * @param x  The first operand.
 * @param y  The second operand.
 * @param op The arithmetic operator.
 * @return The result.
 */
#define bbzfloat_binaryop_arith(x, y, op) bbzfloat_fromfloat(bbzfloat_tofloat(x) op bbzfloat_tofloat(y))

/*
 * Calculates the sum of two bbzfloats (x+y).
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_add(x, y) bbzfloat_binaryop_arith(x, y, +)

/*
 * Calculates the subtraction of two bbzfloats (x-y).
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_sub(x, y) bbzfloat_binaryop_arith(x, y, -)

/*
 * Calculates the multiplication of two bbzfloats (x*y).
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_mul(x, y) bbzfloat_binaryop_arith(x, y, *)

/*
 * Calculates the division between two bbzfloats (x/y).
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_div(x, y) bbzfloat_binaryop_arith(x, y, /)

/*
 * Performs a binary comparison between two bbzfloats.
 * @param x  The first operand.
 * @param y  The second operand.
 * @param op The comparison operator.
 * @return The result.
 */
#define bbzfloat_binaryop_comp(x, y, op) (bbzfloat_tofloat(x) op bbzfloat_tofloat(y))

/*
 * Returns 1 if x == y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_eq(x, y) bbzfloat_binaryop_comp(x, y, ==)

/*
 * Returns 1 if x != y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_neq(x, y) bbzfloat_binaryop_comp(x, y, !=)

/*
 * Returns 1 if x < y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_lt(x, y) bbzfloat_binaryop_comp(x, y, <)

/*
 * Returns 1 if x <= y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_le(x, y) bbzfloat_binaryop_comp(x, y, <=)

/*
 * Returns 1 if x > y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_gt(x, y) bbzfloat_binaryop_comp(x, y, >)

/*
 * Returns 1 if x >= y, 0 otherwise
 * @param x The first operand.
 * @param y The second operand.
 * @return The result.
 */
#define bbzfloat_ge(x, y) bbzfloat_binaryop_comp(x, y, >=)

/*
 * Returns 1 if x is infinite, 0 otherwise.
 * @param x The operand.
 * @return The result.
 */
int bbzfloat_isinf(bbzfloat x);

/*
 * Returns 1 if x is NaN, 0 otherwise.
 * @param x The operand.
 * @return The result.
 */
int bbzfloat_isnan(bbzfloat x);

#endif
