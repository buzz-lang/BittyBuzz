#include <bittybuzz/bbzneighbors.h>

#define NUM_TEST_CASES 2
#define TEST_MODULE neighbors
#include "testingconfig.h"

TEST(test1) {
    ASSERT(0);
    ASSERT_EQUAL(0, 1);
    REQUIRE(0);
}

TEST(test2) {
    ASSERT(0);
    ASSERT_EQUAL(0, 1);
    REQUIRE(0);
}

TEST_LIST {
    ADD_TEST(test1);
    ADD_TEST(test2);
}