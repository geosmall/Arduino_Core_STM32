#pragma once

#include "per/gpio.h"
#include "per/spi.h"

namespace daisy
{

class ICM42688
{
  public:
    enum GyroFS : uint8_t
    {
        dps2000 = 0x00, // (default)
        dps1000 = 0x01,
        dps500 = 0x02,
        dps250 = 0x03,
        dps125 = 0x04,
        dps62_5 = 0x05,
        dps31_25 = 0x06,
        dps15_625 = 0x07
    };

    enum AccelFS : uint8_t
    {
        gpm16 = 0x00, // (default)
        gpm8 = 0x01,
        gpm4 = 0x02,
        gpm2 = 0x03
    };

    enum ODR : uint8_t
    {
        odr32k = 0x01, // LN mode only
        odr16k = 0x02, // LN mode only
        odr8k = 0x03,  // LN mode only
        odr4k = 0x04,  // LN mode only
        odr2k = 0x05,  // LN mode only
        odr1k = 0x06,  // LN mode only (default)
        odr200 = 0x07,
        odr100 = 0x08,
        odr50 = 0x09,
        odr25 = 0x0A,
        odr12_5 = 0x0B,
        odr6a25 = 0x0C,   // LP mode only (accel only)
        odr3a125 = 0x0D,  // LP mode only (accel only)
        odr1a5625 = 0x0E, // LP mode only (accel only)
        odr500 = 0x0F,
    };

    enum aafConfig_e
    {
        AAF_CONFIG_258HZ = 0,
        AAF_CONFIG_536HZ,
        AAF_CONFIG_997HZ,
        AAF_CONFIG_1962HZ,
        AAF_CONFIG_COUNT
    };

    struct AAFConfig
    {
        uint8_t delt;
        uint16_t deltSqr;
        uint8_t bitshift;

        AAFConfig(uint8_t d, uint16_t ds, uint8_t bs) : delt(d), deltSqr(ds), bitshift(bs)
        {
        }
    };

    enum class Result
    {
        OK,
        ERR,
    };

    ICM42688()
    {
    }
    ~ICM42688()
    {
    }

    /** 
     * @brief Initializes the ICM42688 using SPI.
     * @param spi SPI handle for communication.
     * @return OK if successful, ERR otherwise.
     */
    Result Init(SpiHandle spi);

    /** 
     * @brief Sets the full scale range for the accelerometer.
     * @param fssel Full scale selection for the accelerometer.
     * @return OK if successful, ERR otherwise.
     */
    Result setAccelFS(AccelFS fssel);

    /** 
     * @brief Sets the full scale range for the gyroscope.
     * @param fssel Full scale selection for the gyroscope.
     * @return OK if successful, ERR otherwise.
     */
    Result setGyroFS(GyroFS fssel);

    /** 
     * @brief Sets the Output Data Rate (ODR) for the accelerometer.
     * @param odr Output data rate selection for the accelerometer.
     * @return OK if successful, ERR otherwise.
     */
    Result setAccelODR(ODR odr);

    /** 
     * @brief Sets the Output Data Rate (ODR) for the gyroscope.
     * @param odr Output data rate selection for the gyroscope.
     * @return OK if successful, ERR otherwise.
     */
    Result setGyroODR(ODR odr);

    /** 
     * @brief Sets the AAF filter for the gyroscope.
     * @param gyroAAF AAF filter configuration for the gyroscope.
     * @return OK if successful, ERR otherwise.
     */
    Result setGyroAAF(AAFConfig gyroAFF);

    /** 
     * @brief Sets the AAF filter for the accelerometer.
     * @param accelAAF AAF filter configuration for the accelerometer.
     * @return OK if successful, ERR otherwise.
     */
    Result setAccelAAF(AAFConfig accelAAF);

    /** 
     * @brief Enables the data ready interrupt.
     * @return OK if successful, ERR otherwise.
     */
    Result enableDataReadyInterrupt();

    /** 
     * @brief Disables the data ready interrupt.
     * @return OK if successful, ERR otherwise.
     */
    Result disableDataReadyInterrupt();

    /** 
     * @brief Retrieves accelerometer, gyroscope, and temperature data.
     * @return OK if successful, ERR otherwise.
     */
    Result getAGT();

    /**
     * @brief Retrieves accelerometer, gyroscope data.
     * @param AcX int16_t accelerometer X-axis data, passed by reference.
     * @param AcY int16_t accelerometer Y-axis data, passed by reference.
     * @param AcZ int16_t accelerometer Z-axis data, passed by reference.
     * @param GyX int16_t gyroscope X-axis data, passed by reference.
     * @param GyY int16_t gyroscope Y-axis data, passed by reference.
     * @param GyZ int16_t gyroscope Z-axis data, passed by reference.
     * @return OK if successful, ERR otherwise.
     */
    Result getIMU(int16_t& AcX, int16_t& AcY, int16_t& AcZ, int16_t& GyX, int16_t& GyY, int16_t& GyZ);


  private:
    SpiHandle spi_;
    // SpiHandle::Config::BaudPrescaler baud_PS_HS;
    // SpiHandle::Config::BaudPrescaler baud_PS_LS;

    dsy_gpio nss_pin_;
    bool nss_pin_is_SOFT;

    // buffer for reading from sensor
    // 15 bytes is the max read size
    // 1 byte for the register address
    // 2 bytes for TEMP_DATA
    // 6 bytes for X,Y,Z ACCEL_DATA
    // 6 bytes for X,Y,Z GYRO_DATA
    // 15 bytes total
    static constexpr uint8_t ICM42688_BUFFER_SIZE = 15;
    uint8_t txBuffer_[ICM42688_BUFFER_SIZE] = {0};
    uint8_t rxBuffer_[ICM42688_BUFFER_SIZE] = {0};

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

    ///\brief Constants
    static constexpr uint8_t WHO_AM_I = 0x47;      ///< expected value in UB0_REG_WHO_AM_I reg
    static constexpr int NUM_CALIB_SAMPLES = 1000; ///< for gyro/accel bias calib

    ///\brief Conversion formula to get temperature in Celsius (Sec 4.13)
    static constexpr float TEMP_DATA_REG_SCALE = 132.48f;
    static constexpr float TEMP_OFFSET = 25.0f;

    ///\brief Conversion formula to normalize sensor data to +/- 1.0
    static constexpr float NORMALIZE_SENSOR_VAL = 32768.0f;

    enum class PwrState
    {
        POWER_OFF,
        POWER_ON,
    };

    // Possible gyro Anti-Alias Filter (AAF) cutoffs for ICM-42688P
    AAFConfig aafLUT42688[AAF_CONFIG_COUNT] = {
      // see table in section 5.3 ICM-42688-P datasheet v1.8
      [AAF_CONFIG_258HZ]  = AAFConfig{ 6,   36, 10},
      [AAF_CONFIG_536HZ]  = AAFConfig{12,  144,  8},
      [AAF_CONFIG_997HZ]  = AAFConfig{21,  440,  6},
      [AAF_CONFIG_1962HZ] = AAFConfig{37, 1376,  4},
    };

    uint8_t _bank = 0; ///< current user bank

    /** 
     * @brief Writes the specified byte to the register at the specified address.
     * @param addr Address of the register.
     * @param data Byte to write.
     * @return Result of the operation.
     */
    Result writeRegister(uint8_t addr, uint8_t data);

    /** 
     * @brief Writes the specified byte to the register using a mask to modify only certain bits.
     * @param addr Address of the register.
     * @param data Byte to write.
     * @param mask Mask to apply.
     * @return Result of the operation.
     */
    Result writeRegisterMask(uint8_t addr, uint8_t data, uint8_t mask);

    /** 
     * @brief Reads count bytes into dest at the specified register address.
     * @param addr Address of the register.
     * @param count Number of bytes to read.
     * @param dest Destination buffer.
     * @return Result of the operation.
     */
    Result readRegisters(uint8_t addr, uint8_t count, uint8_t* dest);

    /** 
     * @brief Read single byte into rxBuffer_ at the specified register address.
     * @param addr Address of the register.
     * @return The byte read.
     */
    uint8_t readRegister(uint8_t addr);

    /** 
     * @brief Sets the active register bank number.
     * @param bank Bank number to set.
     * @return Result of the operation.
     */
    Result setBank(uint8_t bank);

    /** 
     * @brief Performs a software reset of the device.
     */
    void reset();

    /** 
     * @brief Reads the WHO_AM_I register.
     * @return Value of the WHO_AM_I register, -1 if error
     */
    int16_t whoAmI();

    /** 
     * @brief Turns power to accelerometer and gyroscope on or off.
     * @param state Desired power state, POWER_ON or POWER_OFF.
     * @return OK if successful, ERR otherwise.
     */
    Result SetGyroAccPwrState(PwrState state);

    /** 
     * @brief Initializes software NSS (Slave Select) pin.
     */
    void initializeSoftNSSPin();
};

} // namespace daisy
