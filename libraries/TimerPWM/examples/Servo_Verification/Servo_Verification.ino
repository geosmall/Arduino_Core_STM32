/**
 * Servo_Verification.ino - Servo PWM verification example
 *
 * Verifies servo PWM output using TIM2 Input Capture to measure TIM3 PWM output.
 * Repurposes Motor4 pin from JHEF411 config for servo testing at 50 Hz.
 * No oscilloscope or logic analyzer needed - just a jumper wire!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE with JHEF411 config
 * - PWM Output: PB0 (Arduino A3, TIM3_CH3 - repurposed from Motor4)
 * - Input Capture: PB10 (Arduino D6, TIM2_CH3)
 * - Connect jumper wire: A3 → D6 (same as motor_pwm_verification Motor4 test)
 *
 * This example validates:
 * - PWM frequency (50 Hz ± 2%) via period measurement
 * - Pulse width set to 1500 µs (center position, not measured)
 */

#include <PWMOutputBank.h>
#include <ci_log.h>
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"

// Servo configuration (repurposed from Motor4)
namespace ServoConfig {
  static inline TIM_TypeDef* const timer = TIM3;
  static constexpr uint32_t frequency_hz = 50;
  static constexpr uint32_t pin = PB0_ALT1;  // TIM3_CH3
  static constexpr uint32_t channel = 3;
  static constexpr uint32_t min_us = 1000;
  static constexpr uint32_t max_us = 2000;
}

// PWM Output on TIM3
PWMOutputBank pwm;

// Input Capture on TIM2
// HOW THIS TEST WORKS:
// 1. TIM3 generates servo PWM on A3 (50 Hz, 1500 µs pulse)
// 2. Jumper wire connects A3 → D6 (same as motor_pwm_verification Motor4 test)
// 3. TIM2 input capture on D6 measures time between rising edges
// 4. Callback fires on each rising edge, calculates period
// 5. Period validates PWM frequency is within ±2% tolerance
HardwareTimer tim2(TIM2);
volatile uint32_t capture_period_us = 0;
volatile bool measurement_ready = false;

void captureCallback() {
  // Hardware interrupt ensures precise timing measurement
  // Callback fires immediately on rising edge
  // Static variable preserves last_capture between interrupts
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(3);  // CH3

  // Calculate period in microseconds
  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate measurement range (10-30 ms for 33-100 Hz)
  if (period_us > 10000 && period_us < 30000) {
    capture_period_us = period_us;
    measurement_ready = true;
  }
}

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Servo PWM Verification Test ===\n");
  CI_LOG("Board: NUCLEO_F411RE (JHEF411 config)\n");
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Configure PWM Output using local servo config (repurposed Motor4)
  if (!pwm.Init(ServoConfig::timer, ServoConfig::frequency_hz)) {
    CI_LOG("ERROR: Failed to initialize PWM timer\n");
    while (1);
  }
  CI_LOGF("PWM Timer: TIM3 @ %lu Hz\n", ServoConfig::frequency_hz);

  if (!pwm.AttachChannel(ServoConfig::channel, ServoConfig::pin, ServoConfig::min_us, ServoConfig::max_us)) {
    CI_LOG("ERROR: Failed to attach PWM channel\n");
    while (1);
  }
  CI_LOG("PWM Output: PB0 (Arduino A3, repurposed Motor4)\n");

  // Set 1500 µs pulse width (center position)
  pwm.SetPulseWidth(ServoConfig::channel, 1500);
  pwm.Start();
  CI_LOG("PWM Pulse: 1500 µs\n\n");

  // Configure Input Capture with local pin definition
  // Input capture pins are test infrastructure, not in target config
  const uint32_t CAPTURE_PIN = PB10;  // TIM2_CH3 (Arduino D6)
  const uint32_t CAPTURE_CH = 3;

  tim2.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);  // Max period (32-bit timer)
  tim2.setMode(CAPTURE_CH, TIMER_INPUT_CAPTURE_RISING, CAPTURE_PIN);
  tim2.attachInterrupt(CAPTURE_CH, captureCallback);
  tim2.resume();

  CI_LOG("Input Capture: PB10 (Arduino D6, TIM2_CH3)\n");
  CI_LOG("Connect jumper: A3 → D6\n\n");

  CI_READY_TOKEN();
}

void loop() {
  static int measurement_count = 0;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    CI_LOG("\n✗ TIMEOUT: No measurements received after 15 seconds\n");
    CI_LOG("Check jumper connection: A3 → D6\n");
    CI_LOG("*STOP*\n");
    while(1); // Halt
  }

  if (measurement_ready) {
    measurement_ready = false;

    float measured_freq = 1000000.0 / capture_period_us;

    CI_LOGF("Period: %lu µs, Freq: ", capture_period_us);
    CI_LOG_FLOAT("", measured_freq, 2);
    CI_LOG(" Hz\n");

    // Validation with ±2% tolerance (50 Hz ± 1 Hz = 49-51 Hz)
    bool freq_valid = (measured_freq >= 49.0 && measured_freq <= 51.0);

    if (freq_valid) {
      CI_LOG("✓ PASS: Frequency within tolerance (49-51 Hz)\n");
    } else {
      CI_LOG("✗ FAIL: Frequency out of range (expected 49-51 Hz)\n");
    }

    CI_LOG("\n");

    // Stop after 3 successful measurements for HIL testing
    measurement_count++;
    if (measurement_count >= 3) {
      CI_LOG("*STOP*\n");
      while(1); // Halt for HIL framework
    }
  }

  delay(1000);
}
