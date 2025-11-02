/*
 * bf_bus_test.ino - Hardware validation test for Betaflight bus abstraction
 *
 * Purpose: Validate bf_bus.h/.cpp SPI wrapper functions work correctly
 *          with actual hardware (ICM-42688-P on NUCLEO_F411RE)
 *
 * Hardware Setup:
 *   - NUCLEO_F411RE
 *   - ICM-42688-P on SPI1 (PA4 CS, PA5 SCK, PA6 MISO, PA7 MOSI)
 *
 * Test Cases:
 *   1. spiReadRegMsk - Read WHO_AM_I register (should return 0x47)
 *   2. spiWriteReg - Write/readback USER_BANK_SEL register
 *   3. spiReadRegBuf - Read multiple registers (burst read)
 *   4. spiWriteRegBuf - Write multiple registers (burst write)
 *
 * Expected Results:
 *   ✅ WHO_AM_I returns 0x47 (ICM-42688-P)
 *   ✅ Write/readback matches
 *   ✅ Burst read returns valid data
 *   ✅ All SPI operations complete without errors
 *
 * License: MIT
 */

#include <SPI.h>
#include <ci_log.h>
#include "bf_bus.h"
#include "bf_types.h"

// ICM-42688-P register addresses
#define ICM42688_WHO_AM_I        0x75
#define ICM42688_REG_BANK_SEL    0x76
#define ICM42688_ACCEL_DATA_X1   0x1F  // Start of accel data (6 bytes)
#define ICM42688_GYRO_DATA_X1    0x25  // Start of gyro data (6 bytes)

// Expected WHO_AM_I value
#define ICM42688P_WHO_AM_I_VALUE 0x47

// Hardware configuration (NUCLEO_F411RE + ICM42688P)
#define CS_PIN          PA4
#define SPI_FREQ        1000000  // 1 MHz

// SPI pins for NUCLEO_F411RE SPI1
#define MOSI_PIN        PA7
#define MISO_PIN        PA6
#define SCK_PIN         PA5

// Create SPI instance (software CS control)
SPIClass spi_bus(MOSI_PIN, MISO_PIN, SCK_PIN);

// Test state
extDevice_t imu_device;
bool all_tests_passed = true;

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Betaflight Bus Abstraction Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize SPI
  spi_bus.begin();
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);

  // Setup device structure
  imu_device.spi = &spi_bus;
  imu_device.cs_pin = CS_PIN;
  imu_device.freq = SPI_FREQ;

  CI_LOG("\nHardware Configuration:\n");
  CI_LOGF("  SPI Bus: SPI_1\n");
  CI_LOGF("  CS Pin: PA4\n");
  CI_LOGF("  SPI Freq: %lu Hz\n", SPI_FREQ);

  delay(100);  // Allow IMU to power up

  // Run tests
  test_spiReadRegMsk();
  test_spiWriteReg();
  test_spiReadRegBuf();
  test_spiWriteRegBuf();

  // Final result
  CI_LOG("\n=== Test Summary ===\n");
  if (all_tests_passed) {
    CI_LOG("*TEST_PASS* All bus abstraction tests passed!\n");
  } else {
    CI_LOG("*TEST_FAIL* Some tests failed\n");
  }

  CI_LOG("*STOP*\n");
}

void loop() {
  // Test complete - halt
}

void test_spiReadRegMsk() {
  CI_LOG("\n[TEST 1] spiReadRegMsk - Read WHO_AM_I\n");

  uint8_t whoami = spiReadRegMsk(&imu_device, ICM42688_WHO_AM_I);

  CI_LOGF("  WHO_AM_I register: 0x%02X\n", whoami);

  if (whoami == ICM42688P_WHO_AM_I_VALUE) {
    CI_LOG("  ✅ PASS - WHO_AM_I correct (0x47)\n");
  } else {
    CI_LOGF("  ❌ FAIL - Expected 0x%02X, got 0x%02X\n",
            ICM42688P_WHO_AM_I_VALUE, whoami);
    all_tests_passed = false;
  }
}

void test_spiWriteReg() {
  CI_LOG("\n[TEST 2] spiWriteReg - Write/Readback Test\n");

  // Read current bank select value
  uint8_t original = spiReadRegMsk(&imu_device, ICM42688_REG_BANK_SEL);
  CI_LOGF("  Original REG_BANK_SEL: 0x%02X\n", original);

  // Write test value (bank 0)
  uint8_t test_value = 0x00;
  spiWriteReg(&imu_device, ICM42688_REG_BANK_SEL, test_value);
  delay(1);  // Allow register write to settle

  // Read back
  uint8_t readback = spiReadRegMsk(&imu_device, ICM42688_REG_BANK_SEL);
  CI_LOGF("  Write 0x%02X, readback: 0x%02X\n", test_value, readback);

  // Restore original value
  spiWriteReg(&imu_device, ICM42688_REG_BANK_SEL, original);

  if (readback == test_value) {
    CI_LOG("  ✅ PASS - Write/readback matches\n");
  } else {
    CI_LOGF("  ❌ FAIL - Expected 0x%02X, got 0x%02X\n", test_value, readback);
    all_tests_passed = false;
  }
}

void test_spiReadRegBuf() {
  CI_LOG("\n[TEST 3] spiReadRegBuf - Burst Read (Gyro Data)\n");

  uint8_t gyro_data[6];
  bool success = spiReadRegBuf(&imu_device, ICM42688_GYRO_DATA_X1, gyro_data, 6);

  if (!success) {
    CI_LOG("  ❌ FAIL - spiReadRegBuf returned false\n");
    all_tests_passed = false;
    return;
  }

  CI_LOG("  Gyro data (6 bytes): ");
  for (int i = 0; i < 6; i++) {
    CI_LOGF("0x%02X ", gyro_data[i]);
  }
  CI_LOG("\n");

  // Convert to 16-bit values
  int16_t gx = (int16_t)((gyro_data[0] << 8) | gyro_data[1]);
  int16_t gy = (int16_t)((gyro_data[2] << 8) | gyro_data[3]);
  int16_t gz = (int16_t)((gyro_data[4] << 8) | gyro_data[5]);

  CI_LOGF("  Gyro raw: X=%d, Y=%d, Z=%d\n", gx, gy, gz);

  // Sanity check - values should not all be 0x00 or 0xFF
  bool all_zero = true;
  bool all_ff = true;
  for (int i = 0; i < 6; i++) {
    if (gyro_data[i] != 0x00) all_zero = false;
    if (gyro_data[i] != 0xFF) all_ff = false;
  }

  if (!all_zero && !all_ff) {
    CI_LOG("  ✅ PASS - Burst read returned valid data\n");
  } else {
    CI_LOG("  ❌ FAIL - Burst read returned invalid data (all 0x00 or 0xFF)\n");
    all_tests_passed = false;
  }
}

void test_spiWriteRegBuf() {
  CI_LOG("\n[TEST 4] spiWriteRegBuf - Burst Write Test\n");

  // Note: ICM-42688-P doesn't have a safe multi-byte write test register
  // This test validates the API works without modifying critical registers

  // Write dummy pattern to unused registers (if available)
  // For now, we'll just test the function returns true with valid params
  uint8_t dummy_data[2] = {0x00, 0x00};
  bool success = spiWriteRegBuf(&imu_device, ICM42688_REG_BANK_SEL, dummy_data, 1);

  if (success) {
    CI_LOG("  ✅ PASS - spiWriteRegBuf executed successfully\n");
  } else {
    CI_LOG("  ❌ FAIL - spiWriteRegBuf returned false\n");
    all_tests_passed = false;
  }

  CI_LOG("  Note: Full write validation skipped (no safe test registers)\n");
}
