#pragma once
#include "config/ConfigTypes.h"

// HIL-008: NUCLEO-G474RE multi-fixture rig
//   Probe: ST-Link V3E onboard (S/N 002F002F3133510337363734)
//   LA:    Saleae Logic16 paired (permanent)
//
// Fixture set (mutually compatible — no pin overlap):
//
// 1. DShot capture (4-jumper, permanent):
//      PB4 -> PA0,  PA8 -> PA1,  PB0 -> PB10,  PB6 -> PA10
//    Source pins (PB4/PA8/PB0/PB6) drive DShot signals; target pins
//    capture them via timer input-capture for waveform / timing
//    verification. BoardConfig::Motor namespace not yet populated;
//    deferred until dRehmFlight integration needs it (timer/channel
//    assignments require per-test alignment and LA capture).
//
// 2. ELRS CRSF receiver (Radiomaster ER6, RX-only):
//      ER6 TX  -> PC11  (UART4 RX, Morpho CN7 pin 2)
//      ER6 RX  =  NC    (no telemetry)
//      ER6 VCC -> 5V    (Arduino header CN6)
//      ER6 GND -> GND   (Arduino header CN6)
//    CRSF @ 420000 baud, 8N1, not inverted. SYSCLK 168 MHz makes
//    BRR=400 exact (0 ppm error) — see doc/ELRS_CLOCK_CONFIGURATION.md.
namespace BoardConfig {
  static constexpr RCReceiverConfig rc_receiver{PC11, PC10, 420000, 1000, 300};
}
