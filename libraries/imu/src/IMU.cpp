#include "IMU.h"
#include "stm32yyxx_ll_system.h"  // For DWT cycle counter
#include "icm42688p.h"  // For ICM-42688-P filter API

// ============================================================================
// TDK Driver Required External Functions
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

// IRQ nesting counter
static uint32_t sDisableIntCount = 0;

void inv_disable_irq(void)
{
    if (sDisableIntCount == 0) {
        __disable_irq();
    }
    sDisableIntCount++;
}

void inv_enable_irq(void)
{
    sDisableIntCount--;
    if (sDisableIntCount == 0) {
        __enable_irq();
    }
}

uint64_t inv_icm426xx_get_time_us(void)
{
    return micros();
}

void inv_icm426xx_sleep_us(uint32_t us)
{
    delayMicroseconds(us);
}

uint64_t inv_timer_get_counter(unsigned timer_num)
{
    (void)timer_num;
    return micros();
}

void inv_delay_us(uint32_t us)
{
    delayMicroseconds(us);
}

#ifdef __cplusplus
}
#endif

// ============================================================================
// IMU Class Implementation
// ============================================================================

IMU::IMU()
{
    // Initialize DWT ticks per microsecond
    us_ticks_ = SystemCoreClock / 1000000;
}

IMU::Result IMU::Init(SPIClass& spi, uint32_t cs_pin, uint32_t spi_freq_hz)
{
    // Store SPI reference and configuration
    p_spi_ = &spi;
    cs_pin_ = cs_pin;
    spi_freq_hz_ = spi_freq_hz;
    cs_pin_name_ = digitalPinToPinName(cs_pin);

    // Initialize CS pin
    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);

    // Initialize SPI
    p_spi_->begin(SPISettings(spi_freq_hz_, MSBFIRST, SPI_MODE0));

    // Set up the TDK driver serif (serial interface) structure
    serif_.context = this;  // Store this pointer for callbacks
    serif_.read_reg = spiReadRegs;
    serif_.write_reg = spiWriteRegs;
    serif_.max_read = IMU_MAX_READ;
    serif_.max_write = IMU_MAX_WRITE;
    serif_.serif_type = ICM426XX_UI_SPI4;

    // Initialize the TDK high-level driver
    int rc = inv_icm426xx_init(&driver_, &serif_, NULL);
    if (rc != 0) {
        return Result::ERR;
    }

    // Check WHO_AM_I
    uint8_t who_am_i = 0;
    rc = inv_icm426xx_get_who_am_i(&driver_, &who_am_i);
    if (rc != 0 || who_am_i != ICM_WHOAMI) {
        return Result::ERR;
    }

    initialized_ = true;
    return Result::OK;
}

IMU::Result IMU::ConfigureInvDevice(AccelFS acc_fsr_g, GyroFS gyr_fsr_dps,
                                     AccelODR acc_freq, GyroODR gyr_freq)
{
    if (!initialized_) {
        return Result::ERR;
    }

    int rc = 0;

    // Set FSR
    rc |= inv_icm426xx_set_accel_fsr(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t>(acc_fsr_g));
    rc |= inv_icm426xx_set_gyro_fsr(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t>(gyr_fsr_dps));

    // Set ODR
    rc |= inv_icm426xx_set_accel_frequency(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_ODR_t>(acc_freq));
    rc |= inv_icm426xx_set_gyro_frequency(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_ODR_t>(gyr_freq));

    // Update sensitivity values
    rc |= inv_icm426xx_get_accel_fsr(&driver_, reinterpret_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t*>(&acc_fsr_g));
    rc |= inv_icm426xx_get_gyro_fsr(&driver_, reinterpret_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t*>(&gyr_fsr_dps));

    // Calculate sensitivity from FSR
    switch (acc_fsr_g) {
        case gpm2:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_2G;  break;
        case gpm4:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_4G;  break;
        case gpm8:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_8G;  break;
        case gpm16: accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G; break;
    }

    switch (gyr_fsr_dps) {
        case dps250:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_250;  break;
        case dps500:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_500;  break;
        case dps1000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_1000; break;
        case dps2000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000; break;
    }

    // Track ODR for filter validation
    switch (acc_freq) {
        case accel_odr500: accel_odr_hz_ = 500; break;
        case accel_odr1k:  accel_odr_hz_ = 1000; break;
        case accel_odr2k:  accel_odr_hz_ = 2000; break;
        case accel_odr4k:  accel_odr_hz_ = 4000; break;
        case accel_odr8k:  accel_odr_hz_ = 8000; break;
        default: accel_odr_hz_ = 0; break;
    }

    switch (gyr_freq) {
        case gyr_odr500: gyro_odr_hz_ = 500; break;
        case gyr_odr1k:  gyro_odr_hz_ = 1000; break;
        case gyr_odr2k:  gyro_odr_hz_ = 2000; break;
        case gyr_odr4k:  gyro_odr_hz_ = 4000; break;
        case gyr_odr8k:  gyro_odr_hz_ = 8000; break;
        default: gyro_odr_hz_ = 0; break;
    }

    return (rc == 0) ? Result::OK : Result::ERR;
}

int IMU::Reset()
{
    if (!initialized_) return -1;
    return inv_icm426xx_device_reset(&driver_);
}

int IMU::SetPwrState(PwrState state)
{
    if (!initialized_) return -1;

    if (state == POWER_ON) {
        return inv_icm426xx_enable_accel_low_noise_mode(&driver_) |
               inv_icm426xx_enable_gyro_low_noise_mode(&driver_);
    } else {
        return inv_icm426xx_disable_accel(&driver_) |
               inv_icm426xx_disable_gyro(&driver_);
    }
}

int IMU::EnableAccelLNMode()
{
    if (!initialized_) return -1;
    return inv_icm426xx_enable_accel_low_noise_mode(&driver_);
}

int IMU::DisableAccel()
{
    if (!initialized_) return -1;
    return inv_icm426xx_disable_accel(&driver_);
}

int IMU::EnableGyroLNMode()
{
    if (!initialized_) return -1;
    return inv_icm426xx_enable_gyro_low_noise_mode(&driver_);
}

int IMU::DisableGyro()
{
    if (!initialized_) return -1;
    return inv_icm426xx_disable_gyro(&driver_);
}

int IMU::SetAccelODR(AccelODR frequency)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_accel_frequency(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_ODR_t>(frequency));

    // Track ODR for filter validation
    if (rc == 0) {
        switch (frequency) {
            case accel_odr500: accel_odr_hz_ = 500; break;
            case accel_odr1k:  accel_odr_hz_ = 1000; break;
            case accel_odr2k:  accel_odr_hz_ = 2000; break;
            case accel_odr4k:  accel_odr_hz_ = 4000; break;
            case accel_odr8k:  accel_odr_hz_ = 8000; break;
            default: accel_odr_hz_ = 0; break;
        }
    }
    return rc;
}

int IMU::SetGyroODR(GyroODR frequency)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_gyro_frequency(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_ODR_t>(frequency));

    // Track ODR for filter validation
    if (rc == 0) {
        switch (frequency) {
            case gyr_odr500: gyro_odr_hz_ = 500; break;
            case gyr_odr1k:  gyro_odr_hz_ = 1000; break;
            case gyr_odr2k:  gyro_odr_hz_ = 2000; break;
            case gyr_odr4k:  gyro_odr_hz_ = 4000; break;
            case gyr_odr8k:  gyro_odr_hz_ = 8000; break;
            default: gyro_odr_hz_ = 0; break;
        }
    }
    return rc;
}

int IMU::SetAccelFSR(AccelFS fsr)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_accel_fsr(&driver_, static_cast<ICM426XX_ACCEL_CONFIG0_FS_SEL_t>(fsr));

    // Update sensitivity
    if (rc == 0) {
        switch (fsr) {
            case gpm2:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_2G;  break;
            case gpm4:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_4G;  break;
            case gpm8:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_8G;  break;
            case gpm16: accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G; break;
        }
    }
    return rc;
}

int IMU::SetGyroFSR(GyroFS fsr)
{
    if (!initialized_) return -1;
    int rc = inv_icm426xx_set_gyro_fsr(&driver_, static_cast<ICM426XX_GYRO_CONFIG0_FS_SEL_t>(fsr));

    // Update sensitivity
    if (rc == 0) {
        switch (fsr) {
            case dps250:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_250;  break;
            case dps500:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_500;  break;
            case dps1000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_1000; break;
            case dps2000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000; break;
        }
    }
    return rc;
}

IMU::ChipType IMU::GetChipType()
{
    if (!initialized_) {
        return ChipType::UNKNOWN;
    }

    uint8_t who_am_i = 0;
    int rc = inv_icm426xx_get_who_am_i(&driver_, &who_am_i);
    if (rc != 0) {
        return ChipType::UNKNOWN;
    }

    // Map WHO_AM_I value to ChipType enum
    switch (who_am_i) {
        case 0x47: return ChipType::ICM42688_P;
        case 0x68: return ChipType::MPU_6000;
        case 0x71: return ChipType::MPU_9250;
        default:   return ChipType::UNKNOWN;
    }
}

int IMU::EnableDataReadyInt1()
{
    if (!initialized_) return -1;

    inv_icm426xx_interrupt_parameter_t config_int;

    // Get current interrupt configuration
    int rc = inv_icm426xx_get_config_int1(&driver_, &config_int);
    if (rc != 0) {
        return rc;
    }

    // Enable data ready interrupt
    config_int.INV_ICM426XX_UI_DRDY = INV_ICM426XX_ENABLE;

    // Set updated configuration
    return inv_icm426xx_set_config_int1(&driver_, &config_int);
}

int IMU::DisableDataReadyInt1()
{
    if (!initialized_) return -1;

    inv_icm426xx_interrupt_parameter_t config_int;

    // Get current interrupt configuration
    int rc = inv_icm426xx_get_config_int1(&driver_, &config_int);
    if (rc != 0) {
        return rc;
    }

    // Disable data ready interrupt
    config_int.INV_ICM426XX_UI_DRDY = INV_ICM426XX_DISABLE;

    // Set updated configuration
    return inv_icm426xx_set_config_int1(&driver_, &config_int);
}

int IMU::RunSelfTest(int* result, std::array<int, 6>* bias)
{
    if (!initialized_) return -1;

    // Run self-test (result is ACCEL_SUCCESS<<1 | GYRO_SUCCESS)
    int rc = inv_icm426xx_run_selftest(&driver_, result);
    if (rc != 0) {
        return rc;
    }

    // If bias array provided, retrieve bias values
    if (bias != nullptr) {
        rc = inv_icm426xx_get_st_bias(&driver_, bias->data());
    }

    return rc;
}

int IMU::ReadDataFromRegisters()
{
    if (!initialized_) return -1;
    return inv_icm426xx_get_data_from_registers(&driver_);
}

int IMU::ReadIMU6(std::array<int16_t, 6>& buf)
{
    if (!initialized_) return -1;

    uint8_t raw_data[NUM_DATA_BYTES];
    int rc = 0;

    // Read all 12 bytes (6 accel + 6 gyro) at once
    SelectDevice();

    // Send read command for ACCEL_DATA_X0 register
    uint8_t reg_addr = MPUREG_ACCEL_DATA_X0_UI | 0x80;  // Set read bit
    p_spi_->beginTransaction(SPISettings(spi_freq_hz_, MSBFIRST, SPI_MODE0));
    p_spi_->transfer(reg_addr);

    // Read data
    for (uint32_t i = 0; i < NUM_DATA_BYTES; i++) {
        raw_data[i] = p_spi_->transfer(0xFF);
    }

    p_spi_->endTransaction();
    DeselectDevice();

    // Convert to int16_t
    buf[0] = (int16_t)((raw_data[0] << 8) | raw_data[1]);   // Accel X
    buf[1] = (int16_t)((raw_data[2] << 8) | raw_data[3]);   // Accel Y
    buf[2] = (int16_t)((raw_data[4] << 8) | raw_data[5]);   // Accel Z
    buf[3] = (int16_t)((raw_data[6] << 8) | raw_data[7]);   // Gyro X
    buf[4] = (int16_t)((raw_data[8] << 8) | raw_data[9]);   // Gyro Y
    buf[5] = (int16_t)((raw_data[10] << 8) | raw_data[11]); // Gyro Z

    return rc;
}

int IMU::getMotion6(int16_t* ax, int16_t* ay, int16_t* az,
                     int16_t* gx, int16_t* gy, int16_t* gz)
{
    std::array<int16_t, 6> imu_data;
    int status = ReadIMU6(imu_data);

    if (status != 0) {
        return status;
    }

    *ax = imu_data[0];
    *ay = imu_data[1];
    *az = imu_data[2];
    *gx = imu_data[3];
    *gy = imu_data[4];
    *gz = imu_data[5];

    return 0;
}

int IMU::ReadDataFromFifo()
{
    if (!initialized_) return -1;
    return inv_icm426xx_get_data_from_fifo(&driver_);
}

void IMU::SetSensorEventCallback(void (*userCb)(inv_icm426xx_sensor_event_t *event))
{
    user_event_cb_ = userCb;
}

// ============================================================================
// Private Methods
// ============================================================================

void IMU::DelayNs(uint32_t delay_ns)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t ticks = (delay_ns * us_ticks_) / 1000;
    while ((DWT->CYCCNT - start) < ticks) {}
}

void IMU::SelectDevice()
{
    digitalWriteFast(cs_pin_name_, LOW);
    DelayNs(SETUP_TIME_NS);
}

void IMU::DeselectDevice()
{
    DelayNs(HOLD_TIME_NS);
    digitalWriteFast(cs_pin_name_, HIGH);
}

void IMU::DriverEventCb(inv_icm426xx_sensor_event_t *event)
{
    // Event callback for TDK driver (used when reading FIFO or registers)
    // Not used for simple operations like self-test
    (void)event;
}

// ============================================================================
// TDK Transport Layer Callbacks
// ============================================================================

int IMU::spiReadRegs(struct inv_icm426xx_serif *serif,
                     uint8_t reg,
                     uint8_t *buf,
                     uint32_t len)
{
    IMU* imu = static_cast<IMU*>(serif->context);
    if (!imu || !imu->p_spi_) return -1;

    imu->SelectDevice();

    imu->p_spi_->beginTransaction(SPISettings(imu->spi_freq_hz_, MSBFIRST, SPI_MODE0));
    imu->p_spi_->transfer(reg | 0x80);  // Set read bit

    for (uint32_t i = 0; i < len; i++) {
        buf[i] = imu->p_spi_->transfer(0xFF);
    }

    imu->p_spi_->endTransaction();
    imu->DeselectDevice();

    return 0;
}

int IMU::spiWriteRegs(struct inv_icm426xx_serif *serif,
                      uint8_t reg,
                      const uint8_t *buf,
                      uint32_t len)
{
    IMU* imu = static_cast<IMU*>(serif->context);
    if (!imu || !imu->p_spi_) return -1;

    for (uint32_t i = 0; i < len; i++) {
        imu->SelectDevice();

        imu->p_spi_->beginTransaction(SPISettings(imu->spi_freq_hz_, MSBFIRST, SPI_MODE0));
        imu->p_spi_->transfer(reg + i);  // Write bit is 0
        imu->p_spi_->transfer(buf[i]);
        imu->p_spi_->endTransaction();

        imu->DeselectDevice();
    }

    return 0;
}

int IMU::spiConfigure(struct inv_icm426xx_serif *serif)
{
    // No-op for Arduino - SPI already configured
    (void)serif;
    return 0;
}

// ============================================================================
// Filter Configuration Methods
// ============================================================================

int IMU::SetGyroFilterHz(icm42688p_aaf_bandwidth_t bandwidth)
{
    if (!initialized_) return -1;

    // Only ICM-42688-P supported currently
    ChipType chip = GetChipType();
    if (chip != ChipType::ICM42688_P) {
        return -1;  // Chip not supported
    }

    // Call ICM-42688-P filter configuration
    return icm42688p_set_gyro_aaf(&driver_, bandwidth);
}

int IMU::SetAccelFilterHz(icm42688p_aaf_bandwidth_t bandwidth)
{
    if (!initialized_) return -1;

    // Only ICM-42688-P supported currently
    ChipType chip = GetChipType();
    if (chip != ChipType::ICM42688_P) {
        return -1;  // Chip not supported
    }

    // Call ICM-42688-P filter configuration
    return icm42688p_set_accel_aaf(&driver_, bandwidth);
}

int IMU::SetUiFiltersWide()
{
    if (!initialized_) return -1;

    // Only ICM-42688-P supported currently
    ChipType chip = GetChipType();
    if (chip != ChipType::ICM42688_P) {
        return -1;  // Chip not supported
    }

    // Call ICM-42688-P UI filter configuration
    return icm42688p_set_ui_filters_wide(&driver_);
}

int IMU::VerifyAafConfig(icm42688p_aaf_bandwidth_t gyro_bandwidth,
                         icm42688p_aaf_bandwidth_t accel_bandwidth)
{
    if (!initialized_) return -1;

    // Only ICM-42688-P supported currently
    ChipType chip = GetChipType();
    if (chip != ChipType::ICM42688_P) {
        return -1;  // Chip not supported
    }

    // Call ICM-42688-P AAF verification
    return icm42688p_verify_aaf(&driver_, gyro_bandwidth, accel_bandwidth);
}

int IMU::VerifyUiFiltersWide()
{
    if (!initialized_) return -1;

    // Only ICM-42688-P supported currently
    ChipType chip = GetChipType();
    if (chip != ChipType::ICM42688_P) {
        return -1;  // Chip not supported
    }

    // Call ICM-42688-P UI filter verification
    return icm42688p_verify_ui_filters_wide(&driver_);
}
