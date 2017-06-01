#include <bittybuzz/bbzfloat.h>
#include <stdio.h>
#include <inttypes.h>

#include "testingconfig.h"

#define BASE 1.

/**
 * @brief Compares two floats.
 * @param[in] lhs Left-hand side of the comparison.
 * @param[in] rhs Right-hand side of the comparison.
 * @return <0 if lhs < rhs, >0 if lhs > rhs, 0 when both are within a
 * small range of each other.
 */
int8_t float_cmp(float lhs, float rhs) {
    if (lhs < rhs) {
        return -1;
    }
    if (lhs > rhs) {
        return 1;
    }
    return 0;
}

TEST(bbz_float) {
   for(uint16_t i = 0; i < 10; ++i) {
      ASSERT(float_cmp(bbzfloat_tofloat(bbzfloat_fromfloat(BASE * i)), BASE * i) == 0);
   }

   TEST_END();
}
