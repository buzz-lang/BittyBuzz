#include "bbzfloat.h"

/****************************************/
/****************************************/

/**
 * @brief The sign bit is 0 for plus, 1 for minus.
 *
 * The other bits work as follows:
 *
 * |----------+------------+--------|
 * | Exponent |  Fraction  | Value  |
 * |----------+------------+--------|
 * |        0 |         0  | +-   0 |
 * |        0 |       <>0  | +- 0.f |
 * |     1-62 |  any value | +- 1.f |
 * |       63 |         0  | +- inf |
 * |       63 |       <>0  |    NaN |
 * |----------+------------+--------|
 *
 * The precision of the representation for normalized values (i.e.,
 * those is in 1.f form) is set by the exponent. If the actual
 * exponent is E, we set its range between -30 and +31 (with 6 bits we
 * can express at most 62 values, as 0 and 63 are taken for other
 * uses). Thus, E = e - 31. With this, normalized numbers are in the
 * absolute range [2^-30, 2^31].
 *
 * Denormalized values (i.e., those in the 0.f format), have an
 * exponent E = -31 and the absolute range is [-2^-31, +2^-31].
 * 
 */

#define BBZFLOAT_SIGNMASK 0x8000
#define BBZFLOAT_EXPMASK  0x7E00
#define BBZFLOAT_FRACMASK 0x01FF

#define BBZFLOAT_SIGNSHIFT 15
#define BBZFLOAT_EXPSHIFT  9

#define bbzfloat_sign(x) ((x & BBZFLOAT_SIGNMASK) >> BBZFLOAT_SIGNSHIFT)
#define bbzfloat_exp(x)  ((x & BBZFLOAT_EXPMASK) >> BBZFLOAT_EXPSHIFT)
#define bbzfloat_frac(x) (x & BBZFLOAT_FRACMASK)

/****************************************/
/****************************************/

bbzfloat bbzfloat_fromint(int16_t i) {
   return bbzfloat_fromfloat(i);
}

/****************************************/
/****************************************/

bbzfloat bbzfloat_fromfloat(float f) {
   /* We assume float respect the IEEE754 format */
   /* Buffer for the bbzfloat */
   bbzfloat x;
   /* Copy the sign */
   x = ((*(uint32_t*)(&f)) & 0x80000000) >> 16;
   /* Check exponent */
   int e = ((*(uint32_t*)(&f)) & 0x7F800000) >> 23;
   if(e > 96 && e < 158) {
      /* Float value is within the range of bbzfloat */
      /* Set exponent */
      x |= (e - 96) << BBZFLOAT_EXPSHIFT;
      /* Set fraction */
      x |= ((*(uint32_t*)(&f)) & 0x7FFFFF) >> 14;
   }
   else if(e > 0 && e < 0xFF) {
      /* Normalized value, but too big to fit -> Inf */
      /* Set exponent */
      x |= BBZFLOAT_EXPMASK;
   }
   else if(e == 0xFF) {
      /* Inf or NaN */
      /* Set exponent */
      x |= BBZFLOAT_EXPMASK;
      /* Set fraction, actual value doesn't matter, only 0 or <>0 */
      x |= (*(uint32_t*)(&f)) & BBZFLOAT_FRACMASK;
   }
   /* Nothing to do for the case e <= 96, as the values are already all zeroes */
   return x;
}

/****************************************/
/****************************************/

float bbzfloat_tofloat(bbzfloat x) {
   /* We assume float respect the IEEE754 format */
   /* 32bit buffer for the float */
   uint32_t f;
   /* Copy the sign */
   f = bbzfloat_sign(x);
   f <<= 31;
   /* Check the exponent */
   int e = bbzfloat_exp(x);
   if(e == 0) {
      if(bbzfloat_frac(x) > 0) {
         /* Denormalized values, set exponent to 96 */
         f |= 0x30000000;
         /* Copy fraction */
         f |= bbzfloat_frac(x) << 14;
      }
      /* The case bbzfloat_frac(x) == 0 is already taken care of by
       * setting the sign of f above */
   }
   else if(e < 62) {
      /* Normalized values */
      /* Copy exponent */
      f |= ((uint32_t)(e + 96) << 23);
      /* Copy fraction */
      f |= bbzfloat_frac(x) << 14;
   }
   else
      /* Inf and NaN */
      f |= 0x7F800000;
   return *(float*)(&f);
}

/****************************************/
/****************************************/

int bbzfloat_isinf(bbzfloat x) {
   return
      ((x & BBZFLOAT_EXPMASK) == BBZFLOAT_EXPMASK) &&
      ((x & BBZFLOAT_FRACMASK) == 0);
}

/****************************************/
/****************************************/

int bbzfloat_isnan(bbzfloat x) {
   return
      ((x & BBZFLOAT_EXPMASK) == BBZFLOAT_EXPMASK) &&
      ((x & BBZFLOAT_FRACMASK) != 0);
}

/****************************************/
/****************************************/
