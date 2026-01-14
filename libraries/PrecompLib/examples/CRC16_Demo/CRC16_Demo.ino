/**
 * CRC16_Demo.ino - Precompiled Library Example
 *
 * Demonstrates PrecompLib precompiled library:
 *   - CRC-16 CCITT checksum calculation
 *   - Standard test vector verification
 *   - Streaming CRC update API
 */

#include <PrecompLib.h>

// Standard CRC-16 CCITT test vector
// ASCII "123456789" -> CRC = 0x29B1
static const uint8_t test_vector[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
static const uint16_t expected_crc = 0x29B1;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println("=== PrecompLib CRC-16 Demo ===");

    // Display library version
    Serial.printf("Library version: %s\n\n", precomplib_version());

    // Test 1: Calculate CRC over buffer
    Serial.println("Test 1: Buffer CRC calculation");
    Serial.println("  Input: \"123456789\"");

    uint16_t crc = crc16_calculate(test_vector, sizeof(test_vector));

    Serial.printf("  Expected: 0x%04X\n", expected_crc);
    Serial.printf("  Calculated: 0x%04X\n", crc);

    bool test1_pass = (crc == expected_crc);
    Serial.printf("  Result: %s\n\n", test1_pass ? "PASS" : "FAIL");

    // Test 2: Streaming CRC update
    Serial.println("Test 2: Streaming CRC update");
    Serial.println("  Processing byte-by-byte...");

    uint16_t streaming_crc = 0xFFFF;
    for (size_t i = 0; i < sizeof(test_vector); i++) {
        streaming_crc = crc16_update(streaming_crc, test_vector[i]);
    }

    Serial.printf("  Expected: 0x%04X\n", expected_crc);
    Serial.printf("  Calculated: 0x%04X\n", streaming_crc);

    bool test2_pass = (streaming_crc == expected_crc);
    Serial.printf("  Result: %s\n\n", test2_pass ? "PASS" : "FAIL");

    // Test 3: Empty buffer (NULL check)
    Serial.println("Test 3: NULL buffer handling");

    uint16_t null_crc = crc16_calculate(NULL, 0);

    Serial.printf("  NULL buffer CRC: 0x%04X\n", null_crc);
    Serial.printf("  Expected (initial): 0x%04X\n", (uint16_t)0xFFFF);

    bool test3_pass = (null_crc == 0xFFFF);
    Serial.printf("  Result: %s\n\n", test3_pass ? "PASS" : "FAIL");

    // Summary
    Serial.println("=== Test Summary ===");
    int passed = (test1_pass ? 1 : 0) + (test2_pass ? 1 : 0) + (test3_pass ? 1 : 0);
    Serial.printf("Tests passed: %d/3\n", passed);

    if (passed == 3) {
        Serial.println("\n*TEST_PASS*");
    } else {
        Serial.println("\n*TEST_FAIL*");
    }

    Serial.println("*STOP*");
}

void loop() {
    // Empty - single-run test
}
