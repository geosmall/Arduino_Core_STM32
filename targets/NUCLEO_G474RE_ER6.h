#pragma once
#include "config/ConfigTypes.h"

// NUCLEO-G474RE ER6 ELRS receiver fixture (RX-only)
//   ER6 TX  -> PC11  (UART4 RX, Morpho CN7 pin 2)
//   ER6 RX  =  NC    (no telemetry)
//   ER6 VCC -> 5V    (Arduino header CN6)
//   ER6 GND -> GND   (Arduino header CN6)
// CRSF @ 420000 baud, 8N1, not inverted.
namespace BoardConfig {
  static constexpr RCReceiverConfig rc_receiver{PC11, PC10, 420000, 1000, 300};
}
