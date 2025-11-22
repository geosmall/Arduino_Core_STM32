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

    // Detect and initialize ICM42688 using driver
    driver_ = ICM42688::detect(device_bus_);
    if (!driver_) {
        delete device_bus_;
        device_bus_ = nullptr;
        return Result::ERR;
    }

    // Apply default BALANCED preset (recommended for 2kHz PID loop)
    driver_->applyPreset(ImuPreset::FILTER_BALANCED);

    // Set default sensitivity values for ±2000dps/±16g (preset default)
    gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000;
    accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G;

    // Track ODR from BALANCED preset (4kHz gyro, 1kHz accel)
    gyro_odr_hz_ = 4000;
    accel_odr_hz_ = 1000;

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

IMU::Result IMU::ConfigureInvDevice(AccelFS acc_fsr_g, GyroFS gyr_fsr_dps,
                                     AccelODR acc_freq, GyroODR gyr_freq)
{
    // Legacy API - deprecated, use ApplyPreset() instead
    // This method is kept for backward compatibility but does minimal configuration
    if (!initialized_) {
        return Result::ERR;
    }

    // Update sensitivity values based on FSR selection
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

    // Note: Actual register configuration now handled by preset system
    // This legacy API updates tracking only - use ApplyPreset() for full config
    return Result::OK;
}

int IMU::Reset()
{
    // Reset not implemented in driver - use Init() for full re-initialization
    if (!initialized_) return -1;
    return 0;
}

int IMU::SetPwrState(PwrState state)
{
    // Power state management handled internally by driver
    // Sensors are enabled during detect() and preset application
    if (!initialized_) return -1;
    (void)state;
    return 0;
}

int IMU::EnableAccelLNMode()
{
    // Accel is enabled in Low Noise mode by default
    if (!initialized_) return -1;
    return 0;
}

int IMU::DisableAccel()
{
    // Not implemented - use power management at application level
    if (!initialized_) return -1;
    return 0;
}

int IMU::EnableGyroLNMode()
{
    // Gyro is enabled in Low Noise mode by default
    if (!initialized_) return -1;
    return 0;
}

int IMU::DisableGyro()
{
    // Not implemented - use power management at application level
    if (!initialized_) return -1;
    return 0;
}

int IMU::SetAccelODR(AccelODR frequency)
{
    // Legacy API - ODR now set via ApplyPreset()
    if (!initialized_) return -1;

    // Track ODR for reference
    switch (frequency) {
        case accel_odr500: accel_odr_hz_ = 500; break;
        case accel_odr1k:  accel_odr_hz_ = 1000; break;
        case accel_odr2k:  accel_odr_hz_ = 2000; break;
        case accel_odr4k:  accel_odr_hz_ = 4000; break;
        case accel_odr8k:  accel_odr_hz_ = 8000; break;
        default: accel_odr_hz_ = 0; break;
    }
    return 0;
}

int IMU::SetGyroODR(GyroODR frequency)
{
    // Legacy API - ODR now set via ApplyPreset()
    if (!initialized_) return -1;

    // Track ODR for reference
    switch (frequency) {
        case gyr_odr500: gyro_odr_hz_ = 500; break;
        case gyr_odr1k:  gyro_odr_hz_ = 1000; break;
        case gyr_odr2k:  gyro_odr_hz_ = 2000; break;
        case gyr_odr4k:  gyro_odr_hz_ = 4000; break;
        case gyr_odr8k:  gyro_odr_hz_ = 8000; break;
        default: gyro_odr_hz_ = 0; break;
    }
    return 0;
}

int IMU::SetAccelFSR(AccelFS fsr)
{
    // Legacy API - FSR now fixed at ±16g via ApplyPreset()
    if (!initialized_) return -1;

    // Update sensitivity tracking
    switch (fsr) {
        case gpm2:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_2G;  break;
        case gpm4:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_4G;  break;
        case gpm8:  accel_sensitivity_ = ICM42688P_ACCEL_SENS_8G;  break;
        case gpm16: accel_sensitivity_ = ICM42688P_ACCEL_SENS_16G; break;
    }
    return 0;
}

int IMU::SetGyroFSR(GyroFS fsr)
{
    // Legacy API - FSR now fixed at ±2000dps via ApplyPreset()
    if (!initialized_) return -1;

    // Update sensitivity tracking
    switch (fsr) {
        case dps250:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_250;  break;
        case dps500:  gyro_sensitivity_ = ICM42688P_GYRO_SENS_500;  break;
        case dps1000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_1000; break;
        case dps2000: gyro_sensitivity_ = ICM42688P_GYRO_SENS_2000; break;
    }
    return 0;
}

// ============================================================================
// Advanced Filter Configuration (ICM-42688-P only)
// ============================================================================

// AAF lookup table for ICM-42688-P (from Betaflight)
// Index: 0=258Hz, 1=536Hz, 2=997Hz, 3=1962Hz
static const ICM42688::AAFConfig aafPresets[] = {
    {  6,   36, 10 },  // 0: 258 Hz (Betaflight default)
    { 12,  144,  8 },  // 1: 536 Hz
    { 21,  440,  6 },  // 2: 997 Hz
    { 37, 1376,  4 },  // 3: 1962 Hz
};
static constexpr uint8_t AAF_PRESET_COUNT = sizeof(aafPresets) / sizeof(aafPresets[0]);

int IMU::SetGyroAAF(uint8_t aaf_index)
{
    if (!initialized_ || !driver_) return -1;
    if (aaf_index >= AAF_PRESET_COUNT) return -1;

    driver_->setGyroAAF(aafPresets[aaf_index]);
    return 0;
}

int IMU::SetAccelAAF(uint8_t aaf_index)
{
    if (!initialized_ || !driver_) return -1;
    if (aaf_index >= AAF_PRESET_COUNT) return -1;

    driver_->setAccelAAF(aafPresets[aaf_index]);
    return 0;
}

int IMU::SetUIFilters(uint8_t gyro_bw, uint8_t accel_bw, uint8_t gyro_order, uint8_t accel_order)
{
    if (!initialized_ || !driver_) return -1;

    // Validate parameters
    if (gyro_bw > 15 || accel_bw > 15) return -1;
    if (gyro_order < 1 || gyro_order > 3) return -1;
    if (accel_order < 1 || accel_order > 3) return -1;

    driver_->setUIFilters(gyro_bw, accel_bw, gyro_order, accel_order);
    return 0;
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
        case 0x47: return ChipType::ICM42688_P;
        case 0x68: return ChipType::MPU_6000;
        case 0x71: return ChipType::MPU_9250;
        default:   return ChipType::UNKNOWN;
    }
}

int IMU::EnableDataReadyInt1()
{
    if (!initialized_ || !driver_) return -1;
    driver_->enableDataReadyInt1();
    return 0;
}

int IMU::DisableDataReadyInt1()
{
    if (!initialized_ || !driver_) return -1;
    driver_->disableDataReadyInt1();
    return 0;
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
