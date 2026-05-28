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

// Motor protocol selection — per-board via BoardConfig::Motor::protocol
enum class Protocol {
    ONESHOT125,  // OneShot125 PWM, 125-250 µs pulse (uses MotorConfig::min_us/max_us)
    DSHOT600     // DShot600 digital, 0=disarm / 48-2047 throttle (range protocol-defined)
};

// IMU chip-to-board alignment — mirrors Betaflight's 8 cardinal sensor_align_e values.
// Per-target compile-time constant; consumed by BoardAlignment to build the
// sensor-to-board rotation matrix R_sensor_to_board. Composed at init with the
// user-configured board-to-vehicle alignment into a single hot-path matrix.
// Non-90°-boundary mounts (Betaflight's ALIGN_CUSTOM) are out of scope here.
enum class IMUAlignment : uint8_t {
    CW0_DEG,         // No rotation
    CW90_DEG,        // 90° clockwise about Z (yaw)
    CW180_DEG,       // 180° about Z
    CW270_DEG,       // 270° clockwise about Z
    CW0_DEG_FLIP,    // 180° roll then no yaw
    CW90_DEG_FLIP,   // 180° roll then 90° yaw
    CW180_DEG_FLIP,  // 180° roll then 180° yaw
    CW270_DEG_FLIP   // 180° roll then 270° yaw
};

namespace BoardConfig {
  // Aggregate (no user-defined ctor) so `static constexpr SPIConfig x{..., SPI1}`
  // compiles. CMSIS peripheral macros (`SPI1`, `SPI3`, ...) expand to
  // `reinterpret_cast<SPI_TypeDef*>(SPI<n>_BASE)`, which is not a core
  // constant expression — a user-defined constexpr ctor receiving such a
  // value rejects compile-time initialization. Aggregate brace-init does
  // not, matching the precedent set by MotorConfig + `TIM1`.
  //
  // `instance` disambiguates multi-mapping pins (e.g. G473 PB3/4/5, which
  // carry both SPI1+AF5 and SPI3+AF6 entries in PinMap_SPI_*). nullptr
  // preserves legacy first-match resolution — kept as a default member
  // initializer so existing target headers compile unchanged. See
  // Arduino_Core_STM32/doc/PIN_USE.md for the peripheral-aware pin model.
  struct SPIConfig {
    Pin mosi_pin;
    Pin miso_pin;
    Pin sclk_pin;
    Pin cs_pin;
    uint32_t freq_hz = 1000000;
    SPI_TypeDef* instance = nullptr;
    CS_Mode cs_mode = CS_Mode::SOFTWARE;

    // Helper: Get SSEL pin for SPIClass constructor
    // SW mode: returns NC_PIN (disables hardware SSEL)
    // HW mode: returns cs_pin (STM32 SPI peripheral controls CS)
    constexpr Pin get_ssel_pin() const {
      return (cs_mode == CS_Mode::HARDWARE) ? cs_pin : NC_PIN;
    }
  };

  // Aggregate (no user-defined ctor) — see SPIConfig for the rationale.
  // `instance` disambiguates multi-mapping UART pins; nullptr preserves
  // legacy first-match resolution. The USART_TypeDef* alias also accepts
  // LPUART1/LPUART2 (CMSIS declares them as (USART_TypeDef*)<base>).
  struct UARTConfig {
    Pin tx_pin;
    Pin rx_pin;
    uint32_t baud_rate;
    USART_TypeDef* instance = nullptr;
  };

  // Aggregate (no user-defined ctor) — see SPIConfig for the rationale.
  // `instance` disambiguates multi-mapping I2C pins; nullptr preserves
  // legacy first-match resolution for backward compat.
  struct I2CConfig {
    Pin sda_pin;
    Pin scl_pin;
    uint32_t freq_hz = 100000;
    I2C_TypeDef* instance = nullptr;
  };

  // Aggregate (no user-defined ctor) — see SPIConfig for the rationale.
  struct StorageConfig {
    StorageBackend backend_type;
    Pin mosi_pin;
    Pin miso_pin;
    Pin sclk_pin;
    Pin cs_pin;
    uint32_t freq_hz = 1000000;
    SPI_TypeDef* instance = nullptr;
  };

  struct IMUConfig {
    constexpr IMUConfig(const SPIConfig& spi_config, Pin interrupt_pin = NC_PIN,
                       uint32_t setup_freq_hz = 0,
                       IMUAlignment chip_alignment = IMUAlignment::CW0_DEG)
      : spi(spi_config), int_pin(interrupt_pin), setup_freq_hz(setup_freq_hz),
        alignment(chip_alignment) {}

    const SPIConfig spi;
    const Pin int_pin;             // NC_PIN = no interrupt
    const uint32_t setup_freq_hz;  // 0 = use spi.freq_hz for setup (slow initialization)
    const IMUAlignment alignment;  // Chip-to-board rotation (Betaflight GYRO_x_ALIGN)

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

  // Aggregate (no user-defined ctor) — see SPIConfig / UARTConfig for the
  // rationale on the `instance` field.
  struct RCReceiverConfig {
    Pin rx_pin;                   // UART RX pin (receiver output)
    Pin tx_pin;                   // UART TX pin (receiver input, usually unused)
    uint32_t baud_rate;           // Protocol baudrate (115200=IBus, 100000=SBUS)
    uint32_t timeout_ms = 1000;   // Failsafe timeout in milliseconds
    uint32_t idle_threshold_us = 300;  // Software idle detection threshold (0=disabled)
    USART_TypeDef* instance = nullptr;
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

// Motor abstraction for runtime timer grouping with protocol dispatch.
//
// Init(motors, count, frequency_hz, protocol) selects the backend at startup.
// SetMotor(idx, scaled_0_1) is protocol-aware: caller passes a normalized
// 0..1 float; backend converts to OneShot125 µs (min_us..max_us) or DShot
// throttle (48..2047 with 0 = disarm). Update() pushes pending values
// (required for DShot; no-op for OneShot125). Disarm() drives all motors
// to safe state.
#include <PWMOutputBank.h>
#include <DShot.h>

class MotorManager {
private:
  static constexpr int MAX_TIMERS = 4;
  static constexpr int MAX_MOTORS = 8;

  // OneShot125 backend
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
    uint32_t min_us;
    uint32_t max_us;
  };
  MotorInfo motor_info[MAX_MOTORS];
  int num_motors;

  // DShot backend — always embedded, only initialized when protocol == DSHOT600
  DShotOutput dshot;

  Protocol protocol;

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
  MotorManager() : num_banks(0), num_motors(0), protocol(Protocol::ONESHOT125) {}

  template<typename MotorArray>
  bool Init(const MotorArray& motors, int count, uint32_t frequency_hz,
            Protocol proto = Protocol::ONESHOT125) {
    if (count > MAX_MOTORS) return false;
    num_motors = count;
    protocol = proto;

    if (proto == Protocol::ONESHOT125) {
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
        motor_info[i].min_us = motor.min_us;
        motor_info[i].max_us = motor.max_us;
      }
      return true;
    }
    if (proto == Protocol::DSHOT600) {
      return dshot.Init(motors, count, DShot::DSHOT600);
    }
    return false;
  }

  // Protocol-aware motor command.
  // scaled_0_1: normalized 0..1 throttle. Values outside [0,1] are clamped.
  //   OneShot125: 0 -> min_us pulse, 1 -> max_us pulse, linear in between.
  //   DSHOT600  : 0 -> throttle 0 (disarm command), >0 -> 48 + scaled*(2047-48).
  void SetMotor(int motor_idx, float scaled_0_1) {
    if (motor_idx < 0 || motor_idx >= num_motors) return;
    if (scaled_0_1 < 0.0f) scaled_0_1 = 0.0f;
    if (scaled_0_1 > 1.0f) scaled_0_1 = 1.0f;

    if (protocol == Protocol::ONESHOT125) {
      const auto& info = motor_info[motor_idx];
      uint32_t pulse_us = info.min_us +
                          (uint32_t)(scaled_0_1 * (info.max_us - info.min_us) + 0.5f);
      banks[info.bank_index].pwm.SetPulseWidth(info.channel, pulse_us);
    } else if (protocol == Protocol::DSHOT600) {
      uint16_t throttle = (scaled_0_1 <= 0.0f)
                          ? 0
                          : (uint16_t)(48.0f + scaled_0_1 * (2047.0f - 48.0f) + 0.5f);
      dshot.SetThrottle(motor_idx, throttle);
    }
  }

  // Push pending values to motors. Required for DShot (sends one frame).
  // No-op for OneShot125 — TimerPWM CCR registers latch automatically.
  void Update() {
    if (protocol == Protocol::DSHOT600) {
      dshot.Send();
    }
  }

  // Drive all motors to safe/disarmed state.
  //   OneShot125: writes min_us pulse to every motor.
  //   DSHOT600  : sends throttle 0 (disarm) to every motor and pushes a frame.
  void Disarm() {
    if (protocol == Protocol::ONESHOT125) {
      for (int i = 0; i < num_motors; i++) {
        const auto& info = motor_info[i];
        banks[info.bank_index].pwm.SetPulseWidth(info.channel, info.min_us);
      }
    } else if (protocol == Protocol::DSHOT600) {
      dshot.Disarm();
      dshot.Send();
    }
  }

  // Legacy alias for code that still calls ArmAll(). Same effect as Disarm().
  void ArmAll() { Disarm(); }

  int GetNumMotors() const { return num_motors; }
  Protocol GetProtocol() const { return protocol; }
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
