/**
 * @file bbzfloat.h
 * @brief Definition of 16-bit IEEE-754-compliant floating-point values.
 */

#ifndef BBZFLOAT
#define BBZFLOAT

#include <stdint.h>

#include "bbzinclude.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * @brief 16-bit floating-point type.
 * @details https://en.wikipedia.org/wiki/Half-precision_floating-point_format
 * http://half.sourceforge.net/index.html
 * @warning Most microcontrollers do not support floating-point operations.
 * See the specification of the MCU you are programming for to see if such
 * operations are possible.
 */
typedef uint16_t bbzfloat;

/**
 * @brief Makes a bbzfloat out of a signed 16-bit integer.
 * @param[in] i The signed 16-bit value.
 * @return The bbzfloat.
 */
bbzfloat bbzfloat_fromint(int16_t i);

/**
 * @brief Makes a bbzfloat out of a 32-bit floating-point value.
 * @param[in] f The 32-bit floating-point value.
 * @return The bbzfloat.
 */
bbzfloat bbzfloat_fromfloat(float f);

/**
 * @brief Makes a 32-bit floating-point value out of a bbzfloat.
 * @param[in] x The bbzfloat value.
 * @return The 32-bit floating-point value.
 */
float bbzfloat_tofloat(bbzfloat x);

/**
 * @brief Apply a negation on a float, changin the sign
 * @param[in] f bbzfloat value.
 * @return The bbzfloat negated.
 */
bbzfloat bbzfloat_negate(bbzfloat f);

/**
 * @brief Calculates a binary arithmetic operation on two bbzfloats (x+y).
 * @param[in] x  The first operand.
 * @param[in] y  The second operand.
 * @param[in] op The arithmetic operator.
 * @return The result.
 */
#define bbzfloat_binaryop_arith(x, y, op) bbzfloat_fromfloat(bbzfloat_tofloat(x) op bbzfloat_tofloat(y))

/**
 * @brief Calculates the sum of two bbzfloats (x+y).
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_add(x, y) bbzfloat_binaryop_arith(x, y, +)

/**
 * @brief Calculates the subtraction of two bbzfloats (x-y).
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_sub(x, y) bbzfloat_binaryop_arith(x, y, -)

/**
 * @brief Calculates the multiplication of two bbzfloats (x*y).
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_mul(x, y) bbzfloat_binaryop_arith(x, y, *)

/**
 * @brief Calculates the division between two bbzfloats (x/y).
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_div(x, y) bbzfloat_binaryop_arith(x, y, /)

/**
 * @brief Performs a binary comparison between two bbzfloats.
 * @param[in] x  The first operand.
 * @param[in] y  The second operand.
 * @param[in] op The comparison operator.
 * @return The result.
 */
#define bbzfloat_binaryop_comp(x, y, op) (bbzfloat_tofloat(x) op bbzfloat_tofloat(y))

/**
 * @brief Returns 1 if x == y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_eq(x, y) bbzfloat_binaryop_comp(x, y, ==)

/**
 * @brief Returns 1 if x != y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_neq(x, y) bbzfloat_binaryop_comp(x, y, !=)

/**
 * @brief Returns 1 if x < y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_lt(x, y) bbzfloat_binaryop_comp(x, y, <)

/**
 * @brief Returns 1 if x <= y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_le(x, y) bbzfloat_binaryop_comp(x, y, <=)

/**
 * @brief Returns 1 if x > y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_gt(x, y) bbzfloat_binaryop_comp(x, y, >)

/**
 * @brief Returns 1 if x >= y, 0 otherwise
 * @param[in] x The first operand.
 * @param[in] y The second operand.
 * @return The result.
 */
#define bbzfloat_ge(x, y) bbzfloat_binaryop_comp(x, y, >=)

/**
 * @brief Returns 1 if x is infinite, 0 otherwise.
 * @param[in] x The operand.
 * @return The result.
 */
uint8_t bbzfloat_isinf(bbzfloat x);

/**
 * @brief Returns 1 if x is NaN, 0 otherwise.
 * @param[in] x The operand.
 * @return The result.
 */
uint8_t bbzfloat_isnan(bbzfloat x);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif
