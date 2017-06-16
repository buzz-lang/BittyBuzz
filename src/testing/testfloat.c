#include <bittybuzz/bbzfloat.h>
#include <stdio.h>
#include <inttypes.h>

#define NUM_TEST_CASES 1
#define TEST_MODULE float
#include "testingconfig.h"

#define BASE 1.
#define FLOAT_EQUAL_THRESHOLD 0.005

/**
 * @brief Compares two floats.
 * @param[in] lhs Left-hand side of the comparison.
 * @param[in] rhs Right-hand side of the comparison.
 * @return <0 if lhs < rhs, >0 if lhs > rhs, 0 when both are within a
 * small range of each other.
 */
int8_t float_cmp(float lhs, float rhs) {
    if (lhs < rhs - FLOAT_EQUAL_THRESHOLD) {
        return -1;
    }
    if (lhs > rhs + FLOAT_EQUAL_THRESHOLD) {
        return 1;
    }
    return 0;
}

TEST(float_equality) {
   for(uint16_t i = 0; i < 10; ++i) {
      ASSERT(float_cmp(bbzfloat_tofloat(bbzfloat_fromfloat(BASE * i)), BASE * i) == 0);
   }
}

TEST_LIST {
    ADD_TEST(float_equality);
}