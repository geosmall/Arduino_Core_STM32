/**
 * DShot_Basic - Manual 4-motor DShot600 output
 *
 * Sends DShot600 on 4 channels using TIM3 and TIM4:
 *   Motor 1: PB4 (TIM3_CH1)
 *   Motor 2: PB5 (TIM3_CH2)
 *   Motor 3: PB6 (TIM4_CH1)
 *   Motor 4: PB7 (TIM4_CH2)
 *
 * Cycles throttle 0-2047 continuously.
 */

#include <DShot.h>

DShotOutput motors;

static uint16_t throttle = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("DShot_Basic: Initializing...");

  pinMode(LED_BUILTIN, OUTPUT);

  // Add 4 motors manually
  motors.AddMotor(TIM3, PB4, 1, DShot::DSHOT600);
  motors.AddMotor(TIM3, PB5, 2, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB6, 1, DShot::DSHOT600);
  motors.AddMotor(TIM4, PB7, 2, DShot::DSHOT600);

  Serial.print("Motors configured: ");
  Serial.println(motors.GetNumMotors());
}

void loop()
{
  // Set all motors to the same throttle
  motors.SetAllThrottle(throttle);
  motors.Send();

  // Toggle LED to show activity
  digitalWrite(LED_BUILTIN, throttle & 0x100 ? HIGH : LOW);

  throttle++;
  if (throttle > 2047) {
    throttle = 0;
  }
}
