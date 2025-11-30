// DualTimerPWM_Multi - Simultaneous Servo and Motor Control
// Demonstrates using multiple PWMOutputBank instances for different device types
//
// Supported Targets:
//   BLACKPILL_F411CE: Servos on TIM2, Motors on TIM3
//   NUCLEO_F411RE:    Servos on TIM2 (PA15/PB10), Motors on TIM1 (JHEF411 config)
//   MATEK_H743VI:     Servos on TIM15, Motors on TIM3 (MTKS-MATEKH743 config)
//
// This example shows how to control servos and motors simultaneously using
// separate timer banks with different frequencies.

#include <PWMOutputBank.h>
#include <ci_log.h>

// Board target selection based on Arduino board define
#if defined(ARDUINO_MATEK_H743VI)
  #include "../../../../targets/MTKS-MATEKH743.h"
  #define BOARD_NAME "MATEK_H743VI (MTKS-MATEKH743)"
#elif defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../../../targets/BLACKPILL_F411CE.h"
  #define BOARD_NAME "BLACKPILL_F411CE"
#elif defined(ARDUINO_NUCLEO_F411RE)
  #define HAS_SERVOS  // Enable servo/motor PWM configuration
  #include "../../../../targets/NUCLEO_F411RE_JHEF411.h"
  #define BOARD_NAME "NUCLEO_F411RE_JHEF411"
#else
  #error "Unsupported board variant. Supported: BLACKPILL_F411CE, NUCLEO_F411RE, MATEK_H743VI"
#endif

PWMOutputBank servo_pwm;
PWMOutputBank motor_pwm;

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== DualTimerPWM Multi-Target Example ===\n");
  CI_LOGF("Board: %s\n", BOARD_NAME);
  CI_BUILD_INFO();
  CI_LOG("\n");

  // Verify we have servos and motors configured
  if (BoardConfig::Servo::num_servos < 2) {
    CI_LOG("ERROR: Need at least 2 servos configured\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  if (BoardConfig::Motor::num_motors < 2) {
    CI_LOG("ERROR: Need at least 2 motors configured\n");
    CI_LOG("*STOP*\n");
    while (1);
  }

  // Initialize Servo PWM Bank
  auto& servo1 = BoardConfig::Servo::servos[0];
  auto& servo2 = BoardConfig::Servo::servos[1];

  CI_LOGF("Initializing Servo PWM @ %lu Hz...\n", BoardConfig::Servo::frequency_hz);

  if (!servo_pwm.Init(servo1.timer, BoardConfig::Servo::frequency_hz)) {
    CI_LOG("ERROR: Servo PWM Init failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  if (!servo_pwm.AttachChannel(servo1.channel, servo1.pin, servo1.min_us, servo1.max_us)) {
    CI_LOG("ERROR: Servo1 AttachChannel failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  if (!servo_pwm.AttachChannel(servo2.channel, servo2.pin, servo2.min_us, servo2.max_us)) {
    CI_LOG("ERROR: Servo2 AttachChannel failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  servo_pwm.Start();
  CI_LOGF("  Servo1: pin=0x%02lX ch=%lu\n", servo1.pin, servo1.channel);
  CI_LOGF("  Servo2: pin=0x%02lX ch=%lu\n\n", servo2.pin, servo2.channel);

  // Initialize Motor PWM Bank
  auto& motor1 = BoardConfig::Motor::motors[0];
  auto& motor2 = BoardConfig::Motor::motors[1];

  CI_LOGF("Initializing Motor PWM @ %lu Hz...\n", BoardConfig::Motor::frequency_hz);

  if (!motor_pwm.Init(motor1.timer, BoardConfig::Motor::frequency_hz)) {
    CI_LOG("ERROR: Motor PWM Init failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  if (!motor_pwm.AttachChannel(motor1.channel, motor1.pin, motor1.min_us, motor1.max_us)) {
    CI_LOG("ERROR: Motor1 AttachChannel failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  if (!motor_pwm.AttachChannel(motor2.channel, motor2.pin, motor2.min_us, motor2.max_us)) {
    CI_LOG("ERROR: Motor2 AttachChannel failed\n");
    CI_LOG("*STOP*\n");
    while (1);
  }
  motor_pwm.Start();
  CI_LOGF("  Motor1: pin=0x%02lX ch=%lu\n", motor1.pin, motor1.channel);
  CI_LOGF("  Motor2: pin=0x%02lX ch=%lu\n\n", motor2.pin, motor2.channel);

  CI_LOG("Starting dual PWM sweep:\n");
  CI_LOGF("- Servos: %lu-%lu us @ %lu Hz\n", servo1.min_us, servo1.max_us, BoardConfig::Servo::frequency_hz);
  CI_LOGF("- Motors: %lu-%lu us @ %lu Hz\n\n", motor1.min_us, motor1.max_us, BoardConfig::Motor::frequency_hz);

  CI_READY_TOKEN();
}

void loop() {
  static int sweep_count = 0;
  const int MAX_SWEEPS = 3;

  auto& servo1 = BoardConfig::Servo::servos[0];
  auto& servo2 = BoardConfig::Servo::servos[1];
  auto& motor1 = BoardConfig::Motor::motors[0];
  auto& motor2 = BoardConfig::Motor::motors[1];

  // Sweep servos from min to max
  for (uint32_t pulse = servo1.min_us; pulse <= servo1.max_us; pulse += 10) {
    servo_pwm.SetPulseWidth(servo1.channel, pulse);
    servo_pwm.SetPulseWidth(servo2.channel, servo1.min_us + servo1.max_us - pulse);  // Opposite
    CI_LOGF("S1:%4lu S2:%4lu | ", pulse, servo1.min_us + servo1.max_us - pulse);

    // Map servo to motor range
    uint32_t motor_pulse = map(pulse, servo1.min_us, servo1.max_us, motor1.min_us, motor1.max_us);
    motor_pwm.SetPulseWidth(motor1.channel, motor_pulse);
    motor_pwm.SetPulseWidth(motor2.channel, motor_pulse);
    CI_LOGF("M:%3lu\n", motor_pulse);

    delay(20);
  }

  // Sweep back
  for (uint32_t pulse = servo1.max_us; pulse >= servo1.min_us; pulse -= 10) {
    servo_pwm.SetPulseWidth(servo1.channel, pulse);
    servo_pwm.SetPulseWidth(servo2.channel, servo1.min_us + servo1.max_us - pulse);
    CI_LOGF("S1:%4lu S2:%4lu | ", pulse, servo1.min_us + servo1.max_us - pulse);

    uint32_t motor_pulse = map(pulse, servo1.min_us, servo1.max_us, motor1.min_us, motor1.max_us);
    motor_pwm.SetPulseWidth(motor1.channel, motor_pulse);
    motor_pwm.SetPulseWidth(motor2.channel, motor_pulse);
    CI_LOGF("M:%3lu\n", motor_pulse);

    delay(20);
  }

  sweep_count++;
  CI_LOGF("\nSweep %d/%d complete\n\n", sweep_count, MAX_SWEEPS);

  if (sweep_count >= MAX_SWEEPS) {
    CI_LOG("Demo complete\n");
    CI_LOG("*STOP*\n");
    while(1);
  }
}
