#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE + SD Card HIL Test Rig Configuration
// Hardware: STM32F411RE Nucleo with SD card module/breakout
// Validated: Standard SD/SDHC cards via SPI interface
// Uses Arduino pin macros for compatibility with existing code
namespace BoardConfig {
  // Storage: SD Card for SDFS (SPI module/breakout)
  static constexpr StorageConfig storage{StorageBackend::SDFS, PC12, PC11, PC10, PD2, 1000000};
  //                                     Backend        MOSI  MISO  SCLK  CS   Freq

  // IMU: Available SPI pins for sensors (if needed)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PC4};

  // GPS: UART communication (if needed)
  static constexpr UARTConfig gps{PA9, PA10, 115200};

  // RC Receiver: IBus on USART1 (RX=PA10, TX=PA9)
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 115200, 1000, 300};

  // I2C: Available for additional sensors (if needed)
  static constexpr I2CConfig sensors{PB9, PB8, 400000};
}