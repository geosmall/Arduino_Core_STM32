#ifndef IMU_H
#define IMU_H

#include <cstdint>
#include <cstdio>
#include <array>
#include <SPI.h>
#include <Arduino.h>
#include "icm42688p.h"  // ICM-42688-P C driver + C++ filter API

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

    enum AccelFS : uint8_t
    {
        gpm16 = ICM426XX_ACCEL_CONFIG0_FS_SEL_16g, // (default)
        gpm8 = ICM426XX_ACCEL_CONFIG0_FS_SEL_8g,
        gpm4 = ICM426XX_ACCEL_CONFIG0_FS_SEL_4g,
        gpm2 = ICM426XX_ACCEL_CONFIG0_FS_SEL_2g
    };

    enum GyroFS : uint8_t
    {
        dps2000 = ICM426XX_GYRO_CONFIG0_FS_SEL_2000dps, // (default)
        dps1000 = ICM426XX_GYRO_CONFIG0_FS_SEL_1000dps,
        dps500 = ICM426XX_GYRO_CONFIG0_FS_SEL_500dps,
        dps250 = ICM426XX_GYRO_CONFIG0_FS_SEL_250dps
    };

    enum AccelODR : uint8_t
    {
        accel_odr500 = ICM426XX_ACCEL_CONFIG0_ODR_500_HZ, /*!< 500 Hz (2 ms)*/
        accel_odr1k = ICM426XX_ACCEL_CONFIG0_ODR_1_KHZ, /*!< 1 KHz (1 ms)*/
        accel_odr2k = ICM426XX_ACCEL_CONFIG0_ODR_2_KHZ, /*!< 2 KHz (500 us)*/
        accel_odr4k = ICM426XX_ACCEL_CONFIG0_ODR_4_KHZ, /*!< 4 KHz (250 us)*/
        accel_odr8k = ICM426XX_ACCEL_CONFIG0_ODR_8_KHZ, /*!< 8 KHz (125 us)*/
    };

    enum GyroODR : uint8_t
    {
        gyr_odr500 = ICM426XX_GYRO_CONFIG0_ODR_500_HZ, /*!< 500 Hz (2 ms)*/
        gyr_odr1k = ICM426XX_GYRO_CONFIG0_ODR_1_KHZ, /*!< 1 KHz (1 ms)*/
        gyr_odr2k = ICM426XX_GYRO_CONFIG0_ODR_2_KHZ, /*!< 2 KHz (500 us)*/
        gyr_odr4k = ICM426XX_GYRO_CONFIG0_ODR_4_KHZ, /*!< 4 KHz (250 us)*/
        gyr_odr8k = ICM426XX_GYRO_CONFIG0_ODR_8_KHZ, /*!< 8 KHz (125 us)*/
    };

    enum class Result
    {
        OK,
        ERR,
    };

    enum class ChipType : uint8_t
    {
        UNKNOWN = 0x00,
        ICM42688_P = 0x47,
        MPU_6000 = 0x68,
        MPU_9250 = 0x71,
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
     */
    Result ConfigureInvDevice(AccelFS acc_fsr_g, GyroFS gyr_fsr_dps, AccelODR acc_freq, GyroODR gyr_freq);

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

    /**
     * @brief Configure gyroscope Anti-Alias Filter (AAF) bandwidth.
     * @param bandwidth Desired AAF bandwidth from icm42688p_aaf_bandwidth_t enum
     * @return 0 on success, negative error code on failure.
     *
     * @note ICM-42688-P only. Configures AAF using datasheet presets.
     *       Common values: ICM42688P_AAF_258HZ (Betaflight standard), ICM42688P_AAF_213HZ (tighter), ICM42688P_AAF_303HZ (looser).
     *       Full range: ICM42688P_AAF_42HZ to ICM42688P_AAF_1051HZ (22 presets).
     */
    int SetGyroFilterHz(icm42688p_aaf_bandwidth_t bandwidth);

    /**
     * @brief Configure accelerometer Anti-Alias Filter (AAF) bandwidth.
     * @param bandwidth Desired AAF bandwidth from icm42688p_aaf_bandwidth_t enum
     * @return 0 on success, negative error code on failure.
     *
     * @note ICM-42688-P only. Configures AAF using datasheet presets.
     *       Common values: ICM42688P_AAF_170HZ (default), ICM42688P_AAF_126HZ (tighter), ICM42688P_AAF_213HZ (looser).
     *       Full range: ICM42688P_AAF_42HZ to ICM42688P_AAF_1051HZ (22 presets).
     */
    int SetAccelFilterHz(icm42688p_aaf_bandwidth_t bandwidth);

    /**
     * @brief Configure UI (User Interface) filters to ODR/2 bandwidth with 1st-order
     * @return 0 on success, negative error code on failure.
     *
     * @note ICM-42688-P only. Sets both gyro and accel UI filters to:
     *       - Filter order: 1st order (minimal phase lag)
     *       - Bandwidth code: 0 (ODR/2, allows AAF to dominate)
     *       This matches MPU-6000 DLPF 260 "wide" feel.
     */
    int SetUiFiltersOdr2_1st();

    /**
     * @brief Verify AAF (Anti-Alias Filter) configuration by reading back registers
     *
     * @param gyro_bandwidth Expected gyro AAF bandwidth
     * @param accel_bandwidth Expected accel AAF bandwidth
     * @return 0 if verified, -1 if mismatch or unsupported chip
     *
     * Reads back AAF registers from hardware and verifies they match the
     * expected configuration. Only supported on ICM-42688-P.
     */
    int VerifyAafConfig(icm42688p_aaf_bandwidth_t gyro_bandwidth,
                        icm42688p_aaf_bandwidth_t accel_bandwidth);

    /**
     * @brief Verify UI filter configuration by reading back registers
     *
     * @return 0 if verified as 1st-order ODR/2 mode, -1 if mismatch or unsupported chip
     *
     * Reads back UI filter registers from hardware and verifies they are
     * configured for 1st-order, ODR/2 bandwidth. Only supported on ICM-42688-P.
     */
    int VerifyUiFiltersOdr2_1st();

    /**
     * @brief Configure UI (User Interface) filters with custom bandwidth code and filter order
     * @param bw_code Filter bandwidth code (0-15):
     *                - 0: ODR/2 (widest, lowest delay)
     *                - 1-14: Progressively narrower bandwidths
     *                - 15: Low-latency path (trivial decimation, Betaflight default)
     * @param gyro_order Gyro filter order (1-3): 1st, 2nd, or 3rd order
     * @param accel_order Accel filter order (1-3): 1st, 2nd, or 3rd order
     * @return 0 on success, negative error code on failure.
     *
     * @note ICM-42688-P only. General-purpose UI filter configuration.
     *       For Betaflight defaults, use SetUiFiltersBetaflight() instead.
     */
    int SetUiFilters(uint8_t bw_code, uint8_t gyro_order, uint8_t accel_order);

    /**
     * @brief Configure UI filters to Betaflight defaults (code 15, 2nd-order)
     * @return 0 on success, negative error code on failure.
     *
     * @note ICM-42688-P only. Sets both gyro and accel UI filters to:
     *       - BW Code 15: Low-latency path (trivial decimation, minimal delay)
     *       - Filter order: 2nd order (balance between noise and phase lag)
     *       Betaflight relies on software filters for fine control, so hardware
     *       UI filters are kept minimal to reduce delay.
     */
    int SetUiFiltersBetaflight();

    /**
     * @brief Verify UI filter configuration by reading back registers
     *
     * @param expected_bw_code Expected bandwidth code (0-15)
     * @param expected_gyro_order Expected gyro filter order (1-3)
     * @param expected_accel_order Expected accel filter order (1-3)
     * @return 0 if verified, -1 if mismatch or unsupported chip
     *
     * Reads back UI filter registers from hardware and verifies they match the
     * expected configuration. Only supported on ICM-42688-P.
     */
    int VerifyUiFilters(uint8_t expected_bw_code, uint8_t expected_gyro_order, uint8_t expected_accel_order);

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

    /**
     * @brief Perform IMU self-test.
     * @param result (ACCEL_SUCCESS<<1 | GYRO_SUCCESS), 3 means both passed.
     * @param bias Optional array of 6 int, stores bias values (3 for accel, 3 for gyro).
     * @return 0 on success, negative error code on failure.
     */
    int RunSelfTest(int* result, std::array<int, 6>* bias = nullptr);

    /**
     * @brief Read sensor data from registers (bypassing FIFO).
     * @return 0 on success, negative error code on failure.
     */
    int ReadDataFromRegisters();

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

    /**
     * @brief Read sensor data from FIFO.
     * @return Number of FIFO packets read on success, or negative error code on failure.
     */
    int ReadDataFromFifo();

    /**
     * @brief Provide a user callback for sensor events. This is called by the TDK driver
     *        whenever data is read from registers or FIFO.
     *
     * @param userCb The function pointer for your callback, or nullptr to disable.
     */
    void SetSensorEventCallback(void (*userCb)(inv_icm426xx_sensor_event_t *event));

    // Define maximum read and write sizes for IMU as private static constants
    static constexpr uint32_t IMU_MAX_READ = 255;
    static constexpr uint32_t IMU_MAX_WRITE = 255;
    static constexpr uint32_t NUM_DATA_BYTES = (ACCEL_DATA_SIZE + GYRO_DATA_SIZE);

private:
    /**
     * @brief TDK driver instance for this IMU.
     */
    struct inv_icm426xx driver_{};

    /**
     * @brief TDK serif (serial interface) structure for communication callbacks.
     */
    struct inv_icm426xx_serif serif_{};

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

    /**
     * @brief The TDK driver calls this function when new sensor data arrives.
     */
    static void DriverEventCb(inv_icm426xx_sensor_event_t *event);

    /**
     * @brief User-defined callback pointer (per-instance via driver_.transport.context)
     */
    void (*user_event_cb_)(inv_icm426xx_sensor_event_t *event) = nullptr;

    // -------------------------------------------------------------------------
    // The TDK transport layer requires read_reg, write_reg, configure
    // function pointers with the following signatures:
    //   int foo(struct inv_icm426xx_serif *serif, uint8_t reg, ..., uint32_t len);
    // We'll implement them as static methods. We retrieve the IMU instance via
    //   (IMU*)serif->context.
    // -------------------------------------------------------------------------

    /**
     * @brief  TDK read callback for SPI-based register reads.
     */
    static int spiReadRegs(struct inv_icm426xx_serif *serif,
                           uint8_t                    reg,
                           uint8_t                   *buf,
                           uint32_t                   len);

    /**
     * @brief  TDK write callback for SPI-based register writes.
     */
    static int spiWriteRegs(struct inv_icm426xx_serif *serif,
                            uint8_t                    reg,
                            const uint8_t             *buf,
                            uint32_t                   len);

    /**
     * @brief  TDK configure callback, if used. Often a no-op for many systems.
     */
    static int spiConfigure(struct inv_icm426xx_serif *serif);
};

#endif // IMU_H
