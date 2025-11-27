/*
 * MPU9250 Reference Test using invensense-imu library
 *
 * This test uses the known-working invensense-imu library to verify
 * hardware connectivity before testing our IMU library.
 *
 * Based on: libraries/invensense-imu/examples/arduino/mpu9250/spi/spi.ino
 * Modified to use CI tools for HIL testing on BlackPill F411CE
 *
 * Hardware: MPU-9250 on SPI2 (PB13=SCK, PB14=MISO, PB15=MOSI, PB12=CS)
 */

#include <ci_log.h>
#include <libPrintf.h>
#include <SPI.h>
#include "mpu9250.h"

// putchar_ required for CI_PRINTF / libPrintf
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

// SPI2 bus: MOSI=PB15, MISO=PB14, SCK=PB13
SPIClass SPI_bus(PB15, PB14, PB13);

// Mpu9250 object on SPI_bus with CS on PB12
bfs::Mpu9250 imu(&SPI_bus, PB12);

void setup() {
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_LOG("\n=== MPU9250 Reference Test (invensense-imu) ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Start the SPI2 bus
    SPI_bus.begin();

    // Initialize and configure IMU
    CI_LOG("Initializing MPU-9250...\n");
    if (!imu.Begin()) {
        CI_LOG("ERROR: Failed to initialize MPU-9250!\n");
        CI_LOG("Check hardware connections.\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    CI_LOG("SUCCESS: MPU-9250 initialized!\n");

    // Set the sample rate divider (50 Hz output)
    if (!imu.ConfigSrd(19)) {
        CI_LOG("WARNING: Failed to configure SRD\n");
    }

    CI_LOG("\nReading sensor data (10 samples)...\n");
}

int sample_count = 0;

void loop() {
    if (sample_count >= 10) {
        CI_LOG("\n=== Test Complete ===\n");
        CI_LOG("*STOP*\n");
        while (1);
    }

    // Check if data read
    if (imu.Read()) {
        sample_count++;

        CI_PRINTF("Sample %d:\n", sample_count);
        CI_PRINTF("  Accel: X=%.2f Y=%.2f Z=%.2f m/s^2\n",
                imu.accel_x_mps2(), imu.accel_y_mps2(), imu.accel_z_mps2());
        CI_PRINTF("  Gyro:  X=%.3f Y=%.3f Z=%.3f rad/s\n",
                imu.gyro_x_radps(), imu.gyro_y_radps(), imu.gyro_z_radps());
        CI_PRINTF("  Mag:   X=%.1f Y=%.1f Z=%.1f uT\n",
                imu.mag_x_ut(), imu.mag_y_ut(), imu.mag_z_ut());
        CI_PRINTF("  Temp:  %.1f C\n", imu.die_temp_c());
    }

    delay(100);
}
