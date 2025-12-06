// dps3xx_inav_style_CI.ino - DPS310 iNav-style DMA barometer demo
//
// Demonstrates non-blocking DMA I2C reads for flight controller applications:
//   - Continuous pressure measurement (iNav-style configuration)
//   - Poll PRS_RDY, 3-byte DMA read
//   - 4-point moving average + 2 Hz PT1 low-pass filter
//   - Altitude output @ ~16 Hz (decimated by 2)
//
// Supported targets:
//   NUCLEO_F411RE:    External DPS310 on sensors I2C (PB9/PB8) @ 0x77
//   BLACKPILL_F411CE: External DPS310 on sensors I2C (PB9/PB8) @ 0x77
//   MATEK_H743VI:     Onboard DPS310 on baro I2C (PB11/PB10) @ 0x76
//
// Hardware validation: Demonstrates CPU freedom during DMA transfer

#include <Dps3xxDma.h>
#include <Wire.h>
#include <ci_log.h>
#include <libPrintf.h>

// BoardConfig for target-specific I2C pins and addresses
#if defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE.h"
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define BARO_ADDR 0x77
  #define TARGET_NAME "NUCLEO_F411RE"
#elif defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../../../targets/BLACKPILL_F411CE.h"
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define BARO_ADDR 0x77
  #define TARGET_NAME "BLACKPILL_F411CE"
#elif defined(ARDUINO_MATEK_H743VI)
  #include "../../../../targets/MTKS-MATEKH743.h"
  #define BARO_SDA BoardConfig::baro.sda_pin
  #define BARO_SCL BoardConfig::baro.scl_pin
  #define BARO_ADDR 0x76
  #define TARGET_NAME "MATEK_H743VI"
#else
  #error "Unsupported board. Use NUCLEO_F411RE, BLACKPILL_F411CE, or MATEK_H743VI."
#endif

// putchar_() implementation for CI_PRINTF (libPrintf output routing)
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

// I2C bus and barometer instances
TwoWire baroWire(BARO_SDA, BARO_SCL);
Dps3xxDma baro;

// DMA buffer - must use WIRE_DMA_BUFFER on H7 for cache coherency
WIRE_DMA_BUFFER uint8_t prsRaw[3];

/* ------------------- Filter State --------------------------------- */

// Moving average (4-point box filter)
static float maBuf[4] = {};
static uint8_t maIdx = 0;

// PT1 low-pass filter state
static constexpr float PT1_CUTOFF_HZ = 2.0f;
static constexpr float SAMPLE_DT_S = 1.0f / 16.0f;  // After decimate-by-2
static constexpr float PT1_ALPHA = (2.0f * PI * PT1_CUTOFF_HZ * SAMPLE_DT_S)
                                   / (1.0f + 2.0f * PI * PT1_CUTOFF_HZ * SAMPLE_DT_S);
static float pt1State = 0.0f;

// Statistics
static uint32_t sampleCount = 0;
static uint32_t dmaErrorCount = 0;
static uint32_t printCount = 0;
static uint32_t lastPrint_ms = 0;

/* ------------------- Helper Functions ----------------------------- */

// Pressure (Pa) to altitude (m) using barometric formula
static inline float paToAlt(float pa)
{
    constexpr float SEA_LEVEL_PA = 101325.0f;
    return 44330.0f * (1.0f - powf(pa / SEA_LEVEL_PA, 0.190284f));
}

// Convert 24-bit big-endian two's complement to int32_t
static inline int32_t unpack24(const uint8_t* p)
{
    int32_t v = (int32_t(p[0]) << 16) | (int32_t(p[1]) << 8) | p[2];
    if (v & 0x00800000) v -= 0x01000000;  // Sign-extend
    return v;
}

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== DPS310 iNav-Style DMA Demo ===\n");
    CI_READY_TOKEN();
    CI_LOG("\n");

    // Display board configuration
    CI_LOG("Board Configuration:\n");
    CI_PRINTF("  Target: %s\n", TARGET_NAME);
    CI_PRINTF("  Baro I2C Addr: 0x%02X\n", BARO_ADDR);
    CI_LOG("\n");

    // Initialize I2C
    baroWire.begin();
    baroWire.setClock(400000);
    CI_LOG("I2C initialized at 400 kHz\n");

    // Initialize barometer (Dps3xxDma.begin() doesn't call Wire.begin())
    if (!baro.begin(baroWire, BARO_ADDR)) {
        CI_LOG("FAIL: Baro init failed\n");
        CI_LOG("*TEST_FAIL*\n*STOP*\n");
        while(1);
    }
    CI_LOG("PASS: Baro initialized\n");

    // Do a blocking read first (like i2c_dma_test does)
    CI_LOG("Blocking read first...\n");
    baroWire.beginTransmission(BARO_ADDR);
    baroWire.write(0x0D);  // Product ID
    baroWire.endTransmission(false);
    uint8_t bytesRead = baroWire.requestFrom((uint8_t)BARO_ADDR, (uint8_t)1);
    if (bytesRead > 0) {
        uint8_t prodId = baroWire.read();
        CI_PRINTF("  Blocking read: Product ID = 0x%02X\n", prodId);
    } else {
        CI_LOG("  Blocking read failed\n");
    }

    // Test DMA after blocking read
    // IMPORTANT: Use endTransmission() with STOP (not repeated start) for DMA
    CI_LOG("DMA test after blocking read...\n");
    baroWire.beginTransmission(BARO_ADDR);
    baroWire.write(0x0D);  // Product ID
    uint8_t res1 = baroWire.endTransmission();  // STOP, not repeated start
    bool dmaOk1 = baroWire.requestFromDMA(BARO_ADDR, prsRaw, 1, true);
    CI_PRINTF("  endTransmission: %d, requestFromDMA: %d\n", res1, dmaOk1 ? 1 : 0);
    if (dmaOk1) {
        while (!baroWire.dmaTransferDone()) {}
        CI_PRINTF("  Product ID (DMA): 0x%02X\n", prsRaw[0]);
        CI_LOG("PASS: DMA works after blocking read\n");
    } else {
        CI_LOG("FAIL: DMA after blocking read\n");
        CI_LOG("*TEST_FAIL*\n*STOP*\n");
        while(1);
    }

    // Get initial temperature for compensation
    float tempC;
    delay(50);
    baro.measureTempOnce(tempC, DPS__OVERSAMPLING_RATE_2);
    CI_PRINTF("Initial temp: %.1f C\n", tempC);

    // Start continuous pressure measurement (iNav style: 32 Hz, OSR 16x)
    int16_t ret = baro.startPressure32HzNoFIFO();
    if (ret != DPS__SUCCEEDED) {
        CI_PRINTF("FAIL: startPressure32HzNoFIFO returned %d\n", ret);
        CI_LOG("*TEST_FAIL*\n*STOP*\n");
        while(1);
    }
    CI_LOG("PASS: Continuous pressure started (32 Hz, OSR 16x)\n");

    // Enable PRS-ready interrupt flag
    baro.setInterruptSources(dps::INT_FLAG_PRS);

    CI_LOG("\nRunning for 5 seconds...\n\n");
    lastPrint_ms = millis();

    // Quick test: one DMA read to verify it works before loop
    // IMPORTANT: Use endTransmission() with STOP for DMA (STM32 HAL requirement)
    CI_LOG("Quick DMA test...\n");
    baroWire.beginTransmission(BARO_ADDR);
    baroWire.write(0x00);
    uint8_t res = baroWire.endTransmission();  // STOP, not repeated start
    CI_PRINTF("  endTransmission: %d\n", res);

    bool dmaOk = baroWire.requestFromDMA(BARO_ADDR, prsRaw, 3, true);
    CI_PRINTF("  requestFromDMA: %d\n", dmaOk ? 1 : 0);
    if (dmaOk) {
        while (!baroWire.dmaTransferDone()) {}
        CI_PRINTF("  DMA data: %02X %02X %02X\n", prsRaw[0], prsRaw[1], prsRaw[2]);
        CI_LOG("PASS: Setup DMA test\n\n");
    } else {
        CI_LOG("FAIL: Setup DMA test\n");
        CI_LOG("*TEST_FAIL*\n*STOP*\n");
        while(1);
    }
}

void loop()
{
    static bool primed = false;
    static bool keep = false;
    static uint8_t skip = 4;  // Skip first 4 samples (warm-up)

    // Poll PRS_RDY bit
    if (!baro.pressureSampleIsReady()) {
        return;  // No new sample yet
    }

    // Set register pointer and kick off DMA read
    // IMPORTANT: Use STOP condition for DMA (STM32 HAL requirement)
    baroWire.beginTransmission(BARO_ADDR);
    baroWire.write(0x00);  // PSR_B2 register
    if (baroWire.endTransmission() != 0) {  // STOP, not repeated start
        dmaErrorCount++;
        return;
    }

    if (!baroWire.requestFromDMA(BARO_ADDR, prsRaw, 3, true)) {
        dmaErrorCount++;
        return;
    }

    // Wait for DMA completion with timeout
    uint32_t waitStart = millis();
    while (!baroWire.dmaTransferDone() && (millis() - waitStart) < 10) {}
    if (!baroWire.dmaTransferDone()) {
        dmaErrorCount++;
        return;
    }

    sampleCount++;

    // Skip warm-up samples
    if (skip > 0) {
        --skip;
        return;
    }

    // Convert raw 24-bit value
    int32_t rawPressure = unpack24(prsRaw);
    float pa = baro.pressurePaFromRaw(rawPressure);
    float alt = paToAlt(pa);

    // Prime filters on first valid sample
    if (!primed) {
        primed = true;
        for (int i = 0; i < 4; ++i) maBuf[i] = alt;
        maIdx = 0;
        pt1State = alt;
        CI_PRINTF("Startup Alt: %.2f m\n", alt);
        return;
    }

    // Decimate by 2 (keep every second sample) -> 16 Hz
    keep = !keep;
    if (!keep) return;

    // 4-sample moving average
    maBuf[maIdx] = alt;
    maIdx = (maIdx + 1) & 3;
    float altMA = (maBuf[0] + maBuf[1] + maBuf[2] + maBuf[3]) * 0.25f;

    // 2 Hz PT1 low-pass filter
    pt1State += PT1_ALPHA * (altMA - pt1State);

    // Print at ~16 Hz (every 62 ms)
    if (millis() - lastPrint_ms >= 62) {
        lastPrint_ms = millis();
        printCount++;
        CI_PRINTF("Alt: %.2f m (samples: %lu)\n", pt1State, sampleCount);
    }

    // Run for 5 seconds then stop
    if (millis() > 5000) {
        CI_LOG("\n=== Summary ===\n");
        CI_PRINTF("Total samples: %lu\n", sampleCount);
        CI_PRINTF("DMA errors: %lu\n", dmaErrorCount);
        CI_PRINTF("Print count: %lu (~16 Hz output)\n", printCount);

        // Expect ~160 samples in 5s at 32 Hz (allow margin for sensor config)
        if (sampleCount >= 100 && sampleCount <= 500 && dmaErrorCount == 0) {
            CI_LOG("PASS: DMA reads working, sample rate OK\n");
            CI_LOG("*TEST_PASS*\n");
        } else if (dmaErrorCount > 0) {
            CI_PRINTF("FAIL: %lu DMA errors occurred\n", dmaErrorCount);
            CI_LOG("*TEST_FAIL*\n");
        } else {
            CI_PRINTF("FAIL: Expected 100-500 samples, got %lu\n", sampleCount);
            CI_LOG("*TEST_FAIL*\n");
        }
        CI_LOG("*STOP*\n");
        while(1);
    }
}
