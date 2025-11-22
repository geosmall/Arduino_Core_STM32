/*
 * IMU Library - Interrupt-Driven Raw Data Example
 *
 * Demonstrates interrupt-driven raw sensor data acquisition using the IMU library.
 * Uses data-ready interrupt on INT1 pin to trigger readings when new data is available.
 *
 * HARDWARE CONFIGURATION:
 * - Uses BoardConfig for automatic board detection (NUCLEO_F411RE / BLACKPILL_F411CE)
 * - Pin assignments and SPI frequency from board configuration
 * - Interrupt pin configured for data-ready signaling
 *
 * CI/HIL INTEGRATION:
 * - RTT output for automated testing
 * - Serial output for Arduino IDE
 * - Deterministic exit with "*STOP*" wildcard
 * - Build traceability with git SHA and timestamp
 */

#include <IMU.h>
#include <ci_log.h>
#include <SPI.h>
#include <libPrintf.h>

// Board configuration - Multi-board support
#if defined(ARDUINO_BKMN_NERO)
#include "../../../../targets/BKMN-NERO.h"
#elif defined(ARDUINO_BLACKPILL_F411CE)
#include "../../../../targets/BLACKPILL_F411CE.h"
#else
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
#endif

// Create SPI instance using BoardConfig (software CS control)
SPIClass spi_bus(BoardConfig::imu.spi.mosi_pin,
                 BoardConfig::imu.spi.miso_pin,
                 BoardConfig::imu.spi.sclk_pin,
                 BoardConfig::imu.spi.get_ssel_pin());

// Create IMU instance
IMU imu;

// Interrupt flag
volatile bool data_ready = false;

// Interrupt handler
void imu_data_ready_handler() {
    data_ready = true;
}

void setup() {
    // Initialize communication (Serial or RTT)
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
#endif

    CI_LOG("\n=== IMU Library - Interrupt-Driven Data Example ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Display pin configuration
    CI_LOG("Pin Configuration (BoardConfig):\n");
    printf("  CS: %d, MOSI: %d, MISO: %d, SCLK: %d\n",
           (int)BoardConfig::imu.spi.cs_pin,
           (int)BoardConfig::imu.spi.mosi_pin,
           (int)BoardConfig::imu.spi.miso_pin,
           (int)BoardConfig::imu.spi.sclk_pin);
    printf("  SPI Speed: %lu Hz\n", (unsigned long)BoardConfig::imu.spi.freq_hz);

    if (BoardConfig::imu.int_pin != 0) {
        printf("  Interrupt Pin: %d\n\n", (int)BoardConfig::imu.int_pin);
    } else {
        CI_LOG("  Interrupt Pin: None configured\n");
        CI_LOG("ERROR: This example requires interrupt pin!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Give IMU time to stabilize
    delay(5);

    // Initialize IMU
    CI_LOG("Initializing IMU...\n");
    if (imu.Init(spi_bus, BoardConfig::imu.spi.cs_pin, BoardConfig::imu.spi.freq_hz) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to initialize IMU!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }
    CI_LOG("✓ IMU initialized successfully\n");

    // Detect chip type
    IMU::ChipType chip = imu.GetChipType();
    const char* chip_name = "UNKNOWN";
    switch (chip) {
        case IMU::ChipType::ICM42688_P: chip_name = "ICM-42688-P"; break;
        case IMU::ChipType::MPU_6000:   chip_name = "MPU-6000"; break;
        case IMU::ChipType::MPU_9250:   chip_name = "MPU-9250"; break;
        case IMU::ChipType::ICM20601:   chip_name = "ICM-20601"; break;
        case IMU::ChipType::ICM20602:   chip_name = "ICM-20602"; break;
        case IMU::ChipType::ICM20689:   chip_name = "ICM-20689"; break;
        default: break;
    }
    printf("Detected chip: %s (0x%02X)\n", chip_name, static_cast<uint8_t>(chip));
    CI_LOG("\n");

    // Configure IMU for interrupt-driven operation
    CI_LOG("Configuring IMU...\n");

    // Apply SAFE preset for interrupt mode (1kHz with DLPF enabled)
    // Note: SAFE preset uses DLPF which is required for proper data-ready interrupts
    // on MPU-6000, MPU-9250, and ICM-206xx (DLPF=0 bypass mode has irregular interrupts)
    if (imu.ApplyPreset(IMU::Preset::SAFE) != IMU::Result::OK) {
        CI_LOG("ERROR: Failed to apply SAFE preset!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    // Configure interrupt pin
    pinMode(BoardConfig::imu.int_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(BoardConfig::imu.int_pin),
                    imu_data_ready_handler, RISING);

    // Enable data ready interrupt on INT
    if (imu.EnableDataReadyInt() != 0) {
        CI_LOG("ERROR: Failed to enable data ready interrupt!\n");
        CI_LOG("*STOP*\n");
        while (1) delay(1000);
    }

    CI_LOG("✓ IMU configured for interrupt-driven operation\n");
    CI_LOG("  Preset: SAFE (1kHz with DLPF filtering)\n");
    CI_LOG("  Gyro: ±2000 DPS, 1kHz ODR\n");
    CI_LOG("  Accel: ±16G, 1kHz ODR\n");
    CI_LOG("  INT1: Data Ready enabled\n\n");

    // Collect 100 samples
    CI_LOG("Collecting 100 samples...\n\n");

    std::array<int16_t, 6> imu_data;
    int sample_count = 0;
    const int target_samples = 100;

    while (sample_count < target_samples) {
        if (data_ready) {
            data_ready = false;

            // Read raw IMU data
            if (imu.ReadIMU6(imu_data) == 0) {
                sample_count++;

                // Print every 5th sample
                if (sample_count % 5 == 0) {
                    printf("Sample %d: ", sample_count);
                    printf("Accel[%6d,%6d,%6d] ",
                           imu_data[0], imu_data[1], imu_data[2]);
                    printf("Gyro[%6d,%6d,%6d]\n",
                           imu_data[3], imu_data[4], imu_data[5]);
                }
            }
        }
    }

    CI_LOG("\n✓ Data collection complete\n");

    // Disable interrupt
    imu.DisableDataReadyInt();
    detachInterrupt(digitalPinToInterrupt(BoardConfig::imu.int_pin));

    CI_LOG("\n=== Test Complete ===\n");
    CI_LOG("*STOP*\n");
}

void loop() {
    // Nothing to do
}

/* --------------------------------------------------------------------------------------
 *  libPrintf putchar_ implementation for RTT/Serial routing
 * -------------------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

void putchar_(char c) {
#ifdef USE_RTT
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);
#else
    Serial.print(c);
#endif
}

#ifdef __cplusplus
}
#endif
