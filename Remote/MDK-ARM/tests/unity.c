/* Minimal Unity Test Framework implementation */
#include "unity.h"

int unity_fail_count = 0;
int unity_pass_count = 0;
const char *unity_current_test = "";

static int unity_test_count = 0;
static const char *unity_file = "";

void unity_begin(const char *file)
{
    unity_fail_count = 0;
    unity_pass_count = 0;
    unity_test_count = 0;
    unity_file = file;
    printf("\n=== Unity Test Runner ===\n");
    printf("File: %s\n\n", file);
}

int unity_end(void)
{
    printf("\n=== Test Results ===\n");
    printf("Total:  %d\n", unity_test_count);
    printf("Passed: %d\n", unity_pass_count);
    printf("Failed: %d\n", unity_fail_count);
    printf("Success rate: %.1f%%\n",
        unity_test_count > 0
            ? (100.0f * unity_pass_count / unity_test_count)
            : 0.0f);

    if (unity_fail_count > 0) {
        printf("\n*** TESTS FAILED ***\n");
    } else {
        printf("\n*** ALL TESTS PASSED ***\n");
    }

    return unity_fail_count;
}

void unity_run_test(void (*test_func)(void), const char *test_name)
{
    int fail_before = unity_fail_count;

    unity_test_count++;
    unity_current_test = test_name;
    printf("  [RUN ] %s\n", test_name);
    test_func();

    if (unity_fail_count == fail_before) {
        unity_pass_count++;
        printf("  [PASS] %s\n", test_name);
    } else {
        printf("  [FAIL] %s\n", test_name);
    }
}

static void unity_fail(int line, const char *message)
{
    unity_fail_count++;
    printf("    FAIL at %s:%d - %s\n", unity_file, line, message);
}

static void unity_print_uint8_hex(unsigned char value)
{
    printf("0x%02X", value);
}

void unity_assert_equal_int(int line, int expected, int actual)
{
    if (expected != actual) {
        printf("    Expected: %d\n    Actual:   %d\n", expected, actual);
        unity_fail(line, "Values not equal (int)");
    }
}

void unity_assert_equal_uint8(int line, unsigned char expected, unsigned char actual)
{
    if (expected != actual) {
        printf("    Expected: ");
        unity_print_uint8_hex(expected);
        printf("\n    Actual:   ");
        unity_print_uint8_hex(actual);
        printf("\n");
        unity_fail(line, "Values not equal (uint8)");
    }
}

void unity_assert_equal_memory(int line, const void *expected, const void *actual, unsigned int len)
{
    if (memcmp(expected, actual, len) != 0) {
        printf("    Memory mismatch (%u bytes)\n", len);
        unity_fail(line, "Memory values not equal");
    }
}

void unity_assert_true(int line, int condition)
{
    if (!condition) {
        unity_fail(line, "Expected TRUE, got FALSE");
    }
}

void unity_assert_false(int line, int condition)
{
    if (condition) {
        unity_fail(line, "Expected FALSE, got TRUE");
    }
}

void unity_assert_null(int line, const void *pointer)
{
    if (pointer != NULL) {
        printf("    Pointer: %p\n", pointer);
        unity_fail(line, "Expected NULL pointer");
    }
}

void unity_assert_not_null(int line, const void *pointer)
{
    if (pointer == NULL) {
        unity_fail(line, "Expected non-NULL pointer");
    }
}

void unity_assert_equal_string(int line, const char *expected, const char *actual)
{
    if (strcmp(expected, actual) != 0) {
        printf("    Expected: \"%s\"\n    Actual:   \"%s\"\n", expected, actual);
        unity_fail(line, "Strings not equal");
    }
}

void unity_pass_message(int line, const char *msg)
{
    (void)line;
    (void)msg;
    /* Explicit pass - already tracked */
}

void unity_fail_message(int line, const char *msg)
{
    unity_fail(line, msg);
}
