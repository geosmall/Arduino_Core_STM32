/*
 * BoardConfig for the Matek F405-WMO (STM32F405RGT6).
 *
 * Derived from betaflight_target_converter.py output for Betaflight's
 * MATEKF405TE_SD config.h (generated 2026-10-31), then hand-edited.
 * Betaflight and INAV's MATEKF405TE_SD describe the whole F405-TE family;
 * this header is trimmed to the WMO's hardware per the Matek F405-WMO
 * mapping (https://www.mateksys.com/?portfolio=f405-wmo):
 *   - Outputs S1-S9 only. S10/S11 ({TIM13, PA6}, {TIM4, PB6}) are
 *     F405-WMN-only and are dropped.
 *   - Motors S1-S4 (TIM8, TIM1); servos S5-S9 (TIM2, TIM12). Betaflight
 *     lists S1-S8 as motors; this split covers quads and quad VTOLs.
 *   - UARTs 2/3/4/5 only. USART1 (PA9/PA10) and USART6 (PC6/PC7) are not
 *     brought out on the WMO.
 *   - Current scale 250 (Matek's INAV value; Betaflight's config.h has 150).
 *   - RC receiver at SBUS baud on USART2.
 *   - MAX7456 OSD chip select (shares SPI1 with the IMU) exposed as osd_cs_pin.
 *   - Motor::protocol declared explicitly (the converter only emits it as a
 *     comment for OneShot125).
 * Do not regenerate over this file without re-applying these edits.
 */

#pragma once

// Include ConfigTypes.h from targets/config directory
#include "config/ConfigTypes.h"

// UF2 bootloader config flash region (must match bootuf2 ports/stm32f4/boards.h)
#define BOARD_FLASH_CONFIG_START  0x08008000UL
#define BOARD_FLASH_CONFIG_SIZE   (16 * 1024)
#define BOARD_FLASH_CONFIG_ALIGN  4

// Application blob-storage flash region (xcos2c diagram blobs): the 16 KB
// between the config region above and the app slot (0x08010000, bootuf2
// ports/stm32f4/boards.h BOARD_FLASH_APP_START) — F405 sector 3, one erase
// unit. The bootloader neither reads nor erases it; a bootloader install's
// mass-erase clears it. Application-owned: record format and per-family
// erase/program live with the application, as for the config region.
#define BOARD_FLASH_BLOB_START  0x0800C000UL
#define BOARD_FLASH_BLOB_SIZE   (16 * 1024)
#define BOARD_FLASH_BLOB_ALIGN  8

// Board: MATEKF405TE_SD (Matek F405-WMO)
// Manufacturer: MTKS
// MCU: STM32F405
// Gyro: ICM42688P (INAV prints "ICM42605" for the whole 4260x/4268x family;
//       confirm via WHO_AM_I at bring-up)
namespace BoardConfig {
  // Storage: SD card on SPI2
  static constexpr StorageConfig storage{StorageBackend::SDFS, PC3, PC2, PB13, PC1, 8000000, SPI2};

  // IMU: ICM42688P on SPI1
  static constexpr SPIConfig imu_spi{PA7, PB4, PA5, PC14, 8000000, CS_Mode::SOFTWARE, SPI1};
  static constexpr IMUConfig imu{imu_spi, PC15, 1000000, IMUAlignment::CW270_DEG_FLIP};

  // MAX7456 OSD chip select. The OSD shares SPI1 with the IMU: drive this pin
  // HIGH (OSD deselected) before the first SPI1 transaction, or the OSD will
  // contend on MISO during IMU reads.
  static constexpr Pin osd_cs_pin = PB12;

  // I2C1: Environmental sensors (SPL06 baro at 0x76)
  static constexpr I2CConfig sensors{PB7, PB8, 400000, I2C1};

  // USART2: Serial port (RX2 / SBUS pad; see rc_receiver)
  static constexpr UARTConfig uart2{PA2, PA3, 115200, USART2};

  // USART3: Serial port
  static constexpr UARTConfig uart3{PC10, PC11, 115200, USART3};

  // UART4: Serial port
  static constexpr UARTConfig uart4{PA0, PA1, 115200, UART4};

  // UART5: Serial port
  static constexpr UARTConfig uart5{PC12, PD2, 115200, UART5};

  // ADC: Battery voltage and current monitoring.
  // Current scale 250 assumes the standard PDB shunt. PDBs with a shunt
  // marked "3m0" (3 mOhm, mis-fitted at SMT) need scale 2500 (13.2 A range).
  static constexpr ADCConfig battery{PC4, PC5, 210, 250};

  // Status LEDs (PA14 blue, PA13 green). These are the SWD pins; the WMO has
  // no SWD header, so they are free for GPIO.
  static constexpr LEDConfig status_leds{PA14, PA13};

  // RC Receiver: USART2 at SBUS baud. The SBUS pad is RX2 behind an onboard
  // hardware inverter, so no GPIO inverter control and no USART RX inversion.
  static constexpr RCReceiverConfig rc_receiver{PA3, PA2, 100000, 1000, 300, USART2};

  // Servo outputs - 50 Hz PWM for standard servos
  namespace Servo {
    static constexpr uint32_t frequency_hz = 50;

    // S5-S8 are the whole of TIM2 and S9 is TIM12, so no servo shares a
    // timer with a motor (one timer = one output frequency).
    static constexpr ServoConfig servos[] = {
      {TIM2, PB11, 4, 1000, 2000},   // S5: TIM2_CH4
      {TIM2, PB10, 3, 1000, 2000},   // S6: TIM2_CH3
      {TIM2, PB3, 2, 1000, 2000},    // S7: TIM2_CH2
      {TIM2, PA15, 1, 1000, 2000},   // S8: TIM2_CH1
      {TIM12, PB14, 1, 1000, 2000},  // S9: TIM12_CH1
    };

    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);
  };
  // Motors: ONESHOT125 protocol (125-250 µs)
  namespace Motor {
    static constexpr uint32_t frequency_hz = 2000;
    static constexpr Protocol protocol = Protocol::ONESHOT125;

    // Motor array - hardware timer assignments from Betaflight config.
    // S1-S4 only (TIM8 + TIM1): quad, or the lift motors of a quad VTOL.
    static constexpr MotorConfig motors[] = {
      {TIM8, PC9, 4, 125, 250, DMA2, 7, 7},  // S1: TIM8_CH4, DMA2 S7
      {TIM8, PC8, 3, 125, 250, DMA2, 2, 0},  // S2: TIM8_CH3, DMA2 S2
      {TIM1, PB15, 3, 125, 250, DMA2, 6, 6},  // S3: TIM1_CH3N, DMA2 S6
      {TIM1, PA8, 1, 125, 250, DMA2, 1, 6},  // S4: TIM1_CH1, DMA2 S1
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
