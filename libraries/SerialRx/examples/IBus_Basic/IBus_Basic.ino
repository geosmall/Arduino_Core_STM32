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
 *   NUCLEO_F411RE_HIL005: Uses USART1 (RX=PB7, TX=PB6)
 *   MTKS-MATEKH743: Uses LPUART1 (RX=PA10, TX=PA9)
 */

#include <SerialRx.h>

// Board target selection
#if defined(ARDUINO_NUCLEO_H743ZI) || defined(ARDUINO_GENERIC_H743ZITX)
  #include "../../../../../targets/MTKS-MATEKH743.h"
#elif defined(ARDUINO_NUCLEO_F411RE)
  #include "../../../../../targets/NUCLEO_F411RE_HIL005.h"
#else
  #include "../../../../../targets/BLACKPILL_F411CE.h"
#endif

// Create HardwareSerial instance using BoardConfig
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);

// Create SerialRx instance (protocol configured in setup)
SerialRx rc;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("IBus RC Receiver - Basic Example");
  Serial.println("===================================");

  // Configure RC receiver using BoardConfig
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = BoardConfig::rc_receiver.baud_rate;
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;

  if (rc.begin(config)) {
    Serial.print("RC Receiver initialized (RX=0x");
    Serial.print(BoardConfig::rc_receiver.rx_pin, HEX);
    Serial.print(", TX=0x");
    Serial.print(BoardConfig::rc_receiver.tx_pin, HEX);
    Serial.print(", ");
    Serial.print(BoardConfig::rc_receiver.baud_rate);
    Serial.println(" baud)");
    Serial.print("Software idle detection: ");
    Serial.print(BoardConfig::rc_receiver.idle_threshold_us > 0 ? "ENABLED" : "DISABLED");
    Serial.print(" (");
    Serial.print(BoardConfig::rc_receiver.idle_threshold_us);
    Serial.println(" us threshold)");
    Serial.println("Waiting for IBus frames...\n");
  } else {
    Serial.println("ERROR: Failed to initialize RC receiver!");
    Serial.println("*STOP*");
    while (1);  // Halt on error
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
    Serial.println("*STOP*");
    while (1);  // Halt for deterministic testing
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
        // Print all 14 IBus channels on single line
        Serial.print(msg.channels[0]); Serial.print(" ");
        Serial.print(msg.channels[1]); Serial.print(" ");
        Serial.print(msg.channels[2]); Serial.print(" ");
        Serial.print(msg.channels[3]); Serial.print(" ");
        Serial.print(msg.channels[4]); Serial.print(" ");
        Serial.print(msg.channels[5]); Serial.print(" ");
        Serial.print(msg.channels[6]); Serial.print(" ");
        Serial.print(msg.channels[7]); Serial.print(" ");
        Serial.print(msg.channels[8]); Serial.print(" ");
        Serial.print(msg.channels[9]); Serial.print(" ");
        Serial.print(msg.channels[10]); Serial.print(" ");
        Serial.print(msg.channels[11]); Serial.print(" ");
        Serial.print(msg.channels[12]); Serial.print(" ");
        Serial.print(msg.channels[13]);
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

  // No delay - fast polling required to avoid UART buffer overflow
  // IBus sends at 100 Hz, UART buffer is only 64 bytes (2 frames)
}
