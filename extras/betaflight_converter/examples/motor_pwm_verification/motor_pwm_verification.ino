/**
 * motor_pwm_verification.ino - Motor PWM Hardware Verification
 *
 * Hardware validation for motor PWM outputs using input capture.
 * Verifies TIM1 and TIM3 motor banks from auto-generated Betaflight config.
 * No oscilloscope needed - just jumper wires!
 *
 * Hardware Setup:
 * - Board: JHEF411 (NOXE V3) or compatible F411 board
 * - Motor 1 (TIM1): PA8
 * - Motor 4 (TIM3): PB0
 * - Capture Timer: TIM2 (dual channel input capture)
 *
 * Jumper Connections:
 * 1. D7/PA8 → A0/PA0 (Motor1/TIM1 output to TIM2_CH1)
 * 2. A3/PB0 → D6/PB10 (Motor4/TIM3 output to TIM2_CH3)
 *
 * Validation Criteria:
 * - Motor frequency within ± 2% of configured value
 * - Demonstrates auto-generated config usage
 * - Validates timer bank separation (TIM1 vs TIM3)
 */

#include <PWMOutputBank.h>
#include <ci_log.h>
#include "../../output/JHEF-JHEF411.h"

// PWM Output Banks
PWMOutputBank motor_tim1;
PWMOutputBank motor_tim3;

// Derived constants from config
static constexpr uint32_t MOTOR_FREQ = BoardConfig::Motor::frequency_hz;
static constexpr uint32_t PERIOD_US = 1000000 / MOTOR_FREQ;
static constexpr uint32_t PULSE_US = PERIOD_US / 2;  // 50% duty cycle
static constexpr float FREQ_MIN = MOTOR_FREQ * 0.98f;
static constexpr float FREQ_MAX = MOTOR_FREQ * 1.02f;

// Input Capture Timer
// HOW THIS TEST WORKS:
// 1. TIM1 generates motor1 PWM on D7/PA8
// 2. TIM3 generates motor4 PWM on A3/PB0
// 3. Jumper wires: D7/PA8 → A0/PA0 (TIM2_CH1), A3/PB0 → D6/PB10 (TIM2_CH3)
// 4. TIM2 input capture on 2 channels measures both frequencies
// 5. Separate callbacks validate each motor bank independently
// 6. Proves timer grouping from Betaflight converter works correctly
HardwareTimer tim2(TIM2);

// Measurement state
volatile uint32_t motor1_period_us = 0;
volatile uint32_t motor4_period_us = 0;
volatile bool motor1_ready = false;
volatile bool motor4_ready = false;

volatile uint32_t motor1_callback_count = 0;
volatile uint32_t motor4_callback_count = 0;

void motor1CaptureCallback() {
  // TIM1 motor measurement on TIM2_CH1
  motor1_callback_count++;
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(1);  // CH1

  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Accept periods within 4x range of expected (rejects first/spurious)
  if (period_us > PERIOD_US / 4 && period_us < PERIOD_US * 4) {
    motor1_period_us = period_us;
    motor1_ready = true;
  }
}

void motor4CaptureCallback() {
  // TIM3 motor measurement on TIM2_CH3
  motor4_callback_count++;
  static uint32_t last_capture = 0;
  uint32_t current_capture = tim2.getCaptureCompare(3);  // CH3

  uint32_t period_us = current_capture - last_capture;
  last_capture = current_capture;

  // Accept periods within 4x range of expected (rejects first/spurious)
  if (period_us > PERIOD_US / 4 && period_us < PERIOD_US * 4) {
    motor4_period_us = period_us;
    motor4_ready = true;
  }
}

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Motor PWM Verification Test ===\n");
  CI_LOG("Board: JHEF411 (NOXE V3)\n");
  CI_BUILD_INFO();
  CI_LOGF("Config frequency: %lu Hz (period: %lu us, pulse: %lu us)\n\n",
          MOTOR_FREQ, PERIOD_US, PULSE_US);

  // ========== Configure TIM1 Motor Bank (using generated config) ==========
  // Motor 1: motors[0] = {TIM1, PA8, 1, ...}
  auto& motor1 = BoardConfig::Motor::motors[0];
  CI_LOGF("Initializing Motor TIM1 Bank @ %lu Hz...\n", MOTOR_FREQ);
  if (!motor_tim1.Init(motor1.timer, MOTOR_FREQ)) {
    CI_LOG("ERROR: TIM1 Motor Init failed\n");
    while (1);
  }

  if (!motor_tim1.AttachChannel(motor1.channel, motor1.pin, 0, PERIOD_US)) {
    CI_LOG("ERROR: Motor1 AttachChannel failed\n");
    while (1);
  }

  motor_tim1.SetPulseWidth(motor1.channel, PULSE_US);  // 50% duty
  motor_tim1.Start();
  CI_LOGF("Motor1 (TIM1): CH%lu @ %lu Hz\n\n", motor1.channel, MOTOR_FREQ);

  // ========== Configure TIM3 Motor Bank (using generated config) ==========
  // Motor 4: motors[3] = {TIM3, PB0_ALT1, 3, ...}
  auto& motor4 = BoardConfig::Motor::motors[3];
  CI_LOGF("Initializing Motor TIM3 Bank @ %lu Hz...\n", MOTOR_FREQ);
  if (!motor_tim3.Init(motor4.timer, MOTOR_FREQ)) {
    CI_LOG("ERROR: TIM3 Motor Init failed\n");
    while (1);
  }

  if (!motor_tim3.AttachChannel(motor4.channel, motor4.pin, 0, PERIOD_US)) {
    CI_LOG("ERROR: Motor4 AttachChannel failed\n");
    while (1);
  }

  motor_tim3.SetPulseWidth(motor4.channel, PULSE_US);  // 50% duty
  motor_tim3.Start();
  CI_LOGF("Motor4 (TIM3): CH%lu @ %lu Hz\n\n", motor4.channel, MOTOR_FREQ);

  // ========== Configure Input Capture (TIM2 with 2 channels) ==========
  // Configure timer base FIRST (critical for input capture to work!)
  tim2.setPrescaleFactor(99);  // 100 MHz / 100 = 1 MHz tick rate
  tim2.setOverflow(0xFFFFFFFF);  // Max period (32-bit timer)

  // THEN configure channels
  tim2.setMode(1, TIMER_INPUT_CAPTURE_RISING, PA0);  // CH1: A0/PA0
  tim2.attachInterrupt(1, motor1CaptureCallback);
  CI_LOG("Motor1 Capture: A0/PA0 (TIM2_CH1)\n");

  tim2.setMode(3, TIMER_INPUT_CAPTURE_RISING, PB10);  // CH3: D6/PB10
  tim2.attachInterrupt(3, motor4CaptureCallback);
  CI_LOG("Motor4 Capture: D6/PB10 (TIM2_CH3)\n");

  // Start the timer
  tim2.resume();
  CI_LOG("\n");

  CI_LOG("Jumper Connections Required:\n");
  CI_LOG("1. D7/PA8 -> A0/PA0 (Motor1/TIM1 to TIM2_CH1)\n");
  CI_LOG("2. A3/PB0 -> D6/PB10 (Motor4/TIM3 to TIM2_CH3)\n\n");

  CI_READY_TOKEN();
}

void loop() {
  static int measurement_count = 0;
  static bool motor1_measured = false;
  static bool motor4_measured = false;
  static uint32_t start_time = millis();

  // Timeout if no measurements after 15 seconds
  const uint32_t TIMEOUT_MS = 15000;
  if (millis() - start_time > TIMEOUT_MS && measurement_count == 0) {
    CI_LOG("\nTIMEOUT: No measurements received after 15 seconds\n");
    CI_LOGF("Callback counts: Motor1=%lu, Motor4=%lu\n", motor1_callback_count, motor4_callback_count);
    CI_LOG("Check jumper connections:\n");
    CI_LOG("  1. D7/PA8 -> A0/PA0 (Motor1/TIM1)\n");
    CI_LOG("  2. A3/PB0 -> D6/PB10 (Motor4/TIM3)\n");
    CI_LOG("*STOP*\n");
    while(1); // Halt
  }

  // Check for Motor1 (TIM1) measurement
  if (motor1_ready && !motor1_measured) {
    motor1_ready = false;

    float measured_freq = 1000000.0 / motor1_period_us;
    bool motor1_valid = (measured_freq >= FREQ_MIN && measured_freq <= FREQ_MAX);

    CI_LOGF("[Motor1/TIM1] Period: %lu us, Freq: ", motor1_period_us);
    CI_LOG_FLOAT("", measured_freq, 1);
    CI_LOG(" Hz - ");

    if (motor1_valid) {
      CI_LOG("PASS\n");
      motor1_measured = true;
    } else {
      CI_LOG("FAIL\n");
    }
  }

  // Check for Motor4 (TIM3) measurement
  if (motor4_ready && !motor4_measured) {
    motor4_ready = false;

    float measured_freq = 1000000.0 / motor4_period_us;
    bool motor4_valid = (measured_freq >= FREQ_MIN && measured_freq <= FREQ_MAX);

    CI_LOGF("[Motor4/TIM3] Period: %lu us, Freq: ", motor4_period_us);
    CI_LOG_FLOAT("", measured_freq, 1);
    CI_LOG(" Hz - ");

    if (motor4_valid) {
      CI_LOG("PASS\n");
      motor4_measured = true;
    } else {
      CI_LOG("FAIL\n");
    }
  }

  // Stop after both measurements complete 3 times
  if (motor1_measured && motor4_measured) {
    measurement_count++;
    motor1_measured = false;
    motor4_measured = false;

    CI_LOG("\n");

    if (measurement_count >= 3) {
      CI_LOGF("Hardware Validation Complete @ %lu Hz\n", MOTOR_FREQ);
      CI_LOG("Timer Bank Separation Verified:\n");
      CI_LOG("  - TIM1 Bank: PASS\n");
      CI_LOG("  - TIM3 Bank: PASS\n");
      CI_LOG("*STOP*\n");
      while(1); // Halt for HIL framework
    }
  }

  delay(100);
}
