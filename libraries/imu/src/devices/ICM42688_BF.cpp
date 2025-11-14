/*
 * Modified from Betaflight for Arduino integration
 * Original: https://github.com/betaflight/betaflight/blob/master/src/main/drivers/accgyro/accgyro_spi_icm426xx.c
 * Pattern: Madflight ICM426XX wrapper (https://github.com/qqqlab/madflight)
 *
 * Modifications applied (systematic find-replace):
 * 1. "spiWriteReg(dev, " → "bus_->writeReg("
 * 2. "spiReadRegMsk(dev, " → "bus_->readReg("
 * 3. "setUserBank(dev, " → "setUserBank("
 * 4. "extDevice_t" → "DeviceBus"
 * 5. C++ class with factory pattern
 * 6. Constructor-based initialization (no separate begin())
 * 7. Removed: External clock support, soft reset, Betaflight framework dependencies
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

// Helper function: Select register bank
void ICM42688_BF::setUserBank(uint8_t bank) {
    bus_->writeReg(ICM426XX_RA_REG_BANK_SEL, bank & 7);
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
    // Read 12 bytes: ax,ay,az,gx,gy,gz (little endian)
    bus_->readRegs(ICM426XX_RA_ACCEL_DATA_X1, (uint8_t*)accgyr, 12);
}
