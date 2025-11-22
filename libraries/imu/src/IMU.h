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

    enum PwrState : bool
    {
        POWER_OFF = false,
        POWER_ON = true,
    };

    // Accelerometer Full-Scale Range (register encoding)
    // ACCEL_CONFIG0 bits[6:5]: 00=±16g, 01=±8g, 10=±4g, 11=±2g
    enum AccelFS : uint8_t
    {
        gpm16 = 0,  // ±16g (default)
        gpm8  = 1,  // ±8g
        gpm4  = 2,  // ±4g
        gpm2  = 3   // ±2g
    };

    // Gyroscope Full-Scale Range (register encoding)
    // GYRO_CONFIG0 bits[6:5]: 00=±2000dps, 01=±1000dps, 10=±500dps, 11=±250dps
    enum GyroFS : uint8_t
    {
        dps2000 = 0,  // ±2000 dps (default)
        dps1000 = 1,  // ±1000 dps
        dps500  = 2,  // ±500 dps
        dps250  = 3   // ±250 dps
    };

    // Output Data Rate (register encoding)
    // CRITICAL: ODR encoding is non-sequential per datasheet
    // CONFIG0 bits[3:0]: 3=8kHz, 5=4kHz, 6=2kHz, 7=1kHz, 9=500Hz
    enum AccelODR : uint8_t
    {
        accel_odr500 = 9,  // 500 Hz (2 ms)
        accel_odr1k  = 7,  // 1 kHz (1 ms) - NOT 6!
        accel_odr2k  = 6,  // 2 kHz (500 us)
        accel_odr4k  = 5,  // 4 kHz (250 us) - NOT 4!
        accel_odr8k  = 3,  // 8 kHz (125 us)
    };

    enum GyroODR : uint8_t
    {
        gyr_odr500 = 9,  // 500 Hz (2 ms)
        gyr_odr1k  = 7,  // 1 kHz (1 ms) - NOT 6!
        gyr_odr2k  = 6,  // 2 kHz (500 us)
        gyr_odr4k  = 5,  // 4 kHz (250 us) - NOT 4!
        gyr_odr8k  = 3,  // 8 kHz (125 us)
    };

    // Intent-based preset configurations (imu_hal.md philosophy)
    // Recommended API: use ApplyPreset() instead of individual FSR/ODR setters
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
     * @brief Configure the device full scales and output frequencies.
     * @param acc_fsr_g Accelerometer full-scale range.
     * @param gyr_fsr_dps Gyroscope full-scale range.
     * @param acc_freq Accelerometer Output Data Rate.
     * @param gyr_freq Gyroscope Output Data Rate.
     * @return IMU::Result::OK on success, IMU::Result::ERR on failure.
     * @deprecated Use ApplyPreset() instead for validated filter configurations
     */
    Result ConfigureInvDevice(AccelFS acc_fsr_g, GyroFS gyr_fsr_dps, AccelODR acc_freq, GyroODR gyr_freq);

    /**
     * @brief Apply intent-based preset configuration (recommended API)
     * @param preset Preset configuration (SAFE, SMOOTH, BALANCED, ACRO)
     * @return IMU::Result::OK on success, IMU::Result::ERR on failure.
     *
     * Configures ODR, FSR, AAF filters, and UI filters per imu_hal.md specification.
     * All presets use ±2000dps/±16g FSR. BALANCED is recommended default.
     */
    Result ApplyPreset(Preset preset);

    /**
     * @brief Perform a soft reset of the device.
     * @return 0 on success, negative error code on failure.
     */
    int Reset();

    /**
     * @brief Set power state of device on or off.
     * @return 0 on success, negative error code on failure.
     */
    int SetPwrState(PwrState state);

    /**
     * @brief Enable accelerometer in Low Noise mode.
     * @return 0 on success, negative error code on failure.
     */
    int EnableAccelLNMode();

    /**
     * @brief Disable accelerometer.
     * @return 0 on success, negative error code on failure.
     */
    int DisableAccel();

    /**
     * @brief Enable gyroscope in Low Noise mode.
     * @return 0 on success, negative error code on failure.
     */
    int EnableGyroLNMode();

    /**
     * @brief Disable gyroscope.
     * @return 0 on success, negative error code on failure.
     */
    int DisableGyro();

    /**
     * @brief Configure the accelerometer Output Data Rate.
     * @param frequency e.g. ICM426XX_ACCEL_CONFIG0_ODR_1_KHZ
     * @return 0 on success, negative error code on failure.
     */
    int SetAccelODR(AccelODR frequency);

    /**
     * @brief Configure the gyroscope Output Data Rate.
     * @param frequency e.g. ICM426XX_GYRO_CONFIG0_ODR_1_KHZ
     * @return 0 on success, negative error code on failure.
     */
    int SetGyroODR(GyroODR frequency);

    /**
     * @brief Set the accelerometer full-scale range.
     * @param fsr e.g. ICM426XX_ACCEL_CONFIG0_FS_SEL_4g
     * @return 0 on success, negative error code on failure.
     */
    int SetAccelFSR(AccelFS fsr);

    /**
     * @brief Set the gyroscope full-scale range.
     * @param fsr e.g. ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps
     * @return 0 on success, negative error code on failure.
     */
    int SetGyroFSR(GyroFS fsr);

    // ========================================================================
    // Advanced Filter Configuration (for power users)
    // For most use cases, prefer ApplyPreset() for validated configurations.
    // ========================================================================

    /**
     * @brief Configure gyroscope Anti-Alias Filter (AAF) using preset index
     * @param aaf_index Index into AAF lookup table (0-3):
     *                  0=258Hz (Betaflight default), 1=536Hz, 2=997Hz, 3=1962Hz
     * @return 0 on success, -1 on failure or unsupported chip
     *
     * @note ICM-42688-P only. For validated configurations, use ApplyPreset().
     */
    int SetGyroAAF(uint8_t aaf_index);

    /**
     * @brief Configure accelerometer Anti-Alias Filter (AAF) using preset index
     * @param aaf_index Index into AAF lookup table (0-3):
     *                  0=258Hz (Betaflight default), 1=536Hz, 2=997Hz, 3=1962Hz
     * @return 0 on success, -1 on failure or unsupported chip
     *
     * @note ICM-42688-P only. For validated configurations, use ApplyPreset().
     */
    int SetAccelAAF(uint8_t aaf_index);

    /**
     * @brief Configure UI (User Interface) filters
     * @param gyro_bw Gyro bandwidth code (0-15, 15=low-latency Betaflight default)
     * @param accel_bw Accel bandwidth code (0-15)
     * @param gyro_order Gyro filter order (1-3)
     * @param accel_order Accel filter order (1-3)
     * @return 0 on success, -1 on failure or unsupported chip
     *
     * @note ICM-42688-P only. For validated configurations, use ApplyPreset().
     */
    int SetUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order);

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
     * @brief Enable the data ready interrupt on INT1 pin.
     * @return 0 on success, negative error code on failure.
     */
    int EnableDataReadyInt1();

    /**
     * @brief Disable the data ready interrupt on INT1 pin.
     * @return 0 on success, negative error code on failure.
     */
    int DisableDataReadyInt1();

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
