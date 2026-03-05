#pragma once

// Configuration type definitions for board-specific settings
// Uses Arduino pin macros for compatibility with existing code

// Include STM32 pinmap for NP definition (No Pin)
#include "stm32/pinmap.h"
// Include pins_arduino for PNUM_NOT_DEFINED
#include "pins_arduino.h"

// Storage backend types
enum class StorageBackend {
    NONE,      // No storage hardware attached
    LITTLEFS,  // SPI flash storage
    SDFS       // SD card storage
};


// SPI chip select control modes
enum class CS_Mode {
    SOFTWARE,  // Software-controlled CS pin
    HARDWARE   // Hardware-controlled CS via SPI peripheral
};

namespace BoardConfig {
  struct SPIConfig {
    constexpr SPIConfig(uint32_t mosi, uint32_t miso, uint32_t sclk, uint32_t cs,
                       uint32_t frequency_hz = 1000000,
                       CS_Mode mode = CS_Mode::SOFTWARE)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        freq_hz(frequency_hz), cs_mode(mode) {}

    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
    const CS_Mode cs_mode;

    // Helper: Get SSEL pin for SPIClass constructor
    // SW mode: returns PNUM_NOT_DEFINED (disables hardware SSEL)
    // HW mode: returns cs_pin (STM32 SPI peripheral controls CS)
    constexpr uint32_t get_ssel_pin() const {
      return (cs_mode == CS_Mode::HARDWARE) ? cs_pin : PNUM_NOT_DEFINED;
    }
  };

  struct UARTConfig {
    constexpr UARTConfig(uint32_t tx, uint32_t rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint32_t tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(uint32_t sda, uint32_t scl, uint32_t frequency_hz = 100000)
      : sda_pin(sda), scl_pin(scl), freq_hz(frequency_hz) {}

    const uint32_t sda_pin, scl_pin;
    const uint32_t freq_hz;
  };

  struct StorageConfig {
    constexpr StorageConfig(StorageBackend backend, uint32_t mosi, uint32_t miso,
                           uint32_t sclk, uint32_t cs, uint32_t frequency_hz = 1000000)
      : backend_type(backend), mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk),
        cs_pin(cs), freq_hz(frequency_hz) {}

    const StorageBackend backend_type;
    const uint32_t mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
  };

  struct IMUConfig {
    constexpr IMUConfig(const SPIConfig& spi_config, uint32_t interrupt_pin = 0,
                       uint32_t setup_freq_hz = 0)
      : spi(spi_config), int_pin(interrupt_pin), setup_freq_hz(setup_freq_hz) {}

    const SPIConfig spi;
    const uint32_t int_pin;        // 0 = no interrupt
    const uint32_t setup_freq_hz;  // 0 = use spi.freq_hz for setup (slow initialization)

    // Helper: Get effective setup frequency (slow initialization)
    constexpr uint32_t get_setup_freq() const {
      return (setup_freq_hz > 0) ? setup_freq_hz : spi.freq_hz;
    }

    // Helper: Get effective runtime frequency (normal operation)
    constexpr uint32_t get_runtime_freq() const {
      return spi.freq_hz;  // Always use the SPI config frequency for runtime
    }
  };

  struct ADCConfig {
    constexpr ADCConfig(uint32_t voltage_pin, uint32_t current_pin,
                       uint16_t voltage_scale, uint16_t current_scale)
      : voltage_pin(voltage_pin), current_pin(current_pin),
        voltage_scale(voltage_scale), current_scale(current_scale) {}

    const uint32_t voltage_pin;    // ADC pin for battery voltage
    const uint32_t current_pin;    // ADC pin for battery current
    const uint16_t voltage_scale;  // Betaflight vbat_scale (voltage divider ratio * 10)
    const uint16_t current_scale;  // Betaflight ibata_scale (current sensor sensitivity)
  };

  struct LEDConfig {
    constexpr LEDConfig(uint32_t pin1, uint32_t pin2 = 0)
      : led1_pin(pin1), led2_pin(pin2) {}

    const uint32_t led1_pin;  // Primary status LED
    const uint32_t led2_pin;  // Secondary status LED (0 = not present)
  };

  struct RCReceiverConfig {
    constexpr RCReceiverConfig(uint32_t rx, uint32_t tx, uint32_t baud,
                               uint32_t timeout_ms = 1000,
                               uint32_t idle_threshold_us = 300)
      : rx_pin(rx), tx_pin(tx), baud_rate(baud),
        timeout_ms(timeout_ms), idle_threshold_us(idle_threshold_us) {}

    const uint32_t rx_pin;              // UART RX pin (receiver output)
    const uint32_t tx_pin;              // UART TX pin (receiver input, usually unused)
    const uint32_t baud_rate;           // Protocol baudrate (115200=IBus, 100000=SBUS)
    const uint32_t timeout_ms;          // Failsafe timeout in milliseconds
    const uint32_t idle_threshold_us;   // Software idle detection threshold (0=disabled)
  };

  struct GPSConfig {
    constexpr GPSConfig(uint32_t tx, uint32_t rx, uint32_t baud = 9600)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const uint32_t tx_pin;    // UART TX pin (to GPS RX)
    const uint32_t rx_pin;    // UART RX pin (from GPS TX)
    const uint32_t baud_rate; // GPS baudrate (typically 9600 or 115200)
  };

  // Motor output configuration (used by MotorManager and DShotOutput)
  // DMA fields are optional: when dma is nullptr, DShotOutput auto-resolves
  // DMA streams at runtime. When non-null, the specified DMA resource is used
  // directly (from Betaflight dma_opt translation or manual assignment).
  struct MotorConfig {
    TIM_TypeDef* timer;
    uint32_t pin;
    uint32_t channel;        // Timer channel (1-4)
    uint32_t min_us;         // Min pulse width (PWM protocols)
    uint32_t max_us;         // Max pulse width (PWM protocols)
    DMA_TypeDef* dma;        // DMA controller (nullptr = auto-resolve)
    uint32_t dma_stream;     // Stream/channel number (0-7)
    uint32_t dma_channel_sel; // F4/F7 channel select index (0-7, ignored on G4/H7)
  };

  // Servo output configuration (used by ServoManager)
  struct ServoConfig {
    TIM_TypeDef* timer;
    uint32_t pin;
    uint32_t channel;        // Timer channel (1-4)
    uint32_t min_us;         // Min pulse width (typically 1000)
    uint32_t max_us;         // Max pulse width (typically 2000)
  };
}

// Motor abstraction for runtime timer grouping
#include <PWMOutputBank.h>

class MotorManager {
private:
  static constexpr int MAX_TIMERS = 4;
  static constexpr int MAX_MOTORS = 8;

  struct TimerBank {
    TIM_TypeDef* timer;
    PWMOutputBank pwm;
    bool initialized;
  };

  TimerBank banks[MAX_TIMERS];
  int num_banks;

  struct MotorInfo {
    int bank_index;
    uint32_t channel;
  };
  MotorInfo motor_info[MAX_MOTORS];
  int num_motors;

  int FindOrCreateBank(TIM_TypeDef* timer) {
    for (int i = 0; i < num_banks; i++) {
      if (banks[i].timer == timer) return i;
    }
    if (num_banks < MAX_TIMERS) {
      banks[num_banks].timer = timer;
      banks[num_banks].initialized = false;
      return num_banks++;
    }
    return -1;
  }

public:
  MotorManager() : num_banks(0), num_motors(0) {}

  template<typename MotorArray>
  bool Init(const MotorArray& motors, int count, uint32_t frequency_hz) {
    if (count > MAX_MOTORS) return false;
    num_motors = count;

    for (int i = 0; i < count; i++) {
      const auto& motor = motors[i];
      int bank_idx = FindOrCreateBank(motor.timer);
      if (bank_idx < 0) return false;

      if (!banks[bank_idx].initialized) {
        banks[bank_idx].pwm.Init(motor.timer, frequency_hz);
        banks[bank_idx].initialized = true;
      }

      banks[bank_idx].pwm.AttachChannel(motor.channel, motor.pin, motor.min_us, motor.max_us);
      motor_info[i].bank_index = bank_idx;
      motor_info[i].channel = motor.channel;
    }
    return true;
  }

  void SetMotor(int motor_idx, uint32_t pulse_width_us) {
    if (motor_idx < 0 || motor_idx >= num_motors) return;
    banks[motor_info[motor_idx].bank_index].pwm.SetPulseWidth(
      motor_info[motor_idx].channel, pulse_width_us
    );
  }

  void ArmAll(uint32_t min_pulse_us = 125) {
    for (int i = 0; i < num_motors; i++) {
      SetMotor(i, min_pulse_us);
    }
  }

  int GetNumMotors() const { return num_motors; }
};

// Servo abstraction for runtime timer grouping
class ServoManager {
private:
  static constexpr int MAX_TIMERS = 4;
  static constexpr int MAX_SERVOS = 8;

  struct TimerBank {
    TIM_TypeDef* timer;
    PWMOutputBank pwm;
    bool initialized;
  };

  TimerBank banks[MAX_TIMERS];
  int num_banks;

  struct ServoInfo {
    int bank_index;
    uint32_t channel;
  };
  ServoInfo servo_info[MAX_SERVOS];
  int num_servos;

  int FindOrCreateBank(TIM_TypeDef* timer) {
    for (int i = 0; i < num_banks; i++) {
      if (banks[i].timer == timer) return i;
    }
    if (num_banks < MAX_TIMERS) {
      banks[num_banks].timer = timer;
      banks[num_banks].initialized = false;
      return num_banks++;
    }
    return -1;
  }

public:
  ServoManager() : num_banks(0), num_servos(0) {}

  template<typename ServoArray>
  bool Init(const ServoArray& servos, int count, uint32_t frequency_hz) {
    if (count > MAX_SERVOS) return false;
    num_servos = count;

    for (int i = 0; i < count; i++) {
      const auto& servo = servos[i];
      int bank_idx = FindOrCreateBank(servo.timer);
      if (bank_idx < 0) return false;

      if (!banks[bank_idx].initialized) {
        banks[bank_idx].pwm.Init(servo.timer, frequency_hz);
        banks[bank_idx].initialized = true;
      }

      banks[bank_idx].pwm.AttachChannel(servo.channel, servo.pin, servo.min_us, servo.max_us);
      servo_info[i].bank_index = bank_idx;
      servo_info[i].channel = servo.channel;
    }
    return true;
  }

  void SetServo(int servo_idx, uint32_t pulse_width_us) {
    if (servo_idx < 0 || servo_idx >= num_servos) return;
    banks[servo_info[servo_idx].bank_index].pwm.SetPulseWidth(
      servo_info[servo_idx].channel, pulse_width_us
    );
  }

  void SetAllServos(uint32_t pulse_width_us) {
    for (int i = 0; i < num_servos; i++) {
      SetServo(i, pulse_width_us);
    }
  }

  int GetNumServos() const { return num_servos; }
};