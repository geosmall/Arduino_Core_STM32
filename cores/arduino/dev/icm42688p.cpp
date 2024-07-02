#include "icm42688p.h"
#include "icm42688p_regs.h"
#include "sys/system.h"

using namespace ICM42688reg;

namespace daisy
{

ICM42688::Result ICM42688::Init(SpiHandle spi)
{
    // save the SPI handle
    spi_ = spi;

    // if the NSS pin is configured as SOFT, initialize as SW NSS pin
    nss_pin_is_SOFT = (spi_.GetConfig().nss == SpiHandle::Config::NSS::SOFT);
    if (nss_pin_is_SOFT)
    {
        initializeSoftNSSPin();
        dsy_gpio_write(&nss_pin_, 1);
    }

    // reset the ICM42688
    reset();

    // check the WHO AM I byte
    if (whoAmI() != WHO_AM_I)
    {
        return Result::ERR;
    }

    // turn on accel and gyro in Low Noise (LN) Mode
    if (writeRegister(UB0_REG_PWR_MGMT0, 0x0F) != Result::OK)
    {
        return Result::ERR;
    }

    // 16G is default -- do this to set up accel resolution scaling
    if (setAccelFS(gpm16) != Result::OK)
    {
        return Result::ERR;
    }

    // 2000DPS is default -- do this to set up gyro resolution scaling
    if (setGyroFS(dps2000) != Result::OK)
    {
        return Result::ERR;
    }

    // Success
    return Result::OK;
}

/* sets the accelerometer full scale range to values other than default */
ICM42688::Result ICM42688::setAccelFS(AccelFS accelFullScale)
{
    uint8_t reg;

    // use low speed SPI for register setting
    _useSPIHS = false;

    setBank(0);

    // read current register value
    if (readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg) != Result::OK)
    {
        return Result::ERR;
    }

    // only change FS_SEL in reg
    reg = (accelFullScale << 5) | (reg & 0x1F);

    if (writeRegister(UB0_REG_ACCEL_CONFIG0, reg) != Result::OK)
    {
        return Result::ERR;
    }

    _accelScale = static_cast<float>(1 << (4 - accelFullScale)) / NORMALIZE_SENSOR_VAL;
    _accelFS = accelFullScale;

    return Result::OK;
}

/* sets the gyro full scale range to values other than default */
ICM42688::Result ICM42688::setGyroFS(GyroFS gyroFullScale)
{
    uint8_t reg;

    // use low speed SPI for register setting
    _useSPIHS = false;

    setBank(0);

    // read current register value
    if (readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg) != Result::OK)
    {
        return Result::ERR;
    }

    // only change FS_SEL in reg
    reg = (gyroFullScale << 5) | (reg & 0x1F);

    if (writeRegister(UB0_REG_GYRO_CONFIG0, reg) != Result::OK)
    {
        return Result::ERR;
    }

    _gyroScale = (2000.0f / static_cast<float>(1 << gyroFullScale)) / NORMALIZE_SENSOR_VAL;
    _gyroFS = gyroFullScale;

    return Result::OK;
}

ICM42688::Result ICM42688::readRegisters(uint8_t addr, uint8_t count, uint8_t* dest)
{
    uint8_t buf = (addr | 0x80);

    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 0);
    spi_.BlockingTransmit(&buf, 1); // specify the starting register address
    // uint8_t dummy = 0;
    // for(uint8_t i = 0; i < count; i++) {
    //   SPI_BlockingTransmitAndReceiveByte(ICM42688_SPI_Handle, &dummy,
    //   &dest[i]); // read the data
    // }
    spi_.BlockingReceive(dest, count);
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 1);

    return Result::OK;
}

ICM42688::Result ICM42688::writeRegister(uint8_t addr, uint8_t data)
{
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 0);
    spi_.BlockingTransmit(&addr, 1);
    spi_.BlockingTransmit(&data, 1);
    if (nss_pin_is_SOFT) dsy_gpio_write(&nss_pin_, 1);

    System::Delay(10);

    /* read back the register */
    readRegisters(addr, 1, _buffer);
    /* check the read back register against the written register */
    if (_buffer[0] == data)
    {
        return Result::OK;
    }
    else
    {
        return Result::ERR;
    }

    return Result::OK;
}

ICM42688::Result ICM42688::setBank(uint8_t bank)
{
    // if we are already on this bank, bail
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

    // wait for ICM42688 to come back up
    System::Delay(1);
}

/* gets the ICM42688 WHO_AM_I register value */
int16_t ICM42688::whoAmI()
{
    setBank(0);

    // read the WHO AM I register
    if (readRegisters(UB0_REG_WHO_AM_I, 1, _buffer) != Result::OK)
    {
        return -1;
    }
    // return the register value
    return (_buffer[0] & 0xFF);
}

void ICM42688::initializeSoftNSSPin()
{
    nss_pin_.pin = spi_.GetConfig().pin_config.nss;
    nss_pin_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    nss_pin_.pull = DSY_GPIO_NOPULL;
    dsy_gpio_init(&nss_pin_);
}

} // namespace daisy
