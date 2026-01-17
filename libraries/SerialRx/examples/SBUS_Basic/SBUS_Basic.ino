/*
 * SBUS_Basic - Basic SBUS RC receiver example
 *
 * Demonstrates reading RC channels from an SBUS receiver (FrSky/Futaba)
 * using the SerialRx library with BoardConfig integration.
 *
 * Hardware connections:
 *   RC Receiver SBUS → UART RX pin (see BoardConfig::rc_receiver)
 *   RC Receiver GND  → GND
 *   RC Receiver VCC  → 5V (if receiver needs 5V power)
 *
 * IMPORTANT: SBUS uses inverted signal!
 *   - STM32: Enable USART RX inversion (RXINV bit in USART_CR2)
 *   - OR use external inverter (transistor, 74HC04, etc.)
 *
 * Protocol: SBUS @ 100000 baud
 * Expected: 16 RC channels (0-2047 range, typical 172-1811)
 *
 * Board Configuration (from BoardConfig::rc_receiver):
 *   NUCLEO_F411RE_HIL006: Uses USART1 (RX=PA10, TX=PA9) @ 100000 baud
 *   BLACKPILL_F411CE: Uses UART1 (RX=PB3, TX=PA15)
 */

#include <SerialRx.h>
#include <ci_log.h>
#include "../../../../targets/NUCLEO_F411RE_HIL006.h"

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

  CI_LOG("SBUS RC Receiver - Basic Example\n");
  CI_LOG("==================================\n");

  // Configure RC receiver using BoardConfig
  // Note: SBUS uses 100000 baud (not 115200)
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::SBUS;
  config.baudrate = 100000;  // SBUS protocol baudrate (override BoardConfig default)
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;

  if (rc.begin(config)) {
    CI_LOGF("RC Receiver initialized (RX=0x%02X, TX=0x%02X, %lu baud)\n",
            BoardConfig::rc_receiver.rx_pin,
            BoardConfig::rc_receiver.tx_pin,
            config.baudrate);
    CI_LOGF("Software idle detection: %s (%lu us threshold)\n",
            config.idle_threshold_us > 0 ? "ENABLED" : "DISABLED",
            config.idle_threshold_us);

    // Enable USART RX inversion for SBUS inverted signal
    // STM32 HAL function (requires access to UART handle)
    // Note: This may require custom HardwareSerial modification
    CI_LOG("WARNING: SBUS requires inverted signal!\n");
    CI_LOG("  - Enable USART RX inversion in hardware, OR\n");
    CI_LOG("  - Use external signal inverter\n");

    CI_LOG("Waiting for SBUS frames...\n\n");
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
      // Print first 4 channels
      CI_LOGF("Ch1: %4d  Ch2: %4d  Ch3: %4d  Ch4: %4d  ",
              msg.channels[0], msg.channels[1],
              msg.channels[2], msg.channels[3]);

      // Print flags if present
      if (msg.error_flags) {
        CI_LOGF("Flags: 0x%02X ", msg.error_flags);
      }

      // Print all 16 channels on second line
      CI_LOGF("All: ");
      for (int i = 0; i < 16; i++) {
        CI_LOGF("%d:%4d ", i + 1, msg.channels[i]);
      }
      CI_LOG("\n");
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

  delay(100);  // Print at 10 Hz
}
