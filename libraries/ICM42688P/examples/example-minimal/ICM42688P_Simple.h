#ifndef ICM42688P_SIMPLE_H
#define ICM42688P_SIMPLE_H

#include <Arduino.h>
#include <SPI.h>

// ICM42688P register addresses
#define ICM42688P_WHO_AM_I     0x75
#define ICM42688P_DEVICE_ID    0x47

class ICM42688P_Simple {
public:
    // Constructor
    ICM42688P_Simple();

    // Simple initialization with SPI
    bool begin(SPIClass& spi, uint8_t cs_pin, uint32_t spi_speed = 1000000);

    // Read WHO_AM_I register
    uint8_t readWhoAmI();

    // Check if device is connected and responding
    bool isConnected();

    // Get SPI speed
    uint32_t getSPISpeed() const { return spi_speed_; }

private:
    SPIClass* spi_;
    uint8_t cs_pin_;
    uint32_t spi_speed_;
    bool initialized_;

    // Low-level SPI register read
    uint8_t readRegister(uint8_t reg);

    // Low-level SPI register write
    void writeRegister(uint8_t reg, uint8_t value);
};

#endif // ICM42688P_SIMPLE_H