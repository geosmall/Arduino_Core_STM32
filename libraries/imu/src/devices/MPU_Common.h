// Arduino_Core_STM32 IMU Library - MPU/ICM Common Register Definitions
// Extracted from Betaflight accgyro_mpu.h
// Shared by: MPU6000, MPU6500, MPU9250, ICM20601, ICM20602, ICM20689, etc.
//
// Original: https://github.com/betaflight/betaflight/.../accgyro_mpu.h
//
// This header contains register definitions common to all InvenSense MPU/ICM
// family IMUs that share the legacy MPU-6000 register map.

#pragma once

#include <stdint.h>

// ============================================================================
// WHO_AM_I Constants
// ============================================================================

#define MPU6000_WHO_AM_I_CONST              (0x68)
#define MPU6500_WHO_AM_I_CONST              (0x70)
#define MPU9250_WHO_AM_I_CONST              (0x71)
#define MPU9255_WHO_AM_I_CONST              (0x73)
#define ICM20601_WHO_AM_I_CONST             (0xAC)
#define ICM20602_WHO_AM_I_CONST             (0x12)
#define ICM20608G_WHO_AM_I_CONST            (0xAF)
#define ICM20649_WHO_AM_I_CONST             (0xE1)
#define ICM20689_WHO_AM_I_CONST             (0x98)

// ============================================================================
// Register Addresses (RA = Register Address)
// ============================================================================

#define MPU_RA_WHO_AM_I         0x75
#define MPU_RA_PRODUCT_ID       0x0C    // Product ID Register (MPU6000 only)

// Offset registers
#define MPU_RA_XG_OFFS_TC       0x00
#define MPU_RA_YG_OFFS_TC       0x01
#define MPU_RA_ZG_OFFS_TC       0x02
#define MPU_RA_XA_OFFS_H        0x06
#define MPU_RA_YA_OFFS_H        0x08
#define MPU_RA_ZA_OFFS_H        0x0A
#define MPU_RA_XG_OFFS_USRH     0x13
#define MPU_RA_XG_OFFS_USRL     0x14
#define MPU_RA_YG_OFFS_USRH     0x15
#define MPU_RA_YG_OFFS_USRL     0x16
#define MPU_RA_ZG_OFFS_USRH     0x17
#define MPU_RA_ZG_OFFS_USRL     0x18

// Configuration registers
#define MPU_RA_SMPLRT_DIV       0x19    // Sample Rate Divider
#define MPU_RA_CONFIG           0x1A    // Configuration
#define MPU_RA_GYRO_CONFIG      0x1B    // Gyroscope Configuration
#define MPU_RA_ACCEL_CONFIG     0x1C    // Accelerometer Configuration

// Interrupt registers
#define MPU_RA_INT_PIN_CFG      0x37    // Interrupt Pin Configuration
#define MPU_RA_INT_ENABLE       0x38    // Interrupt Enable
#define MPU_RA_INT_STATUS       0x3A    // Interrupt Status

// Data output registers (big-endian)
#define MPU_RA_ACCEL_XOUT_H     0x3B
#define MPU_RA_ACCEL_XOUT_L     0x3C
#define MPU_RA_ACCEL_YOUT_H     0x3D
#define MPU_RA_ACCEL_YOUT_L     0x3E
#define MPU_RA_ACCEL_ZOUT_H     0x3F
#define MPU_RA_ACCEL_ZOUT_L     0x40
#define MPU_RA_TEMP_OUT_H       0x41
#define MPU_RA_TEMP_OUT_L       0x42
#define MPU_RA_GYRO_XOUT_H      0x43
#define MPU_RA_GYRO_XOUT_L      0x44
#define MPU_RA_GYRO_YOUT_H      0x45
#define MPU_RA_GYRO_YOUT_L      0x46
#define MPU_RA_GYRO_ZOUT_H      0x47
#define MPU_RA_GYRO_ZOUT_L      0x48

// Control registers
#define MPU_RA_SIGNAL_PATH_RESET    0x68    // Signal Path Reset
#define MPU_RA_USER_CTRL            0x6A    // User Control
#define MPU_RA_PWR_MGMT_1           0x6B    // Power Management 1
#define MPU_RA_PWR_MGMT_2           0x6C    // Power Management 2
#define MPU_RA_FIFO_R_W             0x74    // FIFO Read/Write

// ============================================================================
// Register Bit Definitions
// ============================================================================

// PWR_MGMT_1 (0x6B)
#define BIT_H_RESET             0x80    // Device reset
#define BIT_SLEEP               0x40    // Sleep mode enable
#define MPU_CLK_SEL_PLLGYROZ    0x03    // PLL with Z-axis gyro reference
#define INV_CLK_PLL             0x01    // PLL clock source

// USER_CTRL (0x6A)
#define BIT_I2C_IF_DIS          0x10    // Disable I2C interface (SPI only)

// SIGNAL_PATH_RESET (0x68)
#define BIT_GYRO                0x04    // Reset gyro signal path
#define BIT_ACC                 0x02    // Reset accel signal path
#define BIT_TEMP                0x01    // Reset temp signal path

// INT_ENABLE (0x38)
#define MPU_RF_DATA_RDY_EN      (1 << 0)    // Data ready interrupt enable

// ============================================================================
// Full Scale Range Enumerations
// ============================================================================

// Gyroscope full scale ranges
enum gyro_fsr_e {
    INV_FSR_250DPS = 0,     // ±250 dps
    INV_FSR_500DPS,         // ±500 dps
    INV_FSR_1000DPS,        // ±1000 dps
    INV_FSR_2000DPS,        // ±2000 dps
    NUM_GYRO_FSR
};

// Accelerometer full scale ranges
enum accel_fsr_e {
    INV_FSR_2G = 0,         // ±2g
    INV_FSR_4G,             // ±4g
    INV_FSR_8G,             // ±8g
    INV_FSR_16G,            // ±16g
    NUM_ACCEL_FSR
};

// ============================================================================
// Constants
// ============================================================================

// MPU9250-specific bit
#define MPU9250_BIT_RESET       0x80    // Same as BIT_H_RESET
