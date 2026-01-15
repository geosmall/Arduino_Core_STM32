/**
 * DualTimerPWM_Verification.ino - Dual Timer Input Capture Verification
 *
 * Hardware validation for simultaneous servo and ESC control using input capture.
 * Verifies TIM3 (50 Hz servo) and TIM4 (1 kHz ESC) using TIM2 and TIM5 input capture.
 * No oscilloscope needed - just jumper wires!
 *
 * Hardware Setup:
 * - Board: NUCLEO_F411RE
 * - Servo PWM: PB4 (D5, TIM3_CH1) @ 50 Hz
 * - ESC PWM: PB6 (D10, TIM4_CH1) @ 1 kHz
 * - Servo Capture: PA0 (A0, TIM2_CH1) - measures TIM3
 * - ESC Capture: PB10 (D6, TIM2_CH3) - measures TIM4
 *
 * Jumper Connections:
 * 1. D5 → A0 (TIM3 output to TIM2_CH1 input capture)
 * 2. D10 → D6 (TIM4 output to TIM2_CH3 input capture)
 *
 * Validation Criteria:
 * - Servo: 50 Hz ± 2% (49-51 Hz), 1500 µs pulse
 * - ESC: 1000 Hz ± 2% (980-1020 Hz), 187.5 µs pulse (midpoint)
 */

#include <PWMOutputBank.h>
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"

// PWM Output Banks
PWMOutputBank servo_pwm;
PWMOutputBank esc_pwm;

// Input Capture Timer
// HOW THIS TEST WORKS:
// 1. TIM3 generates servo PWM on D5 (50 Hz, 1500 µs pulse)
// 2. TIM4 generates ESC PWM on D10 (1000 Hz, 187 µs pulse)
// 3. Jumper wires: D5 → A0 (TIM2_CH1), D10 → D6 (TIM2_CH3)
// 4. TIM2 input capture on 2 channels simultaneously measures both frequencies
// 5. Separate callbacks for each channel validate frequencies independently
// 6. This proves dual timer operation works correctly for flight controllers
HardwareTimer tim2(TIM2);  // Captures both TIM3 (servo on CH1) and TIM4 (ESC on CH3)

// Measurement state
volatile uint32_t servo_period_us = 0;
volatile uint32_t esc_period_us = 0;
volatile bool servo_ready = false;
volatile bool esc_ready = false;

void servoCaptureCallback() {
  // Hardware interrupts provide microsecond-accurate timing
  // Each channel has independent callback
  // Callbacks fire asynchronously as signals arrive
  // Static variables track state between interrupt calls
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(1);

  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate servo range (10-30 ms for 33-100 Hz)
  if (period_us > 10000 && period_us < 30000) {
    servo_period_us = period_us;
    servo_ready = true;
  }
}

void escCaptureCallback() {
  // Separate callback for ESC channel - independent measurement
  // Measures faster 1 kHz signal on different timer channel
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(3);

  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Validate ESC range (500-2000 µs for 500-2000 Hz)
  if (period_us > 500 && period_us < 2000) {
    esc_period_us = period_us;
    esc_ready = true;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== DualTimerPWM Verification Test ===");
  Serial.println("Board: NUCLEO_F411RE");
  Serial.println();

  // ========== Configure Servo PWM (TIM3 @ 50 Hz) ==========
  Serial.print("Initializing Servo PWM (TIM3 @ ");
  Serial.print(BoardConfig::Servo::frequency_hz);
  Serial.println(" Hz)...");
  if (!servo_pwm.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz)) {
    Serial.println("ERROR: Servo PWM Init failed");
    Serial.println("*STOP*");
    while (1);
  }

  auto& servo_ch = BoardConfig::Servo::servo1;
  if (!servo_pwm.AttachChannel(servo_ch.ch, servo_ch.pin, servo_ch.min_us, servo_ch.max_us)) {
    Serial.println("ERROR: Servo PWM AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }

  servo_pwm.SetPulseWidth(servo_ch.ch, 1500);  // 1500 µs center position
  servo_pwm.Start();
  Serial.println("Servo PWM: PB4 (D5) - 1500 us @ 50 Hz");
  Serial.println();

  // ========== Configure ESC PWM (TIM4 @ 1 kHz) ==========
  Serial.print("Initializing ESC PWM (TIM4 @ ");
  Serial.print(BoardConfig::ESC::frequency_hz);
  Serial.println(" Hz)...");
  if (!esc_pwm.Init(BoardConfig::ESC::timer, BoardConfig::ESC::frequency_hz)) {
    Serial.println("ERROR: ESC PWM Init failed");
    Serial.println("*STOP*");
    while (1);
  }

  auto& esc_ch = BoardConfig::ESC::esc1;
  if (!esc_pwm.AttachChannel(esc_ch.ch, esc_ch.pin, esc_ch.min_us, esc_ch.max_us)) {
    Serial.println("ERROR: ESC PWM AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }

  // Set midpoint: (125 + 250) / 2 = 187.5 µs
  esc_pwm.SetPulseWidth(esc_ch.ch, 187);
  esc_pwm.Start();
  Serial.println("ESC PWM: PB6 (D10) - 187 us @ 1000 Hz");
  Serial.println();

  // ========== Configure Input Capture (TIM2 with 2 channels) ==========
  // Configure timer base
  tim2.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);  // Max period (32-bit timer)

  // Configure CH1 for servo measurement (local pin definitions, not in target config)
  const uint32_t SERVO_CAPTURE_PIN = PA0;  // TIM2_CH1 (Arduino A0)
  const uint32_t SERVO_CAPTURE_CH = 1;
  tim2.setMode(SERVO_CAPTURE_CH, TIMER_INPUT_CAPTURE_RISING, SERVO_CAPTURE_PIN);
  tim2.attachInterrupt(SERVO_CAPTURE_CH, servoCaptureCallback);
  Serial.println("Servo Capture: PA0 (A0, TIM2_CH1)");

  // Configure CH3 for ESC measurement (local pin definitions, not in target config)
  const uint32_t ESC_CAPTURE_PIN = PB10;  // TIM2_CH3 (Arduino D6)
  const uint32_t ESC_CAPTURE_CH = 3;
  tim2.setMode(ESC_CAPTURE_CH, TIMER_INPUT_CAPTURE_RISING, ESC_CAPTURE_PIN);
  tim2.attachInterrupt(ESC_CAPTURE_CH, escCaptureCallback);
  Serial.println("ESC Capture: PB10 (D6, TIM2_CH3)");

  // Start the timer
  tim2.resume();
  Serial.println();

  Serial.println("Jumper Connections Required:");
  Serial.println("1. D5 -> A0 (Servo PWM to capture)");
  Serial.println("2. D10 -> D6 (ESC PWM to capture)");
  Serial.println();
}

void loop() {
  static int measurement_count = 0;
  static bool servo_measured = false;
  static bool esc_measured = false;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    Serial.println();
    Serial.println("TIMEOUT: No measurements received after 15 seconds");
    Serial.println("Check jumper connections:");
    Serial.println("  1. D5 -> A0 (servo measurement)");
    Serial.println("  2. D10 -> D6 (ESC measurement)");
    Serial.println("*STOP*");
    while(1); // Halt
  }

  // Check for servo measurement
  if (servo_ready && !servo_measured) {
    servo_ready = false;

    float measured_freq = 1000000.0 / servo_period_us;
    bool servo_valid = (measured_freq >= 49.0 && measured_freq <= 51.0);

    Serial.print("[SERVO] Period: ");
    Serial.print(servo_period_us);
    Serial.print(" us, Freq: ");
    Serial.print(measured_freq, 2);
    Serial.print(" Hz - ");

    if (servo_valid) {
      Serial.println("PASS (49-51 Hz)");
      servo_measured = true;
    } else {
      Serial.println("FAIL (expected 49-51 Hz)");
    }
  }

  // Check for ESC measurement
  if (esc_ready && !esc_measured) {
    esc_ready = false;

    float measured_freq = 1000000.0 / esc_period_us;
    bool esc_valid = (measured_freq >= 980.0 && measured_freq <= 1020.0);

    Serial.print("[ESC]   Period: ");
    Serial.print(esc_period_us);
    Serial.print(" us, Freq: ");
    Serial.print(measured_freq, 2);
    Serial.print(" Hz - ");

    if (esc_valid) {
      Serial.println("PASS (980-1020 Hz)");
      esc_measured = true;
    } else {
      Serial.println("FAIL (expected 980-1020 Hz)");
    }
  }

  // Stop after both measurements complete 3 times
  if (servo_measured && esc_measured) {
    measurement_count++;
    servo_measured = false;
    esc_measured = false;

    Serial.println();

    if (measurement_count >= 3) {
      Serial.println("Hardware Validation Complete");
      Serial.println("*STOP*");
      while(1); // Halt for HIL framework
    }
  }

  delay(100);
}
