#include <bittybuzz/bbzfloat.h>
#include <stdio.h>
#include <inttypes.h>

#include <testing/testingconfig.h>
#ifdef BBZ_USE_AUTOMATED_TESTS
#define BOOST_TEST_DYN_LINK // Use Boost as a dynamic library
#define BOOST_TEST_MODULE BBZ_FLOAT_TEST
#include <boost/test/unit_test.hpp>
#endif // BBZ_USE_AUTOMATED_TESTS

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
