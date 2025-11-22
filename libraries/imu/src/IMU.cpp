#include "IMU.h"
#include "stm32yyxx_ll_system.h"  // For DWT cycle counter

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
    p_spi_->begin();

    // Create DeviceBus for driver
    device_bus_ = new DeviceBusSPI(&spi, cs_pin);
    if (!device_bus_) {
        return Result::ERR;
    }
    device_bus_->setFreq(spi_freq_hz);

    // Try detecting each supported IMU in order of preference
    // ICM42688 first (most capable), then MPU6000, MPU9250, ICM206xx

    // 1. Try ICM-42688-P (best filter options, newest chip)
    driver_ = ICM42688::detect(device_bus_);
    if (driver_) {
        // Apply default BALANCED preset
        driver_->applyPreset(ImuPreset::FILTER_BALANCED);
        gyro_odr_hz_ = 4000;
        accel_odr_hz_ = 1000;
        goto detected;
    }

    // 2. Try MPU-6000 (classic, widely used)
    driver_ = MPU6000::detect(device_bus_);
    if (driver_) {
        driver_->applyPreset(ImuPreset::FILTER_BALANCED);
        gyro_odr_hz_ = 4000;
        accel_odr_hz_ = 1000;
        goto detected;
    }

    // 3. Try MPU-9250/9255 (9-axis, MPU6500-class)
    driver_ = MPU9250::detect(device_bus_);
    if (driver_) {
        driver_->applyPreset(ImuPreset::FILTER_BALANCED);
        gyro_odr_hz_ = 8000;  // MPU9250 BALANCED uses bypass mode
        accel_odr_hz_ = 8000;
        goto detected;
    }

    // 4. Try ICM-206xx family (ICM-20601, ICM-20602, ICM-20689)
    driver_ = ICM206xx::detect(device_bus_);
    if (driver_) {
        driver_->applyPreset(ImuPreset::FILTER_BALANCED);
        gyro_odr_hz_ = 8000;  // ICM206xx BALANCED uses bypass mode
        accel_odr_hz_ = 8000;
        goto detected;
    }

    // No IMU detected
    delete device_bus_;
    device_bus_ = nullptr;
    return Result::ERR;

detected:
    // Set default sensitivity values for ±2000dps/±16g (all presets use this)
    gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000;
    accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G;

    initialized_ = true;
    return Result::OK;
}

IMU::Result IMU::ApplyPreset(Preset preset)
{
    if (!initialized_ || !driver_) {
        return Result::ERR;
    }

    // Map IMU::Preset to ImuPreset (same order)
    ImuPreset driver_preset = static_cast<ImuPreset>(preset);

    // Apply preset and verify configuration (returns false if verification fails)
    if (!driver_->applyPreset(driver_preset)) {
        return Result::ERR;
    }

    // All presets use ±2000dps/±16g FSR
    gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000;
    accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G;

    // Update ODR tracking based on preset
    switch (preset) {
        case Preset::SAFE:
            gyro_odr_hz_ = 1000;
            accel_odr_hz_ = 1000;
            break;
        case Preset::SMOOTH:
        case Preset::BALANCED:
            gyro_odr_hz_ = 4000;
            accel_odr_hz_ = 1000;
            break;
        case Preset::ACRO:
            gyro_odr_hz_ = 8000;
            accel_odr_hz_ = 1000;
            break;
    }

    return Result::OK;
}

IMU::ChipType IMU::GetChipType()
{
    if (!initialized_ || !driver_) {
        return ChipType::UNKNOWN;
    }

    // driver stores WHO_AM_I in base class
    uint8_t who_am_i = driver_->whoAmI_;

    // Map WHO_AM_I value to ChipType enum
    switch (who_am_i) {
        case 0x12: return ChipType::ICM20602;
        case 0x47: return ChipType::ICM42688_P;
        case 0x68: return ChipType::MPU_6000;
        case 0x71: return ChipType::MPU_9250;
        case 0x98: return ChipType::ICM20689;
        case 0xAC: return ChipType::ICM20601;
        default:   return ChipType::UNKNOWN;
    }
}

int IMU::EnableDataReadyInt()
{
    if (!initialized_ || !driver_) return -1;
    driver_->enableDataReadyInt();
    return 0;
}

int IMU::DisableDataReadyInt()
{
    if (!initialized_ || !driver_) return -1;
    driver_->disableDataReadyInt();
    return 0;
}

// ============================================================================
// Tier 2 Extended API: FSR Configuration
// ============================================================================

IMU::Result IMU::SetGyroFSR_Ex(GyroFSR fsr)
{
    if (!initialized_ || !driver_) return Result::ERR;

    if (!driver_->setGyroFSR(fsr)) {
        return Result::ERR;
    }

    // Update sensitivity tracking based on FSR
    switch(fsr) {
        case GyroFSR::DPS_250:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_250;  break;
        case GyroFSR::DPS_500:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_500;  break;
        case GyroFSR::DPS_1000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_1000; break;
        case GyroFSR::DPS_2000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000; break;
    }
    return Result::OK;
}

IMU::Result IMU::SetAccelFSR_Ex(AccelFSR fsr)
{
    if (!initialized_ || !driver_) return Result::ERR;

    if (!driver_->setAccelFSR(fsr)) {
        return Result::ERR;
    }

    // Update sensitivity tracking based on FSR
    switch(fsr) {
        case AccelFSR::G_2:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_2G;  break;
        case AccelFSR::G_4:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_4G;  break;
        case AccelFSR::G_8:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_8G;  break;
        case AccelFSR::G_16: accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G; break;
    }
    return Result::OK;
}

// ============================================================================
// Tier 3 Extended API: Direct Register Access
// ============================================================================

uint8_t IMU::ReadReg_Ex(uint8_t reg)
{
    if (!initialized_ || !driver_) return 0;
    return driver_->readReg(reg);
}

IMU::Result IMU::WriteReg_Ex(uint8_t reg, uint8_t value)
{
    if (!initialized_ || !driver_) return Result::ERR;
    return driver_->writeReg(reg, value) ? Result::OK : Result::ERR;
}

IMU::Result IMU::WriteRegVerify_Ex(uint8_t reg, uint8_t value)
{
    if (!initialized_ || !driver_) return Result::ERR;
    return driver_->writeRegVerify(reg, value) ? Result::OK : Result::ERR;
}

int IMU::ReadIMU6(std::array<int16_t, 6>& buf)
{
    if (!initialized_ || !driver_) return -1;

    // Use driver's read method
    driver_->read(buf.data());
    return 0;
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
