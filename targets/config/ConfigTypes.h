#pragma once

// Configuration type definitions for board-specific settings
// Uses Pin struct for type-safe pin identification

#include "Pin.h"
#include "stm32/pinmap.h"

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
    constexpr SPIConfig(Pin mosi, Pin miso, Pin sclk, Pin cs,
                       uint32_t frequency_hz = 1000000,
                       CS_Mode mode = CS_Mode::SOFTWARE)
      : mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk), cs_pin(cs),
        freq_hz(frequency_hz), cs_mode(mode) {}

    const Pin mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
    const CS_Mode cs_mode;

    // Helper: Get SSEL pin for SPIClass constructor
    // SW mode: returns NC_PIN (disables hardware SSEL)
    // HW mode: returns cs_pin (STM32 SPI peripheral controls CS)
    constexpr Pin get_ssel_pin() const {
      return (cs_mode == CS_Mode::HARDWARE) ? cs_pin : NC_PIN;
    }
  };

  struct UARTConfig {
    constexpr UARTConfig(Pin tx, Pin rx, uint32_t baud)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const Pin tx_pin, rx_pin;
    const uint32_t baud_rate;
  };

  struct I2CConfig {
    constexpr I2CConfig(Pin sda, Pin scl, uint32_t frequency_hz = 100000)
      : sda_pin(sda), scl_pin(scl), freq_hz(frequency_hz) {}

    const Pin sda_pin, scl_pin;
    const uint32_t freq_hz;
  };

  struct StorageConfig {
    constexpr StorageConfig(StorageBackend backend, Pin mosi, Pin miso,
                           Pin sclk, Pin cs, uint32_t frequency_hz = 1000000)
      : backend_type(backend), mosi_pin(mosi), miso_pin(miso), sclk_pin(sclk),
        cs_pin(cs), freq_hz(frequency_hz) {}

    const StorageBackend backend_type;
    const Pin mosi_pin, miso_pin, sclk_pin, cs_pin;
    const uint32_t freq_hz;
  };

  struct IMUConfig {
    constexpr IMUConfig(const SPIConfig& spi_config, Pin interrupt_pin = NC_PIN,
                       uint32_t setup_freq_hz = 0)
      : spi(spi_config), int_pin(interrupt_pin), setup_freq_hz(setup_freq_hz) {}

    const SPIConfig spi;
    const Pin int_pin;             // NC_PIN = no interrupt
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
    constexpr ADCConfig(Pin voltage_pin, Pin current_pin,
                       uint16_t voltage_scale, uint16_t current_scale)
      : voltage_pin(voltage_pin), current_pin(current_pin),
        voltage_scale(voltage_scale), current_scale(current_scale) {}

    const Pin voltage_pin;         // ADC pin for battery voltage
    const Pin current_pin;         // ADC pin for battery current
    const uint16_t voltage_scale;  // Betaflight vbat_scale (voltage divider ratio * 10)
    const uint16_t current_scale;  // Betaflight ibata_scale (current sensor sensitivity)
  };

  struct LEDConfig {
    constexpr LEDConfig(Pin pin1, Pin pin2 = NC_PIN)
      : led1_pin(pin1), led2_pin(pin2) {}

    const Pin led1_pin;   // Primary status LED
    const Pin led2_pin;   // Secondary status LED (NC_PIN = not present)
  };

  struct RCReceiverConfig {
    constexpr RCReceiverConfig(Pin rx, Pin tx, uint32_t baud,
                               uint32_t timeout_ms = 1000,
                               uint32_t idle_threshold_us = 300)
      : rx_pin(rx), tx_pin(tx), baud_rate(baud),
        timeout_ms(timeout_ms), idle_threshold_us(idle_threshold_us) {}

    const Pin rx_pin;                   // UART RX pin (receiver output)
    const Pin tx_pin;                   // UART TX pin (receiver input, usually unused)
    const uint32_t baud_rate;           // Protocol baudrate (115200=IBus, 100000=SBUS)
    const uint32_t timeout_ms;          // Failsafe timeout in milliseconds
    const uint32_t idle_threshold_us;   // Software idle detection threshold (0=disabled)
  };

  struct GPSConfig {
    constexpr GPSConfig(Pin tx, Pin rx, uint32_t baud = 9600)
      : tx_pin(tx), rx_pin(rx), baud_rate(baud) {}

    const Pin tx_pin;     // UART TX pin (to GPS RX)
    const Pin rx_pin;     // UART RX pin (from GPS TX)
    const uint32_t baud_rate; // GPS baudrate (typically 9600 or 115200)
  };

  // Motor output configuration (used by MotorManager and DShotOutput)
  // DMA fields are optional: when dma is nullptr, DShotOutput auto-resolves
  // DMA streams at runtime. When non-null, the specified DMA resource is used
  // directly (from Betaflight dma_opt translation or manual assignment).
  struct MotorConfig {
    TIM_TypeDef* timer;
    Pin pin;
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
    Pin pin;
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
