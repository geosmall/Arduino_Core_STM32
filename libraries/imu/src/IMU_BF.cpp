// Arduino_Core_STM32 IMU Library - Betaflight Driver Facade
// Implementation of high-level API

#include "IMU_BF.h"

// Gravity constant for m/s² conversion
static constexpr float GRAVITY = 9.80665f;

// Degrees to radians conversion (renamed to avoid Arduino core conflict)
static constexpr float IMU_DEG_TO_RAD = 0.017453292519943295f;

IMU_BF::IMU_BF()
    : bus_(nullptr)
    , icm42688_device_(nullptr)
    , mpu6000_device_(nullptr)
    , detected_type_(ImuType::Unknown)
{
}

IMU_BF::~IMU_BF()
{
    // Delete devices first (use bus)
    if (icm42688_device_ != nullptr) {
        delete icm42688_device_;
        icm42688_device_ = nullptr;
    }
    if (mpu6000_device_ != nullptr) {
        delete mpu6000_device_;
        mpu6000_device_ = nullptr;
    }

    // Then delete bus
    if (bus_ != nullptr) {
        delete bus_;
        bus_ = nullptr;
    }
}

void IMU_BF::attachSPI(SPIClass& spi, uint8_t csPin, uint32_t freq_hz)
{
    // Clean up existing bus if present
    if (bus_ != nullptr) {
        delete bus_;
    }

    // Create new SPI bus
    bus_ = new DeviceBusSPI(&spi, csPin);
    bus_->setFreq(freq_hz);
}

void IMU_BF::attachI2C(TwoWire& wire, uint8_t addr)
{
    // Clean up existing bus if present
    if (bus_ != nullptr) {
        delete bus_;
    }

    // Create new I2C bus
    bus_ = new DeviceBusI2C(&wire, addr);
}

bool IMU_BF::begin(ImuType type)
{
    // Validate bus is attached
    if (bus_ == nullptr) {
        return false;
    }

    // Clean up existing devices if present
    if (icm42688_device_ != nullptr) {
        delete icm42688_device_;
        icm42688_device_ = nullptr;
    }
    if (mpu6000_device_ != nullptr) {
        delete mpu6000_device_;
        mpu6000_device_ = nullptr;
    }

    // Handle auto-detection
    if (type == ImuType::Auto) {
        return autoDetect();
    }

    // Manual device type selection (future enhancement)
    // For now, only auto-detection is supported
    return autoDetect();
}

bool IMU_BF::autoDetect()
{
    // Try ICM42688 family detection (supports 0x42, 0x47, 0x56)
    icm42688_device_ = ICM42688_BF::detect(bus_);

    if (icm42688_device_ != nullptr) {
        // Map WHO_AM_I to ImuType enum
        switch (icm42688_device_->whoAmI_) {
            case 0x42:
                detected_type_ = ImuType::ICM42605;
                break;
            case 0x47:
                detected_type_ = ImuType::ICM42688P;
                break;
            case 0x56:
                detected_type_ = ImuType::IIM42653;
                break;
            default:
                detected_type_ = ImuType::Unknown;
                break;
        }
        return true;
    }

    // Try MPU6000 detection (WHO_AM_I = 0x68)
    mpu6000_device_ = MPU6000_BF::detect(bus_);
    if (mpu6000_device_ != nullptr) {
        detected_type_ = ImuType::MPU6000;
        return true;
    }

    // Future: Add MPU9250, ICM206xx detection here

    detected_type_ = ImuType::Unknown;
    return false;
}

bool IMU_BF::read(ImuSample& sample)
{
    // Read raw data: ax, ay, az, gx, gy, gz (int16_t)
    int16_t rawData[6];
    float accScale, gyrScale;

    // Dispatch to appropriate device
    if (icm42688_device_ != nullptr) {
        icm42688_device_->read(rawData);
        accScale = icm42688_device_->accScale_;
        gyrScale = icm42688_device_->gyrScale_;
    } else if (mpu6000_device_ != nullptr) {
        mpu6000_device_->read(rawData);
        accScale = mpu6000_device_->accScale_;
        gyrScale = mpu6000_device_->gyrScale_;
    } else {
        return false;  // No device initialized
    }

    // Convert to physical units using device scales
    // All devices store accScale_ in [G/LSB] and gyrScale_ in [dps/LSB]
    sample.ax = rawData[0] * accScale * GRAVITY;  // [m/s²]
    sample.ay = rawData[1] * accScale * GRAVITY;
    sample.az = rawData[2] * accScale * GRAVITY;

    sample.gx = rawData[3] * gyrScale * IMU_DEG_TO_RAD;  // [rad/s]
    sample.gy = rawData[4] * gyrScale * IMU_DEG_TO_RAD;
    sample.gz = rawData[5] * gyrScale * IMU_DEG_TO_RAD;

    sample.timestamp_us = micros();

    return true;
}

const char* IMU_BF::typeName() const
{
    // Return name from active device
    if (icm42688_device_ != nullptr) {
        return icm42688_device_->typeName();
    }
    if (mpu6000_device_ != nullptr) {
        return mpu6000_device_->typeName();
    }

    // Fallback to detected type
    switch (detected_type_) {
        case ImuType::ICM42688P: return "ICM42688P";
        case ImuType::ICM42605:  return "ICM42605";
        case ImuType::IIM42653:  return "IIM42653";
        case ImuType::MPU6000:   return "MPU6000";
        case ImuType::MPU9250:   return "MPU9250";
        case ImuType::Auto:      return "Auto";
        default:                 return "Unknown";
    }
}
