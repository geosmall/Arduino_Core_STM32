#include "icm42688p.h"
// #include "icm42688p_regs.h"
#include "sys/system.h"

// using namespace ICM42688reg;

namespace daisy
{

// Accesible from all user banks
static constexpr uint8_t REG_BANK_SEL = 0x76;

// User Bank 0
static constexpr uint8_t UB0_REG_DEVICE_CONFIG = 0x11;
static constexpr uint8_t UB0_REG_INT_CONFIG = 0x14;
    #define INT1_MODE_PULSED (0 << 2)
    #define INT1_MODE_LATCHED (1 << 2)
    #define INT1_DRIVE_CIRCUIT_OD (0 << 1)
    #define INT1_DRIVE_CIRCUIT_PP (1 << 1)
    #define INT1_POLARITY_ACTIVE_LOW (0 << 0)
    #define INT1_POLARITY_ACTIVE_HIGH (1 << 0)
static constexpr uint8_t UB0_REG_TEMP_DATA1 = 0x1D;
static constexpr uint8_t UB0_REG_ACCEL_DATA_X1 = 0x1F;
static constexpr uint8_t UB0_REG_INTF_CONFIG1 = 0x4D;
    #define INTF_CONFIG1_AFSR_MASK 0xC0
    #define INTF_CONFIG1_AFSR_DISABLE 0x40
static constexpr uint8_t UB0_REG_PWR_MGMT0 = 0x4E;
    #define PWR_MGMT0_ACCEL_MODE_LN (3 << 0)
    #define PWR_MGMT0_GYRO_MODE_LN (3 << 2)
    #define PWR_MGMT0_GYRO_ACCEL_MODE_OFF ((0 << 0) | (0 << 2))
    #define PWR_MGMT0_TEMP_DISABLE_OFF (0 << 5)
    #define PWR_MGMT0_TEMP_DISABLE_ON (1 << 5)
static constexpr uint8_t UB0_REG_GYRO_CONFIG0 = 0x4F;
static constexpr uint8_t UB0_REG_ACCEL_CONFIG0 = 0x50;
static constexpr uint8_t UB0_REG_GYRO_ACCEL_CONFIG0 = 0x52;
    #define ACCEL_UI_FILT_BW_LOW_LATENCY (15 << 4) 
    #define GYRO_UI_FILT_BW_LOW_LATENCY (15 << 0)
static constexpr uint8_t UB0_REG_INT_CONFIG0 = 0x63;
    #define UI_DRDY_INT_CLEAR_MASK             ((1 << 5) | (1 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR           ((0 << 5) | (0 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR_DUPLICATE ((0 << 5) | (1 << 4)) // duplicate settings in datasheet, Rev 1.8.
    #define UI_DRDY_INT_CLEAR_ON_F1BR          ((1 << 5) | (0 << 4))
    #define UI_DRDY_INT_CLEAR_ON_SBR_AND_F1BR  ((1 << 5) | (1 << 4))
static constexpr uint8_t UB0_REG_INT_CONFIG1 = 0x64;
    #define INT_CONFIG1_MASK          ((1 << 6) | (1 << 5) | (1 << 4))
    #define INT_ASYNC_RESET_BIT       4
    #define INT_TDEASSERT_DISABLE_BIT 5
    #define INT_TDEASSERT_ENABLED     (0 << INT_TDEASSERT_DISABLE_BIT)
    #define INT_TDEASSERT_DISABLED    (1 << INT_TDEASSERT_DISABLE_BIT)
    #define INT_TPULSE_DURATION_BIT   6
    #define INT_TPULSE_DURATION_100   (0 << INT_TPULSE_DURATION_BIT)
    #define INT_TPULSE_DURATION_8     (1 << INT_TPULSE_DURATION_BIT)
static constexpr uint8_t UB0_REG_INT_SOURCE0 = 0x65;
    #define UI_DRDY_INT1_MASK         (1 << 3)
    #define UI_DRDY_INT1_EN_DISABLED  (0 << 3)
    #define UI_DRDY_INT1_EN_ENABLED   (1 << 3)
static constexpr uint8_t UB0_REG_WHO_AM_I = 0x75;

// User Bank 1
static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC3 = 0x0C;
static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC4 = 0x0D;
static constexpr uint8_t UB1_REG_GYRO_CONFIG_STATIC5 = 0x0E;

// User Bank 2
static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC2 = 0x03;
static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC3 = 0x04;
static constexpr uint8_t UB2_REG_ACCEL_CONFIG_STATIC4 = 0x05;

ICM42688::Result ICM42688::Init(SpiHandle spi)
{
    // Save the SPI handle and its configured high speed prescaler
    spi_ = spi;

    /* Wait some time for ICM to be properly supplied */
    System::Delay(5);

    // If the NSS pin is configured as SOFT, initialize as SW NSS pin
    nss_pin_is_SOFT = (spi_.GetConfig().nss == SpiHandle::Config::NSS::SOFT);
    if (nss_pin_is_SOFT)
    {
        dsy_gpio_write(&nss_pin_, 1);
        initializeSoftNSSPin();
    }

    // Turn off ACC and GYRO so they can be configured
    // See section 12.9 in ICM-42688-P datasheet v1.8
    // The only register settings that user can modify during sensor operation are
    // GYRO_ODR, ACCEL_ODR, GYRO_FS_SEL, ACCEL_FS_SEL, GYRO_MODE, ACCEL_MODE.
    _bank = 0;
    writeRegister(REG_BANK_SEL, _bank);
    if (SetGyroAccPwrState(PwrState::POWER_OFF) != Result::OK) return Result::ERR;

    // Reset the ICM42688
    reset();

    // Check the WHO AM I byte
    if (whoAmI() != WHO_AM_I) return Result::ERR;

    // Config gyro Anti-Alias Filter (see section 5.3 "ANTI-ALIAS FILTER")
    if (setGyroAAF(aafLUT42688[AAF_CONFIG_258HZ]) != Result::OK) return Result::ERR;

    // Config accel Anti-Alias Filter for 1KHz+ ODR, similar to MPU-6000 filter
    if (setAccelAAF(aafLUT42688[AAF_CONFIG_258HZ]) != Result::OK) return Result::ERR;

    // Configure gyro and acc UI Filters
    setBank(0);
    writeRegister(UB0_REG_GYRO_ACCEL_CONFIG0, ACCEL_UI_FILT_BW_LOW_LATENCY | GYRO_UI_FILT_BW_LOW_LATENCY);

    // Configure INT1 pin
    writeRegister(UB0_REG_INT_CONFIG, INT1_MODE_PULSED | INT1_DRIVE_CIRCUIT_PP | INT1_POLARITY_ACTIVE_HIGH);
    writeRegister(UB0_REG_INT_CONFIG0, UI_DRDY_INT_CLEAR_ON_SBR);

    // Disable interrupt INT1
    writeRegisterMask(UB0_REG_INT_SOURCE0, UI_DRDY_INT1_EN_DISABLED, UI_DRDY_INT1_MASK);

    uint8_t intConfig1Value = readRegister(UB0_REG_INT_CONFIG1);
    // Datasheet: "change setting to 0 from default setting of 1, for proper INT1 and INT2 pin operation"
    intConfig1Value &= ~(1 << INT_ASYNC_RESET_BIT);
    intConfig1Value |= (INT_TPULSE_DURATION_8 | INT_TDEASSERT_DISABLED);

    writeRegister(UB0_REG_INT_CONFIG1, intConfig1Value);

    // Fix for stalls in gyro output. See GitHub
    // https://github.com/ArduPilot/ardupilot/pull/25332
    uint8_t intfConfig1Value = readRegister(UB0_REG_INTF_CONFIG1);
    intfConfig1Value &= ~INTF_CONFIG1_AFSR_MASK;
    intfConfig1Value |= INTF_CONFIG1_AFSR_DISABLE;
    writeRegister(UB0_REG_INTF_CONFIG1, intfConfig1Value);

    // Turn on accel and gyro in Low Noise (LN) Mode
    if (SetGyroAccPwrState(PwrState::POWER_ON) != Result::OK) return Result::ERR;

    // 16G is default -- do this to set up accel resolution scaling
    if (setAccelFS(gpm16) != Result::OK) return Result::ERR;

    // 2000DPS is default -- do this to set up gyro resolution scaling
    if (setGyroFS(dps2000) != Result::OK) return Result::ERR;

    // Similar to MPU-6000, 1KHz max accel output
    if (setAccelODR(odr1k) != Result::OK) return Result::ERR;

    // Similar to MPU-6000, 8KHz max gyro output
    if (setGyroODR(odr8k) != Result::OK) return Result::ERR;

    // Success
    return Result::OK;
}

ICM42688::Result ICM42688::setAccelFS(AccelFS accelFullScale)
{
    setBank(0);

    // Read current register value
    uint8_t reg = readRegister(UB0_REG_ACCEL_CONFIG0);

    // Only change FS_SEL in reg
    reg = (accelFullScale << 5) | (reg & 0x1F);

    if (writeRegister(UB0_REG_ACCEL_CONFIG0, reg) != Result::OK) return Result::ERR;

    // Calculate normalized accel scale factor and save it
    _accelScale = static_cast<float>(1 << (4 - accelFullScale)) / NORMALIZE_SENSOR_VAL;
    _accelFS = accelFullScale;

    return Result::OK;
}

ICM42688::Result ICM42688::setGyroFS(GyroFS gyroFullScale)
{
    setBank(0);

    // Read current register value
    uint8_t reg = readRegister(UB0_REG_GYRO_CONFIG0);

    // Only change FS_SEL in reg
    reg = (gyroFullScale << 5) | (reg & 0x1F);

    // Write updated register value, return if error
    if (writeRegister(UB0_REG_GYRO_CONFIG0, reg) != Result::OK) return Result::ERR;

    // Calculate normalized gyro scale factor and save it
    _gyroScale = (2000.0f / static_cast<float>(1 << gyroFullScale)) / NORMALIZE_SENSOR_VAL;
    _gyroFS = gyroFullScale;

    return Result::OK;
}

ICM42688::Result ICM42688::setAccelODR(ODR odr)
{
    setBank(0);

    // Get the current register value
    uint8_t reg = readRegister(UB0_REG_ACCEL_CONFIG0);

    // Only change ODR in reg
    reg = odr | (reg & 0xF0);

    // Write updated register value and return result
    return writeRegister(UB0_REG_ACCEL_CONFIG0, reg);
}

ICM42688::Result ICM42688::setGyroODR(ODR odr)
{
    setBank(0);

    // Get the current register value
    uint8_t reg = readRegister(UB0_REG_GYRO_CONFIG0);

    // Only change ODR in reg
    reg = odr | (reg & 0xF0);

    // Write updated register value and return result
    return writeRegister(UB0_REG_GYRO_CONFIG0, reg);
}

ICM42688::Result ICM42688::setGyroAAF(ICM42688::AAFConfig gyroAFF)
{
    setBank(1);

    if (writeRegister(UB1_REG_GYRO_CONFIG_STATIC3, gyroAFF.delt) != Result::OK) return Result::ERR;
    if (writeRegister(UB1_REG_GYRO_CONFIG_STATIC4, gyroAFF.deltSqr & 0xFF) != Result::OK) return Result::ERR;
    return writeRegister(UB1_REG_GYRO_CONFIG_STATIC5, (gyroAFF.deltSqr >> 8) | (gyroAFF.bitshift << 4));
}

ICM42688::Result ICM42688::setAccelAAF(AAFConfig accAAF)
{
    setBank(2);

    if (writeRegister(UB2_REG_ACCEL_CONFIG_STATIC2, accAAF.delt << 1) != Result::OK) return Result::ERR;
    if (writeRegister(UB2_REG_ACCEL_CONFIG_STATIC3, accAAF.deltSqr & 0xFF) != Result::OK) return Result::ERR;
    return writeRegister(UB2_REG_ACCEL_CONFIG_STATIC4, (accAAF.deltSqr >> 8) | (accAAF.bitshift << 4));
}

ICM42688::Result ICM42688::enableDataReadyInterrupt()
{
    setBank(0);

    // User should change setting to 0 from default setting of 1, for proper INT pin operation
    if (writeRegisterMask(UB0_REG_INT_CONFIG1, 0, INT_ASYNC_RESET_BIT) != Result::OK)
    {
        return Result::ERR;
    }

    // Route UI data ready interrupt to INT1
    return writeRegisterMask(UB0_REG_INT_SOURCE0, UI_DRDY_INT1_EN_ENABLED, UI_DRDY_INT1_MASK);
}

ICM42688::Result ICM42688::disableDataReadyInterrupt()
{
    setBank(0);

    // Restore to default setting of 1
    if (writeRegisterMask(UB0_REG_INT_CONFIG1, INT_ASYNC_RESET_BIT, INT_ASYNC_RESET_BIT) != Result::OK)
    {
        return Result::ERR;
    }

    // Disable interrupt INT1
    return writeRegisterMask(UB0_REG_INT_SOURCE0, UI_DRDY_INT1_EN_DISABLED, UI_DRDY_INT1_MASK);
}

ICM42688::Result ICM42688::getIMU(ImuData& data)
{
    return readRegisters(UB0_REG_ACCEL_DATA_X1, sizeof(ImuData), reinterpret_cast<uint8_t*>(&data));
}

ICM42688::Result
ICM42688::getIMU6(int16_t* AcX, int16_t* AcY, int16_t* AcZ, int16_t* GyX, int16_t* GyY, int16_t* GyZ)
{
    // Validate pointers before using them
    if (!AcX || !AcY || !AcZ || !GyX || !GyY || !GyZ) return Result::ERR;

    // Read the data from the ICM42688
    if (readRegisters(UB0_REG_ACCEL_DATA_X1, 12, rxBuffer_) != Result::OK) return Result::ERR;

    // Combine bytes into signed 16 bit values
    *AcX = ((int16_t) rxBuffer_[0] << 8) | rxBuffer_[1];
    *AcY = ((int16_t) rxBuffer_[2] << 8) | rxBuffer_[3];
    *AcZ = ((int16_t) rxBuffer_[4] << 8) | rxBuffer_[5];
    *GyX = ((int16_t) rxBuffer_[6] << 8) | rxBuffer_[7];
    *GyY = ((int16_t) rxBuffer_[8] << 8) | rxBuffer_[9];
    *GyZ = ((int16_t) rxBuffer_[10] << 8) | rxBuffer_[11];

    return Result::OK;
}

ICM42688::Result ICM42688::getAGT()
{
    // Read the data from the ICM42688
    if (readRegisters(UB0_REG_TEMP_DATA1, 14, rxBuffer_) != Result::OK) return Result::ERR;

    // Combine bytes into 16 bit values
    int16_t rawMeas[7]; // temp, accel xyz, gyro xyz
    for (size_t i = 0; i < 7; i++)
    {
        rawMeas[i] = ((int16_t) rxBuffer_[i * 2] << 8) | rxBuffer_[i * 2 + 1];
    }

    _t = (static_cast<float>(rawMeas[0]) / TEMP_DATA_REG_SCALE) + TEMP_OFFSET;

    _acc[0] = ((rawMeas[1] * _accelScale) - _accB[0]) * _accS[0];
    _acc[1] = ((rawMeas[2] * _accelScale) - _accB[1]) * _accS[1];
    _acc[2] = ((rawMeas[3] * _accelScale) - _accB[2]) * _accS[2];

    _gyr[0] = (rawMeas[4] * _gyroScale) - _gyrB[0];
    _gyr[1] = (rawMeas[5] * _gyroScale) - _gyrB[1];
    _gyr[2] = (rawMeas[6] * _gyroScale) - _gyrB[2];

    return Result::OK;
}

ICM42688::Result ICM42688::readRegisters(uint8_t addr, uint8_t count, uint8_t* dest)
{
    // Verify requested size fits in buffer, must be
    // ICM42688_BUFFER_SIZE -1 to cover address byte
    if (count > (ICM42688_BUFFER_SIZE-1)) return Result::ERR;

    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 0);
    txBuffer_[0] = (addr | 0x80);
    txBuffer_[1] = 0; // in case writeRegister() has modified
    spi_.BlockingTransmitAndReceive(txBuffer_, dest, count+1);
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 1);

    return Result::OK;
}

uint8_t ICM42688::readRegister(uint8_t addr)
{
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 0);
    txBuffer_[0] = (addr | 0x80);
    txBuffer_[1] = 0; // In case writeRegister() has modified
    spi_.BlockingTransmitAndReceive(txBuffer_, rxBuffer_, 2);
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 1);

    return rxBuffer_[1];
}

ICM42688::Result ICM42688::writeRegister(uint8_t addr, uint8_t data)
{
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 0);
    txBuffer_[0] = addr;
    txBuffer_[1] = data;
    spi_.BlockingTransmitAndReceive(txBuffer_, rxBuffer_, 2);
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 1);

    System::Delay(10);

    /* Read back the register, addr in rxBuffer_[0], data in rxBuffer_[1] */
    readRegisters(addr, 1, rxBuffer_);
    /* Check the read back register against the written register */
    return (rxBuffer_[1] == data) ? Result::OK : Result::ERR;
}

ICM42688::Result ICM42688::writeRegisterMask(uint8_t addr, uint8_t data, uint8_t mask)
{
    // Read current register value and check for error
    if (readRegisters(addr, 1, rxBuffer_) != Result::OK) return Result::ERR;

    // Addr in rxBuffer_[0], data in rxBuffer_[1]
    uint8_t reg = rxBuffer_[1];

    // Apply mask to data and write to register
    return writeRegister(addr, (reg & ~mask) | (data & mask));
}

ICM42688::Result ICM42688::setBank(uint8_t bank)
{
    // If we are already on this bank, return immediately.
    if (_bank == bank)
    {
        return Result::ERR;
    }

    _bank = bank;
    return writeRegister(REG_BANK_SEL, bank);
}

void ICM42688::reset()
{
    setBank(0);

    writeRegister(UB0_REG_DEVICE_CONFIG, 0x01);

    // Wait for ICM42688 to come back up
    System::Delay(1);
}

int16_t ICM42688::whoAmI()
{
    setBank(0);

    // Read the WHO AM I register
    if (readRegisters(UB0_REG_WHO_AM_I, 1, rxBuffer_) != Result::OK)
    {
        return -1;
    }
    // Return the WHO_AM_I register value.
    // addr in rxBuffer_[0], data in rxBuffer_[1]
    return (rxBuffer_[1] & 0xFF);
}

ICM42688::Result ICM42688::SetGyroAccPwrState(PwrState state)
{
    setBank(0);

    Result res = Result::ERR;
    if (state == PwrState::POWER_OFF)
    {
        res = writeRegister(UB0_REG_PWR_MGMT0, PWR_MGMT0_GYRO_ACCEL_MODE_OFF);
    }
    else
    {
        res = writeRegister(UB0_REG_PWR_MGMT0,
                            (PWR_MGMT0_TEMP_DISABLE_OFF | PWR_MGMT0_ACCEL_MODE_LN | PWR_MGMT0_GYRO_MODE_LN));
    }
    if (res != Result::OK) return res;

    /* Per DK-42688-P_SmartMotion_eMD, powering the gyroscope on immediately after powering it off
     * can cause device failure. The gyroscope proof mass can continue vibrating after it has been powered
     * off, and powering it back on immediately can result in unpredictable proof mass movement. After
     * powering the gyroscope off, a period of > 150ms should be allowed to elapse before it is powered back
     * on. */
    System::Delay(150);

    // Success
    return Result::OK;
}

void ICM42688::initializeSoftNSSPin()
{
    nss_pin_.pin = spi_.GetConfig().pin_config.nss;
    nss_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    nss_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&nss_pin_);
}

} // namespace daisy
