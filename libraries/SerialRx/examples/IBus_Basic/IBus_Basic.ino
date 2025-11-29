/*
 * IBus_Basic - Basic IBus RC receiver example
 *
 * Demonstrates reading RC channels from a FlySky IBus receiver
 * using the SerialRx library with BoardConfig integration.
 *
 * Hardware connections:
 *   RC Receiver IBus → UART RX pin (see BoardConfig::rc_receiver)
 *   RC Receiver GND  → GND
 *   RC Receiver VCC  → 5V (if receiver needs 5V power)
 *
 * Protocol: IBus @ 115200 baud
 * Expected: 14 RC channels (1000-2000 us typical range)
 *
 * Board Configuration:
 *   NUCLEO_F411RE_JHEF411: Uses USART2 (RX=PA3, TX=PA2)
 *   MTKS-MATEKH743: Uses LPUART1 (RX=PA10, TX=PA9)
 */

#include <SerialRx.h>
#include <ci_log.h>

// Board target selection - uncomment ONE target
#if defined(ARDUINO_NUCLEO_H743ZI) || defined(ARDUINO_GENERIC_H743ZITX)
  #include "../../../../../targets/MTKS-MATEKH743.h"
#else
  // Default: NUCLEO_F411RE with JHEF411 pin mapping
  #include "../../../../../targets/NUCLEO_F411RE_JHEF411.h"
#endif

// Create HardwareSerial instance using BoardConfig
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);

// Create SerialRx instance (protocol configured in setup)
SerialRx rc;

void setup() {
  // Initialize Serial for debug output (not needed with RTT)
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("IBus RC Receiver - Basic Example\n");
  CI_LOG("===================================\n");

  // Configure RC receiver using BoardConfig
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = BoardConfig::rc_receiver.baud_rate;
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;

  if (rc.begin(config)) {
    CI_LOGF("RC Receiver initialized (RX=0x%02X, TX=0x%02X, %lu baud)\n",
            BoardConfig::rc_receiver.rx_pin,
            BoardConfig::rc_receiver.tx_pin,
            BoardConfig::rc_receiver.baud_rate);
    CI_LOGF("Software idle detection: %s (%lu us threshold)\n",
            BoardConfig::rc_receiver.idle_threshold_us > 0 ? "ENABLED" : "DISABLED",
            BoardConfig::rc_receiver.idle_threshold_us);
    CI_LOG("Waiting for IBus frames...\n\n");
  } else {
    CI_LOG("ERROR: Failed to initialize RC receiver!\n");
    while (1);  // Halt on error
  }

  CI_BUILD_INFO();
  CI_READY_TOKEN();
}

// Track signal state for failsafe reporting
static bool signal_lost = false;

// HIL testing mode: Run for 15 seconds then exit
#ifdef USE_RTT
static const uint32_t TEST_DURATION_MS = 15000;
static uint32_t test_start_time = 0;
#endif

void loop() {
#ifdef USE_RTT
  // Initialize test timer on first loop iteration
  if (test_start_time == 0) {
    test_start_time = millis();
  }

  // Check if 15-second test duration elapsed
  if (millis() - test_start_time >= TEST_DURATION_MS) {
    CI_LOG("\n=== 15-Second Test Complete ===\n");
    CI_LOGF("Frames received: %lu\n", rc.getFramesReceived());
    CI_LOGF("Frames failed:   %lu\n", rc.getFramesFailed());
    CI_LOG_FLOAT("Loss rate:       ", rc.getFrameLossPercent(), 2);
    CI_LOG("%\n");
    CI_LOG("*STOP*\n");
    while (1);  // Halt for deterministic HIL testing
  }
#endif

  // Update RC receiver (polls Serial.available())
  rc.update();

  // Check for new messages
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      // Print at 10 Hz to avoid flooding output
      static uint32_t last_print = 0;
      if (millis() - last_print >= 100) {
        last_print = millis();
        // Print all 14 IBus channels on single line
        CI_LOGF("%d %d %d %d %d %d %d %d %d %d %d %d %d %d | Rx:%lu\n",
                msg.channels[0], msg.channels[1], msg.channels[2],
                msg.channels[3], msg.channels[4], msg.channels[5],
                msg.channels[6], msg.channels[7], msg.channels[8],
                msg.channels[9], msg.channels[10], msg.channels[11],
                msg.channels[12], msg.channels[13], rc.getFramesReceived());
      }
    }
  }

  // Check for timeout (failsafe)
  if (rc.timeout(1000)) {
    if (!signal_lost) {
      uint32_t time_since = rc.timeSinceLastMessage();
      CI_LOGF("WARNING: RC signal timeout (%lu ms since last message)\n", time_since);
      signal_lost = true;
    }
  } else {
    if (signal_lost) {
      CI_LOG("RC signal recovered\n");
      signal_lost = false;
    }
  }

  // No delay - fast polling required to avoid UART buffer overflow
  // IBus sends at 100 Hz, UART buffer is only 64 bytes (2 frames)
}
