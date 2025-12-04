// i2c_command_CI.ino - DPS3xx CI/HIL test with BoardConfig
//
// Tests DPS3xx barometric pressure sensor initialization and measurement.
// Uses BoardConfig system for I2C pin configuration.
//
// Supported targets:
//   MATEK_H743VI:     Uses BoardConfig::baro I2C (PB11/PB10)
//   BLACKPILL_F411CE: Uses BoardConfig::sensors I2C (PB9/PB8)
//   NUCLEO_F411RE:    Uses BoardConfig::sensors I2C (PB9/PB8)

#include <Dps3xx.h>
#include <Wire.h>
#include <ci_log.h>
#include <libPrintf.h>

// BoardConfig for target-specific I2C pins
#if defined(ARDUINO_MATEK_H743VI)
  #include "../../../../targets/MTKS-MATEKH743.h"
  // MATEK H743 has dedicated baro I2C bus
  #define BARO_SDA BoardConfig::baro.sda_pin
  #define BARO_SCL BoardConfig::baro.scl_pin
  #define TARGET_NAME "MATEK_H743VI"
#elif defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../../../targets/BLACKPILL_F411CE.h"
  // BlackPill uses sensors I2C for barometer
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define TARGET_NAME "BLACKPILL_F411CE"
#elif defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE.h"
  // Nucleo uses sensors I2C for barometer
  #define BARO_SDA BoardConfig::sensors.sda_pin
  #define BARO_SCL BoardConfig::sensors.scl_pin
  #define TARGET_NAME "NUCLEO_F411RE"
#else
  #error "Unsupported board. Use MATEK_H743VI, BLACKPILL_F411CE, or NUCLEO_F411RE."
#endif

// putchar_() implementation for CI_PRINTF (libPrintf output routing)
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

// DPS3xx sensor
Dps3xx baroSensor;
TwoWire baroWire(BARO_SDA, BARO_SCL);

// Test counters
int passCount = 0;
int failCount = 0;

// Expected ranges for validation
const float TEMP_MIN = -40.0f;   // Sensor spec minimum
const float TEMP_MAX = 85.0f;    // Sensor spec maximum
const float PRES_MIN = 30000.0f; // ~30 kPa (very high altitude)
const float PRES_MAX = 110000.0f;// ~110 kPa (below sea level)

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== DPS3xx i2c_command_CI ===\n");
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

    // Initialize DPS3xx
    CI_LOG("Initializing DPS3xx...\n");
    baroSensor.begin(baroWire);

    // Check if initialization succeeded by reading product ID
    uint8_t productId = baroSensor.getProductId();
    uint8_t revisionId = baroSensor.getRevisionId();

    if (productId == 0) {
        CI_LOG("FAIL: DPS3xx not detected (product ID = 0)\n");
        failCount++;
        CI_LOG("\n");
        CI_PRINTF("Tests passed: %d\n", passCount);
        CI_PRINTF("Tests failed: %d\n", failCount);
        CI_LOG("\n*TEST_FAIL*\n");
        CI_LOG("*STOP*\n");
        return;
    }

    CI_PRINTF("PASS: DPS3xx detected (Product ID: 0x%02X, Revision: 0x%02X)\n", productId, revisionId);
    passCount++;
    int16_t ret;
    CI_LOG("\n");

    // Perform measurements
    CI_LOG("=== Measurements ===\n");
    uint8_t oversampling = 7;  // Highest precision

    // Temperature measurement
    float temperature;
    ret = baroSensor.measureTempOnce(temperature, oversampling);
    if (ret != 0) {
        CI_PRINTF("FAIL: Temperature measurement failed (ret=%d)\n", ret);
        failCount++;
    } else {
        CI_PRINTF("Temperature: %.2f C\n", temperature);
        if (temperature >= TEMP_MIN && temperature <= TEMP_MAX) {
            CI_LOG("PASS: Temperature in valid range\n");
            passCount++;
        } else {
            CI_PRINTF("FAIL: Temperature out of range [%.1f, %.1f]\n", TEMP_MIN, TEMP_MAX);
            failCount++;
        }
    }

    // Pressure measurement
    float pressure;
    ret = baroSensor.measurePressureOnce(pressure, oversampling);
    if (ret != 0) {
        CI_PRINTF("FAIL: Pressure measurement failed (ret=%d)\n", ret);
        failCount++;
    } else {
        CI_PRINTF("Pressure: %.2f Pa (%.2f hPa)\n", pressure, pressure / 100.0f);
        if (pressure >= PRES_MIN && pressure <= PRES_MAX) {
            CI_LOG("PASS: Pressure in valid range\n");
            passCount++;
        } else {
            CI_PRINTF("FAIL: Pressure out of range [%.0f, %.0f]\n", PRES_MIN, PRES_MAX);
            failCount++;
        }
    }

    // Multiple reading consistency test
    CI_LOG("\n=== Consistency Test (5 readings) ===\n");
    float temps[5], pres[5];
    bool consistencyPass = true;

    for (int i = 0; i < 5; i++) {
        ret = baroSensor.measureTempOnce(temps[i], oversampling);
        if (ret != 0) {
            CI_PRINTF("FAIL: Temp reading %d failed\n", i);
            consistencyPass = false;
            break;
        }
        ret = baroSensor.measurePressureOnce(pres[i], oversampling);
        if (ret != 0) {
            CI_PRINTF("FAIL: Pressure reading %d failed\n", i);
            consistencyPass = false;
            break;
        }
        CI_PRINTF("[%d] T=%.2f C  P=%.2f hPa\n", i, temps[i], pres[i] / 100.0f);
        delay(50);
    }

    if (consistencyPass) {
        // Check variance (readings should be within 1C and 100 Pa)
        float tempMin = temps[0], tempMax = temps[0];
        float presMin = pres[0], presMax = pres[0];
        for (int i = 1; i < 5; i++) {
            if (temps[i] < tempMin) tempMin = temps[i];
            if (temps[i] > tempMax) tempMax = temps[i];
            if (pres[i] < presMin) presMin = pres[i];
            if (pres[i] > presMax) presMax = pres[i];
        }

        float tempVariance = tempMax - tempMin;
        float presVariance = presMax - presMin;

        CI_PRINTF("Temp variance: %.3f C\n", tempVariance);
        CI_PRINTF("Pressure variance: %.2f Pa\n", presVariance);

        if (tempVariance < 1.0f && presVariance < 100.0f) {
            CI_LOG("PASS: Readings consistent\n");
            passCount++;
        } else {
            CI_LOG("FAIL: Readings too variable\n");
            failCount++;
        }
    } else {
        failCount++;
    }

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
