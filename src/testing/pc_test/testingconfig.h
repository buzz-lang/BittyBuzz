/**
 * @file testingconfig.h
 * @brief Definition of testing macros that yield output similar to
 * Boost's Unit Test Framework.
 */

#ifndef TESTING_CONFIG_H
#define TESTING_CONFIG_H

#include <stdio.h>
#include <inttypes.h>


static uint16_t __error_count__ = 0;

/**
 * @brief Declares the test function.
 * @warning There can only be one test function per translation unit.
 * @param[in] name Name of the unit test.
 */
#define TEST(name) int main()

/**
 * @brief Performs a check, printing an error when the check is false.
 * @param[in] expr The expression to assert.
 */
#define ASSERT(expr)                                                    \
    {                                                                   \
        uint8_t eval = expr;                                            \
        if (!eval) {                                                    \
            fprintf(stderr, __FILE__ "(%d): error: "                    \
            "check " #expr " failed\n", __LINE__);                      \
            ++__error_count__;                                          \
        }                                                               \
    }

/**
 * @brief Checks if the two operands are equal, printing an error
 * as well as the values of the operands when they are not.
 * @warning The two operands must be printable.
 * @param[in] lhs First operand.
 * @param[in] rhs Second operand.
 */
#define ASSERT_EQUAL(lhs, rhs)                                          \
    {                                                                   \
        uint8_t eval = (lhs == rhs);                                    \
        if (!eval) {                                                    \
            fprintf(stderr, __FILE__ "(%d): error: "                    \
            "check " #lhs " == " #rhs " failed [%d != %d]\n",           \
            __LINE__,                                                   \
            (int)(lhs), (int)(rhs));                                    \
            ++__error_count__;                                          \
        }                                                               \
    }

/**
 * @brief Exits the current test if expr is false.
 * @param[in] expr The expression to check.
 */
#define REQUIRE(expr)                                                   \
    {                                                                   \
        uint8_t eval = expr;                                            \
        if (!eval) {                                                    \
            fprintf(stderr, __FILE__ "(%d): fatal error: "              \
            "critical check " #expr " failed\n", __LINE__);             \
            ++__error_count__;                                          \
            TEST_END();                                                 \
        }                                                               \
    }

/**
 * @brief Terminates the current test function.
 */
#define TEST_END()                                                      \
    if (__error_count__ > 0) {                                          \
        fprintf(stderr, "\n\n*** %" PRIu16 " failures detected in test file \""  \
               __FILE__ "\"\n", __error_count__);                       \
        return 1;                                                       \
    }                                                                   \
    else {                                                              \
        fprintf(stderr, "\n\n*** No errors detected\n");                \
        return 0;                                                       \
    }


#endif // !TESTING_CONFIG_H