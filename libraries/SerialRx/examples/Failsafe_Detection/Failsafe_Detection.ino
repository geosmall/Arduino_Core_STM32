/*
 * Failsafe_Detection - RC signal loss detection example
 *
 * Demonstrates the SerialRx 4-layer failsafe detection system:
 *   Layer 1: Protocol failsafe flag (SBUS only)
 *   Layer 2: Frame timeout (no frames for 100ms)
 *   Layer 3: Range checking (pulse < 885 or > 2115 µs)
 *   Layer 4: Per-channel expiry (stale values for 300ms)
 *
 * Primary API: rx.isSignalLost() — returns true when signal is lost
 * Diagnostics: rx.getSignalStatusString() — "OK", "TIMEOUT", etc.
 *
 * TX/RX Configuration:
 *   SBUS: Any failsafe mode works (protocol flag provides Layer 1)
 *   IBus: Endpoint trick REQUIRED for reliable detection (see README)
 *         Without it, ~988 µs is above 885 threshold → NOT detected
 *
 * Hardware:
 *   RC Receiver → UART RX pin (see BoardConfig::rc_receiver)
 *
 * Test procedure:
 *   1. Power on with TX on — observe "OK" status and channel values
 *   2. Turn off TX — observe status change to failsafe
 *   3. Turn on TX — observe recovery to "OK"
 *
 * Build:
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/SerialRx/examples/Failsafe_Detection STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/SerialRx/examples/Failsafe_Detection
 */

#include <SerialRx.h>

// Board target selection (auto-detects protocol from board config)
#if defined(ARDUINO_OPEN_REVO)
  #include "targets/OPEN-REVO.h"
  #define BOARD_NAME "OpenPilot Revolution (F405)"
  #define RC_PROTOCOL SerialRx::SBUS
#elif defined(ARDUINO_DEVEBOX_H743)
  #include "targets/DEVEBOX_H743_HIL006.h"
  #define BOARD_NAME "DevEBox H743 (HIL-006)"
  #define RC_PROTOCOL SerialRx::SBUS
#elif defined(ARDUINO_NUCLEO_H743ZI) || defined(ARDUINO_GENERIC_H743ZITX)
  #include "targets/MTKS-MATEKH743.h"
  #define BOARD_NAME "MATEK H743"
  #define RC_PROTOCOL SerialRx::SBUS
#elif defined(ARDUINO_NUCLEO_F411RE)
  #include "targets/NUCLEO_F411RE_HIL005.h"
  #define BOARD_NAME "Nucleo F411RE (HIL-005)"
  #define RC_PROTOCOL SerialRx::IBUS
#else
  #include "targets/BLACKPILL_F411CE.h"
  #define BOARD_NAME "BlackPill F411CE"
  #define RC_PROTOCOL SerialRx::IBUS
#endif

// Create HardwareSerial instance using BoardConfig
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);

// Create SerialRx instance
SerialRx rc;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("SerialRx Failsafe Detection Example");
  Serial.println("====================================");
  Serial.print("Board: ");
  Serial.println(BOARD_NAME);

  // OPEN_REVO: Set inverter pin for protocol
#if defined(ARDUINO_OPEN_REVO)
  pinMode(BoardConfig::rc_inverter_pin, OUTPUT);
  if (RC_PROTOCOL == SerialRx::SBUS) {
    digitalWrite(BoardConfig::rc_inverter_pin, HIGH);  // HIGH = SBUS (inverted)
    Serial.println("Inverter: ON (PC0 HIGH) for SBUS");
  } else {
    digitalWrite(BoardConfig::rc_inverter_pin, LOW);   // LOW = IBus (non-inverted)
    Serial.println("Inverter: OFF (PC0 LOW) for IBus");
  }
#endif

  // Configure RC receiver
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = RC_PROTOCOL;
  config.baudrate = BoardConfig::rc_receiver.baud_rate;
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;
  // Failsafe defaults: 100ms timeout, 300ms expiry, 885-2115 µs range
  // IBus endpoint trick: FS-i6X produces ~900 µs failsafe, raise threshold
  if (RC_PROTOCOL == SerialRx::IBUS) {
    config.valid_pulse_min = 925;
  }

  // SBUS requires RX signal inversion
  if (RC_PROTOCOL == SerialRx::SBUS) {
    config.invert_rx = true;
  }

  Serial.print("Protocol: ");
  Serial.println((RC_PROTOCOL == SerialRx::SBUS) ? "SBUS" : "IBus");
  Serial.print("Baudrate: ");
  Serial.println(config.baudrate);

  if (rc.begin(config)) {
    Serial.println("RC initialized — waiting for frames...");
    Serial.println("Turn TX off/on to test failsafe detection\n");
  } else {
    Serial.println("ERROR: RC init failed!");
    Serial.println("*STOP*");
    while (1);
  }
}

static bool prev_signal_lost = true;
static uint32_t test_start_time = 0;
static const uint32_t TEST_DURATION_MS = 30000;

void loop() {
  if (test_start_time == 0) {
    test_start_time = millis();
  }

  // 30-second test duration
  if (millis() - test_start_time >= TEST_DURATION_MS) {
    Serial.println("\n=== Test Complete ===");
    Serial.print("Frames: ");
    Serial.print(rc.getFramesReceived());
    Serial.print("  Failed: ");
    Serial.print(rc.getFramesFailed());
    Serial.print("  Loss: ");
    Serial.print(rc.getFrameLossPercent(), 2);
    Serial.println("%");
    Serial.println("*STOP*");
    while (1);
  }

  // Update RC receiver
  rc.update();

  // Consume messages (required to keep FIFO flowing)
  // Static so last valid message persists across loop() iterations
  static RCMessage msg;
  while (rc.available()) {
    rc.getMessage(&msg);
  }

  // Check failsafe status at 2 Hz
  static uint32_t last_print = 0;
  if (millis() - last_print >= 500) {
    last_print = millis();

    bool lost = rc.isSignalLost();

    // Report state transitions
    if (lost != prev_signal_lost) {
      if (lost) {
        Serial.print(">>> SIGNAL LOST: ");
        Serial.println(rc.getSignalStatusString());
      } else {
        Serial.println(">>> SIGNAL RECOVERED");
      }
      prev_signal_lost = lost;
    }

    // Print status line
    Serial.print("[");
    Serial.print(rc.getSignalStatusString());
    Serial.print("] ");

    if (!lost) {
      // Print AETR channels as PWM
      for (uint8_t ch = 0; ch < 4; ch++) {
        Serial.print("CH");
        Serial.print(ch + 1);
        Serial.print(":");
        Serial.print(rc.channelToPWM(msg.channels[ch]));
        Serial.print(" ");
      }
    } else {
      // Print held values during failsafe
      Serial.print("HELD: ");
      for (uint8_t ch = 0; ch < 4; ch++) {
        Serial.print(rc.getLastValidPWM(ch));
        Serial.print(" ");
      }
    }
    Serial.println();
  }
}
