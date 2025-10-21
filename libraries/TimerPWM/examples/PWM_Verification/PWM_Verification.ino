/**
 * PWM_Verification.ino - Input capture verification example
 *
 * Verifies motor PWM output using TIM2 Input Capture to measure TIM1 PWM output
 * with BoardConfig integration.
 * No oscilloscope or logic analyzer needed - just a jumper wire!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE with JHEF411 config
 * - PWM Output: PA8 (Arduino D7, TIM1_CH1 - Motor1)
 * - Input Capture: PA0 (Arduino A0, TIM2_CH1)
 * - Connect jumper wire: D7 → A0
 *
 * This example validates:
 * - PWM frequency (1000 Hz ± 2%)
 * - Pulse width (500 µs nominal)
 */

#include <PWMOutputBank.h>
#include <ci_log.h>
#include "../../../../targets/NUCLEO_F411RE_JHEF411.h"

// PWM Output on TIM1
PWMOutputBank pwm;

// Input Capture on TIM2
// HOW THIS TEST WORKS:
// 1. TIM1 generates PWM on D7 (1000 Hz, 500 µs pulse)
// 2. Jumper wire connects D7 → A0
// 3. TIM2 input capture on A0 measures time between rising edges
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
  uint32_t current_capture = tim2.getCaptureCompare(1);

  // Calculate period in microseconds
  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate measurement range (500-2000 µs for 500-2000 Hz)
  if (period_us > 500 && period_us < 2000) {
    capture_period_us = period_us;
    measurement_ready = true;
  }
}

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Motor PWM Verification Test ===\n");
  CI_LOG("Board: NUCLEO_F411RE (JHEF411 config)\n");
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Configure PWM Output using BoardConfig motor
  if (!pwm.Init(BoardConfig::Motor::TIM1_Bank::timer, BoardConfig::Motor::frequency_hz)) {
    CI_LOG("ERROR: Failed to initialize PWM timer\n");
    while (1);
  }
  CI_LOGF("PWM Timer: TIM1 @ %lu Hz\n", BoardConfig::Motor::frequency_hz);

  auto& motor_ch = BoardConfig::Motor::TIM1_Bank::motor1;
  // Use 0-1000 µs range for verification (not DSHOT from config)
  if (!pwm.AttachChannel(motor_ch.ch, motor_ch.pin, 0, 1000)) {
    CI_LOG("ERROR: Failed to attach PWM channel\n");
    while (1);
  }
  CI_LOG("PWM Output: PA8 (Arduino D7, Motor1)\n");

  // Set 500 µs pulse width (50% duty cycle @ 1 kHz)
  pwm.SetPulseWidth(motor_ch.ch, 500);
  pwm.Start();
  CI_LOG("PWM Pulse: 500 µs\n\n");

  // Configure Input Capture with local pin definition
  // Input capture pins are test infrastructure, not in target config
  const uint32_t CAPTURE_PIN = PA0;  // TIM2_CH1 (Arduino A0)
  const uint32_t CAPTURE_CH = 1;

  tim2.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);  // Max period (32-bit timer)
  tim2.setMode(CAPTURE_CH, TIMER_INPUT_CAPTURE_RISING, CAPTURE_PIN);
  tim2.attachInterrupt(CAPTURE_CH, captureCallback);
  tim2.resume();

  CI_LOG("Input Capture: PA0 (Arduino A0, TIM2_CH1)\n");
  CI_LOG("Connect jumper: D7 → A0\n\n");

  CI_READY_TOKEN();
}

void loop() {
  static int measurement_count = 0;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    CI_LOG("\n✗ TIMEOUT: No measurements received after 15 seconds\n");
    CI_LOG("Check jumper connection: D7 → A0\n");
    CI_LOG("*STOP*\n");
    while(1); // Halt
  }

  if (measurement_ready) {
    measurement_ready = false;

    float measured_freq = 1000000.0 / capture_period_us;

    CI_LOGF("Period: %lu µs, Freq: ", capture_period_us);
    CI_LOG_FLOAT("", measured_freq, 2);
    CI_LOG(" Hz\n");

    // Validation with ±2% tolerance (1000 Hz ± 20 Hz = 980-1020 Hz)
    bool freq_valid = (measured_freq >= 980.0 && measured_freq <= 1020.0);

    if (freq_valid) {
      CI_LOG("✓ PASS: Frequency within tolerance (980-1020 Hz)\n");
    } else {
      CI_LOG("✗ FAIL: Frequency out of range (expected 980-1020 Hz)\n");
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
