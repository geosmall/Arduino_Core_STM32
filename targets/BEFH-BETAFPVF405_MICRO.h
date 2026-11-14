/*
 * BEFH-BETAFPVF405_MICRO.h — BetaFPV F4 2-3S 20A AIO (BETAFPVF405), micro
 * tailsitter wiring.
 *
 * Board: the same circuit board as BEFH-BETAFPVF405.h (the Pavo Pico II):
 * STM32F405RG, 8 MHz HSE, IMU on SPI1, log flash on SPI2, four built-in ESCs,
 * built-in ELRS receiver on USART3. Same variant files and BootUF2 bootloader.
 *
 * Wiring modification (a wiring variant, xcos2c ARCH.md section 9): the T6 and
 * R6 pads, UART6 TX/RX on the stock board, carry the two elevon servo signals
 *   T6 = PC6 = TIM8_CH1 (AF3), left elevon
 *   R6 = PC7 = TIM8_CH2 (AF3), right elevon
 * at 50 Hz, servo power from the 5V pad beside them (doc/tailsitter/
 * TAILSITTER_CLONE_MICRO.md, connection map and servo PWM). UART6 is not
 * declared, so nothing initializes it on PC6/PC7. TIM8 is not used by the
 * DShot motors (TIM3, TIM2), so the servos have it alone. All four ESC
 * channels stay declared as motors; the airframe drives two of them.
 *
 * Unit in hand (hw_records/micro/): gyro ICM-42605 (WHO_AM_I 0x42), log flash
 * Puya PY25Q128HA (JEDEC 0x852018, 16 MB).
 *
 * Maintained by hand. BEFH-BETAFPVF405.h is generator output
 * (betaflight_target_converter.py); regenerating it does not reach this file,
 * so a change to the base header is copied here by hand.
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32f4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  4

// Application blob-storage flash region (xcos2c diagram blobs): F405 sector 3,
// as on BEFH-BETAFPVF405.h.
#define BOARD_FLASH_BLOB_START  0x0800C000UL
#define BOARD_FLASH_BLOB_SIZE   (16 * 1024)
#define BOARD_FLASH_BLOB_ALIGN  8

// Board: BETAFPVF405
// Manufacturer: BEFH
// MCU: STM32F405
namespace BoardConfig {
  // Storage: 16 MB SPI flash on SPI2 (Puya PY25Q128HA on the unit in hand)
  static constexpr StorageConfig storage{StorageBackend::LITTLEFS, PB15, PB14, PB13, PB12, 8000000, SPI2};

  // IMU on SPI1 (ICM-42605 on the unit in hand)
  static constexpr SPIConfig imu_spi{PA7, PA6, PA5, PA4, 8000000};
  static constexpr IMUConfig imu{imu_spi, PC4, 1000000, IMUAlignment::CW270_DEG};

  // I2C1: Environmental sensors
  static constexpr I2CConfig sensors{PB9, PB8, 400000, I2C1};

  // USART1: Serial port
  static constexpr UARTConfig uart1{PA9, PA10, 115200, USART1};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PB10, PB11, 115200, USART3};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PA0, PA1, 115200, UART4};

  // USART6 not declared: PC6/PC7 are the servo outputs below.

  // ADC: Battery voltage and current monitoring
  static constexpr ADCConfig battery{PC2, PC1, 110, 170};

  // Status LEDs
  static constexpr LEDConfig status_leds{PB5};

  // RC Receiver: USART3 (built-in ELRS)
  static constexpr RCReceiverConfig rc_receiver{PB11, PB10, 115200, 1000, 300, USART3};

  // Servo outputs: the elevons on the T6/R6 pads, TIM8 alone.
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    static constexpr ServoConfig servos[] = {
      {TIM8, PC6, 1, 1000, 2000},  // T6: TIM8_CH1, left elevon
      {TIM8, PC7, 2, 1000, 2000},  // R6: TIM8_CH2, right elevon
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
  // Motors: DSHOT600 (CHx + CHxN auto-detected by DShot lib), as on
  // BEFH-BETAFPVF405.h. All four built-in ESC channels.
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::DSHOT600;

    static constexpr MotorConfig motors[] = {
      {TIM3, PB0, 3, 125, 250, DMA1, 7, 5},  // Motor 1: TIM3_CH3, DMA1 S7
      {TIM3, PB1, 4, 125, 250, DMA1, 2, 5},  // Motor 2: TIM3_CH4, DMA1 S2
      {TIM2, PA3, 4, 125, 250, DMA1, 6, 3},  // Motor 3: TIM2_CH4, DMA1 S6
      {TIM2, PA2, 3, 125, 250, DMA1, 1, 3},  // Motor 4: TIM2_CH3, DMA1 S1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
