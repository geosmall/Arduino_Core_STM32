/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Original Betaflight source:
 * https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_mpu9250.c
 * Copyright: Betaflight contributors
 * License: GPLv3
 *
 * Modifications for Arduino (2024):
 * - C++ class with factory pattern
 * - DeviceBus abstraction for portability
 * - Shared register map (MPU_Common.h)
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MPU9250.h"
#include "MPU_Common.h"
#include "ak8963.h"

// ============================================================================
// MPU9250-Specific Definitions
// ============================================================================

// Max SPI frequency: 20 MHz (from Betaflight)
#define MPU9250_MAX_SPI_CLK_HZ 20000000

// DLPF (Digital Low Pass Filter) configuration register
#define MPU9250_CONFIG  0x1A

// ============================================================================
// MPU9250 Implementation
// ============================================================================

MPU9250::MPU9250(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus)
    , mag_initialized_(false)
    , mag_scale_x_(1.0f)
    , mag_scale_y_(1.0f)
    , mag_scale_z_(1.0f)
    , mag_bias_x_(0.0f)
    , mag_bias_y_(0.0f)
    , mag_bias_z_(0.0f)
    , mag_scale_factor_x_(1.0f)
    , mag_scale_factor_y_(1.0f)
    , mag_scale_factor_z_(1.0f)
{
    whoAmI_ = whoAmI;

    // Power management - use PLL with Z-axis gyro reference
    bus_->writeReg(MPU_RA_PWR_MGMT_1, INV_CLK_PLL);
    delayMicroseconds(1);

    // Gyro full scale: ±2000 dps
    bus_->writeReg(MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    delayMicroseconds(1);

    // DLPF configuration (from Betaflight default)
    // Note: This would normally come from gyroConfig()->gyro_hardware_lpf
    // Using a conservative 188Hz LPF (DLPF_CFG = 1)
    bus_->writeReg(MPU9250_CONFIG, 0x01);
    delayMicroseconds(1);

    // Sample rate divider: 0 = 1kHz (8kHz / (1 + 0))
    bus_->writeReg(MPU_RA_SMPLRT_DIV, 0);
    delayMicroseconds(1);

    // Accel full scale: ±16g
    bus_->writeReg(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
    delayMicroseconds(1);

    // INT pin configuration
    // Bit 4: INT_ANYRD_2CLEAR = 1 (interrupt status cleared on any read)
    // Note: Bypass mode NOT enabled here - use initMagnetometer() for I2C master mode
    bus_->writeReg(MPU_RA_INT_PIN_CFG, (1 << 4));
    delayMicroseconds(1);

    // Enable data ready interrupt
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x01);
    delayMicroseconds(1);

    // Set scale factors
    // MPU9250: ±2000 dps = 16.4 LSB/(dps), ±16g = 2048 LSB/g
    gyrScale_ = 1.0f / 16.4f;     // [dps/LSB]
    accScale_ = 1.0f / 2048.0f;   // [G/LSB]
    samplingRateHz_ = 1000;       // 1 kHz
}

MPU9250* MPU9250::detect(DeviceBus* bus)
{
    if (!bus) {
        return nullptr;
    }

    // Reset the device
    bus->writeReg(MPU_RA_PWR_MGMT_1, MPU9250_BIT_RESET);

    // Wait for reset to complete with retry mechanism (from Betaflight)
    uint8_t attemptsRemaining = 20;
    do {
        delay(150);  // Delay between attempts
        const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);

        // Check for MPU9250 (0x71) or MPU9255 (0x73)
        if (whoAmI == MPU9250_WHO_AM_I_CONST || whoAmI == MPU9255_WHO_AM_I_CONST) {
            // Device detected - create and initialize instance
            return new MPU9250(bus, whoAmI);
        }

        if (!attemptsRemaining) {
            return nullptr;  // Detection failed
        }
    } while (attemptsRemaining--);

    return nullptr;
}

void MPU9250::read(int16_t* accgyr)
{
    // Read 14 bytes: AX_H, AX_L, AY_H, AY_L, AZ_H, AZ_L, TEMP_H, TEMP_L, GX_H, GX_L, GY_H, GY_L, GZ_H, GZ_L
    uint8_t buf[14];
    bus_->readRegs(MPU_RA_ACCEL_XOUT_H, buf, 14);

    // Parse big-endian data
    accgyr[0] = (int16_t)((buf[0] << 8) | buf[1]);   // Accel X
    accgyr[1] = (int16_t)((buf[2] << 8) | buf[3]);   // Accel Y
    accgyr[2] = (int16_t)((buf[4] << 8) | buf[5]);   // Accel Z
    // Skip temperature (buf[6-7])
    accgyr[3] = (int16_t)((buf[8] << 8) | buf[9]);   // Gyro X
    accgyr[4] = (int16_t)((buf[10] << 8) | buf[11]); // Gyro Y
    accgyr[5] = (int16_t)((buf[12] << 8) | buf[13]); // Gyro Z
}

const char* MPU9250::typeName() const
{
    // Distinguish between MPU9250 and MPU9255 based on WHO_AM_I
    if (whoAmI_ == MPU9255_WHO_AM_I_CONST) {
        return "MPU9255";
    }
    return "MPU9250";
}

// ============================================================================
// Preset Configuration (imu_hal.md specification)
// ============================================================================

// MPU9250 preset configuration structure
struct MPU9250PresetConfig {
    uint8_t  dlpf_cfg_gyro;   // CONFIG.DLPF_CFG (0-7)
    uint8_t  dlpf_cfg_accel;  // ACCEL_CONFIG2.A_DLPF_CFG (0-7)
    uint8_t  smplrt_div;      // SMPLRT_DIV (only for DLPF_CFG=1-6)
    uint16_t gyro_odr_hz;     // Effective rate
};

// Preset LUT from imu_hal.md
// MPU9250 (MPU6500-class): DLPF_CFG=0 bypasses divider → 8kHz, DLPF_CFG=1-6 uses divider
static const MPU9250PresetConfig MPU9250_PRESETS[] = {
    {2, 2, 0, 1000},  // SAFE: DLPF=2 (92Hz gyro, 94Hz accel), DIV=0 → 1kHz
    {1, 1, 0, 1000},  // SMOOTH: DLPF=1 (184Hz gyro, 184Hz accel), DIV=0 → 1kHz
    {0, 0, 0, 8000},  // BALANCED: DLPF=0 (bypass), 8kHz (SW decimate to 4kHz if needed)
    {0, 0, 0, 8000}   // ACRO: DLPF=0 (bypass), 8kHz
};

// ACCEL_CONFIG2 register address (MPU9250-specific)
#define MPU_RA_ACCEL_CONFIG2    0x1D

bool MPU9250::applyPreset(ImuPreset preset)
{
    const MPU9250PresetConfig& cfg = MPU9250_PRESETS[static_cast<uint8_t>(preset)];

    // Set gyro DLPF configuration
    bus_->writeReg(MPU_RA_CONFIG, cfg.dlpf_cfg_gyro);
    delayMicroseconds(15);

    // Set accel DLPF configuration (MPU9250 has separate accel filter)
    bus_->writeReg(MPU_RA_ACCEL_CONFIG2, cfg.dlpf_cfg_accel);
    delayMicroseconds(15);

    // Set sample rate divider (only effective when DLPF_CFG=1-6)
    bus_->writeReg(MPU_RA_SMPLRT_DIV, cfg.smplrt_div);
    delayMicroseconds(15);

    // Update sampling rate for user reference
    samplingRateHz_ = cfg.gyro_odr_hz;

    // Wait for gyro digital filter to stabilize after DLPF mode change
    // MPU-9250 datasheet: gyro startup time 35ms typical
    delay(50);

    return true;
}

void MPU9250::enableDataReadyInt()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);
}

void MPU9250::disableDataReadyInt()
{
    bus_->writeReg(MPU_RA_INT_ENABLE, 0x00);
    delayMicroseconds(15);
}

// ============================================================================
// DeviceBase Tier 2/3 Extended API Implementation
// ============================================================================

bool MPU9250::setGyroFSR(GyroFSR fsr)
{
    uint8_t fsr_code;
    float scale;
    switch(fsr) {
        case GyroFSR::DPS_250:  fsr_code = INV_FSR_250DPS;  scale = 1.0f / 131.0f; break;
        case GyroFSR::DPS_500:  fsr_code = INV_FSR_500DPS;  scale = 1.0f / 65.5f;  break;
        case GyroFSR::DPS_1000: fsr_code = INV_FSR_1000DPS; scale = 1.0f / 32.8f;  break;
        case GyroFSR::DPS_2000: fsr_code = INV_FSR_2000DPS; scale = 1.0f / 16.4f;  break;
        default: return false;
    }

    bus_->writeReg(MPU_RA_GYRO_CONFIG, fsr_code << 3);
    delayMicroseconds(15);
    gyrScale_ = scale;
    return true;
}

bool MPU9250::setAccelFSR(AccelFSR fsr)
{
    uint8_t fsr_code;
    float scale;
    switch(fsr) {
        case AccelFSR::G_2:  fsr_code = INV_FSR_2G;  scale = 1.0f / 16384.0f; break;
        case AccelFSR::G_4:  fsr_code = INV_FSR_4G;  scale = 1.0f / 8192.0f;  break;
        case AccelFSR::G_8:  fsr_code = INV_FSR_8G;  scale = 1.0f / 4096.0f;  break;
        case AccelFSR::G_16: fsr_code = INV_FSR_16G; scale = 1.0f / 2048.0f;  break;
        default: return false;
    }

    bus_->writeReg(MPU_RA_ACCEL_CONFIG, fsr_code << 3);
    delayMicroseconds(15);
    accScale_ = scale;
    return true;
}

uint8_t MPU9250::readReg(uint8_t reg)
{
    return bus_->readReg(reg);
}

bool MPU9250::writeReg(uint8_t reg, uint8_t value)
{
    bus_->writeReg(reg, value);
    return true;
}

bool MPU9250::writeRegVerify(uint8_t reg, uint8_t value)
{
    bus_->writeReg(reg, value);
    delayMicroseconds(15);
    uint8_t readback = bus_->readReg(reg);
    return (readback == value);
}

// ============================================================================
// Magnetometer (AK8963) Implementation
// ============================================================================

bool MPU9250::writeAK8963Register(uint8_t reg, uint8_t value)
{
    // Set I2C slave 0 for write operation
    bus_->writeReg(MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR);  // Write mode
    bus_->writeReg(MPU_RA_I2C_SLV0_REG, reg);               // Target register
    bus_->writeReg(MPU_RA_I2C_SLV0_DO, value);              // Data to write
    bus_->writeReg(MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01); // Enable + 1 byte

    delay(10); // Wait for I2C transaction to complete

    // Read back and verify (like invensense-imu library)
    uint8_t readback = 0;
    if (!readAK8963Registers(reg, 1, &readback)) {
        return false;
    }
    return (readback == value);
}

bool MPU9250::readAK8963Registers(uint8_t reg, uint8_t count, uint8_t *dest)
{
    if (!dest) {
        return false;
    }

    // Set I2C slave 0 for read operation
    bus_->writeReg(MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG);  // Read mode
    bus_->writeReg(MPU_RA_I2C_SLV0_REG, reg);                               // Target register
    bus_->writeReg(MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | count);              // Enable + byte count

    delay(1); // Wait for data to populate EXT_SENS_DATA registers

    // Read from EXT_SENS_DATA registers
    for (uint8_t i = 0; i < count; i++) {
        dest[i] = bus_->readReg(MPU_RA_EXT_SENS_DATA_00 + i);
    }

    return true;
}

uint8_t MPU9250::whoAmIAK8963()
{
    uint8_t who_am_i = 0;
    readAK8963Registers(AK8963_WHO_AM_I, 1, &who_am_i);
    return who_am_i;
}

bool MPU9250::initMagnetometer()
{
    if (mag_initialized_) {
        return true;  // Already initialized
    }

    // Enable I2C master mode (must be done before any AK8963 communication)
    bus_->writeReg(MPU_RA_USER_CTRL, BIT_I2C_MST_EN);
    delay(10);

    // Configure I2C master clock to 400 kHz
    bus_->writeReg(MPU_RA_I2C_MST_CTRL, I2C_MST_CLK_400KHZ);
    delay(10);

    // Power down magnetometer first (don't verify - starts in unknown state)
    bus_->writeReg(MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR);
    bus_->writeReg(MPU_RA_I2C_SLV0_REG, AK8963_CNTL1);
    bus_->writeReg(MPU_RA_I2C_SLV0_DO, AK8963_CNTL1_MODE_POWER_DOWN);
    bus_->writeReg(MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01);
    delay(100);

    // Soft reset magnetometer (SRST bit auto-clears, so don't verify)
    bus_->writeReg(MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR);
    bus_->writeReg(MPU_RA_I2C_SLV0_REG, AK8963_CNTL2);
    bus_->writeReg(MPU_RA_I2C_SLV0_DO, AK8963_CNTL2_SRST);
    bus_->writeReg(MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x01);
    delay(100);

    // Check WHO_AM_I
    uint8_t who_am_i = whoAmIAK8963();
    if (who_am_i != AK8963_WHO_AM_I_RESPONSE) {
        return false;  // AK8963 not detected
    }

    // Power down before entering FUSE ROM mode
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN)) {
        return false;
    }
    delay(100);

    // Enter Fuse ROM access mode to read ASA calibration values
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_MODE_FUSE_ROM_16BIT)) {
        return false;
    }
    delay(100);

    // Read ASA (Adjustable Sensitivity Adjustment) values
    uint8_t asa[3];
    if (!readAK8963Registers(AK8963_ASAX, 3, asa)) {
        return false;
    }

    // Calculate sensitivity scale factors from ASA values
    // Formula: Hadj = H * ((ASA - 128) * 0.5 / 128 + 1)
    mag_scale_x_ = (float)(asa[0] - 128) / 256.0f + 1.0f;
    mag_scale_y_ = (float)(asa[1] - 128) / 256.0f + 1.0f;
    mag_scale_z_ = (float)(asa[2] - 128) / 256.0f + 1.0f;

    // Power down before switching to continuous mode
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_CNTL1_MODE_POWER_DOWN)) {
        return false;
    }
    delay(100);

    // Set continuous measurement mode 2 (100 Hz) with 16-bit output
    if (!writeAK8963Register(AK8963_CNTL1, AK8963_MODE_CONT_2_16BIT)) {
        return false;
    }
    delay(100);

    // Configure I2C Slave 0 for continuous auto-reading of magnetometer data
    // This configures the MPU9250 to automatically read 7 bytes from AK8963
    // at the gyro sample rate and populate EXT_SENS_DATA_00 to EXT_SENS_DATA_06
    bus_->writeReg(MPU_RA_I2C_SLV0_ADDR, AK8963_I2C_ADDR | I2C_READ_FLAG);
    bus_->writeReg(MPU_RA_I2C_SLV0_REG, AK8963_HXL);
    bus_->writeReg(MPU_RA_I2C_SLV0_CTRL, I2C_SLV0_EN | 0x07);  // Enable + 7 bytes
    delay(10);

    mag_initialized_ = true;
    return true;
}

bool MPU9250::readMagnetometer(float* mag)
{
    if (!mag_initialized_ || !mag) {
        return false;
    }

    // Read magnetometer data directly from EXT_SENS_DATA registers
    // The MPU9250 I2C master automatically populates these at the gyro sample rate
    // (7 bytes: HXL, HXH, HYL, HYH, HZL, HZH, ST2)
    uint8_t mag_data[7];
    for (uint8_t i = 0; i < 7; i++) {
        mag_data[i] = bus_->readReg(MPU_RA_EXT_SENS_DATA_00 + i);
    }

    // Check ST2 status register for overflow
    if (mag_data[6] & AK8963_ST2_HOFL) {
        // Magnetic sensor overflow detected
        return false;
    }

    // Extract raw magnetometer data (LSB-first byte order!)
    int16_t mx_raw = (int16_t)((mag_data[1] << 8) | mag_data[0]);
    int16_t my_raw = (int16_t)((mag_data[3] << 8) | mag_data[2]);
    int16_t mz_raw = (int16_t)((mag_data[5] << 8) | mag_data[4]);

    // Apply ASA scale factors and convert to µT
    // AK8963 16-bit: 0.15 µT/LSB (±4912 µT range)
    float mx_ut = (float)mx_raw * mag_scale_x_ * AK8963_SENSITIVITY_SCALE_FACTOR;
    float my_ut = (float)my_raw * mag_scale_y_ * AK8963_SENSITIVITY_SCALE_FACTOR;
    float mz_ut = (float)mz_raw * mag_scale_z_ * AK8963_SENSITIVITY_SCALE_FACTOR;

    // Apply calibration (hard iron bias + soft iron scale)
    mag[0] = (mx_ut - mag_bias_x_) * mag_scale_factor_x_;
    mag[1] = (my_ut - mag_bias_y_) * mag_scale_factor_y_;
    mag[2] = (mz_ut - mag_bias_z_) * mag_scale_factor_z_;

    return true;
}

bool MPU9250::readMagnetometerRaw(int16_t* mag)
{
    if (!mag_initialized_ || !mag) {
        return false;
    }

    // Read magnetometer data directly from EXT_SENS_DATA registers
    // The MPU9250 I2C master automatically populates these at the gyro sample rate
    // (7 bytes: HXL, HXH, HYL, HYH, HZL, HZH, ST2)
    uint8_t mag_data[7];
    for (uint8_t i = 0; i < 7; i++) {
        mag_data[i] = bus_->readReg(MPU_RA_EXT_SENS_DATA_00 + i);
    }

    // Check ST2 status register for overflow
    if (mag_data[6] & AK8963_ST2_HOFL) {
        // Magnetic sensor overflow detected
        return false;
    }

    // Extract raw magnetometer data (LSB-first byte order!)
    // Return raw int16_t values without ASA scaling or calibration
    mag[0] = (int16_t)((mag_data[1] << 8) | mag_data[0]);
    mag[1] = (int16_t)((mag_data[3] << 8) | mag_data[2]);
    mag[2] = (int16_t)((mag_data[5] << 8) | mag_data[4]);

    return true;
}

bool MPU9250::read9DOF(int16_t* accgyr, float* mag)
{
    // Read 6-axis data (accel + gyro)
    read(accgyr);

    // Read magnetometer
    return readMagnetometer(mag);
}

bool MPU9250::calibrateMagnetometer()
{
    if (!mag_initialized_) {
        return false;
    }

    const uint16_t sample_count = 1500;  // 1500 samples over 15 seconds
    const uint16_t sample_delay = 10;    // 10ms between samples (100 Hz)

    float mag_max_x = -10000.0f, mag_max_y = -10000.0f, mag_max_z = -10000.0f;
    float mag_min_x = 10000.0f, mag_min_y = 10000.0f, mag_min_z = 10000.0f;

    // Temporarily disable calibration to get raw scaled values
    float temp_bias_x = mag_bias_x_;
    float temp_bias_y = mag_bias_y_;
    float temp_bias_z = mag_bias_z_;
    float temp_scale_x = mag_scale_factor_x_;
    float temp_scale_y = mag_scale_factor_y_;
    float temp_scale_z = mag_scale_factor_z_;

    mag_bias_x_ = mag_bias_y_ = mag_bias_z_ = 0.0f;
    mag_scale_factor_x_ = mag_scale_factor_y_ = mag_scale_factor_z_ = 1.0f;

    // Collect samples
    for (uint16_t i = 0; i < sample_count; i++) {
        float mag[3];

        if (readMagnetometer(mag)) {
            // Track min/max for each axis
            if (mag[0] > mag_max_x) mag_max_x = mag[0];
            if (mag[0] < mag_min_x) mag_min_x = mag[0];
            if (mag[1] > mag_max_y) mag_max_y = mag[1];
            if (mag[1] < mag_min_y) mag_min_y = mag[1];
            if (mag[2] > mag_max_z) mag_max_z = mag[2];
            if (mag[2] < mag_min_z) mag_min_z = mag[2];
        }

        delay(sample_delay);
    }

    // Calculate hard iron bias (center of sphere)
    mag_bias_x_ = (mag_max_x + mag_min_x) / 2.0f;
    mag_bias_y_ = (mag_max_y + mag_min_y) / 2.0f;
    mag_bias_z_ = (mag_max_z + mag_min_z) / 2.0f;

    // Calculate soft iron scale factors (normalize to average radius)
    float avg_radius = ((mag_max_x - mag_min_x) + (mag_max_y - mag_min_y) + (mag_max_z - mag_min_z)) / 6.0f;
    mag_scale_factor_x_ = avg_radius / ((mag_max_x - mag_min_x) / 2.0f);
    mag_scale_factor_y_ = avg_radius / ((mag_max_y - mag_min_y) / 2.0f);
    mag_scale_factor_z_ = avg_radius / ((mag_max_z - mag_min_z) / 2.0f);

    return true;
}

void MPU9250::setMagCalibration(float bias_x, float bias_y, float bias_z,
                                float scale_x, float scale_y, float scale_z)
{
    mag_bias_x_ = bias_x;
    mag_bias_y_ = bias_y;
    mag_bias_z_ = bias_z;
    mag_scale_factor_x_ = scale_x;
    mag_scale_factor_y_ = scale_y;
    mag_scale_factor_z_ = scale_z;
}

void MPU9250::getMagCalibration(float& bias_x, float& bias_y, float& bias_z,
                                float& scale_x, float& scale_y, float& scale_z) const
{
    bias_x = mag_bias_x_;
    bias_y = mag_bias_y_;
    bias_z = mag_bias_z_;
    scale_x = mag_scale_factor_x_;
    scale_y = mag_scale_factor_y_;
    scale_z = mag_scale_factor_z_;
}
