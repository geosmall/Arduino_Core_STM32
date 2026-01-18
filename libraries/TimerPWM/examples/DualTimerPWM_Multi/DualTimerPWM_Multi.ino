// DualTimerPWM_Multi - Simultaneous Servo and Motor Control
// Demonstrates using multiple PWMOutputBank instances for different device types
//
// Supported Targets:
//   BLACKPILL_F411CE: Servos on TIM2, Motors on TIM3
//   NUCLEO_F411RE:    Servos on TIM3 (PB4/PB0), Motors on TIM1 (PA8/PA9/PA10)
//   MATEK_H743VI:     Servos on TIM15, Motors on TIM3 (MTKS-MATEKH743 config)
//
// This example shows how to control servos and motors simultaneously using
// separate timer banks with different frequencies.

#include <PWMOutputBank.h>

// Board target selection based on Arduino board define
#if defined(ARDUINO_MATEK_H743VI)
  #include "../../../../targets/MTKS-MATEKH743.h"
  #define BOARD_NAME "MATEK_H743VI"
#elif defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../../../targets/BLACKPILL_F411CE.h"
  #define BOARD_NAME "BLACKPILL_F411CE"
#elif defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../targets/NUCLEO_F411RE_HIL001.h"
  #define BOARD_NAME "NUCLEO_F411RE"
#else
  #error "Unsupported board variant. Supported: BLACKPILL_F411CE, NUCLEO_F411RE, MATEK_H743VI"
#endif

PWMOutputBank servo_pwm;
PWMOutputBank motor_pwm;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("=== DualTimerPWM Multi-Target Example ===");
  Serial.print("Board: "); Serial.println(BOARD_NAME);
  Serial.println();

  // Verify we have servos and motors configured
  if (BoardConfig::Servo::num_servos < 2) {
    Serial.println("ERROR: Need at least 2 servos configured");
    Serial.println("*STOP*");
    while (1);
  }
  if (BoardConfig::Motor::num_motors < 2) {
    Serial.println("ERROR: Need at least 2 motors configured");
    Serial.println("*STOP*");
    while (1);
  }

  // Initialize Servo PWM Bank
  auto& servo1 = BoardConfig::Servo::servos[0];
  auto& servo2 = BoardConfig::Servo::servos[1];

  Serial.print("Initializing Servo PWM @ ");
  Serial.print(BoardConfig::Servo::frequency_hz);
  Serial.println(" Hz...");

  if (!servo_pwm.Init(servo1.timer, BoardConfig::Servo::frequency_hz)) {
    Serial.println("ERROR: Servo PWM Init failed");
    Serial.println("*STOP*");
    while (1);
  }
  if (!servo_pwm.AttachChannel(servo1.channel, servo1.pin, servo1.min_us, servo1.max_us)) {
    Serial.println("ERROR: Servo1 AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }
  if (!servo_pwm.AttachChannel(servo2.channel, servo2.pin, servo2.min_us, servo2.max_us)) {
    Serial.println("ERROR: Servo2 AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }
  servo_pwm.Start();
  Serial.print("  Servo1: pin=0x"); Serial.print(servo1.pin, HEX);
  Serial.print(" ch="); Serial.println(servo1.channel);
  Serial.print("  Servo2: pin=0x"); Serial.print(servo2.pin, HEX);
  Serial.print(" ch="); Serial.println(servo2.channel);
  Serial.println();

  // Initialize Motor PWM Bank
  auto& motor1 = BoardConfig::Motor::motors[0];
  auto& motor2 = BoardConfig::Motor::motors[1];

  Serial.print("Initializing Motor PWM @ ");
  Serial.print(BoardConfig::Motor::frequency_hz);
  Serial.println(" Hz...");

  if (!motor_pwm.Init(motor1.timer, BoardConfig::Motor::frequency_hz)) {
    Serial.println("ERROR: Motor PWM Init failed");
    Serial.println("*STOP*");
    while (1);
  }
  if (!motor_pwm.AttachChannel(motor1.channel, motor1.pin, motor1.min_us, motor1.max_us)) {
    Serial.println("ERROR: Motor1 AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }
  if (!motor_pwm.AttachChannel(motor2.channel, motor2.pin, motor2.min_us, motor2.max_us)) {
    Serial.println("ERROR: Motor2 AttachChannel failed");
    Serial.println("*STOP*");
    while (1);
  }
  motor_pwm.Start();
  Serial.print("  Motor1: pin=0x"); Serial.print(motor1.pin, HEX);
  Serial.print(" ch="); Serial.println(motor1.channel);
  Serial.print("  Motor2: pin=0x"); Serial.print(motor2.pin, HEX);
  Serial.print(" ch="); Serial.println(motor2.channel);
  Serial.println();

  Serial.println("Starting dual PWM sweep:");
  Serial.print("- Servos: "); Serial.print(servo1.min_us);
  Serial.print("-"); Serial.print(servo1.max_us);
  Serial.print(" us @ "); Serial.print(BoardConfig::Servo::frequency_hz);
  Serial.println(" Hz");
  Serial.print("- Motors: "); Serial.print(motor1.min_us);
  Serial.print("-"); Serial.print(motor1.max_us);
  Serial.print(" us @ "); Serial.print(BoardConfig::Motor::frequency_hz);
  Serial.println(" Hz");
  Serial.println();
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
    Serial.print("S1:"); Serial.print(pulse);
    Serial.print(" S2:"); Serial.print(servo1.min_us + servo1.max_us - pulse);
    Serial.print(" | ");

    // Map servo to motor range
    uint32_t motor_pulse = map(pulse, servo1.min_us, servo1.max_us, motor1.min_us, motor1.max_us);
    motor_pwm.SetPulseWidth(motor1.channel, motor_pulse);
    motor_pwm.SetPulseWidth(motor2.channel, motor_pulse);
    Serial.print("M:"); Serial.println(motor_pulse);

    delay(20);
  }

  // Sweep back
  for (uint32_t pulse = servo1.max_us; pulse >= servo1.min_us; pulse -= 10) {
    servo_pwm.SetPulseWidth(servo1.channel, pulse);
    servo_pwm.SetPulseWidth(servo2.channel, servo1.min_us + servo1.max_us - pulse);
    Serial.print("S1:"); Serial.print(pulse);
    Serial.print(" S2:"); Serial.print(servo1.min_us + servo1.max_us - pulse);
    Serial.print(" | ");

    uint32_t motor_pulse = map(pulse, servo1.min_us, servo1.max_us, motor1.min_us, motor1.max_us);
    motor_pwm.SetPulseWidth(motor1.channel, motor_pulse);
    motor_pwm.SetPulseWidth(motor2.channel, motor_pulse);
    Serial.print("M:"); Serial.println(motor_pulse);

    delay(20);
  }

  sweep_count++;
  Serial.println();
  Serial.print("Sweep "); Serial.print(sweep_count);
  Serial.print("/"); Serial.print(MAX_SWEEPS);
  Serial.println(" complete");
  Serial.println();

  if (sweep_count >= MAX_SWEEPS) {
    Serial.println("Demo complete");
    Serial.println("*STOP*");
    while(1);
  }
}
