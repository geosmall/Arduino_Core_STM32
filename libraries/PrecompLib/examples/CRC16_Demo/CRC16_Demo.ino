/**
 * CRC16_Demo.ino - Precompiled Library Example
 *
 * Category: Unit Test
 * Framework: ci_log.h (HIL + Serial)
 * Hardware: None
 * CI Automatable: Yes
 *
 * Demonstrates PrecompLib precompiled library:
 *   - CRC-16 CCITT checksum calculation
 *   - Standard test vector verification
 *   - Streaming CRC update API
 *
 * Supported boards:
 *   ./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo --use-rtt --build-id
 *   ./system/ci/aflash.sh libraries/PrecompLib/examples/CRC16_Demo \
 *       STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --use-rtt --build-id
 */

#include <PrecompLib.h>
#include <ci_log.h>

// Standard CRC-16 CCITT test vector
// ASCII "123456789" -> CRC = 0x29B1
static const uint8_t test_vector[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
static const uint16_t expected_crc = 0x29B1;

void setup() {
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("=== PrecompLib CRC-16 Demo ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display library version
    CI_LOGF("Library version: %s\n", precomplib_version());
    CI_LOG("\n");

    // Test 1: Calculate CRC over buffer
    CI_LOG("Test 1: Buffer CRC calculation\n");
    CI_LOG("  Input: \"123456789\"\n");

    uint16_t crc = crc16_calculate(test_vector, sizeof(test_vector));

    CI_LOGF("  Expected: 0x%04X\n", expected_crc);
    CI_LOGF("  Calculated: 0x%04X\n", crc);

    bool test1_pass = (crc == expected_crc);
    CI_LOGF("  Result: %s\n", test1_pass ? "PASS" : "FAIL");
    CI_LOG("\n");

    // Test 2: Streaming CRC update
    CI_LOG("Test 2: Streaming CRC update\n");
    CI_LOG("  Processing byte-by-byte...\n");

    uint16_t streaming_crc = 0xFFFF;
    for (size_t i = 0; i < sizeof(test_vector); i++) {
        streaming_crc = crc16_update(streaming_crc, test_vector[i]);
    }

    CI_LOGF("  Expected: 0x%04X\n", expected_crc);
    CI_LOGF("  Calculated: 0x%04X\n", streaming_crc);

    bool test2_pass = (streaming_crc == expected_crc);
    CI_LOGF("  Result: %s\n", test2_pass ? "PASS" : "FAIL");
    CI_LOG("\n");

    // Test 3: Empty buffer (NULL check)
    CI_LOG("Test 3: NULL buffer handling\n");

    uint16_t null_crc = crc16_calculate(NULL, 0);

    CI_LOGF("  NULL buffer CRC: 0x%04X\n", null_crc);
    CI_LOGF("  Expected (initial): 0x%04X\n", (uint16_t)0xFFFF);

    bool test3_pass = (null_crc == 0xFFFF);
    CI_LOGF("  Result: %s\n", test3_pass ? "PASS" : "FAIL");
    CI_LOG("\n");

    // Summary
    CI_LOG("=== Test Summary ===\n");
    int passed = (test1_pass ? 1 : 0) + (test2_pass ? 1 : 0) + (test3_pass ? 1 : 0);
    CI_LOGF("Tests passed: %d/3\n", passed);

    if (passed == 3) {
        CI_LOG("\n*TEST_PASS*\n");
    } else {
        CI_LOG("\n*TEST_FAIL*\n");
    }

    CI_LOG("*STOP*\n");
}

void loop() {
    // Empty - single-run test
}
