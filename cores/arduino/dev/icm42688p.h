#pragma once

#include "per/spi.h"
#include "per/gpio.h"

namespace daisy
{

class ICM42688
{
  public:
    enum GyroFS : uint8_t
    {
        dps2000 = 0x00,
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
        gpm16 = 0x00,
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
        odr1k = 0x06,  // LN mode only
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

    /** Initializes the ICM42688 using spi */
    Result Init(SpiHandle spi);

    /**
     * @brief      Sets the full scale range for the accelerometer
     * @param[in]  fssel  Full scale selection
     * @return     ret < 0 if error
     */
    Result setAccelFS(AccelFS fssel);

    /**
     * @brief      Sets the full scale range for the gyro
     * @param[in]  fssel  Full scale selection
     * @return     ret < 0 if error
     */
    Result setGyroFS(GyroFS fssel);

    /**
     * @brief      Set the ODR for accelerometer
     * @param[in]  odr   Output data rate
     * @return     ret < 0 if error
     */
    Result setAccelODR(ODR odr);

    /**
     * @brief      Set the ODR for gyro
     * @param[in]  odr   Output data rate
     * @return     ret < 0 if error
     */
    Result setGyroODR(ODR odr);

    /**
     * @brief      Set the ODR for gyro
     * @param[in]  odr   Output data rate
     * @return     ret < 0 if error
     */
    Result setFilters(bool gyroFilters, bool accFilters);

  private:
    SpiHandle spi_;
    bool _useSPIHS = false;
    dsy_gpio nss_pin_;
    bool nss_pin_is_SOFT;

    // buffer for reading from sensor
    uint8_t _buffer[15] = {0};

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

    uint8_t _bank = 0; ///< current user bank

    /** Writes the specified byte to the register at the specified address.*/
    Result writeRegister(uint8_t addr, uint8_t data);
    /** Reads count bytes into dest at the specified register address */
    Result readRegisters(uint8_t addr, uint8_t count, uint8_t* dest);
    Result setBank(uint8_t bank);

    /**
     * @brief      Software reset of the device
     */
    void reset();

    /**
     * @brief      Read the WHO_AM_I register
     * @return     Value of WHO_AM_I register
     */
    int16_t whoAmI();

    /**
     * @brief      Initalize software NSS pin
     */
    void initializeSoftNSSPin();

};

} // namespace daisy
