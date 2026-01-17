#pragma once
#include "config/ConfigTypes.h"

// DevEBox H743 HIL-006 Test Rig Configuration
// Hardware: DevEBox STM32H743VIT6 with SBUS receiver
// Purpose: SBUS protocol testing (FlySky IA6B in SBUS mode)
//
// SBUS Configuration:
//   - Protocol: 100000 baud, 8E2, inverted signal
//   - STM32H7 UART supports hardware RX inversion (RXINV bit)
//   - No external inverter needed
//
namespace BoardConfig {
  // RC Receiver: SBUS on USART1 (RX=PA10, TX=PA9)
  // 100000 baud for SBUS protocol
  static constexpr RCReceiverConfig rc_receiver{PA10, PA9, 100000, 1000, 300};

  // I2C1: Available for sensors (PB7/PB6 = D32/D33)
  static constexpr I2CConfig sensors{PB7, PB6, 400000};

  // Status LED: PE3 (directly from DevEBox schematic)
  static constexpr LEDConfig status_leds{PE3};
}
