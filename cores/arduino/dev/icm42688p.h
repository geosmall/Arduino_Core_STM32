#pragma once

#include "icm42688p_regs.h"

using namespace ICM42688reg;

#if 0 // gls

// Misc configuration macros
#define I2C_MASTER_RESETS_BEFORE_FAIL \
    5 ///< The number of times to try resetting a stuck I2C master before giving up
#define NUM_FINISHED_CHECKS \
    100 ///< How many times to poll I2C_SLV4_DONE before giving up and resetting

// Bank 0
#define ICM20X_B0_WHOAMI 0x00         ///< Chip ID register
#define ICM20X_B0_USER_CTRL 0x03      ///< User Control Reg. Includes I2C Master
#define ICM20X_B0_LP_CONFIG 0x05      ///< Low Power config
#define ICM20X_B0_REG_INT_PIN_CFG 0xF ///< Interrupt config register
#define ICM20X_B0_REG_INT_ENABLE 0x10 ///< Interrupt enable register 0
#define ICM20X_B0_REG_INT_ENABLE_1 0x11 ///< Interrupt enable register 1
#define ICM20X_B0_I2C_MST_STATUS \
    0x17 ///< Records if I2C master bus data is finished
#define ICM20X_B0_REG_BANK_SEL 0x7F ///< register bank selection register
#define ICM20X_B0_PWR_MGMT_1 0x06   ///< primary power management register
#define ICM20X_B0_ACCEL_XOUT_H 0x2D ///< first byte of accel data
#define ICM20X_B0_GYRO_XOUT_H 0x33  ///< first byte of accel data

// Bank 2
#define ICM20X_B2_GYRO_SMPLRT_DIV 0x00    ///< Gyroscope data rate divisor
#define ICM20X_B2_GYRO_CONFIG_1 0x01      ///< Gyro config for range setting
#define ICM20X_B2_ACCEL_SMPLRT_DIV_1 0x10 ///< Accel data rate divisor MSByte
#define ICM20X_B2_ACCEL_SMPLRT_DIV_2 0x11 ///< Accel data rate divisor LSByte
#define ICM20X_B2_ACCEL_CONFIG_1 0x14     ///< Accel config for setting range

// Bank 3
#define ICM20X_B3_I2C_MST_ODR_CONFIG 0x0 ///< Sets ODR for I2C master bus
#define ICM20X_B3_I2C_MST_CTRL 0x1       ///< I2C master bus config
#define ICM20X_B3_I2C_MST_DELAY_CTRL 0x2 ///< I2C master bus config
#define ICM20X_B3_I2C_SLV0_ADDR \
    0x3 ///< Sets I2C address for I2C master bus slave 0
#define ICM20X_B3_I2C_SLV0_REG \
    0x4 ///< Sets register address for I2C master bus slave 0
#define ICM20X_B3_I2C_SLV0_CTRL 0x5 ///< Controls for I2C master bus slave 0
#define ICM20X_B3_I2C_SLV0_DO 0x6   ///< Sets I2C master bus slave 0 data out

#define ICM20X_B3_I2C_SLV4_ADDR \
    0x13 ///< Sets I2C address for I2C master bus slave 4
#define ICM20X_B3_I2C_SLV4_REG \
    0x14 ///< Sets register address for I2C master bus slave 4
#define ICM20X_B3_I2C_SLV4_CTRL 0x15 ///< Controls for I2C master bus slave 4
#define ICM20X_B3_I2C_SLV4_DO 0x16   ///< Sets I2C master bus slave 4 data out
#define ICM20X_B3_I2C_SLV4_DI 0x17   ///< Sets I2C master bus slave 4 data in

#define ICM42688p_CHIP_ID 0xEA ///< ICM42688p default device id from WHOAMI

#define ICM42688p_I2CADDR_DEFAULT 0x69 ///< ICM42688p default i2c address
#define ICM42688p_MAG_ID 0x09          ///< The chip ID for the magnetometer

#define ICM42688p_UT_PER_LSB 0.15 ///< mag data LSB value (fixed)

#define AK09916_WIA2 0x01  ///< Magnetometer
#define AK09916_ST1 0x10   ///< Magnetometer
#define AK09916_HXL 0x11   ///< Magnetometer
#define AK09916_HXH 0x12   ///< Magnetometer
#define AK09916_HYL 0x13   ///< Magnetometer
#define AK09916_HYH 0x14   ///< Magnetometer
#define AK09916_HZL 0x15   ///< Magnetometer
#define AK09916_HZH 0x16   ///< Magnetometer
#define AK09916_ST2 0x18   ///< Magnetometer
#define AK09916_CNTL2 0x31 ///< Magnetometer
#define AK09916_CNTL3 0x32 ///< Magnetometer

#define SENSORS_GRAVITY_EARTH (9.80665F)
#define SENSORS_DPS_TO_RADS (0.017453293F)

#endif // gls

namespace daisy
{
/** @addtogroup external 
    @{ 
*/

/** SPI Transport for Icm42688p */
class Icm42688pSpiTransport
{
  public:
    Icm42688pSpiTransport() {}
    ~Icm42688pSpiTransport() {}

    struct Config
    {
        SpiHandle::Config::Peripheral periph;
        Pin                           sclk;
        Pin                           miso;
        Pin                           mosi;
        Pin                           nss;

        Config()
        {
            periph = SpiHandle::Config::Peripheral::SPI_1;
            sclk   = Pin(PORTA, 5);
            miso   = Pin(PORTA, 6);
            mosi   = Pin(PORTA, 7);
            nss    = Pin(PORTA, 4);
        }
    };

    inline void Init(Config config, bool use_hs_SPI)
    {
        SpiHandle::Config spi_conf;
        spi_conf.mode           = SpiHandle::Config::Mode::MASTER;
        spi_conf.direction      = SpiHandle::Config::Direction::TWO_LINES;
        spi_conf.clock_polarity = SpiHandle::Config::ClockPolarity::HIGH;
        spi_conf.clock_phase    = SpiHandle::Config::ClockPhase::TWO_EDGE;
        spi_conf.nss            = SpiHandle::Config::NSS::HARD_OUTPUT;
        if (use_hs_SPI) {
            spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_4;
        } else {
            spi_conf.baud_prescaler = SpiHandle::Config::BaudPrescaler::PS_32;
        }
        spi_conf.periph          = config.periph;
        spi_conf.pin_config.sclk = config.sclk;
        spi_conf.pin_config.miso = config.miso;
        spi_conf.pin_config.mosi = config.mosi;
        spi_conf.pin_config.nss  = config.nss;

        spi_.Init(spi_conf);
    }

    void Write(uint8_t *data, uint16_t size)
    {
        error_ |= SpiHandle::Result::OK != spi_.BlockingTransmit(data, size);
    }

    void Read(uint8_t *data, uint16_t size)
    {
        error_ |= SpiHandle::Result::OK != spi_.BlockingReceive(data, size, 10);
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        uint8_t buffer[2];

        buffer[0] = reg & ~0x80;
        buffer[1] = value;

        Write(buffer, 2);
    }

    /**  Writes a 16 bit value MSB first
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write16(uint8_t reg, uint16_t value)
    {
        uint8_t buffer[3];

        buffer[0] = reg & ~0x80;
        buffer[1] = value >> 8;
        buffer[2] = value & 0xFF;

        Write(buffer, 3);
    }

    /** Read from a reg address a defined number of bytes */
    void ReadReg(uint8_t reg, uint8_t *buff, uint8_t size)
    {
        reg = uint8_t(reg | 0x80);
        Write(&reg, 1);
        Read(buff, size);
    }


    /**  Reads an 8 bit value
        \param reg the register address to read from
        \return the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg)
    {
        uint8_t buffer;
        ReadReg(reg, &buffer, 1);
        return buffer;
    }

    bool GetError()
    {
        bool tmp = error_;
        error_   = false;
        return tmp;
    }

  private:
    SpiHandle spi_;
    bool      error_;
};

/** \brief Device support for ICM42688p IMU sensor
    @author beserge
    @date December 2021
*/
template <typename Transport>
class Icm42688p
{
  public:
    Icm42688p() {}
    ~Icm42688p() {}

    struct Config
    {
        typename Transport::Config transport_config;

        Config() {}
    };

    struct Icm42688pVect
    {
        float x;
        float y;
        float z;
    };

#if 0 // gls

    /** The accelerometer data range */
    enum icm42688p_accel_range_t
    {
        ICM42688p_ACCEL_RANGE_2_G,
        ICM42688p_ACCEL_RANGE_4_G,
        ICM42688p_ACCEL_RANGE_8_G,
        ICM42688p_ACCEL_RANGE_16_G,
    };

    /** The gyro data range */
    enum icm42688p_gyro_range_t
    {
        ICM42688p_GYRO_RANGE_250_DPS,
        ICM42688p_GYRO_RANGE_500_DPS,
        ICM42688p_GYRO_RANGE_1000_DPS,
        ICM42688p_GYRO_RANGE_2000_DPS,
    };

    /** Data rates/modes for the embedded AsahiKASEI AK09916 3-axis magnetometer */
    enum ak09916_data_rate_t
    {
        AK09916_MAG_DATARATE_SHUTDOWN = 0x0, ///< Stops measurement updates
        AK09916_MAG_DATARATE_SINGLE
        = 0x1, ///< Takes a single measurement then switches to
               ///< AK09916_MAG_DATARATE_SHUTDOWN
        AK09916_MAG_DATARATE_10_HZ  = 0x2, ///< updates at 10Hz
        AK09916_MAG_DATARATE_20_HZ  = 0x4, ///< updates at 20Hz
        AK09916_MAG_DATARATE_50_HZ  = 0x6, ///< updates at 50Hz
        AK09916_MAG_DATARATE_100_HZ = 0x8, ///< updates at 100Hz
    };

#endif // gls

    enum GyroFS : uint8_t {
        dps2000 = 0x00,
        dps1000 = 0x01,
        dps500 = 0x02,
        dps250 = 0x03,
        dps125 = 0x04,
        dps62_5 = 0x05,
        dps31_25 = 0x06,
        dps15_625 = 0x07
    };

    enum AccelFS : uint8_t {
        gpm16 = 0x00,
        gpm8 = 0x01,
        gpm4 = 0x02,
        gpm2 = 0x03
    };

    enum ODR : uint8_t {
        odr32k = 0x01, // LN mode only
        odr16k = 0x02, // LN mode only
        odr8k = 0x03, // LN mode only
        odr4k = 0x04, // LN mode only
        odr2k = 0x05, // LN mode only
        odr1k = 0x06, // LN mode only
        odr200 = 0x07,
        odr100 = 0x08,
        odr50 = 0x09,
        odr25 = 0x0A,
        odr12_5 = 0x0B,
        odr6a25 = 0x0C, // LP mode only (accel only)
        odr3a125 = 0x0D, // LP mode only (accel only)
        odr1a5625 = 0x0E, // LP mode only (accel only)
        odr500 = 0x0F,
    };

    enum Result
    {
        OK = 0,
        ERR
    };

    /** Initialize the Icm42688p device
        \param config Configuration settings
    */
    Result Init(Config config)
    {
        config_ = config;

        // initialize transport_ for low speed (use_hs_SPI = false)
        transport_.Init(config_.transport_config, false);

        setBank(0);

        // reset the ICM42688
        reset();

        // check the WHO AM I byte
        if(whoAmI() != WHO_AM_I) {
            return ERR;
        }

        // turn on accel and gyro in Low Noise (LN) Mode
        if(writeRegister(UB0_REG_PWR_MGMT0, 0x0F) < 0) {
            return ERR;
        }

        // 16G is default -- do this to set up accel resolution scaling
        if(setAccelFS(gpm16) < 0) {
            return ERR;
        }

        // 2000DPS is default -- do this to set up gyro resolution scaling
        if(setGyroFS(dps2000) < 0) { 
            return ERR;
        }

        System::Delay(20);

        // initialize transport_ for high speed (use_hs_SPI = true)
        transport_.Init(config_.transport_config, true);

        System::Delay(20);

        return GetTransportError();
    }

#if 0 // gls

    /** Reset the internal registers and restores the default settings */
    void Reset()
    {
        SetBank(0);

        WriteBits(ICM20X_B0_PWR_MGMT_1, 1, 1, 7);
        System::Delay(20);

        while(ReadBits(ICM20X_B0_PWR_MGMT_1, 1, 7))
        {
            System::Delay(10);
        };

        System::Delay(50);
    }

    void ScaleValues()
    {
        icm42688p_gyro_range_t gyro_range
            = (icm42688p_gyro_range_t)current_gyro_range_;
        icm42688p_accel_range_t accel_range
            = (icm42688p_accel_range_t)current_accel_range_;

        float accel_scale = 1.0;
        float gyro_scale  = 1.0;

        if(gyro_range == ICM42688p_GYRO_RANGE_250_DPS)
            gyro_scale = 131.0;
        if(gyro_range == ICM42688p_GYRO_RANGE_500_DPS)
            gyro_scale = 65.5;
        if(gyro_range == ICM42688p_GYRO_RANGE_1000_DPS)
            gyro_scale = 32.8;
        if(gyro_range == ICM42688p_GYRO_RANGE_2000_DPS)
            gyro_scale = 16.4;

        if(accel_range == ICM42688p_ACCEL_RANGE_2_G)
            accel_scale = 16384.0;
        if(accel_range == ICM42688p_ACCEL_RANGE_4_G)
            accel_scale = 8192.0;
        if(accel_range == ICM42688p_ACCEL_RANGE_8_G)
            accel_scale = 4096.0;
        if(accel_range == ICM42688p_ACCEL_RANGE_16_G)
            accel_scale = 2048.0;

        gyroX = rawGyroX / gyro_scale;
        gyroY = rawGyroY / gyro_scale;
        gyroZ = rawGyroZ / gyro_scale;

        accX = rawAccX / accel_scale;
        accY = rawAccY / accel_scale;
        accZ = rawAccZ / accel_scale;

        magX = rawMagX * ICM42688p_UT_PER_LSB;
        magY = rawMagY * ICM42688p_UT_PER_LSB;
        magZ = rawMagZ * ICM42688p_UT_PER_LSB;
    }

    /** Sets the accelerometer's data rate divisor.
        \param  new_accel_divisor The accelerometer's data rate divisor (`uint16_t`). This 12-bit value must be <= 4095
    */
    void SetAccelRateDivisor(uint16_t new_accel_divisor)
    {
        SetBank(2);
        Write16(ICM20X_B2_ACCEL_SMPLRT_DIV_1, new_accel_divisor);
        SetBank(0);
    }

    /** Get the accelerometer's measurement range.
        \return The accelerometer's measurement range (`icm42688p_accel_range_t`).
    */
    icm42688p_accel_range_t GetAccelRange()
    {
        return (icm42688p_accel_range_t)ReadAccelRange();
    }

    /** Get the accelerometer's measurement range.
        \return The accelerometer's measurement range (`icm20x_accel_range_t`).
    */
    uint8_t ReadAccelRange()
    {
        SetBank(2);
        uint8_t range = ReadBits(ICM20X_B2_ACCEL_CONFIG_1, 2, 1);
        SetBank(0);
        return range;
    }

    /** Sets the accelerometer's measurement range.
        \param  new_accel_range Measurement range to be set. Must be an `icm20x_accel_range_t`.
    */
    void WriteAccelRange(uint8_t new_accel_range)
    {
        SetBank(2);
        WriteBits(ICM20X_B2_ACCEL_CONFIG_1, new_accel_range, 2, 1);
        current_accel_range_ = new_accel_range;
        SetBank(0);
    }

    /** Sets the accelerometer's measurement range.
        \param  new_accel_range Measurement range to be set. Must be an `icm42688p_accel_range_t`.
    */
    void SetAccelRange(icm42688p_accel_range_t new_accel_range)
    {
        WriteAccelRange((uint8_t)new_accel_range);
    }


    /** Sets the gyro's data rate divisor.
        @param  new_gyro_divisor The gyro's data rate divisor (`uint8_t`).
    */
    void SetGyroRateDivisor(uint8_t new_gyro_divisor)
    {
        SetBank(2);
        Write8(ICM20X_B2_GYRO_SMPLRT_DIV, new_gyro_divisor);
        SetBank(0);
    }

    /** Get the gyro's measurement range.
        \return The gyro's measurement range (`icm42688p_gyro_range_t`).
    */
    icm42688p_gyro_range_t GetGyroRange()
    {
        return (icm42688p_gyro_range_t)ReadGyroRange();
    }

    /** Sets the gyro's measurement range.
        \param  new_gyro_range Measurement range to be set. Must be an `icm42688p_gyro_range_t`.
    */
    void SetGyroRange(icm42688p_gyro_range_t new_gyro_range)
    {
        WriteGyroRange((uint8_t)new_gyro_range);
    }

    /** Sets the gyro's measurement range.
        \param  new_gyro_range Measurement range to be set. Must be an  `icm20x_gyro_range_t`.
    */
    void WriteGyroRange(uint8_t new_gyro_range)
    {
        SetBank(2);
        WriteBits(ICM20X_B2_GYRO_CONFIG_1, new_gyro_range, 2, 1);
        current_gyro_range_ = new_gyro_range;
        SetBank(0);
    }


    /** Get the gyro's measurement range.
        \return The gyro's measurement range (`icm20x_gyro_range_t`).
    */
    uint8_t ReadGyroRange()
    {
        SetBank(2);
        uint8_t range = ReadBits(ICM20X_B2_GYRO_CONFIG_1, 2, 1);
        SetBank(0);
        return range;
    }

    /** Sets register bank.
        \param bank_number The bank to set to active
    */
    void SetBank(uint8_t bank_number)
    {
        Write8(ICM20X_B0_REG_BANK_SEL, (bank_number & 0b11) << 4);
    }

    /** Updates the measurement data for all sensors simultaneously */
    void Process()
    {
        SetBank(0);

        // reading 9 bytes of mag data to fetch the register that tells the mag we've
        // read all the data
        const uint8_t numbytes
            = 14 + 9; // Read Accel, gyro, temp, and 9 bytes of mag

        uint8_t buffer[numbytes];
        transport_.ReadReg(ICM20X_B0_ACCEL_XOUT_H, buffer, numbytes);

        rawAccX = buffer[0] << 8 | buffer[1];
        rawAccY = buffer[2] << 8 | buffer[3];
        rawAccZ = buffer[4] << 8 | buffer[5];

        rawGyroX = buffer[6] << 8 | buffer[7];
        rawGyroY = buffer[8] << 8 | buffer[9];
        rawGyroZ = buffer[10] << 8 | buffer[11];

        temperature = buffer[12] << 8 | buffer[13];

        rawMagX = ((buffer[16] << 8)
                   | (buffer[15] & 0xFF)); // Mag data is read little endian
        rawMagY = ((buffer[18] << 8) | (buffer[17] & 0xFF));
        rawMagZ = ((buffer[20] << 8) | (buffer[19] & 0xFF));

        ScaleValues();
        SetBank(0);
    }

    Icm42688pVect GetAccelVect()
    {
        Icm42688pVect vect;
        vect.x = accX * SENSORS_GRAVITY_EARTH;
        vect.y = accY * SENSORS_GRAVITY_EARTH;
        vect.z = accZ * SENSORS_GRAVITY_EARTH;

        return vect;
    }

    Icm42688pVect GetGyroVect()
    {
        Icm42688pVect vect;
        vect.x = gyroX * SENSORS_DPS_TO_RADS;
        vect.y = gyroY * SENSORS_DPS_TO_RADS;
        vect.z = gyroZ * SENSORS_DPS_TO_RADS;

        return vect;
    }

    float GetTemp() { return (temperature / 333.87) + 21.0; }

#endif // gls

    /**  Reads an 8 bit value
        \param reg the register address to read from
        \return the data uint8_t read from the device
    */
    uint8_t Read8(uint8_t reg) {
        return transport_.Read8(reg);
    }

    /**  Writes an 8 bit value
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write8(uint8_t reg, uint8_t value)
    {
        return transport_.Write8(reg, value);
    }

    /**  Writes a 16 bit value MSB first
        \param reg the register address to write to
        \param value the value to write to the register
    */
    void Write16(uint8_t reg, uint16_t value)
    {
        return transport_.Write16(reg, value);
    }

    /** Read from a reg address a defined number of bytes */
    void ReadReg(uint8_t reg, uint8_t *buff, uint8_t size)
    {
        return transport_.ReadReg(reg, buff, size);
    }

    uint8_t ReadBits(uint8_t reg, uint8_t bits, uint8_t shift)
    {
        uint8_t val = Read8(reg);
        val >>= shift;
        return val & ((1 << (bits)) - 1);
    }

    void WriteBits(uint8_t reg, uint8_t data, uint8_t bits, uint8_t shift)
    {
        uint8_t val = Read8(reg);

        // mask off the data before writing
        uint8_t mask = (1 << (bits)) - 1;
        data &= mask;

        mask <<= shift;
        val &= ~mask;         // remove the current data at that spot
        val |= data << shift; // and add in the new data

        Write8(reg, val);
    }

// -------------------------------------------------------------------------

    /* Write a byte to ICM42688 register given a register address and data,
       verify the the value of the register was correctly written */
    int writeRegister(uint8_t reg, uint8_t data) {
        // Write8(reg, data);
        transport_.Write8(reg, data);

        System::Delay(10);

        /* read back the register */
        // ReadReg(reg, _buffer, 1);
        transport_.ReadReg(reg, _buffer, 1);
        /* check the read back register against the data value */
        if(_buffer[0] == data) {
            return 1;
        } else {
            return -1;
        }
    }

    /* Read registers from ICM42688P given a starting register address,
       number of bytes, and a pointer to store data */
    int readRegisters(uint8_t reg, uint8_t count, uint8_t *dest) {
        // ReadReg(reg, dest, count);
        transport_.ReadReg(reg, dest, count);
        return 1;
    }

    /* sets the accelerometer full scale range to values other than default */
    int setAccelFS(AccelFS fssel) {
        uint8_t reg;

        setBank(0);

        // read current register value
        if (readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg) < 0) return -1;

        // only change FS_SEL in reg
        reg = (fssel << 5) | (reg & 0x1F);

        if (writeRegister(UB0_REG_ACCEL_CONFIG0, reg) < 0) return -2;

        _accelScale = static_cast<float>(1 << (4 - fssel)) / 32768.0f;
        _accelFS = fssel;

        return 1;
    }

    /* sets the gyro full scale range to values other than default */
    int setGyroFS(GyroFS fssel) {
        uint8_t reg;

        setBank(0);

        // read current register value
        if (readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg) < 0) return -1;

        // only change FS_SEL in reg
        reg = (fssel << 5) | (reg & 0x1F);

        if (writeRegister(UB0_REG_GYRO_CONFIG0, reg) < 0) return -2;

        _gyroScale = (2000.0f / static_cast<float>(1 << fssel)) / 32768.0f;
        _gyroFS = fssel;

        return 1;
    }

    int setAccelODR(ODR odr) {
        uint8_t reg;

        setBank(0);

        // read current register value
        if (readRegisters(UB0_REG_ACCEL_CONFIG0, 1, &reg) < 0) return -1;

        // only change ODR in reg
        reg = odr | (reg & 0xF0);

        if (writeRegister(UB0_REG_ACCEL_CONFIG0, reg) < 0) return -2;

        return 1;
    }

    int setGyroODR(ODR odr) {
        uint8_t reg;

        setBank(0);

        // read current register value
        if (readRegisters(UB0_REG_GYRO_CONFIG0, 1, &reg) < 0) return -1;

        // only change ODR in reg
        reg = odr | (reg & 0xF0);

        if (writeRegister(UB0_REG_GYRO_CONFIG0, reg) < 0) return -2;

        return 1;
    }

    int setFilters(bool gyroFilters, bool accFilters) {
        if (setBank(1) < 0) return -1;

        if (gyroFilters == true) {
            if (writeRegister(UB1_REG_GYRO_CONFIG_STATIC2, GYRO_NF_ENABLE | GYRO_AAF_ENABLE) < 0) {
                return -2;
            }
        } else {
            if (writeRegister(UB1_REG_GYRO_CONFIG_STATIC2, GYRO_NF_DISABLE | GYRO_AAF_DISABLE) < 0) {
                return -3;
            }
        }

        if (setBank(2) < 0) return -4;

        if (accFilters == true) {
            if (writeRegister(UB2_REG_ACCEL_CONFIG_STATIC2, ACCEL_AAF_ENABLE) < 0) {
                return -5;
            }
        } else {
            if (writeRegister(UB2_REG_ACCEL_CONFIG_STATIC2, ACCEL_AAF_DISABLE) < 0) {
                return -6;
            }
        }
        if (setBank(0) < 0) return -7;

        return 1;
    }

    int enableDataReadyInterrupt() {
        uint8_t reg;

        // push-pull, pulsed, active HIGH interrupts
        if (writeRegister(UB0_REG_INT_CONFIG, 0x18 | 0x03) < 0) return -1;

        // need to clear bit 4 to allow proper INT1 and INT2 operation
        if (readRegisters(UB0_REG_INT_CONFIG1, 1, &reg) < 0) return -2;
        reg &= ~0x10;
        if (writeRegister(UB0_REG_INT_CONFIG1, reg) < 0) return -3;

        // route UI data ready interrupt to INT1
        if (writeRegister(UB0_REG_INT_SOURCE0, 0x18) < 0) return -4;

        return 1;
    }

    int disableDataReadyInterrupt() {
        uint8_t reg;

        // set pin 4 to return to reset value
        if (readRegisters(UB0_REG_INT_CONFIG1, 1, &reg) < 0) return -1;
        reg |= 0x10;
        if (writeRegister(UB0_REG_INT_CONFIG1, reg) < 0) return -2;

        // return reg to reset value
        if (writeRegister(UB0_REG_INT_SOURCE0, 0x10) < 0) return -3;

        return 1;
    }

    /**
     * @brief      Get accelerometer data, per axis
     * @return     Acceleration in g's
     */
    float accX() const { return _acc[0]; }
    float accY() const { return _acc[1]; }
    float accZ() const { return _acc[2]; }

    /**
     * @brief      Get gyro data, per axis
     * @return     Angular velocity in dps
     */
    float gyrX() const { return _gyr[0]; }
    float gyrY() const { return _gyr[1]; }
    float gyrZ() const { return _gyr[2]; }

    /**
     * @brief      Get temperature of gyro die
     * @return     Temperature in Celsius
     */
    float temp() const { return _t; }

    /* reads the most current data from ICM42688 and stores in buffer */
    int getAGT() {

        if (readRegisters(UB0_REG_TEMP_DATA1, 14, _buffer) < 0) return -1;

        // combine bytes into 16 bit values
        int16_t rawMeas[7]; // temp, accel xyz, gyro xyz
        for (size_t i=0; i<7; i++) {
            rawMeas[i] = ((int16_t)_buffer[i*2] << 8) | _buffer[i*2+1];
        }

        _t = (static_cast<float>(rawMeas[0]) / TEMP_DATA_REG_SCALE) + TEMP_OFFSET;

        _acc[0] = ((rawMeas[1] * _accelScale) - _accB[0]) * _accS[0];
        _acc[1] = ((rawMeas[2] * _accelScale) - _accB[1]) * _accS[1];
        _acc[2] = ((rawMeas[3] * _accelScale) - _accB[2]) * _accS[2];

        _gyr[0] = (rawMeas[4] * _gyroScale) - _gyrB[0];
        _gyr[1] = (rawMeas[5] * _gyroScale) - _gyrB[1];
        _gyr[2] = (rawMeas[6] * _gyroScale) - _gyrB[2];

        return 1;
    }

    /* estimates the gyro biases */
    int calibrateGyro() {
        // set at a lower range (more resolution) since IMU not moving
        const GyroFS current_fssel = _gyroFS;
        if (setGyroFS(dps250) < 0) return -1;

        // take samples and find bias
        _gyroBD[0] = 0;
        _gyroBD[1] = 0;
        _gyroBD[2] = 0;
        for (size_t i=0; i < NUM_CALIB_SAMPLES; i++) {
            getAGT();
            _gyroBD[0] += (gyrX() + _gyrB[0]) / NUM_CALIB_SAMPLES;
            _gyroBD[1] += (gyrY() + _gyrB[1]) / NUM_CALIB_SAMPLES;
            _gyroBD[2] += (gyrZ() + _gyrB[2]) / NUM_CALIB_SAMPLES;
            System::Delay(1);
        }
        _gyrB[0] = _gyroBD[0];
        _gyrB[1] = _gyroBD[1];
        _gyrB[2] = _gyroBD[2];

        // recover the full scale setting
        if (setGyroFS(current_fssel) < 0) return -4;

        return 1;
    }

    /* finds bias and scale factor calibration for the accelerometer,
    this should be run for each axis in each direction (6 total) to find
    the min and max values along each */
    int calibrateAccel() {
        // set at a lower range (more resolution) since IMU not moving
        const AccelFS current_fssel = _accelFS;
        if (setAccelFS(gpm2) < 0) return -1;

        // take samples and find min / max
        _accBD[0] = 0;
        _accBD[1] = 0;
        _accBD[2] = 0;
        for (size_t i=0; i < NUM_CALIB_SAMPLES; i++) {
            getAGT();
            _accBD[0] += (accX()/_accS[0] + _accB[0]) / NUM_CALIB_SAMPLES;
            _accBD[1] += (accY()/_accS[1] + _accB[1]) / NUM_CALIB_SAMPLES;
            _accBD[2] += (accZ()/_accS[2] + _accB[2]) / NUM_CALIB_SAMPLES;
            System::Delay(1);
        }
        if (_accBD[0] > 0.9f) {
            _accMax[0] = _accBD[0];
        }
        if (_accBD[1] > 0.9f) {
            _accMax[1] = _accBD[1];
        }
        if (_accBD[2] > 0.9f) {
            _accMax[2] = _accBD[2];
        }
        if (_accBD[0] < -0.9f) {
            _accMin[0] = _accBD[0];
        }
        if (_accBD[1] < -0.9f) {
            _accMin[1] = _accBD[1];
        }
        if (_accBD[2] < -0.9f) {
            _accMin[2] = _accBD[2];
        }

        // find bias and scale factor
        if ((abs(_accMin[0]) > 0.9f) && (abs(_accMax[0]) > 0.9f)) {
            _accB[0] = (_accMin[0] + _accMax[0]) / 2.0f;
            _accS[0] = 1/((abs(_accMin[0]) + abs(_accMax[0])) / 2.0f);
        }
        if ((abs(_accMin[1]) > 0.9f) && (abs(_accMax[1]) > 0.9f)) {
            _accB[1] = (_accMin[1] + _accMax[1]) / 2.0f;
            _accS[1] = 1/((abs(_accMin[1]) + abs(_accMax[1])) / 2.0f);
        }
        if ((abs(_accMin[2]) > 0.9f) && (abs(_accMax[2]) > 0.9f)) {
            _accB[2] = (_accMin[2] + _accMax[2]) / 2.0f;
            _accS[2] = 1/((abs(_accMin[2]) + abs(_accMax[2])) / 2.0f);
        }

        // recover the full scale setting
        if (setAccelFS(current_fssel) < 0) return -4;

        return 1;
    }

    int setBank(uint8_t bank) {
        // if we are already on this bank, return
        if (_bank == bank) return 1;

        _bank = bank;

        return writeRegister(REG_BANK_SEL, bank);
    }

    void reset() {
        setBank(0);

        writeRegister(UB0_REG_DEVICE_CONFIG, 0x01);

        // wait for ICM42688 to come back up
        System::Delay(10);
    }

    /* gets the ICM42688 WHO_AM_I register value */
    uint8_t whoAmI() {
        setBank(0);

        // read the WHO AM I register
        if (readRegisters(UB0_REG_WHO_AM_I, 1, _buffer) < 0) {
            return -1;
        }
        // return the register value
        return _buffer[0];
    }

    /** Get and reset the transport error flag
        \return Whether the transport has errored since the last check
    */
    Result GetTransportError() { return transport_.GetError() ? ERR : OK; }

  private:
    Config    config_;
    Transport transport_;

    ///\brief Constants
    static constexpr uint8_t WHO_AM_I = 0x47; ///< expected value in UB0_REG_WHO_AM_I reg
    static constexpr int NUM_CALIB_SAMPLES = 1000; ///< for gyro/accel bias calib

    ///\brief Conversion formula to get temperature in Celsius (Sec 4.13)
    static constexpr float TEMP_DATA_REG_SCALE = 132.48f;
    static constexpr float TEMP_OFFSET = 25.0f;

    const uint8_t FIFO_EN = 0x23;
    const uint8_t FIFO_TEMP_EN = 0x04;
    const uint8_t FIFO_GYRO = 0x02;
    const uint8_t FIFO_ACCEL = 0x01;
    // const uint8_t FIFO_COUNT = 0x2E;
    // const uint8_t FIFO_DATA = 0x30;

    // BANK 1
    // const uint8_t GYRO_CONFIG_STATIC2 = 0x0B;
    const uint8_t GYRO_NF_ENABLE = 0x00;
    const uint8_t GYRO_NF_DISABLE = 0x01;
    const uint8_t GYRO_AAF_ENABLE = 0x00;
    const uint8_t GYRO_AAF_DISABLE = 0x02;

    // BANK 2
    // const uint8_t ACCEL_CONFIG_STATIC2 = 0x03;
    const uint8_t ACCEL_AAF_ENABLE = 0x00;
    const uint8_t ACCEL_AAF_DISABLE = 0x01;

    uint8_t _bank = 0;        ///< current user bank
    uint8_t _buffer[15] = {}; ///< buffer for reading from sensor

    // data buffer
    float _t = 0.0f;
    float _acc[3] = {};
    float _gyr[3] = {};

    ///\brief Full scale resolution factors
    float _accelScale = 0.0f;
    float _gyroScale = 0.0f;

    ///\brief Full scale selections
    AccelFS _accelFS;
    GyroFS _gyroFS;

    ///\brief Accel calibration
    float _accBD[3] = {};
    float _accB[3] = {};
    float _accS[3] = {1.0f, 1.0f, 1.0f};
    float _accMax[3] = {};
    float _accMin[3] = {};

    ///\brief Gyro calibration
    float _gyroBD[3] = {};
    float _gyrB[3] = {};

// -------------------------------------------------------------------------

#if 0 // gls

    uint16_t _sensorid_accel, ///< ID number for accelerometer
        _sensorid_gyro,       ///< ID number for gyro
        _sensorid_mag,        ///< ID number for mag
        _sensorid_temp;       ///< ID number for temperature

    uint8_t current_accel_range_; ///< accelerometer range cache
    uint8_t current_gyro_range_;  ///< gyro range cache

    int16_t rawAccX, ///< temp variables
        rawAccY,     ///< temp variables
        rawAccZ,     ///< temp variables
        rawTemp,     ///< temp variables
        rawGyroX,    ///< temp variables
        rawGyroY,    ///< temp variables
        rawGyroZ,    ///< temp variables
        rawMagX,     ///< temp variables
        rawMagY,     ///< temp variables
        rawMagZ;     ///< temp variables

    float temperature, ///< Last reading's temperature (C)
        accX,          ///< Last reading's accelerometer X axis m/s^2
        accY,          ///< Last reading's accelerometer Y axis m/s^2
        accZ,          ///< Last reading's accelerometer Z axis m/s^2
        gyroX,         ///< Last reading's gyro X axis in rad/s
        gyroY,         ///< Last reading's gyro Y axis in rad/s
        gyroZ,         ///< Last reading's gyro Z axis in rad/s
        magX,          ///< Last reading's mag X axis in rad/s
        magY,          ///< Last reading's mag Y axis in rad/s
        magZ;          ///< Last reading's mag Z axis in rad/s

#endif // gls

};

/** @} */

using Icm42688pSpi = Icm42688p<Icm42688pSpiTransport>;

} // namespace daisy
