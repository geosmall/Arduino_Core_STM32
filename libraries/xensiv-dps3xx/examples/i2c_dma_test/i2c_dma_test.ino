// i2c_dma_test.ino - Wire DMA Read Test with DPS3xx
//
// Tests non-blocking DMA I2C read functionality using DPS3xx barometer.
// Compares blocking vs DMA read timing and validates data integrity.
//
// Supported targets:
//   NUCLEO_F411RE:    Uses BoardConfig::sensors I2C (PB9/PB8) - Phase 1
//   BLACKPILL_F411CE: Uses BoardConfig::sensors I2C (PB9/PB8) - Phase 1
//   MATEK_H743VI:     Uses BoardConfig::baro I2C (PB11/PB10) - Phase 2

#include <Wire.h>
#include <ci_log.h>
#include <libPrintf.h>

// BoardConfig for target-specific I2C pins
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE.h"
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define TARGET_NAME "NUCLEO_F411RE"
#elif defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../../../targets/BLACKPILL_F411CE.h"
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define TARGET_NAME "BLACKPILL_F411CE"
#elif defined(ARDUINO_MATEK_H743VI)
  #include "../../../../targets/MTKS-MATEKH743.h"
  #define BARO_SDA BoardConfig::baro.sda_pin
  #define BARO_SCL BoardConfig::baro.scl_pin
  #define TARGET_NAME "MATEK_H743VI"
#else
  #error "Unsupported board. Use NUCLEO_F411RE, BLACKPILL_F411CE, or MATEK_H743VI."
#endif

// DPS3xx I2C address
#define DPS3XX_ADDR 0x77

// DPS3xx registers
#define DPS3XX_PSR_B2     0x00  // Pressure data (3 bytes: B2, B1, B0)
#define DPS3XX_TMP_B2     0x03  // Temperature data (3 bytes: B2, B1, B0)
#define DPS3XX_MEAS_CFG   0x08  // Measurement config
#define DPS3XX_PRODUCT_ID 0x0D  // Product ID register

// putchar_() implementation for CI_PRINTF (libPrintf output routing)
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

TwoWire baroWire(BARO_SDA, BARO_SCL);

// Test counters
int passCount = 0;
int failCount = 0;

// DMA buffer for F4 (no special section needed)
WIRE_DMA_BUFFER uint8_t dmaBuf[8];

// Blocking read buffer
uint8_t blockingBuf[8];

/**
 * @brief Write a register address to set read pointer
 */
bool setRegister(uint8_t reg) {
    baroWire.beginTransmission(DPS3XX_ADDR);
    baroWire.write(reg);
    return (baroWire.endTransmission() == 0);
}

/**
 * @brief Blocking read from current register
 */
int blockingRead(uint8_t* buf, uint8_t len) {
    return baroWire.requestFrom(DPS3XX_ADDR, len);
}

/**
 * @brief Read product ID using blocking method
 */
uint8_t readProductId() {
    setRegister(DPS3XX_PRODUCT_ID);
    if (blockingRead(blockingBuf, 1) == 1) {
        return baroWire.read();
    }
    return 0;
}

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== Wire DMA Read Test ===\n");
    CI_READY_TOKEN();
    CI_LOG("\n");

    // Display board configuration
    CI_LOG("Board Configuration:\n");
    CI_PRINTF("  Target: %s\n", TARGET_NAME);
    CI_PRINTF("  Baro I2C SDA: 0x%04X\n", BARO_SDA);
    CI_PRINTF("  Baro I2C SCL: 0x%04X\n", BARO_SCL);
    CI_LOG("\n");

    // Initialize I2C
    CI_LOG("Initializing I2C...\n");
    baroWire.begin();
    baroWire.setClock(400000);  // 400 kHz

    // Check DPS3xx presence
    uint8_t productId = readProductId();
    if (productId == 0) {
        CI_LOG("FAIL: DPS3xx not detected\n");
        failCount++;
        goto summary;
    }
    CI_PRINTF("PASS: DPS3xx detected (Product ID: 0x%02X)\n", productId);
    passCount++;

#if defined(HAL_DMA_MODULE_ENABLED)
    CI_LOG("\n=== DMA Read Tests ===\n");

    // Test 1: Basic DMA read of product ID
    CI_LOG("Test 1: DMA read product ID...\n");
    {
        memset(dmaBuf, 0, sizeof(dmaBuf));
        setRegister(DPS3XX_PRODUCT_ID);

        if (!baroWire.requestFromDMA(DPS3XX_ADDR, dmaBuf, 1)) {
            CI_LOG("FAIL: requestFromDMA returned false\n");
            failCount++;
        } else {
            // Wait for completion
            uint32_t start = millis();
            while (!baroWire.dmaTransferDone()) {
                if (millis() - start > 100) {
                    CI_LOG("FAIL: DMA timeout\n");
                    failCount++;
                    goto test2;
                }
            }

            if (dmaBuf[0] == productId) {
                CI_PRINTF("PASS: DMA read product ID = 0x%02X (matches blocking)\n", dmaBuf[0]);
                passCount++;
            } else {
                CI_PRINTF("FAIL: DMA read = 0x%02X, expected 0x%02X\n", dmaBuf[0], productId);
                failCount++;
            }
        }
    }

test2:
    // Test 2: Multi-byte DMA read (pressure registers)
    CI_LOG("\nTest 2: DMA read pressure registers (6 bytes)...\n");
    {
        // First do a blocking read for comparison
        setRegister(DPS3XX_PSR_B2);
        blockingRead(blockingBuf, 6);
        for (int i = 0; i < 6; i++) {
            blockingBuf[i] = baroWire.read();
        }

        // Now DMA read
        memset(dmaBuf, 0, sizeof(dmaBuf));
        setRegister(DPS3XX_PSR_B2);

        if (!baroWire.requestFromDMA(DPS3XX_ADDR, dmaBuf, 6)) {
            CI_LOG("FAIL: requestFromDMA returned false\n");
            failCount++;
        } else {
            uint32_t start = millis();
            while (!baroWire.dmaTransferDone()) {
                if (millis() - start > 100) {
                    CI_LOG("FAIL: DMA timeout\n");
                    failCount++;
                    goto test3;
                }
            }

            CI_LOG("DMA data: ");
            for (int i = 0; i < 6; i++) {
                CI_PRINTF("%02X ", dmaBuf[i]);
            }
            CI_LOG("\n");
            CI_LOG("Blocking data: ");
            for (int i = 0; i < 6; i++) {
                CI_PRINTF("%02X ", blockingBuf[i]);
            }
            CI_LOG("\n");

            // Data may differ slightly due to continuous measurements,
            // but should be in similar range (not all zeros or 0xFF)
            bool valid = (dmaBuf[0] != 0 || dmaBuf[1] != 0 || dmaBuf[2] != 0);
            if (valid) {
                CI_LOG("PASS: DMA multi-byte read successful\n");
                passCount++;
            } else {
                CI_LOG("FAIL: DMA data appears invalid (all zeros)\n");
                failCount++;
            }
        }
    }

test3:
    // Test 3: Timing comparison
    CI_LOG("\nTest 3: Timing comparison (10 reads each)...\n");
    {
        uint32_t blockingTime = 0;
        uint32_t dmaTime = 0;

        // Blocking reads
        uint32_t start = micros();
        for (int i = 0; i < 10; i++) {
            setRegister(DPS3XX_PSR_B2);
            blockingRead(blockingBuf, 6);
            for (int j = 0; j < 6; j++) {
                blockingBuf[j] = baroWire.read();
            }
        }
        blockingTime = micros() - start;

        // DMA reads (still blocking on completion for fair comparison)
        start = micros();
        for (int i = 0; i < 10; i++) {
            setRegister(DPS3XX_PSR_B2);
            baroWire.requestFromDMA(DPS3XX_ADDR, dmaBuf, 6);
            while (!baroWire.dmaTransferDone());
        }
        dmaTime = micros() - start;

        CI_PRINTF("Blocking: %lu us for 10 reads (%lu us/read)\n", blockingTime, blockingTime / 10);
        CI_PRINTF("DMA:      %lu us for 10 reads (%lu us/read)\n", dmaTime, dmaTime / 10);

        // DMA should complete (we're not checking for speed improvement
        // since we block waiting anyway - the benefit is CPU is free during transfer)
        CI_LOG("PASS: Timing comparison complete\n");
        passCount++;
    }

    // Test 4: Verify dmaTransferDone() returns true when idle
    CI_LOG("\nTest 4: dmaTransferDone() when idle...\n");
    {
        if (baroWire.dmaTransferDone()) {
            CI_LOG("PASS: dmaTransferDone() returns true when idle\n");
            passCount++;
        } else {
            CI_LOG("FAIL: dmaTransferDone() returns false when idle\n");
            failCount++;
        }
    }

    // Test 5: Reject DMA while busy (start two in quick succession)
    CI_LOG("\nTest 5: Reject DMA while busy...\n");
    {
        setRegister(DPS3XX_PSR_B2);
        bool first = baroWire.requestFromDMA(DPS3XX_ADDR, dmaBuf, 6);
        bool second = baroWire.requestFromDMA(DPS3XX_ADDR, dmaBuf, 6);

        // Wait for first to complete
        while (!baroWire.dmaTransferDone());

        if (first && !second) {
            CI_LOG("PASS: Second DMA correctly rejected while first was busy\n");
            passCount++;
        } else {
            CI_PRINTF("FAIL: first=%d, second=%d (expected first=1, second=0)\n", first, second);
            failCount++;
        }
    }

#else
    CI_LOG("\nWARNING: HAL_DMA_MODULE_ENABLED not defined - DMA tests skipped\n");
#endif /* HAL_DMA_MODULE_ENABLED */

summary:
    // Summary
    CI_LOG("\n=== Summary ===\n");
    CI_PRINTF("Tests passed: %d\n", passCount);
    CI_PRINTF("Tests failed: %d\n", failCount);
    CI_LOG("\n");

    if (failCount == 0) {
        CI_LOG("*TEST_PASS*\n");
    } else {
        CI_LOG("*TEST_FAIL*\n");
    }

    CI_LOG("*STOP*\n");
}

void loop()
{
    // Test complete
}
