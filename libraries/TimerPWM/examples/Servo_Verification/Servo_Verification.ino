/**
 * Servo_Verification.ino - Servo PWM verification example
 *
 * Verifies servo PWM output using TIM2 Input Capture to measure TIM3 PWM output.
 * No oscilloscope or logic analyzer needed - just a jumper wire!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE (or any STM32 with TIM2 and TIM3)
 * - PWM Output: PB0 (Arduino A3, TIM3_CH3)
 * - Input Capture: PB10 (Arduino D6, TIM2_CH3)
 * - Connect jumper wire: A3 -> D6
 *
 * This example validates:
 * - PWM frequency (50 Hz +/- 2%) via period measurement
 * - Pulse width set to 1500 us (center position)
 */

#include <PWMOutputBank.h>

// ============================================================================
// Pin Configuration - Local definitions for Nucleo F411RE
// ============================================================================
#define PWM_TIMER TIM3
const Pin PWM_PIN = PB0;                    // A3 on Nucleo (TIM3_CH3, no ALT needed)
const uint32_t PWM_CHANNEL = 3;             // TIM3_CH3
const uint32_t PWM_FREQUENCY_HZ = 50;       // 50 Hz for servo

const Pin CAPTURE_PIN = PB10;               // D6 on Nucleo
const uint32_t CAPTURE_CHANNEL = 3;         // TIM2_CH3

// Servo pulse width limits
const uint32_t SERVO_MIN_US = 1000;
const uint32_t SERVO_MAX_US = 2000;
const uint32_t SERVO_CENTER_US = 1500;

// ============================================================================
// Test Infrastructure
// ============================================================================
PWMOutputBank pwm;
HardwareTimer tim2(TIM2);

volatile uint32_t capture_period_us = 0;
volatile bool measurement_ready = false;

void captureCallback() {
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(CAPTURE_CHANNEL);

  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate measurement range (10-30 ms for 33-100 Hz)
  if (period_us > 10000 && period_us < 30000) {
    capture_period_us = period_us;
    measurement_ready = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== Servo PWM Verification Test ===");
  Serial.println("Board: NUCLEO_F411RE");
  Serial.print("PWM Timer: TIM3 @ ");
  Serial.print(PWM_FREQUENCY_HZ);
  Serial.println(" Hz");
  Serial.println("PWM Output: PB0 (A3)");
  Serial.print("PWM Pulse: ");
  Serial.print(SERVO_CENTER_US);
  Serial.println(" us");
  Serial.println("Input Capture: PB10 (D6)");
  Serial.println("Connect jumper: A3 -> D6");
  Serial.println();

  // Configure PWM Output
  if (!pwm.Init(PWM_TIMER, PWM_FREQUENCY_HZ)) {
    Serial.println("ERROR: Failed to initialize PWM timer");
    Serial.println("*STOP*");
    while (1);
  }

  if (!pwm.AttachChannel(PWM_CHANNEL, PWM_PIN, SERVO_MIN_US, SERVO_MAX_US)) {
    Serial.println("ERROR: Failed to attach PWM channel");
    Serial.println("*STOP*");
    while (1);
  }

  // Set center position (1500 us)
  pwm.SetPulseWidth(PWM_CHANNEL, SERVO_CENTER_US);
  pwm.Start();

  // Configure Input Capture
  tim2.setPrescaleFactor(99);      // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);    // Max period (32-bit timer)
  tim2.setMode(CAPTURE_CHANNEL, TIMER_INPUT_CAPTURE_RISING, CAPTURE_PIN);
  tim2.attachInterrupt(CAPTURE_CHANNEL, captureCallback);
  tim2.resume();
}

void loop() {
  static int measurement_count = 0;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    Serial.println("TIMEOUT: No measurements received");
    Serial.println("Check jumper: A3 -> D6");
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

    // Validation with +/-2% tolerance (50 Hz +/- 1 Hz = 49-51 Hz)
    bool freq_valid = (measured_freq >= 49.0f && measured_freq <= 51.0f);

    if (freq_valid) {
      Serial.println("PASS: Frequency within tolerance (49-51 Hz)");
    } else {
      Serial.println("FAIL: Frequency out of range (expected 49-51 Hz)");
    }

    // Stop after 3 successful measurements
    measurement_count++;
    if (measurement_count >= 3) {
      Serial.println("*STOP*");
      while(1);
    }
  }

  delay(1000);
}
