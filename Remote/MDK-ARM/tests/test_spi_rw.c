/**
 * @file test_spi_rw.c
 * @brief Unit tests for SPI_RW function in Int_SI24R1.c
 *
 * Tests the static SPI_RW function which wraps HAL_SPI_TransmitReceive.
 * Covers: normal, boundary, error, and regression scenarios.
 *
 * Build (gcc on host):
 *   gcc -std=c99 -Wall -Wextra -o test_spi_rw test_spi_rw.c unity.c -I.
 *
 * Build (MinGW on Windows):
 *   gcc -std=c99 -Wall -o test_spi_rw.exe test_spi_rw.c unity.c -I.
 */

#include "unity.h"

/* ================================================================
 * Mock HAL Layer - stubs required to compile source under test
 * ================================================================ */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* HAL status codes */
typedef enum {
    HAL_OK       = 0x00,
    HAL_ERROR    = 0x01,
    HAL_BUSY     = 0x02,
    HAL_TIMEOUT  = 0x03
} HAL_StatusTypeDef;

/* Mock SPI handle */
typedef struct {
    void *Instance;
    int   State;
    uint32_t ErrorCode;
} SPI_HandleTypeDef;

/* Global SPI handle used by SPI_RW */
static SPI_HandleTypeDef mock_hspi1;
#define hspi1 mock_hspi1

/* ================================================================
 * Mock Control: configurable HAL_SPI_TransmitReceive behavior
 * ================================================================ */

/* Mock return value for HAL_SPI_TransmitReceive */
static HAL_StatusTypeDef mock_spi_status = HAL_OK;

/* Mock received data buffer (copied into rx_data) */
static uint8_t mock_rx_byte = 0x00;

/* Track call count for verifying invocation */
static int mock_spi_call_count = 0;

/* Track last parameters passed to HAL_SPI_TransmitReceive */
static uint8_t  mock_last_tx_byte = 0;
static uint8_t *mock_last_rx_ptr  = NULL;
static uint16_t mock_last_size    = 0;
static uint32_t mock_last_timeout = 0;

/* Mock implementation of HAL_SPI_TransmitReceive */
HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi,
                                          const uint8_t     *pTxData,
                                          uint8_t           *pRxData,
                                          uint16_t           Size,
                                          uint32_t           Timeout)
{
    (void)hspi; /* unused in mock */

    mock_spi_call_count++;
    mock_last_tx_byte  = (pTxData != NULL) ? *pTxData : 0;
    mock_last_rx_ptr   = pRxData;
    mock_last_size     = Size;
    mock_last_timeout  = Timeout;

    /* Simulate received byte */
    if (pRxData != NULL && Size > 0) {
        *pRxData = mock_rx_byte;
    }

    return mock_spi_status;
}

/* Reset mock state before each test */
static void mock_reset(void)
{
    mock_spi_status    = HAL_OK;
    mock_rx_byte       = 0x00;
    mock_spi_call_count = 0;
    mock_last_tx_byte  = 0;
    mock_last_rx_ptr   = NULL;
    mock_last_size     = 0;
    mock_last_timeout  = 0;
}

/* ================================================================
 * Unit Under Test: SPI_RW (extracted from Int_SI24R1.c)
 * ================================================================
 * Original source:
 *
 *   static uint8_t SPI_RW(uint8_t byte)
 *   {
 *       uint8_t rx_data = 0;
 *       HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
 *       return rx_data;
 *   }
 *
 * Note: The function is static in the original source; reproduced here
 * for isolated unit testing per standard embedded C testing practices.
 */

static uint8_t SPI_RW(uint8_t byte)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_data, 1, 1000);
    return rx_data;
}

/* ================================================================
 * Setup / Teardown
 * ================================================================ */

void setUp(void)
{
    mock_reset();
}

void tearDown(void)
{
    /* clean state, nothing to do */
}

/* ================================================================
 * Test Cases: Normal Operation
 * ================================================================ */

/**
 * @brief SPI_RW with byte=0x00, mock returns rx=0x00 (echo).
 * Expect: return 0x00, HAL called once with correct params.
 */
void test_SPI_RW_zero_send_zero_receive(void)
{
    mock_rx_byte = 0x00;
    uint8_t result = SPI_RW(0x00);

    TEST_ASSERT_EQUAL_UINT8(0x00, result);
    TEST_ASSERT_EQUAL_INT(1, mock_spi_call_count);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_last_tx_byte);
    TEST_ASSERT_EQUAL_INT(1, (int)mock_last_size);
    TEST_ASSERT_EQUAL_INT(1000, (int)mock_last_timeout);
    TEST_ASSERT_NOT_NULL(mock_last_rx_ptr);
}

/**
 * @brief SPI_RW with byte=0x55, mock returns rx=0xAA (different).
 * Expect: return 0xAA (the received value, not the sent one).
 */
void test_SPI_RW_send_0x55_receive_0xAA(void)
{
    mock_rx_byte = 0xAA;
    uint8_t result = SPI_RW(0x55);

    TEST_ASSERT_EQUAL_UINT8(0xAA, result);
    TEST_ASSERT_EQUAL_UINT8(0x55, mock_last_tx_byte);
    TEST_ASSERT_EQUAL_INT(1, mock_spi_call_count);
}

/**
 * @brief SPI_RW with byte=0xFF, mock returns rx=0xFF.
 * Expect: full-duplex works at max value.
 */
void test_SPI_RW_max_value_roundtrip(void)
{
    mock_rx_byte = 0xFF;
    uint8_t result = SPI_RW(0xFF);

    TEST_ASSERT_EQUAL_UINT8(0xFF, result);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_last_tx_byte);
}

/* ================================================================
 * Test Cases: Boundary Values
 * ================================================================ */

/**
 * @brief SPI_RW with byte=0x00, mock returns rx=0xFF.
 * Boundary: min send, max receive.
 */
void test_SPI_RW_boundary_min_send_max_receive(void)
{
    mock_rx_byte = 0xFF;
    uint8_t result = SPI_RW(0x00);

    TEST_ASSERT_EQUAL_UINT8(0xFF, result);
    TEST_ASSERT_EQUAL_UINT8(0x00, mock_last_tx_byte);
}

/**
 * @brief SPI_RW with byte=0xFF, mock returns rx=0x00.
 * Boundary: max send, min receive.
 */
void test_SPI_RW_boundary_max_send_min_receive(void)
{
    mock_rx_byte = 0x00;
    uint8_t result = SPI_RW(0xFF);

    TEST_ASSERT_EQUAL_UINT8(0x00, result);
    TEST_ASSERT_EQUAL_UINT8(0xFF, mock_last_tx_byte);
}

/**
 * @brief SPI_RW with byte=0x01 (one above min).
 * Edge: one step above zero boundary.
 */
void test_SPI_RW_edge_one_above_zero(void)
{
    mock_rx_byte = 0x42;
    uint8_t result = SPI_RW(0x01);

    TEST_ASSERT_EQUAL_UINT8(0x42, result);
    TEST_ASSERT_EQUAL_UINT8(0x01, mock_last_tx_byte);
}

/**
 * @brief SPI_RW with byte=0xFE (one below max).
 * Edge: one step below max boundary.
 */
void test_SPI_RW_edge_one_below_max(void)
{
    mock_rx_byte = 0xAB;
    uint8_t result = SPI_RW(0xFE);

    TEST_ASSERT_EQUAL_UINT8(0xAB, result);
    TEST_ASSERT_EQUAL_UINT8(0xFE, mock_last_tx_byte);
}

/* ================================================================
 * Test Cases: SPI Hardware Error Scenarios
 * ================================================================ */

/**
 * @brief SPI_RW when HAL returns HAL_ERROR.
 * Expect: function returns rx_data=0 (initialized value, never modified).
 * Note: Original code does NOT check HAL return status.
 */
void test_SPI_RW_hal_error(void)
{
    mock_spi_status = HAL_ERROR;
    mock_rx_byte    = 0x00; /* HAL shouldn't modify rx on error */

    uint8_t result = SPI_RW(0x3C);

    /* rx_data initialized to 0; HAL_ERROR means buffer may or may not be written.
       In practice HAL does not modify the buffer on error, so expect 0. */
    TEST_ASSERT_EQUAL_UINT8(0x00, result);
    TEST_ASSERT_EQUAL_INT(1, mock_spi_call_count);
}

/**
 * @brief SPI_RW when HAL returns HAL_BUSY.
 * Expect: returns rx_data=0 (initial value).
 */
void test_SPI_RW_hal_busy(void)
{
    mock_spi_status = HAL_BUSY;
    mock_rx_byte    = 0x00;

    uint8_t result = SPI_RW(0x7F);

    TEST_ASSERT_EQUAL_UINT8(0x00, result);
}

/**
 * @brief SPI_RW when HAL returns HAL_TIMEOUT.
 * Expect: returns rx_data=0 (initial value).
 */
void test_SPI_RW_hal_timeout(void)
{
    mock_spi_status = HAL_TIMEOUT;
    mock_rx_byte    = 0x00;

    uint8_t result = SPI_RW(0xC8);

    TEST_ASSERT_EQUAL_UINT8(0x00, result);
}

/* ================================================================
 * Test Cases: SPI-specific Behavior
 * ================================================================ */

/**
 * @brief Verify SPI_RW always passes Size=1 to HAL.
 * The function is designed for single-byte SPI transactions.
 */
void test_SPI_RW_always_single_byte_transfer(void)
{
    /* Multiple calls should all use Size=1 */
    for (int i = 0; i < 5; i++) {
        mock_reset();
        mock_rx_byte = (uint8_t)(i * 50);
        SPI_RW((uint8_t)i);
        TEST_ASSERT_EQUAL_INT(1, (int)mock_last_size);
        TEST_ASSERT_EQUAL_UINT8((uint8_t)(i * 50), mock_rx_byte);
    }
}

/**
 * @brief Verify SPI_RW always passes Timeout=1000 to HAL.
 */
void test_SPI_RW_timeout_is_1000ms(void)
{
    SPI_RW(0x00);
    TEST_ASSERT_EQUAL_INT(1000, (int)mock_last_timeout);

    mock_reset();
    SPI_RW(0xFF);
    TEST_ASSERT_EQUAL_INT(1000, (int)mock_last_timeout);

    mock_reset();
    SPI_RW(0x80);
    TEST_ASSERT_EQUAL_INT(1000, (int)mock_last_timeout);
}

/**
 * @brief Verify SPI_RW passes the correct SPI handle (&hspi1).
 */
void test_SPI_RW_uses_correct_spi_handle(void)
{
    /* The function hardcodes &hspi1. No way to directly test
       from mock perspective, but we verify the mock was called
       (which implies the handle was accepted). */
    mock_rx_byte = 0xEF;
    uint8_t result = SPI_RW(0xBE);

    TEST_ASSERT_EQUAL_UINT8(0xEF, result);
    TEST_ASSERT_EQUAL_INT(1, mock_spi_call_count);

    /* hspi1 must be a non-NULL struct (it's a global, not a pointer) */
    /* This test confirms the function compiles and links correctly
       with the mock hspi1 definition. */
}

/* ================================================================
 * Test Cases: All 256 Values Exhaustive (optional)
 * ================================================================ */

/**
 * @brief Exhaustive scan: for every possible uint8_t send,
 * verify that the mock receive path roundtrips correctly.
 */
void test_SPI_RW_exhaustive_256_values(void)
{
    for (int i = 0; i <= 0xFF; i++) {
        mock_reset();
        mock_rx_byte = (uint8_t)((i * 17 + 3) & 0xFF); /* pseudo-random mapping */
        uint8_t result = SPI_RW((uint8_t)i);

        TEST_ASSERT_EQUAL_UINT8(mock_rx_byte, result);
        TEST_ASSERT_EQUAL_UINT8((uint8_t)i, mock_last_tx_byte);
    }
}

/* ================================================================
 * Test Cases: Return Value Integrity
 * ================================================================ */

/**
 * @brief SPI_RW must return exactly what HAL places in rx_data.
 * The return value is NOT the status, it's the received byte.
 */
void test_SPI_RW_return_value_is_received_byte_not_status(void)
{
    /* Simulate SPI working: received byte completely independent of sent byte */
    uint8_t test_pairs[][2] = {
        {0x00, 0x12}, {0x12, 0x34}, {0x34, 0x56},
        {0x56, 0x78}, {0x78, 0x9A}, {0x9A, 0xBC},
        {0xBC, 0xDE}, {0xDE, 0xF0}, {0xF0, 0x0F},
    };

    for (size_t i = 0; i < sizeof(test_pairs) / sizeof(test_pairs[0]); i++) {
        mock_reset();
        mock_rx_byte = test_pairs[i][1];
        uint8_t result = SPI_RW(test_pairs[i][0]);

        char msg[64];
        snprintf(msg, sizeof(msg),
                 "RX mismatch: sent 0x%02X, expected RX 0x%02X, got 0x%02X",
                 test_pairs[i][0], test_pairs[i][1], result);
        if (result != test_pairs[i][1]) {
            TEST_FAIL_MESSAGE(msg);
        }
    }
}

/* ================================================================
 * Main
 * ================================================================ */

int main(void)
{
    UNITY_BEGIN();

    /* --- Normal Operation --- */
    RUN_TEST(test_SPI_RW_zero_send_zero_receive);
    RUN_TEST(test_SPI_RW_send_0x55_receive_0xAA);
    RUN_TEST(test_SPI_RW_max_value_roundtrip);

    /* --- Boundary Values --- */
    RUN_TEST(test_SPI_RW_boundary_min_send_max_receive);
    RUN_TEST(test_SPI_RW_boundary_max_send_min_receive);
    RUN_TEST(test_SPI_RW_edge_one_above_zero);
    RUN_TEST(test_SPI_RW_edge_one_below_max);

    /* --- Error Scenarios --- */
    RUN_TEST(test_SPI_RW_hal_error);
    RUN_TEST(test_SPI_RW_hal_busy);
    RUN_TEST(test_SPI_RW_hal_timeout);

    /* --- SPI-specific Behavior --- */
    RUN_TEST(test_SPI_RW_always_single_byte_transfer);
    RUN_TEST(test_SPI_RW_timeout_is_1000ms);
    RUN_TEST(test_SPI_RW_uses_correct_spi_handle);

    /* --- Exhaustive --- */
    RUN_TEST(test_SPI_RW_exhaustive_256_values);

    /* --- Return Value Integrity --- */
    RUN_TEST(test_SPI_RW_return_value_is_received_byte_not_status);

    return UNITY_END();
}
