/**
 * DShot_Basic_FC - BoardConfig-driven DShot600 output for flight controllers
 *
 * Reads motor pin/timer/DMA assignments from the board's target header
 * and sends DShot600 on all configured motors. Cycles throttle 0-2047.
 *
 * Supported targets:
 *   OPEN_REVO       - OpenPilot Revo F405 (6 motors)
 *   JHEF_JHEF411    - JHEF F411 (5 motors)
 *   BKMN_NERO       - NERO F7 (4 motors)
 *   BEFH_BETAFPVG473 - BetaFPV G473 (4 motors)
 *   MATEK_H743VI    - MATEK H743-WLITE (8 motors)
 */

#include <DShot.h>

#if defined(ARDUINO_OPEN_REVO)
  #include "targets/OPEN-REVO.h"
  #define BOARD_NAME "OPEN_REVO"
#elif defined(ARDUINO_JHEF_JHEF411)
  #include "targets/JHEF-JHEF411.h"
  #define BOARD_NAME "JHEF_JHEF411"
#elif defined(ARDUINO_BKMN_NERO)
  #include "targets/BKMN-NERO.h"
  #define BOARD_NAME "BKMN_NERO"
#elif defined(ARDUINO_BEFH_BETAFPVG473)
  #include "targets/BEFH-BETAFPVG473.h"
  #define BOARD_NAME "BEFH_BETAFPVG473"
#elif defined(ARDUINO_MATEK_H743VI)
  #include "targets/MTKS-MATEKH743.h"
  #define BOARD_NAME "MATEK_H743VI"
#else
  #error "Unsupported board. Supported: OPEN_REVO, JHEF_JHEF411, BKMN_NERO, BEFH_BETAFPVG473, MATEK_H743VI"
#endif

DShotOutput motors;

static uint16_t throttle = 0;

void setup()
{
  Serial.begin(115200);
  Serial.print("DShot_Basic_FC [");
  Serial.print(BOARD_NAME);
  Serial.println("]: Initializing...");

  motors.Init(BoardConfig::Motor::motors, BoardConfig::Motor::num_motors, DShot::DSHOT600);

  Serial.print("Motors configured: ");
  Serial.println(motors.GetNumMotors());
}

void loop()
{
  motors.SetAllThrottle(throttle);
  motors.Send();

  throttle++;
  if (throttle > 2047) {
    throttle = 0;
  }
}
