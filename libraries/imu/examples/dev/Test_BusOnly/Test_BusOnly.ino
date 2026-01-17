/*
 * IMU Library - Bus Abstraction Test
 *
 * Tests the DeviceBus abstraction layer by reading WHO_AM_I register
 * from an ICM42688P IMU over SPI.
 *
 * Hardware: NUCLEO_F411RE with ICM42688P on breadboard
 * Expected: WHO_AM_I = 0x47 (ICM42688P) or 0x4B (ICM42605)
 */

#include <SPI.h>
#include <libPrintf.h>

// Include bus abstraction from library src
#include "../../../src/bus/DeviceBusSPI.h"

// printf_() output routing
extern "C" void putchar_(char c) {
    Serial.write(c);
}

// Pin configuration for NUCLEO_F411RE
namespace BoardConfig {
  namespace imu {
    namespace spi {
      static constexpr uint32_t mosi_pin = PA7;
      static constexpr uint32_t miso_pin = PA6;
      static constexpr uint32_t sclk_pin = PA5;
      static constexpr uint32_t cs_pin = PA4;
    }
  }
}

// ICM42688P register addresses
#define ICM42688_WHO_AM_I_REG       0x75
#define ICM42688_WHO_AM_I_VALUE     0x47
#define ICM42605_WHO_AM_I_VALUE     0x4B
#define ICM42688_BANK_SEL           0x76

DeviceBusSPI *bus = nullptr;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== IMU Bus Abstraction Test ===");

  // Initialize SPI bus
  SPI.begin();
  printf_("SPI initialized (MOSI=%d, MISO=%d, SCLK=%d, CS=%d)\n",
          BoardConfig::imu::spi::mosi_pin,
          BoardConfig::imu::spi::miso_pin,
          BoardConfig::imu::spi::sclk_pin,
          BoardConfig::imu::spi::cs_pin);

  // Create bus interface
  bus = new DeviceBusSPI(&SPI, BoardConfig::imu::spi::cs_pin);
  bus->setFreq(1000000);  // 1MHz for initial communication
  Serial.println("DeviceBusSPI created (1MHz)");

  // Small delay for IMU power-up
  delay(100);

  // Select bank 0 (WHO_AM_I is in bank 0)
  bus->writeReg(ICM42688_BANK_SEL, 0x00);
  delay(1);

  // Read WHO_AM_I register
  uint8_t who_am_i = bus->readReg(ICM42688_WHO_AM_I_REG);

  printf_("WHO_AM_I register (0x%02X) = 0x%02X\n",
          ICM42688_WHO_AM_I_REG, who_am_i);

  // Verify chip ID
  if (who_am_i == ICM42688_WHO_AM_I_VALUE) {
    Serial.println("✓ ICM42688P detected");
    Serial.println("*TEST_PASS*");
  } else if (who_am_i == ICM42605_WHO_AM_I_VALUE) {
    Serial.println("✓ ICM42605 detected");
    Serial.println("*TEST_PASS*");
  } else {
    printf_("✗ Unknown/Invalid WHO_AM_I: 0x%02X (expected 0x47 or 0x4B)\n",
            who_am_i);
    Serial.println("*TEST_FAIL*");
  }

  // Test multi-byte read (read WHO_AM_I + BANK_SEL together)
  uint8_t regs[2];
  bus->readRegs(ICM42688_WHO_AM_I_REG, regs, 2);
  printf_("Multi-byte read: WHO_AM_I=0x%02X, BANK_SEL=0x%02X\n",
          regs[0], regs[1]);

  Serial.println("*STOP*");
}

void loop() {
  // Test complete, halt
}
