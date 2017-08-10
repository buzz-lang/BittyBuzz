/**
 * @file testingconfig.h
 * @brief Definition of testing macros that yield output similar to
 * Boost's Unit Test Framework.
 * @details <h2>Example usage:</h2>
 *
 * @code
 *
 * #define NUM_TEST_CASES N // An integer value
 * #define TEST_MODULE MyTestSuite
 * #include "testingconfig.h"
 *
 * TEST(my_test_1) {
 *     <do some testing using ASSERT, ASSERT_EQUAL and REQUIRE>
 * }
 *
 * TEST(my_test_2) {
 *     <...>
 * }
 *
 * <...>
 *
 * TEST(my_test_N) {
 *     <...>
 * }
 *
 * TEST_LIST {
 *     ADD_TEST(my_test_1);
 *     ADD_TEST(my_test_2);
 *     <...>
 *     ADD_TEST(my_test_N);
 * }
 *
 * @endcode
 */

#ifndef TESTING_CONFIG_H
#define TESTING_CONFIG_H

#include <stdio.h>
#include <inttypes.h>

#ifndef NUM_TEST_CASES
#warning "NUM_TEST_CASES undefined. Using default value of 1."
#define NUM_TEST_CASES 1
#endif // !NUM_TEST_CASES

#ifndef TEST_MODULE
#warning "TEST_MODULE undefined. Using default value of DEFAULT_TEST_MODULE_NAME."
#define TEST_MODULE DEFAULT_TEST_MODULE_NAME
#endif // !TEST_MODULE

/**
 * @brief Type of a test case function pointer.
 */
typedef void (*__test_case_funp__)();

/**
 * @brief The test cases.
 */
static __test_case_funp__ __test_cases__[NUM_TEST_CASES] = {};

/**
 * @brief The number of test cases.
 */
static uint16_t __test_cases_size__ = 0;


/**
 * @brief The index of the current test case.
 */
static uint16_t __curr_test_case__ = 0xFFFF;


/**
 * @brief The names of the test cases.
 */
static char* __test_case_names__[NUM_TEST_CASES] = {};

/**
 * @brief The number of errors encountered.
 */
static uint16_t __error_count__ = 0;

/**
 * Evaluator for the __STRINGIFIER__ macro.
 */
#define __STRINGIFIER_EVALUATOR__(x) #x

/**
 * @brief Macro stringifier.
 */
#define __STRINGIFIER__(x) __STRINGIFIER_EVALUATOR__(x)

/**
 * @brief Paster for the TEST_UNIT macro.
 */
#define TEST_UNIT(name)                                                 \
    static void __test_list__();                                        \
                                                                        \
    int main() {                                                        \
        __test_list__();                                                \
        if (__test_cases_size__ > 1) {                                  \
            fprintf(stderr, "Running %d test cases...\n",               \
                    __test_cases_size__);                               \
        }                                                               \
        else if (__test_cases_size__ == 1) {                            \
            fprintf(stderr, "Running 1 test case...\n");                \
        }                                                               \
        else {                                                          \
            fprintf(stderr, "Test setup error: no test case to run. "   \
                    "Did you forget to use ADD_TEST inside "            \
                    "TEST_LIST?\n");                                    \
        }                                                               \
                                                                        \
        for (__curr_test_case__ = 0;                                    \
             __curr_test_case__ < __test_cases_size__;                  \
             ++__curr_test_case__) {                                    \
            __test_cases__[__curr_test_case__]();                       \
        }                                                               \
                                                                        \
        if (__error_count__ > 0) {                                      \
            fprintf(stderr, "\n*** %" PRIu16 " failures detected in "   \
                            "test suite \""                             \
                            __STRINGIFIER__(TEST_MODULE) "\"\n",        \
                            __error_count__);                           \
            return 1;                                                   \
        }                                                               \
        else {                                                          \
            fprintf(stderr, "*** No errors detected\n");                \
            return 0;                                                   \
        }                                                               \
    }

TEST_UNIT(TEST_MODULE);

/**
 * @brief List of tests to run.
 * @details Usage:
 * @code
 * TEST_LIST {
 *     ADD_TEST(my_test_1);
 *     ADD_TEST(my_test_2);
 *     ...
 *     ADD_TEST(my_test_N);
 * }
 * @endcode
 * @see ADD_TEST
 */
#define TEST_LIST static void __test_list__()

/**
 * @brief Adds a test function.
 * @see TEST_LIST
 * @warning There can be multiple test functions per translation unit,
 * but never more than NUM_TEST_CASES.
 * @param[in] name Name of the unit test.
 */
#define ADD_TEST(name)                                                  \
    if (__test_cases_size__ < NUM_TEST_CASES) {                         \
        __test_cases__[__test_cases_size__] = _test_case_ ## name;      \
        __test_case_names__[__test_cases_size__] = #name;               \
        ++__test_cases_size__;                                          \
    }                                                                   \
    else {                                                              \
        fprintf(stderr, "WARNING: Test suite \""                        \
                        __STRINGIFIER__(TEST_MODULE) "\": "             \
                        "Trying to add more test files "                \
                        "than NUM_TEST_CASES. Test case \"%s\" will "   \
                        "not be run. Increase the value of the "        \
                        "NUM_TEST_CASES macro.\n", #name);              \
        fflush(stderr);                                                 \
    }

/**
 * @brief Declares a unit test.
 */
#define TEST(name) static void _test_case_ ## name()

/**
 * @brief Performs a check, printing an error when the check is false.
 * @param[in] expr The expression to assert.
 */
#define ASSERT(expr)                                                    \
    {                                                                   \
        uint8_t eval = expr;                                            \
        if (!eval) {                                                    \
            fprintf(stderr, __FILE__ "(%d): error in \"%s\": "          \
            "check " #expr " failed\n", __LINE__,                       \
            __test_case_names__[__curr_test_case__]);                   \
            ++__error_count__;                                          \
            fflush(stderr);                                             \
        }                                                               \
    }

/**
 * @brief Checks if the two operands are equal, printing an error
 * as well as the values of the operands when they are not.
 * @warning The two operands must be printable as integers.
 * @param[in] lhs First operand.
 * @param[in] rhs Second operand.
 */
#define ASSERT_EQUAL(lhs, rhs)                                          \
    {                                                                   \
        uint8_t eval = ((lhs) == (rhs));                                \
        if (!eval) {                                                    \
            fprintf(stderr, __FILE__ "(%d): error in \"%s\": "          \
                    "check " #lhs " == " #rhs " failed [%d != %d]\n",   \
                    __LINE__, __test_case_names__[__curr_test_case__],  \
                    (int)(lhs), (int)(rhs));                            \
            ++__error_count__;                                          \
            fflush(stderr);                                             \
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
            fprintf(stderr, __FILE__ "(%d): fatal error in \"%s\": "    \
                    "critical check " #expr " failed\n", __LINE__,      \
                    __test_case_names__[__curr_test_case__]);           \
            ++__error_count__;                                          \
            fflush(stderr);                                             \
            return;                                                     \
        }                                                               \
    }


#endif // !TESTING_CONFIG_H