#include "ICM42688P_Simple.h"

ICM42688P_Simple::ICM42688P_Simple()
    : spi_(nullptr), cs_pin_(0), spi_speed_(1000000), initialized_(false)
{
}

bool ICM42688P_Simple::begin(SPIClass& spi, uint8_t cs_pin, uint32_t spi_speed)
{
    spi_ = &spi;
    cs_pin_ = cs_pin;
    spi_speed_ = spi_speed;

    // Initialize CS pin
    pinMode(cs_pin_, OUTPUT);
    digitalWrite(cs_pin_, HIGH);

    // Initialize SPI
    spi_->begin(SPISettings(spi_speed_, MSBFIRST, SPI_MODE0));

    initialized_ = true;

    // Give device time to startup
    delay(10);

    return true;
}

uint8_t ICM42688P_Simple::readWhoAmI()
{
    if (!initialized_) {
        return 0;
    }

    return readRegister(ICM42688P_WHO_AM_I);
}

bool ICM42688P_Simple::isConnected()
{
    if (!initialized_) {
        return false;
    }

    uint8_t who_am_i = readWhoAmI();
    return (who_am_i == ICM42688P_DEVICE_ID);
}

uint8_t ICM42688P_Simple::readRegister(uint8_t reg)
{
    if (!spi_ || !initialized_) {
        return 0;
    }

    // ICM42688P SPI timing requirements:
    // CS setup time: 39ns minimum
    // CS hold time: 18ns minimum
    // Max SPI clock: 24MHz

    // Begin SPI transaction
    // spi_->beginTransaction(SPISettings(spi_speed_, MSBFIRST, SPI_MODE0));

    // Assert CS (active low)
    digitalWrite(cs_pin_, LOW);
    delayMicroseconds(1);  // CS setup delay (much more than 39ns required)

    // Send register address with read bit (bit 7 = 1)
    spi_->transfer(reg | 0x80);

    // Read data byte
    uint8_t value = spi_->transfer(0x00);

    // CS hold time and deassert
    delayMicroseconds(1);  // CS hold delay (much more than 18ns required)
    digitalWrite(cs_pin_, HIGH);

    // End SPI transaction
    // spi_->endTransaction();

    return value;
}

void ICM42688P_Simple::writeRegister(uint8_t reg, uint8_t value)
{
    if (!spi_ || !initialized_) {
        return;
    }

    // Begin SPI transaction
    // spi_->beginTransaction(SPISettings(spi_speed_, MSBFIRST, SPI_MODE0));

    // Assert CS (active low)
    digitalWrite(cs_pin_, LOW);
    delayMicroseconds(1);  // CS setup delay

    // Send register address (write bit = 0)
    spi_->transfer(reg);

    // Send data byte
    spi_->transfer(value);

    // CS hold time and deassert
    delayMicroseconds(1);  // CS hold delay
    digitalWrite(cs_pin_, HIGH);

    // End SPI transaction
    // spi_->endTransaction();
}