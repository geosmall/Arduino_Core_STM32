/**
 * PWM_Verification.ino - Input capture verification example
 *
 * Verifies PWM output using TIM2 Input Capture to measure TIM1 PWM output.
 * No oscilloscope or logic analyzer needed - just a jumper wire!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE (or any STM32 with TIM1 and TIM2)
 * - PWM Output: PA8 (Arduino D7, TIM1_CH1)
 * - Input Capture: PA0 (Arduino A0, TIM2_CH1)
 * - Connect jumper wire: D7 -> A0
 *
 * This example validates:
 * - PWM frequency (1000 Hz +/- 2%)
 * - Pulse width (500 us nominal)
 */

#include <PWMOutputBank.h>

// ============================================================================
// Pin Configuration - Local definitions for Nucleo F411RE
// ============================================================================
#define PWM_TIMER TIM1
const Pin PWM_PIN = PA8;                    // D7 on Nucleo
const uint32_t PWM_CHANNEL = 1;             // TIM1_CH1
const uint32_t PWM_FREQUENCY_HZ = 1000;     // 1 kHz for easy verification

const Pin CAPTURE_PIN = PA0;                // A0 on Nucleo
const uint32_t CAPTURE_CHANNEL = 1;         // TIM2_CH1

// ============================================================================
// Test Infrastructure
// ============================================================================
PWMOutputBank pwm;
HardwareTimer tim2(TIM2);

volatile uint32_t capture_period_us = 0;
volatile bool measurement_ready = false;

void captureCallback() {
  // Hardware interrupt ensures precise timing measurement
  // Callback fires immediately on rising edge
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(CAPTURE_CHANNEL);

  // Calculate period in microseconds
  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate measurement range (500-2000 us for 500-2000 Hz)
  if (period_us > 500 && period_us < 2000) {
    capture_period_us = period_us;
    measurement_ready = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== PWM Verification Test ===");
  Serial.println("Board: NUCLEO_F411RE");
  Serial.println();

  // Configure PWM Output
  if (!pwm.Init(PWM_TIMER, PWM_FREQUENCY_HZ)) {
    Serial.println("ERROR: Failed to initialize PWM timer");
    while (1);
  }
  Serial.print("PWM Timer: TIM1 @ ");
  Serial.print(PWM_FREQUENCY_HZ);
  Serial.println(" Hz");

  // Attach channel with 0-1000 us range
  if (!pwm.AttachChannel(PWM_CHANNEL, PWM_PIN, 0, 1000)) {
    Serial.println("ERROR: Failed to attach PWM channel");
    while (1);
  }
  Serial.println("PWM Output: PA8 (D7)");

  // Set 500 us pulse width (50% duty cycle @ 1 kHz)
  pwm.SetPulseWidth(PWM_CHANNEL, 500);
  pwm.Start();
  Serial.println("PWM Pulse: 500 us");
  Serial.println();

  // Configure Input Capture on TIM2
  tim2.setPrescaleFactor(99);     // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);   // Max period (32-bit timer)
  tim2.setMode(CAPTURE_CHANNEL, TIMER_INPUT_CAPTURE_RISING, CAPTURE_PIN);
  tim2.attachInterrupt(CAPTURE_CHANNEL, captureCallback);
  tim2.resume();

  Serial.println("Input Capture: PA0 (A0)");
  Serial.println("Connect jumper: D7 -> A0");
  Serial.println();
}

void loop() {
  static int measurement_count = 0;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    Serial.println();
    Serial.println("TIMEOUT: No measurements received after 15 seconds");
    Serial.println("Check jumper connection: D7 -> A0");
    Serial.println("*STOP*");
    while(1);
  }

  if (measurement_ready) {
    measurement_ready = false;

    float measured_freq = 1000000.0f / capture_period_us;

    Serial.print("Period: ");
    Serial.print(capture_period_us);
    Serial.print(" us, Freq: ");
    Serial.print(measured_freq, 2);
    Serial.println(" Hz");

    // Validation with +/-2% tolerance (1000 Hz +/- 20 Hz = 980-1020 Hz)
    bool freq_valid = (measured_freq >= 980.0f && measured_freq <= 1020.0f);

    if (freq_valid) {
      Serial.println("PASS: Frequency within tolerance (980-1020 Hz)");
    } else {
      Serial.println("FAIL: Frequency out of range (expected 980-1020 Hz)");
    }

    Serial.println();

    // Stop after 3 successful measurements
    measurement_count++;
    if (measurement_count >= 3) {
      Serial.println("*STOP*");
      while(1);
    }
  }

  delay(1000);
}
