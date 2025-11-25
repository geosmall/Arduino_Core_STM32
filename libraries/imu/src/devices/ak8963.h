/*
 * AK8963 Magnetometer Register Definitions
 * For use with MPU-9250 9-DOF IMU
 *
 * This file defines registers and constants for the AK8963 3-axis magnetometer
 * embedded in the MPU-9250. The AK8963 is accessed via the MPU-9250's auxiliary
 * I2C master interface.
 *
 * Reference: AK8963 datasheet, dRehmFlight Teensy MPU9250 library
 */

#ifndef AK8963_H
#define AK8963_H

#include <stdint.h>

// AK8963 I2C Address
#define AK8963_I2C_ADDR         0x0C    // 7-bit I2C address

// AK8963 Register Addresses
#define AK8963_WHO_AM_I         0x00    // Device ID (should read 0x48)
#define AK8963_INFO             0x01    // Information
#define AK8963_ST1              0x02    // Status 1 (data ready)
#define AK8963_HXL              0x03    // X-axis magnetometer data (LSB)
#define AK8963_HXH              0x04    // X-axis magnetometer data (MSB)
#define AK8963_HYL              0x05    // Y-axis magnetometer data (LSB)
#define AK8963_HYH              0x06    // Y-axis magnetometer data (MSB)
#define AK8963_HZL              0x07    // Z-axis magnetometer data (LSB)
#define AK8963_HZH              0x08    // Z-axis magnetometer data (MSB)
#define AK8963_ST2              0x09    // Status 2 (data overflow)
#define AK8963_CNTL1            0x0A    // Control register 1 (mode)
#define AK8963_CNTL2            0x0B    // Control register 2 (reset)
#define AK8963_ASTC             0x0C    // Self-test control
#define AK8963_I2CDIS           0x0F    // I2C disable
#define AK8963_ASAX             0x10    // X-axis sensitivity adjustment
#define AK8963_ASAY             0x11    // Y-axis sensitivity adjustment
#define AK8963_ASAZ             0x12    // Z-axis sensitivity adjustment

// AK8963 WHO_AM_I Response
#define AK8963_WHO_AM_I_RESPONSE 0x48   // Expected WHO_AM_I value

// AK8963 Control Register 1 (CNTL1) - Mode Selection
#define AK8963_CNTL1_MODE_POWER_DOWN    0x00    // Power-down mode
#define AK8963_CNTL1_MODE_SINGLE        0x01    // Single measurement mode
#define AK8963_CNTL1_MODE_CONT_1        0x02    // Continuous measurement mode 1 (8 Hz)
#define AK8963_CNTL1_MODE_EXT_TRIG      0x04    // External trigger measurement mode
#define AK8963_CNTL1_MODE_CONT_2        0x06    // Continuous measurement mode 2 (100 Hz)
#define AK8963_CNTL1_MODE_SELF_TEST     0x08    // Self-test mode
#define AK8963_CNTL1_MODE_FUSE_ROM      0x0F    // Fuse ROM access mode

// AK8963 Control Register 1 (CNTL1) - Bit Resolution
#define AK8963_CNTL1_BIT_14             0x00    // 14-bit output (0.6 µT/LSB)
#define AK8963_CNTL1_BIT_16             0x10    // 16-bit output (0.15 µT/LSB)

// AK8963 Control Register 2 (CNTL2) - Reset
#define AK8963_CNTL2_SRST               0x01    // Soft reset

// AK8963 Status Register 1 (ST1) - Data Ready
#define AK8963_ST1_DRDY                 0x01    // Data ready bit

// AK8963 Status Register 2 (ST2) - Data Status
#define AK8963_ST2_HOFL                 0x08    // Magnetic sensor overflow
#define AK8963_ST2_BITM                 0x10    // Output bit setting (14/16-bit)

// Combined Mode Settings (Mode | Bit Resolution)
#define AK8963_MODE_POWER_DOWN_16BIT    (AK8963_CNTL1_MODE_POWER_DOWN | AK8963_CNTL1_BIT_16)
#define AK8963_MODE_SINGLE_16BIT        (AK8963_CNTL1_MODE_SINGLE | AK8963_CNTL1_BIT_16)
#define AK8963_MODE_CONT_1_16BIT        (AK8963_CNTL1_MODE_CONT_1 | AK8963_CNTL1_BIT_16)    // 8 Hz, 16-bit
#define AK8963_MODE_CONT_2_16BIT        (AK8963_CNTL1_MODE_CONT_2 | AK8963_CNTL1_BIT_16)    // 100 Hz, 16-bit
#define AK8963_MODE_FUSE_ROM_16BIT      (AK8963_CNTL1_MODE_FUSE_ROM | AK8963_CNTL1_BIT_16)

// I2C Master Control Flags (for MPU9250's I2C master interface)
#define I2C_MST_EN                      0x20    // Enable I2C master mode
#define I2C_MST_CLK_400KHZ              0x0D    // Set I2C master clock to 400 kHz
#define I2C_SLV0_EN                     0x80    // Enable I2C slave 0
#define I2C_READ_FLAG                   0x80    // I2C read flag (OR with address)

// Magnetometer Sensitivity (16-bit mode)
// Formula: Hadj = H * ((ASA - 128) * 0.5 / 128 + 1)
// Output: ±4912 µT, 16-bit resolution = 0.15 µT/LSB
#define AK8963_SENSITIVITY_SCALE_FACTOR 4912.0f / 32760.0f  // µT per LSB

#endif // AK8963_H
