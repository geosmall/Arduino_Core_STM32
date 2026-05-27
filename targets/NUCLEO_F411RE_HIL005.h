#pragma once
#include "config/ConfigTypes.h"

// NUCLEO F411RE HIL-005 Test Rig Configuration
// Hardware: STM32F411RE Nucleo with breadboard peripherals
// Peripherals: MPU-6000 (SPI1), IBus Rx (USART1), DPS310 (I2C1), W25Q SPI flash (SPI2)
//
// Physical Wiring:
//   MPU-6000:  SPI1 (PA7/PA6/PA5/PA4), INT=PB3
//   IBus Rx:   USART1 (RX=PB7, TX=PB6)
//   DPS310:    I2C1 (SDA=PB9, SCL=PB8)
//   SPI Flash: SPI2 (PB15/PB14/PB13, CS=PB2)
//
namespace BoardConfig {
  // Storage: W25Q SPI flash on SPI2 (1 MHz for breadboard/jumper wires)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB2, 1000000};
  //                                                              MOSI  MISO  SCLK  CS   Freq

  // IMU: MPU-6000 on SPI1 (1 MHz for breadboard), interrupt on PB3
  // Chip alignment: CW0_DEG (bench breadboard, flat mount).
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 1000000};
  static constexpr IMUConfig imu{imu_spi, PB3, 1000000, IMUAlignment::CW0_DEG};

  // I2C1: DPS310 barometer and other sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000};

  // RC Receiver: IBus on USART1 (RX=PB7, TX=PB6)
  static constexpr RCReceiverConfig rc_receiver{PB7, PB6, 115200, 1000, 300};

  // GPS: USART2 (Nucleo Arduino header D1/D0)
  static constexpr UARTConfig gps{PA2, PA3, 9600};

  // Status LED
  static constexpr LEDConfig status_leds{PC13};

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PA0, PA1, 110, 170};
}
