// DualTimerPWM Example - Simultaneous Servo and ESC Control
// Demonstrates using multiple PWMOutputBank instances for different device types
//
// Hardware Setup:
// - Servo on PB4 (D5) - TIM3 @ 50 Hz
// - ESC 1 on PB6 (D10) - TIM4 @ 1 kHz (OneShot125)
// - ESC 2 on PB7 (CN7-21) - TIM4 @ 1 kHz (OneShot125)
//
// This example shows how to control servos and ESCs simultaneously using
// separate timer banks with different frequencies.

#include <PWMOutputBank.h>
#include "../../../../targets/NUCLEO_F411RE_LITTLEFS.h"
#include <ci_log.h>

PWMOutputBank servo_pwm;
PWMOutputBank esc_pwm;

void setup() {
  #ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
  #endif

  CI_LOG("=== DualTimerPWM Example ===\n");
  CI_LOGF("Board: NUCLEO_F411RE\n");
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Initialize Servo PWM Bank (TIM3 @ 50 Hz)
  CI_LOGF("Initializing Servo PWM (TIM3 @ %lu Hz)...\n", BoardConfig::Servo::frequency_hz);
  auto& servo_ch = BoardConfig::Servo::servo1;
  if (!servo_pwm.Init(BoardConfig::Servo::timer, BoardConfig::Servo::frequency_hz)) {
    CI_LOG("ERROR: Servo PWM Init failed\n");
    while (1);
  }
  if (!servo_pwm.AttachChannel(servo_ch.ch, servo_ch.pin, servo_ch.min_us, servo_ch.max_us)) {
    CI_LOG("ERROR: Servo PWM AttachChannel failed\n");
    while (1);
  }
  servo_pwm.Start();
  CI_LOGF("✓ Servo ready on PB4 (D5) - TIM3_CH%lu\n\n", servo_ch.ch);

  // Initialize ESC PWM Bank (TIM4 @ 1 kHz)
  CI_LOGF("Initializing ESC PWM (TIM4 @ %lu Hz)...\n", BoardConfig::ESC::frequency_hz);
  if (!esc_pwm.Init(BoardConfig::ESC::timer, BoardConfig::ESC::frequency_hz)) {
    CI_LOG("ERROR: ESC PWM Init failed\n");
    while (1);
  }

  // Attach ESC channels
  auto& esc1_ch = BoardConfig::ESC::esc1;
  auto& esc2_ch = BoardConfig::ESC::esc2;
  if (!esc_pwm.AttachChannel(esc1_ch.ch, esc1_ch.pin, esc1_ch.min_us, esc1_ch.max_us)) {
    CI_LOG("ERROR: ESC1 AttachChannel failed\n");
    while (1);
  }
  if (!esc_pwm.AttachChannel(esc2_ch.ch, esc2_ch.pin, esc2_ch.min_us, esc2_ch.max_us)) {
    CI_LOG("ERROR: ESC2 AttachChannel failed\n");
    while (1);
  }
  esc_pwm.Start();
  CI_LOGF("✓ ESC1 ready on PB6 (D10) - TIM4_CH%lu\n", esc1_ch.ch);
  CI_LOGF("✓ ESC2 ready on PB7 (CN7-21) - TIM4_CH%lu\n\n", esc2_ch.ch);

  CI_LOG("Starting dual PWM sweep:\n");
  CI_LOG("- Servo: 1000-2000 µs @ 50 Hz\n");
  CI_LOG("- ESCs: 125-250 µs @ 1 kHz\n\n");

  CI_READY_TOKEN();
}

void loop() {
  static int sweep_count = 0;
  const int MAX_SWEEPS = 3;  // Run 3 complete sweep cycles then exit

  // Sweep servo from 1000 to 2000 µs
  for (uint32_t pulse = 1000; pulse <= 2000; pulse += 10) {
    servo_pwm.SetPulseWidth(BoardConfig::Servo::servo1.ch, pulse);
    CI_LOGF("Servo: %4lu µs | ", pulse);

    // Map servo position to ESC range (1000-2000 → 125-250)
    uint32_t esc_pulse = map(pulse, 1000, 2000, 125, 250);
    esc_pwm.SetPulseWidth(BoardConfig::ESC::esc1.ch, esc_pulse);
    esc_pwm.SetPulseWidth(BoardConfig::ESC::esc2.ch, esc_pulse);
    CI_LOGF("ESC1/ESC2: %3lu µs\n", esc_pulse);

    delay(20);
  }

  // Sweep back
  for (uint32_t pulse = 2000; pulse >= 1000; pulse -= 10) {
    servo_pwm.SetPulseWidth(BoardConfig::Servo::servo1.ch, pulse);
    CI_LOGF("Servo: %4lu µs | ", pulse);

    uint32_t esc_pulse = map(pulse, 1000, 2000, 125, 250);
    esc_pwm.SetPulseWidth(BoardConfig::ESC::esc1.ch, esc_pulse);
    esc_pwm.SetPulseWidth(BoardConfig::ESC::esc2.ch, esc_pulse);
    CI_LOGF("ESC1/ESC2: %3lu µs\n", esc_pulse);

    delay(20);
  }

  sweep_count++;
  CI_LOGF("\nSweep cycle %d/%d complete\n\n", sweep_count, MAX_SWEEPS);

  if (sweep_count >= MAX_SWEEPS) {
    CI_LOG("Demo complete - 3 sweep cycles finished\n");
    CI_LOG("*STOP*\n");
    while(1);  // Halt for HIL framework
  }
}
