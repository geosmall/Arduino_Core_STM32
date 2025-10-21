/**
 * PWMOutputBank.h - Hardware timer-based PWM library for STM32 Arduino
 *
 * Provides 1 µs resolution PWM output for servo and ESC control using
 * explicit timer bank configuration to prevent frequency conflicts.
 *
 * Phase 1: Single timer, basic PWM output, runtime pulse width updates
 */

#ifndef PWMOUTPUTBANK_H
#define PWMOUTPUTBANK_H

#include <Arduino.h>
#include <HardwareTimer.h>

class PWMOutputBank {
public:
  /**
   * Constructor
   */
  PWMOutputBank();

  /**
   * Destructor
   */
  ~PWMOutputBank();

  /**
   * Initialize timer with specified frequency
   *
   * @param timer STM32 timer peripheral (TIM1, TIM2, TIM3, etc.)
   * @param frequency_hz PWM frequency in Hz (default: 50 Hz for servos)
   * @return true if initialization successful, false otherwise
   */
  bool Init(TIM_TypeDef *timer, uint32_t frequency_hz = 50);

  /**
   * Attach a PWM channel to a specific pin
   *
   * @param channel Timer channel number (1-4)
   * @param pin Arduino pin number
   * @param min_us Minimum pulse width in microseconds (default: 1000)
   * @param max_us Maximum pulse width in microseconds (default: 2000)
   * @return true if channel attached successfully, false otherwise
   */
  bool AttachChannel(uint32_t channel, uint32_t pin,
                     uint32_t min_us = 1000, uint32_t max_us = 2000);

  /**
   * Set pulse width for a channel in microseconds
   *
   * @param channel Timer channel number (1-4)
   * @param pulse_us Pulse width in microseconds
   */
  void SetPulseWidth(uint32_t channel, uint32_t pulse_us);

  /**
   * Write servo value (Arduino Servo library compatible)
   *
   * @param channel Timer channel number (1-4)
   * @param value Servo position: 0-180 degrees or 1000-2000 microseconds
   */
  void Write(uint32_t channel, int value);

  /**
   * Start PWM output on all configured channels
   */
  void Start();

  /**
   * Stop PWM output on all configured channels
   */
  void Stop();

  /**
   * Get current pulse width for a channel
   *
   * @param channel Timer channel number (1-4)
   * @return Current pulse width in microseconds
   */
  uint32_t GetPulseWidth(uint32_t channel);

  /**
   * Check if timer is initialized
   *
   * @return true if initialized, false otherwise
   */
  bool IsInitialized() const { return _initialized; }

private:
  /**
   * Calculate prescaler value for 1 MHz timer tick rate
   *
   * @return Prescaler value
   */
  uint32_t Calculate1MHzPrescaler();

  /**
   * Get timer clock frequency based on APB bus
   *
   * @return Timer clock frequency in Hz
   */
  uint32_t GetTimerClockFreq();

  /**
   * Channel configuration structure
   */
  struct ChannelConfig {
    uint32_t pin;           // Arduino pin number
    uint32_t channel;       // Timer channel (1-4)
    uint32_t min_us;        // Minimum pulse width
    uint32_t max_us;        // Maximum pulse width
    uint32_t current_us;    // Current pulse width
    bool active;            // Channel is configured
  };

  HardwareTimer *_timer;              // Hardware timer instance
  TIM_TypeDef *_timer_instance;       // Timer peripheral
  uint32_t _frequency_hz;             // PWM frequency
  uint32_t _period_us;                // Period in microseconds
  bool _initialized;                  // Initialization status
  ChannelConfig _channels[4];         // Up to 4 channels per timer
};

#endif // PWMOUTPUTBANK_H
