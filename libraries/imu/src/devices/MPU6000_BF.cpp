// Arduino_Core_STM32 IMU Library - MPU6000 Betaflight Driver Implementation
// Modified from Betaflight accgyro_spi_mpu6000.c

#include "MPU6000_BF.h"

// ============================================================================
// Register Map (from Betaflight)
// ============================================================================

#define MPU_RA_WHO_AM_I         0x75
#define MPU_RA_PRODUCT_ID       0x0C
#define MPU_RA_PWR_MGMT_1       0x6B
#define MPU_RA_PWR_MGMT_2       0x6C
#define MPU_RA_SIGNAL_PATH_RESET 0x68
#define MPU_RA_USER_CTRL        0x6A
#define MPU_RA_SMPLRT_DIV       0x19
#define MPU_RA_GYRO_CONFIG      0x1B
#define MPU_RA_ACCEL_CONFIG     0x1C
#define MPU_RA_INT_PIN_CFG      0x37
#define MPU_RA_INT_ENABLE       0x38
#define MPU_RA_ACCEL_XOUT_H     0x3B

// Bits
#define BIT_SLEEP               0x40
#define BIT_H_RESET             0x80
#define MPU_CLK_SEL_PLLGYROZ    0x03
#define BIT_I2C_IF_DIS          0x10
#define BIT_GYRO                0x04
#define BIT_ACC                 0x02
#define BIT_TEMP                0x01
#define MPU_RF_DATA_RDY_EN      (1 << 0)

// Full scale ranges
#define INV_FSR_2000DPS         0x03  // 2000 dps
#define INV_FSR_16G             0x03  // ±16g

// WHO_AM_I constant
#define MPU6000_WHO_AM_I_CONST  0x68

// Product ID revisions
#define MPU6000ES_REV_C4        0x14
#define MPU6000ES_REV_C5        0x15
#define MPU6000ES_REV_D6        0x16
#define MPU6000ES_REV_D7        0x17
#define MPU6000ES_REV_D8        0x18
#define MPU6000_REV_C4          0x54
#define MPU6000_REV_C5          0x55
#define MPU6000_REV_D6          0x56
#define MPU6000_REV_D7          0x57
#define MPU6000_REV_D8          0x58
#define MPU6000_REV_D9          0x59
#define MPU6000_REV_D10         0x5A

// ============================================================================
// MPU6000_BF Implementation
// ============================================================================

MPU6000_BF::MPU6000_BF(DeviceBus* bus, uint8_t whoAmI)
    : bus_(bus)
{
    whoAmI_ = whoAmI;
    // Device was already reset during detection

    // Clock Source PPL with Z axis gyro reference
    bus_->writeReg(MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
    delayMicroseconds(15);

    // Disable Primary I2C Interface
    bus_->writeReg(MPU_RA_USER_CTRL, BIT_I2C_IF_DIS);
    delayMicroseconds(15);

    // Enable all sensors
    bus_->writeReg(MPU_RA_PWR_MGMT_2, 0x00);
    delayMicroseconds(15);

    // Sample Rate = 1kHz / (1 + SMPLRT_DIV)
    // Set SMPLRT_DIV = 0 for 1kHz sampling
    bus_->writeReg(MPU_RA_SMPLRT_DIV, 0);
    delayMicroseconds(15);

    // Gyro +/- 2000 DPS Full Scale
    bus_->writeReg(MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
    delayMicroseconds(15);

    // Accel +/- 16 G Full Scale
    bus_->writeReg(MPU_RA_ACCEL_CONFIG, INV_FSR_16G << 3);
    delayMicroseconds(15);

    // INT_ANYRD_2CLEAR: Interrupt status cleared on any read
    bus_->writeReg(MPU_RA_INT_PIN_CFG, 1 << 4);
    delayMicroseconds(15);

    // Enable data ready interrupt
    bus_->writeReg(MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
    delayMicroseconds(15);

    // Set scale factors
    // MPU6000: ±2000 dps = 16.4 LSB/(dps), ±16g = 2048 LSB/g
    gyrScale_ = 1.0f / 16.4f;     // [dps/LSB]
    accScale_ = 1.0f / 2048.0f;   // [G/LSB]
    samplingRateHz_ = 1000;       // 1 kHz
}

MPU6000_BF* MPU6000_BF::detect(DeviceBus* bus)
{
    if (!bus) {
        return nullptr;
    }

    // Reset the device configuration
    bus->writeReg(MPU_RA_PWR_MGMT_1, BIT_H_RESET);
    delay(100);  // Datasheet specifies 100ms delay after reset

    // Reset the device signal paths
    bus->writeReg(MPU_RA_SIGNAL_PATH_RESET, BIT_GYRO | BIT_ACC | BIT_TEMP);
    delay(100);  // Datasheet specifies 100ms delay after signal path reset

    // Read WHO_AM_I register
    const uint8_t whoAmI = bus->readReg(MPU_RA_WHO_AM_I);
    delayMicroseconds(1);  // Ensure CS high time is met

    if (whoAmI != MPU6000_WHO_AM_I_CONST) {
        return nullptr;  // Not MPU6000
    }

    // Read product ID to verify chip revision
    const uint8_t productID = bus->readReg(MPU_RA_PRODUCT_ID);

    // Verify product revision
    bool validRevision = false;
    switch (productID) {
        case MPU6000ES_REV_C4:
        case MPU6000ES_REV_C5:
        case MPU6000_REV_C4:
        case MPU6000_REV_C5:
        case MPU6000ES_REV_D6:
        case MPU6000ES_REV_D7:
        case MPU6000ES_REV_D8:
        case MPU6000_REV_D6:
        case MPU6000_REV_D7:
        case MPU6000_REV_D8:
        case MPU6000_REV_D9:
        case MPU6000_REV_D10:
            validRevision = true;
            break;
    }

    if (!validRevision) {
        return nullptr;  // Unknown product revision
    }

    // Create and initialize device
    return new MPU6000_BF(bus, whoAmI);
}

void MPU6000_BF::read(int16_t* accgyr)
{
    // Read 14 bytes: AX_H, AX_L, AY_H, AY_L, AZ_H, AZ_L, TEMP_H, TEMP_L, GX_H, GX_L, GY_H, GY_L, GZ_H, GZ_L
    uint8_t buf[14];
    bus_->readRegs(MPU_RA_ACCEL_XOUT_H, buf, 14);

    // Parse big-endian data
    accgyr[0] = (int16_t)((buf[0] << 8) | buf[1]);   // Accel X
    accgyr[1] = (int16_t)((buf[2] << 8) | buf[3]);   // Accel Y
    accgyr[2] = (int16_t)((buf[4] << 8) | buf[5]);   // Accel Z
    // Skip temperature (buf[6-7])
    accgyr[3] = (int16_t)((buf[8] << 8) | buf[9]);   // Gyro X
    accgyr[4] = (int16_t)((buf[10] << 8) | buf[11]); // Gyro Y
    accgyr[5] = (int16_t)((buf[12] << 8) | buf[13]); // Gyro Z
}

const char* MPU6000_BF::typeName() const
{
    return "MPU6000";
}
