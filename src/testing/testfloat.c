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

TEST(bbz_float) {
   for(int i = 0; i < 10; ++i) {
      ASSERT(bbzfloat_tofloat(bbzfloat_fromfloat(BASE * i)) == BASE * i);
   }

   TEST_END();
}
