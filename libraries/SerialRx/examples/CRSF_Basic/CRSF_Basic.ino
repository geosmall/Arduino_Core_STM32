/*
 * CRSF_Basic - Basic CRSF/ELRS RC receiver example
 *
 * Demonstrates reading RC channels from a CRSF receiver (TBS Crossfire
 * or ExpressLRS) using the SerialRx library with BoardConfig integration.
 *
 * Hardware connections:
 *   RC Receiver TX → UART RX pin (see BoardConfig::rc_receiver)
 *   RC Receiver GND → GND
 *   RC Receiver VCC → 5V
 *
 * Protocol: CRSF @ 420000 baud, 8N1, not inverted
 * Expected: 16 RC channels (0-2047 raw, converted to ~988-2012 PWM us)
 * Failsafe: Link Statistics LQ=0 triggers Layer 1 failsafe flag
 *
 * Build:
 *   arduino-cli compile --fqbn STM32_Robotics:stm32:Nucleo_64:pnum=NUCLEO_F411RE \
 *     Arduino_Core_STM32/libraries/SerialRx/examples/CRSF_Basic
 */

#include <SerialRx.h>

#if defined(ARDUINO_NUCLEO_F411RE)
  #include "targets/NUCLEO_F411RE_HIL001.h"
  #define BOARD_NAME "Nucleo F411RE (HIL-001)"
#elif defined(ARDUINO_BKMN_NERO)
  #include "targets/BKMN-NERO.h"
  #define BOARD_NAME "NERO F7"
#elif defined(ARDUINO_NUCLEO_G474RE)
  #include "targets/NUCLEO_G474RE_HIL008.h"
  #define BOARD_NAME "Nucleo G474RE (HIL-008)"
#else
  #error "Unsupported board. Add your board's target header."
#endif

// Create HardwareSerial instance using BoardConfig
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);

// Create SerialRx instance
SerialRx rc;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("CRSF/ELRS RC Receiver - Basic Example");
  Serial.println("======================================");
  Serial.print("Board: ");
  Serial.println(BOARD_NAME);

  // Configure RC receiver for CRSF protocol
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::CRSF;
  config.baudrate = 420000;  // CRSF protocol baudrate
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;
  config.invert_rx = false;  // CRSF is not inverted

  if (rc.begin(config)) {
    Serial.print("RC Receiver initialized (RX=0x");
    Serial.print(BoardConfig::rc_receiver.rx_pin.toPinName(), HEX);
    Serial.print(", ");
    Serial.print(config.baudrate);
    Serial.println(" baud)");
    Serial.println("Waiting for CRSF frames...\n");
  } else {
    Serial.println("ERROR: Failed to initialize RC receiver!");
    while (1);
  }
}

// Track signal state for failsafe reporting
static bool signal_lost = false;

// Test duration: 15 seconds then exit
static const uint32_t TEST_DURATION_MS = 15000;
static uint32_t test_start_time = 0;

void loop() {
  // Initialize test timer on first loop iteration
  if (test_start_time == 0) {
    test_start_time = millis();
  }

  // Check if 15-second test duration elapsed
  if (millis() - test_start_time >= TEST_DURATION_MS) {
    Serial.println("\n=== 15-Second Test Complete ===");
    Serial.print("Frames received: ");
    Serial.println(rc.getFramesReceived());
    Serial.print("Frames failed:   ");
    Serial.println(rc.getFramesFailed());
    Serial.print("Loss rate:       ");
    Serial.print(rc.getFrameLossPercent(), 2);
    Serial.println("%");
    while (1);
  }

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

        // Print first 4 channels as PWM microseconds
        Serial.print("Ail:");
        Serial.print(rc.channelToPWM(msg.channels[0]));
        Serial.print(" Ele:");
        Serial.print(rc.channelToPWM(msg.channels[1]));
        Serial.print(" Thr:");
        Serial.print(rc.channelToPWM(msg.channels[2]));
        Serial.print(" Rud:");
        Serial.print(rc.channelToPWM(msg.channels[3]));

        // Print flags if present (0x02 = failsafe from LQ=0)
        if (msg.error_flags) {
          Serial.print(" Flags:0x");
          Serial.print(msg.error_flags, HEX);
        }

        Serial.print(" | Rx:");
        Serial.println(rc.getFramesReceived());
      }
    }
  }

  // Check for timeout (failsafe)
  if (rc.timeout(1000)) {
    if (!signal_lost) {
      uint32_t time_since = rc.timeSinceLastMessage();
      Serial.print("WARNING: RC signal timeout (");
      Serial.print(time_since);
      Serial.println(" ms since last message)");
      signal_lost = true;
    }
  } else {
    if (signal_lost) {
      Serial.println("RC signal recovered");
      signal_lost = false;
    }
  }
}
