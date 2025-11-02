/*
 * bf_types.h - Minimal Betaflight device types for Arduino
 *
 * This is a clean-room implementation for Arduino compatibility.
 * NOT copied from Betaflight (avoid GPL contamination).
 *
 * License: MIT
 *
 * Purpose: Provide minimal type definitions needed to support
 *          Betaflight-derived MPU-6000/MPU-9250 drivers in Arduino
 *          environment without copying GPL code.
 */

#ifndef BF_TYPES_H
#define BF_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include "bf_bus.h"

// Forward declaration
struct extDevice_s;

// MPU detection result enumeration
typedef enum {
    MPU_NONE = 0,
    MPU_60x0_SPI,
    MPU_9250_SPI,
    ICM_42688P_SPI
} mpuDetectionResult_e;

// Gyro device structure
// Minimal fields needed for MPU driver operation
typedef struct gyroDev_s {
    struct extDevice_s dev;         // SPI bus interface
    float scale;                    // Scale factor (dps per LSB)
    int16_t gyroADCRaw[3];         // Raw gyro data [X, Y, Z]
    mpuDetectionResult_e mpuDetectionResult;
    uint8_t hardware_lpf;          // Hardware low-pass filter setting
    void (*initFn)(struct gyroDev_s *gyro);    // Init function pointer
    bool (*readFn)(struct gyroDev_s *gyro);    // Read function pointer
} gyroDev_t;

// Accelerometer device structure
// Minimal fields needed for MPU driver operation
typedef struct accDev_s {
    struct extDevice_s dev;         // SPI bus interface
    float scale;                    // Scale factor (g per LSB)
    int16_t ADCRaw[3];             // Raw accel data [X, Y, Z]
    void (*initFn)(struct accDev_s *acc);      // Init function pointer
    bool (*readFn)(struct accDev_s *acc);      // Read function pointer
} accDev_t;

// Helper macros for scale factors
#define GYRO_SCALE_2000DPS  (2000.0f / 32768.0f)  // 16.384 dps/LSB for ±2000°/s
#define GYRO_SCALE_250DPS   (250.0f / 32768.0f)   // 131.0 dps/LSB for ±250°/s
#define ACC_1G_LSB          4096                   // For ±8g range

#endif // BF_TYPES_H
