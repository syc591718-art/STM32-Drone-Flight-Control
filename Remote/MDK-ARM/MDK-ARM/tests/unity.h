/* Minimal Unity Test Framework for embedded C unit testing */
#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UNITY_VERSION_MAJOR 2
#define UNITY_VERSION_MINOR 5

/* Result types */
#define TEST_PASS  0
#define TEST_FAIL  1

/* Asserts */
#define TEST_ASSERT_EQUAL_INT(expected, actual)  \
    unity_assert_equal_int(__LINE__, (expected), (actual))

#define TEST_ASSERT_EQUAL_UINT8(expected, actual)  \
    unity_assert_equal_uint8(__LINE__, (expected), (actual))

#define TEST_ASSERT_EQUAL_MEMORY(expected, actual, len)  \
    unity_assert_equal_memory(__LINE__, (expected), (actual), (len))

#define TEST_ASSERT_TRUE(condition)  \
    unity_assert_true(__LINE__, (condition))

#define TEST_ASSERT_FALSE(condition)  \
    unity_assert_false(__LINE__, (condition))

#define TEST_ASSERT_NULL(pointer)  \
    unity_assert_null(__LINE__, (pointer))

#define TEST_ASSERT_NOT_NULL(pointer)  \
    unity_assert_not_null(__LINE__, (pointer))

#define TEST_ASSERT_EQUAL_STRING(expected, actual)  \
    unity_assert_equal_string(__LINE__, (expected), (actual))

#define TEST_PASS_MESSAGE(msg)  \
    unity_pass_message(__LINE__, (msg))

#define TEST_FAIL_MESSAGE(msg)  \
    unity_fail_message(__LINE__, (msg))

/* Test runner macros */
#define RUN_TEST(func)  unity_run_test((func), #func)

#define UNITY_BEGIN()  unity_begin(__FILE__)
#define UNITY_END()    unity_end()

/* Global state */
extern int unity_fail_count;
extern int unity_pass_count;
extern const char *unity_current_test;

/* Internal functions */
void unity_begin(const char *file);
int  unity_end(void);
void unity_run_test(void (*test_func)(void), const char *test_name);
void unity_assert_equal_int(int line, int expected, int actual);
void unity_assert_equal_uint8(int line, unsigned char expected, unsigned char actual);
void unity_assert_equal_memory(int line, const void *expected, const void *actual, unsigned int len);
void unity_assert_true(int line, int condition);
void unity_assert_false(int line, int condition);
void unity_assert_null(int line, const void *pointer);
void unity_assert_not_null(int line, const void *pointer);
void unity_assert_equal_string(int line, const char *expected, const char *actual);
void unity_pass_message(int line, const char *msg);
void unity_fail_message(int line, const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H */
