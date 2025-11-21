/*
 * This file is part of Arduino_Core_STM32 IMU library.
 *
 * This software is derived from Betaflight and is subject to the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Original Betaflight source:
 * https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_icm426xx.c
 * Copyright: Betaflight contributors
 * License: GPLv3
 *
 * Arduino integration pattern from Madflight (MIT):
 * https://github.com/qqqlab/madflight
 *
 * Modifications for Arduino (2024):
 * - C++ class with factory pattern (madflight approach)
 * - DeviceBus abstraction for SPI/I2C portability
 * - Constructor-based initialization
 * - Removed Betaflight framework dependencies
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include "ICM42688_BF.h"

// Register definitions and constants
#define ICM426XX_MAX_SPI_CLK_HZ 24000000

#define MPU_RA_WHO_AM_I         0x75

#define ICM42605_WHO_AM_I_CONST             (0x42)
#define ICM42688P_WHO_AM_I_CONST            (0x47)
#define IIM42653_WHO_AM_I_CONST             (0x56)

#define ICM_42605_SPI ICM42605_WHO_AM_I_CONST
#define ICM_42688P_SPI ICM42688P_WHO_AM_I_CONST
#define IIM_42653_SPI IIM42653_WHO_AM_I_CONST

#define ICM426XX_RA_REG_BANK_SEL                    0x76
#define ICM426XX_BANK_SELECT0                       0x00
#define ICM426XX_BANK_SELECT1                       0x01
#define ICM426XX_BANK_SELECT2                       0x02

// Fix for stalls in gyro output. See https://github.com/ArduPilot/ardupilot/pull/25332
#define ICM426XX_INTF_CONFIG1                       0x4D
#define ICM426XX_INTF_CONFIG1_AFSR_MASK             0xC0
#define ICM426XX_INTF_CONFIG1_AFSR_DISABLE          0x40

#define ICM426XX_RA_PWR_MGMT0                       0x4E  // User Bank 0
#define ICM426XX_PWR_MGMT0_ACCEL_MODE_LN            (3 << 0)
#define ICM426XX_PWR_MGMT0_GYRO_MODE_LN             (3 << 2)
#define ICM426XX_PWR_MGMT0_GYRO_ACCEL_MODE_OFF      ((0 << 0) | (0 << 2))
#define ICM426XX_PWR_MGMT0_TEMP_DISABLE_OFF         (0 << 5)

#define ICM426XX_RA_GYRO_CONFIG0                    0x4F
#define ICM426XX_RA_ACCEL_CONFIG0                   0x50

// Registers for gyro and acc Anti-Alias Filter
#define ICM426XX_RA_GYRO_CONFIG_STATIC3             0x0C  // User Bank 1
#define ICM426XX_RA_GYRO_CONFIG_STATIC4             0x0D  // User Bank 1
#define ICM426XX_RA_GYRO_CONFIG_STATIC5             0x0E  // User Bank 1
#define ICM426XX_RA_ACCEL_CONFIG_STATIC2            0x03  // User Bank 2
#define ICM426XX_RA_ACCEL_CONFIG_STATIC3            0x04  // User Bank 2
#define ICM426XX_RA_ACCEL_CONFIG_STATIC4            0x05  // User Bank 2

// Register & setting for gyro and acc UI Filter
#define ICM426XX_RA_GYRO_ACCEL_CONFIG0              0x52  // User Bank 0
#define ICM426XX_ACCEL_UI_FILT_BW_LOW_LATENCY       (15 << 4)
#define ICM426XX_GYRO_UI_FILT_BW_LOW_LATENCY        (15 << 0)

#define ICM426XX_RA_ACCEL_DATA_X1                   0x1F  // User Bank 0

#define ICM426XX_RA_INT_CONFIG                      0x14  // User Bank 0
#define ICM426XX_INT1_MODE_PULSED                   (0 << 2)
#define ICM426XX_INT1_DRIVE_CIRCUIT_PP              (1 << 1)
#define ICM426XX_INT1_POLARITY_ACTIVE_HIGH          (1 << 0)

#define ICM426XX_RA_INT_CONFIG0                     0x63  // User Bank 0
#define ICM426XX_UI_DRDY_INT_CLEAR_ON_SBR           ((0 << 5) | (0 << 4))

#define ICM426XX_RA_INT_CONFIG1                     0x64   // User Bank 0
#define ICM426XX_INT_ASYNC_RESET_BIT                4
#define ICM426XX_INT_TDEASSERT_DISABLE_BIT          5
#define ICM426XX_INT_TDEASSERT_DISABLED             (1 << ICM426XX_INT_TDEASSERT_DISABLE_BIT)
#define ICM426XX_INT_TPULSE_DURATION_BIT            6
#define ICM426XX_INT_TPULSE_DURATION_8              (1 << ICM426XX_INT_TPULSE_DURATION_BIT)

#define ICM426XX_RA_INT_SOURCE0                     0x65  // User Bank 0
#define ICM426XX_UI_DRDY_INT1_EN_ENABLED            (1 << 3)

#define ICM426XX_RA_INTF_CONFIG0                    0x4C  // User Bank 0
#define ICM426XX_INTF_CONFIG0_LITTLE_ENDIAN         0x00  // select little endian data mode

// Output data rate configurations
typedef enum {
    ODR_CONFIG_8K = 0,
    ODR_CONFIG_4K,
    ODR_CONFIG_2K,
    ODR_CONFIG_1K,
    ODR_CONFIG_COUNT
} odrConfig_e;

// Anti-alias filter configurations
typedef enum {
    AAF_CONFIG_258HZ = 0,
    AAF_CONFIG_536HZ,
    AAF_CONFIG_997HZ,
    AAF_CONFIG_1962HZ,
    AAF_CONFIG_COUNT
} aafConfig_e;

typedef struct {
    uint8_t delt;
    uint16_t deltSqr;
    uint8_t bitshift;
} aafConfig_t;

// Possible output data rates (ODRs)
static uint8_t odrLUT[ODR_CONFIG_COUNT] = {
    [ODR_CONFIG_8K] = 3,
    [ODR_CONFIG_4K] = 4,
    [ODR_CONFIG_2K] = 5,
    [ODR_CONFIG_1K] = 6,
};

// Possible gyro Anti-Alias Filter (AAF) cutoffs for ICM-42688P
static aafConfig_t aafLUT42688[AAF_CONFIG_COUNT] = {
    [AAF_CONFIG_258HZ]  = {  6,   36, 10 },
    [AAF_CONFIG_536HZ]  = { 12,  144,  8 },
    [AAF_CONFIG_997HZ]  = { 21,  440,  6 },
    [AAF_CONFIG_1962HZ] = { 37, 1376,  4 },
};

// Possible gyro Anti-Alias Filter (AAF) cutoffs for ICM-42605
static aafConfig_t aafLUT42605[AAF_CONFIG_COUNT] = {
    [AAF_CONFIG_258HZ]  = { 21,  440,  6 }, // actually 249 Hz
    [AAF_CONFIG_536HZ]  = { 39, 1536,  4 }, // actually 524 Hz
    [AAF_CONFIG_997HZ]  = { 63, 3968,  3 }, // actually 995 Hz
    [AAF_CONFIG_1962HZ] = { 63, 3968,  3 }, // 995 Hz is the max cutoff on the 42605
};

// =============================================================================
// Preset-Based Configuration (imu_hal.md alignment)
// =============================================================================

/**
 * @brief Complete preset configuration structure
 *
 * Encapsulates all settings for a given preset: ODR, FSR, AAF, UI filters.
 * Values from imu_hal.md lines 42-58.
 */
struct ICM42688PresetConfig {
    // ODR Configuration
    uint16_t gyro_odr_hz;      // 1k, 4k, or 8k
    uint16_t accel_odr_hz;     // Always 1k

    // FSR (always same per imu_hal.md)
    uint16_t gyro_fsr_dps;     // Always 2000
    uint8_t  accel_fsr_g;      // Always 16

    // AAF Configuration (register values)
    uint8_t  gyro_aaf_delt;
    uint8_t  gyro_aaf_bitshift;
    uint16_t gyro_aaf_deltsqr;
    uint8_t  accel_aaf_delt;
    uint8_t  accel_aaf_bitshift;
    uint16_t accel_aaf_deltsqr;

    // UI Filter Configuration
    uint8_t  ui_bw_code_gyro;   // 0-15 (15 = wide/low-latency)
    uint8_t  ui_bw_code_accel;  // 0-15
    uint8_t  ui_order_gyro;     // 1 or 2
    uint8_t  ui_order_accel;    // 1 or 2
};

/**
 * @brief Preset lookup table (imu_hal.md specification)
 *
 * Four intent-based presets provide validated filter/ODR combinations.
 * Reference: imu_hal.md lines 42-58 (preset table + AAF register values)
 */
static const ICM42688PresetConfig ICM42688_PRESETS[] = {
    // SAFE: 1kHz ODR, AAF 126/84Hz, UI code 1, 2nd-order
    // Gyro AAF 126Hz: delt=4, deltsqr=0x0010, bitshift=12
    // Accel AAF 84Hz: delt=3, deltsqr=0x0009, bitshift=12
    {1000, 1000, 2000, 16,  4,12,0x0010,  3,12,0x0009,  1,1, 2,2},

    // SMOOTH: 4kHz ODR, AAF 213/126Hz, UI code 1, 2nd-order
    // Gyro AAF 213Hz: delt=5, deltsqr=0x0019, bitshift=11
    // Accel AAF 126Hz: delt=4, deltsqr=0x0010, bitshift=12
    {4000, 1000, 2000, 16,  5,11,0x0019,  4,12,0x0010,  1,1, 2,2},

    // BALANCED: 4kHz ODR, AAF 258/170Hz, UI code 15, 1st-order
    // Gyro AAF 258Hz: delt=6, deltsqr=0x0024, bitshift=10
    // Accel AAF 170Hz: delt=4, deltsqr=0x0010, bitshift=11
    {4000, 1000, 2000, 16,  6,10,0x0024,  4,11,0x0010,  15,15, 1,1},

    // ACRO: 8kHz ODR, AAF 303/170Hz, UI code 15, 1st-order
    // Gyro AAF 303Hz: delt=7, deltsqr=0x0031, bitshift=10
    // Accel AAF 170Hz: delt=4, deltsqr=0x0010, bitshift=11
    {8000, 1000, 2000, 16,  7,10,0x0031,  4,11,0x0010,  15,15, 1,1}
};

// Helper function: Select register bank
void ICM42688_BF::setUserBank(uint8_t bank) {
    bank &= 7;
    if (bank != currentBank_) {
        bus_->writeReg(ICM426XX_RA_REG_BANK_SEL, bank);
        currentBank_ = bank;
    }
}

// Get human-readable chip name
const char* ICM42688_BF::typeName() const {
    switch (whoAmI_) {
    case ICM_42605_SPI:
        return "ICM42605";
    case ICM_42688P_SPI:
        return "ICM42688P";
    case IIM_42653_SPI:
        return "IIM42653";
    }
    return "UNKNOWN";
}

// Factory method: Detect and initialize IMU
ICM42688_BF* ICM42688_BF::detect(DeviceBus* bus) {
    bus->setFreq(ICM426XX_MAX_SPI_CLK_HZ);

    uint8_t attemptsRemaining = 20;
    do {
        const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);
        switch (whoAmI) {
        case ICM42605_WHO_AM_I_CONST:
        case ICM42688P_WHO_AM_I_CONST:
        case IIM42653_WHO_AM_I_CONST: {
            auto icm = new ICM42688_BF(bus, whoAmI);
            return icm;
        }
        }
        delay(150);
    } while (attemptsRemaining--);

    return nullptr;
}

// Protected constructor: Performs full initialization
ICM42688_BF::ICM42688_BF(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus) {

    whoAmI_ = whoAmI;

    // Set scale factors based on chip type
    switch (whoAmI_) {
      case ICM_42605_SPI:
      case ICM_42688P_SPI:
        accScale_ = 16.0f / 32768.0f;   // ±16g (2048 LSB/g)
        gyrScale_ = 2000.0f / 32768.0f; // ±2000 DPS
        break;
      case IIM_42653_SPI:
        accScale_ = 32.0f / 32768.0f;   // ±32g (1024 LSB/g)
        gyrScale_ = 4000.0f / 32768.0f; // ±4000 DPS
        break;
    }

    bus_->setFreq(ICM426XX_MAX_SPI_CLK_HZ);

    // Turn off ACC and GYRO for configuration
    // See section 12.9 in ICM-42688-P datasheet v1.7
    setUserBank(ICM426XX_BANK_SELECT0);
    bus_->writeReg(ICM426XX_RA_PWR_MGMT0, ICM426XX_PWR_MGMT0_GYRO_ACCEL_MODE_OFF);

    // Select Anti-Alias Filter configuration
    aafConfig_t aafConfig;
    switch (whoAmI_) {
    case ICM_42605_SPI:
        aafConfig = aafLUT42605[AAF_CONFIG_258HZ];
        break;
    case ICM_42688P_SPI:
    case IIM_42653_SPI:
    default:
        aafConfig = aafLUT42688[AAF_CONFIG_258HZ];
    }

    // Configure gyro Anti-Alias Filter (section 5.3 "ANTI-ALIAS FILTER")
    setUserBank(ICM426XX_BANK_SELECT1);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG_STATIC3, aafConfig.delt);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG_STATIC4, aafConfig.deltSqr & 0xFF);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG_STATIC5, (aafConfig.deltSqr >> 8) | (aafConfig.bitshift << 4));

    // Configure accel Anti-Alias Filter
    setUserBank(ICM426XX_BANK_SELECT2);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG_STATIC2, aafConfig.delt << 1);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG_STATIC3, aafConfig.deltSqr & 0xFF);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG_STATIC4, (aafConfig.deltSqr >> 8) | (aafConfig.bitshift << 4));

    // Configure gyro and accel UI Filters
    setUserBank(ICM426XX_BANK_SELECT0);
    bus_->writeReg(ICM426XX_RA_GYRO_ACCEL_CONFIG0, ICM426XX_ACCEL_UI_FILT_BW_LOW_LATENCY | ICM426XX_GYRO_UI_FILT_BW_LOW_LATENCY);

    // Configure interrupt pin
    bus_->writeReg(ICM426XX_RA_INT_CONFIG, ICM426XX_INT1_MODE_PULSED | ICM426XX_INT1_DRIVE_CIRCUIT_PP | ICM426XX_INT1_POLARITY_ACTIVE_HIGH);
    bus_->writeReg(ICM426XX_RA_INT_CONFIG0, ICM426XX_UI_DRDY_INT_CLEAR_ON_SBR);
    bus_->writeReg(ICM426XX_RA_INT_SOURCE0, ICM426XX_UI_DRDY_INT1_EN_ENABLED);

    uint8_t intConfig1Value = bus_->readReg(ICM426XX_RA_INT_CONFIG1);
    // Datasheet: "User should change setting to 0 from default setting of 1"
    intConfig1Value &= ~(1 << ICM426XX_INT_ASYNC_RESET_BIT);
    intConfig1Value |= (ICM426XX_INT_TPULSE_DURATION_8 | ICM426XX_INT_TDEASSERT_DISABLED);
    bus_->writeReg(ICM426XX_RA_INT_CONFIG1, intConfig1Value);

    // Disable AFSR to prevent stalls in gyro output
    uint8_t intfConfig1Value = bus_->readReg(ICM426XX_INTF_CONFIG1);
    intfConfig1Value &= ~ICM426XX_INTF_CONFIG1_AFSR_MASK;
    intfConfig1Value |= ICM426XX_INTF_CONFIG1_AFSR_DISABLE;
    bus_->writeReg(ICM426XX_INTF_CONFIG1, intfConfig1Value);

    // Turn on gyro and accel to configure ODR and FSR
    bus_->writeReg(ICM426XX_RA_PWR_MGMT0, ICM426XX_PWR_MGMT0_TEMP_DISABLE_OFF | ICM426XX_PWR_MGMT0_ACCEL_MODE_LN | ICM426XX_PWR_MGMT0_GYRO_MODE_LN);
    delay(1);

    // Set output data rate to 1kHz
    uint8_t odrConfig = odrLUT[ODR_CONFIG_1K];
    samplingRateHz_ = 1000;

    // Set gyro/accel to maximum FSR for chip
    // ICM42605, ICM_42688P: 2000DPS and 16G
    // IIM42653: 4000DPS and 32G
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG0, (0 << 5) | (odrConfig & 0x0F));
    delay(15);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG0, (0 << 5) | (odrConfig & 0x0F));
    delay(15);

    // Select little-endian data mode
    bus_->writeReg(ICM426XX_RA_INTF_CONFIG0, ICM426XX_INTF_CONFIG0_LITTLE_ENDIAN);
}

// Read 6-axis gyro/accel data
void ICM42688_BF::read(int16_t* accgyr) {
    // Ensure we're on Bank 0 where data registers live
    setUserBank(ICM426XX_BANK_SELECT0);
    // Read 12 bytes: ax,ay,az,gx,gy,gz (little endian)
    bus_->readRegs(ICM426XX_RA_ACCEL_DATA_X1, (uint8_t*)accgyr, 12);
}

// =============================================================================
// Preset-Based Configuration Implementation
// =============================================================================

/**
 * @brief Apply intent-based preset configuration
 *
 * Configures ODR, FSR, AAF, and UI filters per imu_hal.md specification.
 * All presets use ±2000dps/±16g FSR per imu_hal.md lines 20-21.
 *
 * @return true if configuration verified successfully, false if verification failed
 */
bool ICM42688_BF::applyPreset(ImuPreset preset) {
    const ICM42688PresetConfig& cfg = ICM42688_PRESETS[static_cast<uint8_t>(preset)];

    // 1. Set ODR (order matters - do this first)
    setGyroODR(cfg.gyro_odr_hz);
    setAccelODR(cfg.accel_odr_hz);

    // 2. Set FSR (always ±2000dps/±16g per imu_hal.md)
    setGyroFSR(cfg.gyro_fsr_dps);
    setAccelFSR(cfg.accel_fsr_g);

    // 3. Configure AAF filters (bank switching required)
    AAFConfig gyro_aaf = {cfg.gyro_aaf_delt, cfg.gyro_aaf_deltsqr, cfg.gyro_aaf_bitshift};
    AAFConfig accel_aaf = {cfg.accel_aaf_delt, cfg.accel_aaf_deltsqr, cfg.accel_aaf_bitshift};
    setGyroAAF(gyro_aaf);
    setAccelAAF(accel_aaf);

    // 4. Configure UI filters
    setUIFilters(cfg.ui_bw_code_gyro, cfg.ui_bw_code_accel,
                 cfg.ui_order_gyro, cfg.ui_order_accel);

    // 5. Ensure AFSR is disabled (prevents gyro stalls per ArduPilot PR #25332)
    disableAFSR();

    // 6. Ensure sensors are enabled in Low Noise mode after configuration
    // This is required per ICM-42688-P datasheet section 12.9
    setUserBank(ICM426XX_BANK_SELECT0);
    bus_->writeReg(ICM426XX_RA_PWR_MGMT0,
                   ICM426XX_PWR_MGMT0_TEMP_DISABLE_OFF |
                   ICM426XX_PWR_MGMT0_ACCEL_MODE_LN |
                   ICM426XX_PWR_MGMT0_GYRO_MODE_LN);
    delay(50);  // Wait for gyro startup (datasheet: 45ms typical)

    // Store effective sampling rate
    samplingRateHz_ = cfg.gyro_odr_hz;

    // 7. Verify critical registers were written correctly
    return verifyConfiguration(preset);
}

/**
 * @brief Verify current register configuration matches expected preset
 *
 * Reads back critical registers and compares to expected values.
 * Verifies FSR, ODR, AFSR disable, and AAF enable bits.
 *
 * @param preset Expected preset configuration
 * @return true if all critical registers match expected values
 */
bool ICM42688_BF::verifyConfiguration(ImuPreset preset) const {
    const ICM42688PresetConfig& cfg = ICM42688_PRESETS[static_cast<uint8_t>(preset)];

    // Cast away const for bank switching (read-only operation, but needs bank select)
    ICM42688_BF* self = const_cast<ICM42688_BF*>(this);

    // Expected ODR codes
    uint8_t expected_gyro_odr, expected_accel_odr;
    switch(cfg.gyro_odr_hz) {
        case 8000: expected_gyro_odr = 0x03; break;
        case 4000: expected_gyro_odr = 0x05; break;
        case 2000: expected_gyro_odr = 0x06; break;
        case 1000: expected_gyro_odr = 0x07; break;
        default: return false;
    }
    switch(cfg.accel_odr_hz) {
        case 8000: expected_accel_odr = 0x03; break;
        case 4000: expected_accel_odr = 0x05; break;
        case 2000: expected_accel_odr = 0x06; break;
        case 1000: expected_accel_odr = 0x07; break;
        default: return false;
    }

    // Expected FSR codes (always 0 for ±2000dps/±16g)
    uint8_t expected_gyro_fsr = 0;  // ±2000 dps
    uint8_t expected_accel_fsr = 0; // ±16g

    // 1. Verify GYRO_CONFIG0 (Bank 0): FSR[7:5] + ODR[3:0]
    self->setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t gyro_config0 = bus_->readReg(ICM426XX_RA_GYRO_CONFIG0);
    uint8_t actual_gyro_fsr = (gyro_config0 >> 5) & 0x07;
    uint8_t actual_gyro_odr = gyro_config0 & 0x0F;
    if (actual_gyro_fsr != expected_gyro_fsr || actual_gyro_odr != expected_gyro_odr) {
        return false;
    }

    // 2. Verify ACCEL_CONFIG0 (Bank 0): FSR[7:5] + ODR[3:0]
    uint8_t accel_config0 = bus_->readReg(ICM426XX_RA_ACCEL_CONFIG0);
    uint8_t actual_accel_fsr = (accel_config0 >> 5) & 0x07;
    uint8_t actual_accel_odr = accel_config0 & 0x0F;
    if (actual_accel_fsr != expected_accel_fsr || actual_accel_odr != expected_accel_odr) {
        return false;
    }

    // 3. Verify INTF_CONFIG1 (Bank 0): AFSR disable bit[6] should be set
    uint8_t intf_config1 = bus_->readReg(ICM426XX_INTF_CONFIG1);
    if ((intf_config1 & ICM426XX_INTF_CONFIG1_AFSR_DISABLE) != ICM426XX_INTF_CONFIG1_AFSR_DISABLE) {
        return false;
    }

    // 4. Verify Gyro AAF enable (Bank 1, reg 0x0B, bit 0)
    self->setUserBank(ICM426XX_BANK_SELECT1);
    uint8_t gyro_aaf_enable = bus_->readReg(0x0B);
    if ((gyro_aaf_enable & 0x01) != 0x01) {
        return false;
    }

    // 5. Verify Accel AAF enable (Bank 2, reg 0x03, bit 0)
    self->setUserBank(ICM426XX_BANK_SELECT2);
    uint8_t accel_aaf_enable = bus_->readReg(0x03);
    if ((accel_aaf_enable & 0x01) != 0x01) {
        return false;
    }

    // Restore to Bank 0
    self->setUserBank(ICM426XX_BANK_SELECT0);

    return true;
}

// =============================================================================
// Protected Low-Level Configuration Methods
// =============================================================================

/**
 * @brief Set gyro output data rate (ODR)
 * @param odr_hz ODR in Hz (1000, 4000, or 8000)
 *
 * ODR encoding per imu_hal.md line 62:
 *   8kHz = 0x03 (NOT 0x04!)
 *   4kHz = 0x05 (NOT 0x04!)
 *   2kHz = 0x06
 *   1kHz = 0x07 (NOT 0x06!)
 */
void ICM42688_BF::setGyroODR(uint16_t odr_hz) {
    uint8_t odr_code;
    switch(odr_hz) {
        case 8000: odr_code = 0x03; break;  // 8kHz
        case 4000: odr_code = 0x05; break;  // 4kHz (NOT 0x04!)
        case 2000: odr_code = 0x06; break;  // 2kHz
        case 1000: odr_code = 0x07; break;  // 1kHz (NOT 0x06!)
        default: return;  // Invalid ODR
    }

    // Read-modify-write GYRO_CONFIG0 (preserve FSR bits[5:3])
    setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_GYRO_CONFIG0);
    reg_val = (reg_val & 0xF0) | (odr_code & 0x0F);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG0, reg_val);
}

/**
 * @brief Set accel output data rate (ODR)
 * @param odr_hz ODR in Hz (1000, 4000, or 8000)
 */
void ICM42688_BF::setAccelODR(uint16_t odr_hz) {
    uint8_t odr_code;
    switch(odr_hz) {
        case 8000: odr_code = 0x03; break;  // 8kHz
        case 4000: odr_code = 0x05; break;  // 4kHz
        case 2000: odr_code = 0x06; break;  // 2kHz
        case 1000: odr_code = 0x07; break;  // 1kHz
        default: return;  // Invalid ODR
    }

    // Read-modify-write ACCEL_CONFIG0 (preserve FSR bits[5:3])
    setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_ACCEL_CONFIG0);
    reg_val = (reg_val & 0xF0) | (odr_code & 0x0F);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG0, reg_val);
}

/**
 * @brief Set gyro full-scale range (FSR)
 * @param fsr_dps FSR in dps (250, 500, 1000, or 2000)
 */
void ICM42688_BF::setGyroFSR(uint16_t fsr_dps) {
    uint8_t fsr_code;
    float scale;
    switch(fsr_dps) {
        case 2000: fsr_code = 0; scale = 2000.0f / 32768.0f; break;  // ±2000 dps
        case 1000: fsr_code = 1; scale = 1000.0f / 32768.0f; break;  // ±1000 dps
        case 500:  fsr_code = 2; scale = 500.0f / 32768.0f; break;   // ±500 dps
        case 250:  fsr_code = 3; scale = 250.0f / 32768.0f; break;   // ±250 dps
        default: return;  // Invalid FSR
    }

    // Read-modify-write GYRO_CONFIG0 (preserve ODR bits[3:0])
    setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_GYRO_CONFIG0);
    reg_val = (reg_val & 0x0F) | (fsr_code << 5);
    bus_->writeReg(ICM426XX_RA_GYRO_CONFIG0, reg_val);

    // Update scale factor for read() method
    gyrScale_ = scale;
}

/**
 * @brief Set accel full-scale range (FSR)
 * @param fsr_g FSR in g (2, 4, 8, or 16)
 */
void ICM42688_BF::setAccelFSR(uint16_t fsr_g) {
    uint8_t fsr_code;
    float scale;
    switch(fsr_g) {
        case 16: fsr_code = 0; scale = 16.0f / 32768.0f; break;   // ±16g
        case 8:  fsr_code = 1; scale = 8.0f / 32768.0f; break;    // ±8g
        case 4:  fsr_code = 2; scale = 4.0f / 32768.0f; break;    // ±4g
        case 2:  fsr_code = 3; scale = 2.0f / 32768.0f; break;    // ±2g
        default: return;  // Invalid FSR
    }

    // Read-modify-write ACCEL_CONFIG0 (preserve ODR bits[3:0])
    setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t reg_val = bus_->readReg(ICM426XX_RA_ACCEL_CONFIG0);
    reg_val = (reg_val & 0x0F) | (fsr_code << 5);
    bus_->writeReg(ICM426XX_RA_ACCEL_CONFIG0, reg_val);

    // Update scale factor for read() method
    accScale_ = scale;
}

/**
 * @brief Configure gyro anti-alias filter (AAF)
 * @param config AAF configuration (delt, deltsqr, bitshift)
 *
 * Register locations per imu_hal.md lines 64-65 (Bank 1):
 *   0x0B: enable
 *   0x0C: DELT
 *   0x0D: DELTSQR[7:0]
 *   0x0E: BITSHIFT[7:4] + DELTSQR[11:8]
 */
void ICM42688_BF::setGyroAAF(const AAFConfig& config) {
    setUserBank(ICM426XX_BANK_SELECT1);  // Switch to Bank 1
    bus_->writeReg(0x0B, 0x01);  // Enable AAF
    bus_->writeReg(0x0C, config.delt);
    bus_->writeReg(0x0D, config.deltsqr & 0xFF);  // DELTSQR[7:0]
    bus_->writeReg(0x0E, (config.bitshift << 4) | ((config.deltsqr >> 8) & 0x0F));  // BITSHIFT + DELTSQR[11:8]
    setUserBank(ICM426XX_BANK_SELECT0);  // Return to Bank 0
}

/**
 * @brief Configure accel anti-alias filter (AAF)
 * @param config AAF configuration (delt, deltsqr, bitshift)
 *
 * Register locations per imu_hal.md lines 66-67 (Bank 2):
 *   0x03: DELT bits[6:1] + enable bit[0]
 *   0x04: DELTSQR[7:0]
 *   0x05: BITSHIFT[7:4] + DELTSQR[11:8]
 */
void ICM42688_BF::setAccelAAF(const AAFConfig& config) {
    setUserBank(ICM426XX_BANK_SELECT2);  // Switch to Bank 2
    bus_->writeReg(0x03, (config.delt << 1) | 0x01);  // DELT[6:1] + enable bit[0]
    bus_->writeReg(0x04, config.deltsqr & 0xFF);  // DELTSQR[7:0]
    bus_->writeReg(0x05, (config.bitshift << 4) | ((config.deltsqr >> 8) & 0x0F));  // BITSHIFT + DELTSQR[11:8]
    setUserBank(ICM426XX_BANK_SELECT0);  // Return to Bank 0
}

/**
 * @brief Configure UI (User Interface) filters
 * @param gyro_bw Gyro UI filter bandwidth code (0-15, 15=wide/low-latency)
 * @param accel_bw Accel UI filter bandwidth code (0-15)
 * @param gyro_order Gyro filter order (1=1st-order, 2=2nd-order)
 * @param accel_order Accel filter order (1=1st-order, 2=2nd-order)
 *
 * Register locations per imu_hal.md lines 68-71 (Bank 0):
 *   0x52: GYRO_ACCEL_CONFIG0 - gyro BW[3:0], accel BW[7:4]
 *   0x51: GYRO_CONFIG1 - gyro order bits[3:2] (3=1st, 2=2nd)
 *   0x53: ACCEL_CONFIG1 - accel order bits[4:3] (3=1st, 2=2nd)
 */
void ICM42688_BF::setUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order) {
    setUserBank(ICM426XX_BANK_SELECT0);

    // GYRO_ACCEL_CONFIG0 (0x52): gyro BW[3:0], accel BW[7:4]
    bus_->writeReg(0x52, (accel_bw << 4) | gyro_bw);

    // GYRO_CONFIG1 (0x51): gyro order bits[3:2] (3=1st-order, 2=2nd-order)
    uint8_t gyro_order_code = (gyro_order == 1) ? 3 : 2;
    uint8_t reg_val = bus_->readReg(0x51);
    reg_val = (reg_val & ~0x0C) | (gyro_order_code << 2);
    bus_->writeReg(0x51, reg_val);

    // ACCEL_CONFIG1 (0x53): accel order bits[4:3] (3=1st-order, 2=2nd-order)
    uint8_t accel_order_code = (accel_order == 1) ? 3 : 2;
    reg_val = bus_->readReg(0x53);
    reg_val = (reg_val & ~0x18) | (accel_order_code << 3);
    bus_->writeReg(0x53, reg_val);
}

/**
 * @brief Disable Auto-FSR (AFSR) workaround
 *
 * Fix for stalls in gyro output per ArduPilot PR #25332.
 * AFSR causes sticky samples during internal range switching.
 */
void ICM42688_BF::disableAFSR() {
    setUserBank(ICM426XX_BANK_SELECT0);
    uint8_t intfConfig1 = bus_->readReg(ICM426XX_INTF_CONFIG1);
    intfConfig1 &= ~ICM426XX_INTF_CONFIG1_AFSR_MASK;
    intfConfig1 |= ICM426XX_INTF_CONFIG1_AFSR_DISABLE;
    bus_->writeReg(ICM426XX_INTF_CONFIG1, intfConfig1);
}
