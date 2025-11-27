#ifndef IMU_H
#define IMU_H

#include <cstdint>
#include <cstdio>
#include <array>
#include <SPI.h>
#include <Arduino.h>
#include "bus/DeviceBusSPI.h"     // DeviceBusSPI for SPI communication
#include "devices/DeviceBase.h"   // Base class with ImuPreset enum
#include "devices/ICM42688.h"     // ICM-42688-P driver
#include "devices/MPU6000.h"      // MPU-6000 driver
#include "devices/MPU9250.h"      // MPU-9250/9255 driver
#include "devices/ICM206xx.h"     // ICM-20601/20602/20689 driver

/*
 * ============================================================
 * Accelerometer Sensitivity (LSB per g)
 * ------------------------------------------------------------
 * The scale factor depends on the full-scale range (FSR),
 * which is configured in the ACCEL_CONFIG0 register.
 *
 *  |  Accel Range (g)  | Sensitivity (LSB/g)  |
 *  |-------------------|----------------------|
 *  |       ±16g        |         2048         |
 *  |        ±8g        |         4096         |
 *  |        ±4g        |         8192         |
 *  |        ±2g        |        16384         |
 *
 * ============================================================
 * Gyroscope Sensitivity (LSB per dps)
 * ------------------------------------------------------------
 * The scale factor depends on the full-scale range (FSR),
 * which is configured in the GYRO_CONFIG0 register.
 *
 *  |  Gyro Range (dps) | Sensitivity (LSB/dps) |
 *  |-------------------|-----------------------|
 *  |      ±2000        |         16.4          |
 *  |      ±1000        |         32.8          |
 *  |      ±500         |         65.5          |
 *  |      ±250         |        131.0          |
 *
 * ============================================================
 * Note:
 * - The raw sensor output is a signed 16-bit integer.
 * - To convert raw values to physical units:
 *      Accel (g)  = raw_value / sensitivity (LSB/g)
 *      Gyro (dps) = raw_value / sensitivity (LSB/dps)
 * - Make sure the FSR is correctly configured before using
 *   the sensitivity values in calculations.
 * ============================================================
 */

/**
 * @brief High-level C++ wrapper for InvenSense IMU sensors
 *        Currently supports ICM-42688-P, designed for easy extension
 *        to MPU-6000, MPU-9250, and other InvenSense parts.
 */
class IMU
{
public:

    // Intent-based preset configurations (imu_hal.md philosophy)
    // Recommended API: use ApplyPreset() for validated configurations
    enum class Preset : uint8_t
    {
        SAFE,      // Bring-up, very noisy frames (1kHz, tight filtering)
        SMOOTH,    // Extra on-chip smoothing (4kHz, moderate filtering)
        BALANCED,  // Default for 2kHz PID (4kHz, balanced filtering)
        ACRO       // Minimum phase lag (8kHz, wide filtering)
    };

    enum class Result
    {
        OK,
        ERR,
    };

    enum class ChipType : uint8_t
    {
        UNKNOWN = 0x00,
        ICM20602 = 0x12,
        ICM42688_P = 0x47,
        MPU_6000 = 0x68,
        MPU_9250 = 0x71,
        ICM20689 = 0x98,
        ICM20601 = 0xAC,
    };

    // ========================================================================
    // Sensor Sensitivity Constants (LSB per physical unit)
    // ========================================================================

    // ICM-42688-P Accelerometer Sensitivity (LSB/g)
    static constexpr float ICM42688P_ACCEL_SENS_2G   = 16384.0f;
    static constexpr float ICM42688P_ACCEL_SENS_4G   = 8192.0f;
    static constexpr float ICM42688P_ACCEL_SENS_8G   = 4096.0f;
    static constexpr float ICM42688P_ACCEL_SENS_16G  = 2048.0f;

    // ICM-42688-P Gyroscope Sensitivity (LSB/(°/s))
    static constexpr float ICM42688P_GYRO_SENS_250   = 131.0f;
    static constexpr float ICM42688P_GYRO_SENS_500   = 65.5f;
    static constexpr float ICM42688P_GYRO_SENS_1000  = 32.8f;
    static constexpr float ICM42688P_GYRO_SENS_2000  = 16.4f;

    /**
     * @brief Construct an IMU object.
     */
    IMU();

    /**
     * @brief Initialize the IMU hardware and driver with Arduino SPI.
     * @param spi Reference to Arduino SPIClass instance
     * @param cs_pin Chip select pin number
     * @param spi_freq_hz SPI frequency in Hz
     * @return IMU::Result::OK on success, IMU::Result::ERR on failure.
     */
    Result Init(SPIClass& spi, uint32_t cs_pin, uint32_t spi_freq_hz);

    /**
     * @brief Apply intent-based preset configuration (recommended API)
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return IMU::Result::OK on success, IMU::Result::ERR on failure.
     *
     * Configures ODR, FSR, AAF filters, and UI filters per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR. BALANCED is recommended default.
     * Works on all supported IMU chips.
     */
    Result ApplyPreset(Preset preset);

    /**
     * @brief Get the accelerometer full-scale range.
     * @return Accel sensitivity value (updated upon change to FS value).
     */
    float GetAccelSensitivity() const { return accel_sensitivity_; }

    /**
     * @brief Get the gyroscope full-scale range.
     * @return Gyro sensitivity value (updated upon change to FS value).
     */
    float GetGyroSensitivity()  const { return gyro_sensitivity_; }

    /**
     * @brief Get the detected chip type via WHO_AM_I register.
     * @return ChipType enum value (UNKNOWN, ICM42688P, MPU6000, MPU9250).
     */
    ChipType GetChipType();

    /**
     * @brief Enable the data ready interrupt on INT pin.
     * @return 0 on success, negative error code on failure.
     */
    int EnableDataReadyInt();

    /**
     * @brief Disable the data ready interrupt on INT pin.
     * @return 0 on success, negative error code on failure.
     */
    int DisableDataReadyInt();

    // ========================================================================
    // Tier 2 Extended API: FSR Configuration (works on all chips)
    // ========================================================================

    /**
     * @brief Set gyroscope full-scale range (Extended API)
     * @param fsr Full-scale range (DPS_250, DPS_500, DPS_1000, DPS_2000)
     * @return Result::OK on success, Result::ERR on failure
     *
     * Updates both the hardware register and sensitivity tracking.
     * Works on all supported IMU chips.
     */
    Result SetGyroFSR_Ex(GyroFSR fsr);

    /**
     * @brief Set accelerometer full-scale range (Extended API)
     * @param fsr Full-scale range (G_2, G_4, G_8, G_16)
     * @return Result::OK on success, Result::ERR on failure
     *
     * Updates both the hardware register and sensitivity tracking.
     * Works on all supported IMU chips.
     */
    Result SetAccelFSR_Ex(AccelFSR fsr);

    // ========================================================================
    // Tier 3 Extended API: Direct Register Access (power users)
    // ========================================================================

    /**
     * @brief Read register directly (Extended API)
     * @param reg Register address
     * @return Register value
     *
     * For ICM-42688-P, reads from Bank 0 only.
     */
    uint8_t ReadReg_Ex(uint8_t reg);

    /**
     * @brief Write register directly (Extended API)
     * @param reg Register address
     * @param value Value to write
     * @return Result::OK on success
     *
     * For ICM-42688-P, writes to Bank 0 only.
     */
    Result WriteReg_Ex(uint8_t reg, uint8_t value);

    /**
     * @brief Write register and verify (Extended API)
     * @param reg Register address
     * @param value Value to write
     * @return Result::OK if write verified, Result::ERR if verification failed
     */
    Result WriteRegVerify_Ex(uint8_t reg, uint8_t value);

    // Note: RunSelfTest() removed - major FC stacks (Betaflight, iNav, ArduPilot, PX4)
    // skip self-test at startup. Use WHO_AM_I + gyro bias calibration instead.
    // See MIGRATION_PLAN.md "Self-Test Decision" section.

    /**
     * @brief Read Acc/Gyro data direct from registers (bypassing transport read for speed).
     * @param buf filled with Accel X Y Z followed by Gyro X Y Z
     * @return 0 on success, negative error code on failure.
     */
    int ReadIMU6(std::array<int16_t, 6>& buf);

    /**
     * @brief MPU6050/MPU9250 compatibility interface for reading IMU data.
     *        Wraps ReadIMU6() to provide familiar out-parameter interface.
     * @param ax, ay, az Accelerometer X, Y, Z output (raw int16_t)
     * @param gx, gy, gz Gyroscope X, Y, Z output (raw int16_t)
     * @return 0 on success, negative error code on failure.
     */
    int getMotion6(int16_t* ax, int16_t* ay, int16_t* az,
                   int16_t* gx, int16_t* gy, int16_t* gz);

    // Note: ReadDataFromFifo() and SetSensorEventCallback() removed.
    // FIFO support eliminated in Phase 2 migration - use ReadIMU6() for polling.
    // See MIGRATION_PLAN.md for rationale.

    // ========================================================================
    // Magnetometer API (MPU-9250/9255 only)
    // ========================================================================

    /**
     * @brief Check if IMU has magnetometer support
     * @return true if magnetometer available (MPU-9250/9255), false otherwise
     *
     * Use this to detect 9-DOF vs 6-DOF capability before calling magnetometer methods.
     * Example: if (imu.HasMagnetometer()) { imu.ReadIMU9(...); } else { imu.ReadIMU6(...); }
     */
    bool HasMagnetometer() const;

    /**
     * @brief Initialize magnetometer (call after Init() for MPU-9250)
     * @return Result::OK on success, Result::ERR on failure or not supported
     *
     * For MPU-9250: Enables I2C master mode, configures AK8963, reads ASA calibration.
     * For other chips: Returns ERR (no magnetometer).
     * Optional - only needed if you want to use magnetometer.
     */
    Result InitMagnetometer();

    /**
     * @brief Read magnetometer data only
     * @param mx Magnetometer X-axis (µT, microtesla)
     * @param my Magnetometer Y-axis (µT)
     * @param mz Magnetometer Z-axis (µT)
     * @return Result::OK on success, Result::ERR on failure or not supported
     *
     * Requires InitMagnetometer() called first for MPU-9250.
     */
    Result ReadMagnetometer(float& mx, float& my, float& mz);

    /**
     * @brief Read 9-axis IMU data (gyro + accel + mag)
     * @param gyro_buf Gyroscope data [gx, gy, gz] (raw int16_t)
     * @param accel_buf Accelerometer data [ax, ay, az] (raw int16_t)
     * @param mag_buf Magnetometer data [mx, my, mz] (µT)
     * @return Result::OK on success, Result::ERR on failure or not supported
     *
     * Efficient 9-DOF reading for MPU-9250 sensor fusion.
     * For chips without magnetometer, returns ERR.
     * Requires InitMagnetometer() called first for MPU-9250.
     */
    Result ReadIMU9(std::array<int16_t, 3>& gyro_buf,
                    std::array<int16_t, 3>& accel_buf,
                    std::array<float, 3>& mag_buf);

    /**
     * @brief MPU9250 compatibility interface for 9-DOF data
     * @param ax, ay, az Accelerometer X, Y, Z output (raw int16_t)
     * @param gx, gy, gz Gyroscope X, Y, Z output (raw int16_t)
     * @param mx, my, mz Magnetometer X, Y, Z output (raw int16_t)
     * @return 0 on success, negative error code on failure
     *
     * Wraps ReadIMU6() and readMagnetometerRaw() for compatibility with
     * Teensy MPU9250 library API. Returns raw magnetometer values.
     * Requires InitMagnetometer() called first for MPU-9250.
     */
    int getMotion9(int16_t* ax, int16_t* ay, int16_t* az,
                   int16_t* gx, int16_t* gy, int16_t* gz,
                   int16_t* mx, int16_t* my, int16_t* mz);

    /**
     * @brief Calibrate magnetometer using figure-8 motion
     * @return Result::OK on success, Result::ERR on failure or not supported
     *
     * Interactive calibration: move IMU in figure-8 pattern for 15 seconds.
     * Calculates hard iron (bias) and soft iron (scale) corrections.
     * Only supported on MPU-9250/9255.
     */
    Result CalibrateMagnetometer();

    /**
     * @brief Set magnetometer calibration values
     * @param bias_x X-axis bias (µT)
     * @param bias_y Y-axis bias (µT)
     * @param bias_z Z-axis bias (µT)
     * @param scale_x X-axis scale factor
     * @param scale_y Y-axis scale factor
     * @param scale_z Z-axis scale factor
     *
     * Apply previously calculated calibration values.
     * Only effective for MPU-9250/9255.
     */
    void SetMagCalibration(float bias_x, float bias_y, float bias_z,
                           float scale_x, float scale_y, float scale_z);

    /**
     * @brief Get current magnetometer calibration values
     * @param bias_x X-axis bias (µT)
     * @param bias_y Y-axis bias (µT)
     * @param bias_z Z-axis bias (µT)
     * @param scale_x X-axis scale factor
     * @param scale_y Y-axis scale factor
     * @param scale_z Z-axis scale factor
     *
     * Retrieve current calibration for storage/display.
     * Returns zeros for chips without magnetometer.
     */
    void GetMagCalibration(float& bias_x, float& bias_y, float& bias_z,
                           float& scale_x, float& scale_y, float& scale_z) const;

private:
    // IMU data sizes (6 bytes accel + 6 bytes gyro)
    static constexpr uint32_t ACCEL_DATA_SIZE = 6;
    static constexpr uint32_t GYRO_DATA_SIZE = 6;
    static constexpr uint32_t NUM_DATA_BYTES = (ACCEL_DATA_SIZE + GYRO_DATA_SIZE);

    // Register addresses for direct SPI access
    static constexpr uint8_t REG_ACCEL_DATA_X0 = 0x1F;  // ICM426xx ACCEL_DATA_X0_UI

    /**
     * @brief IMU driver instance (polymorphic - ICM42688, MPU6000, MPU9250, or ICM206xx)
     */
    DeviceBase* driver_ = nullptr;

    /**
     * @brief DeviceBus for SPI communication (used by driver)
     */
    DeviceBusSPI* device_bus_ = nullptr;

    /**
     * @brief Pointer to the Arduino SPI instance used for IMU SPI transactions.
     * Initialized to null, to be bound in Init()
     */
    SPIClass* p_spi_ = nullptr;

    /**
     * @brief SPI frequency in Hz
     */
    uint32_t spi_freq_hz_ = 0;

    bool initialized_ = false;

    /**
     * @brief IMU chip select pin (using software driven CS).
     */
    uint32_t cs_pin_;
    PinName cs_pin_name_;  // For fast digitalWriteFast()

    float accel_sensitivity_{-1.0f};
    float gyro_sensitivity_{-1.0f};

    // Track current ODR for filter validation
    uint16_t gyro_odr_hz_{0};
    uint16_t accel_odr_hz_{0};

    // CS->CLK delay, MPU6000 - 8ns
    // CS->CLK delay, ICM42688P - 39ns
    static constexpr uint32_t SETUP_TIME_NS  = 39;   // For ICM42688P
    static constexpr uint32_t HOLD_TIME_NS   = 18;   // For ICM42688P

    // DWT ticks per microsecond for timing delays
    uint32_t us_ticks_;

    void SelectDevice();

    void DeselectDevice();

    void DelayNs(uint32_t delay_ns);
};

#endif // IMU_H
