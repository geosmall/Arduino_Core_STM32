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
 *   - STM32F7/H7/G4: Hardware USART RX inversion (RXINV bit in USART_CR2)
 *   - STM32F4: Requires external inverter (transistor, 74HC04, etc.)
 *
 * Protocol: SBUS @ 100000 baud
 * Expected: 16 RC channels (0-2047 raw, converted to ~1000-2000 PWM µs)
 *
 * Supported Boards:
 *   - DEVEBOX_H743 (HIL-006): H7 with hardware RXINV support
 *   - OPEN_REVO: F4 requires external inverter circuit
 *
 * Build:
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/SerialRx/examples/SBUS_Basic STM32_Robotics:stm32:FlightCtr:pnum=DEVEBOX_H743
 *   ./ci/saflash.sh Arduino_Core_STM32/libraries/SerialRx/examples/SBUS_Basic STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO
 */

#include <SerialRx.h>

// Board configuration - multi-board support
#if defined(ARDUINO_DEVEBOX_H743)
  #include "../../../../targets/DEVEBOX_H743_HIL006.h"
  #define BOARD_NAME "DevEBox H743 (HIL-006)"
#elif defined(ARDUINO_OPEN_REVO)
  #include "../../../../targets/OPEN-REVO.h"
  #define BOARD_NAME "OpenPilot Revolution (F405)"
#else
  #error "Unsupported board. Use DEVEBOX_H743 or OPEN_REVO."
#endif

// Create HardwareSerial instance using BoardConfig
HardwareSerial SerialRC(BoardConfig::rc_receiver.rx_pin,
                        BoardConfig::rc_receiver.tx_pin);

// Create SerialRx instance (protocol configured in setup)
SerialRx rc;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("SBUS RC Receiver - Basic Example");
  Serial.println("==================================");
  Serial.print("Board: ");
  Serial.println(BOARD_NAME);

  // Configure RC receiver using BoardConfig
  // Note: SBUS uses 100000 baud (not 115200)
  // CRITICAL: Enable external inverter BEFORE initializing UART
  // F4 lacks hardware RXINV, so the inverter must be set first to avoid
  // receiving garbage during UART initialization
#if !defined(USART_CR2_RXINV) && defined(ARDUINO_OPEN_REVO)
  pinMode(BoardConfig::rc_inverter_pin, OUTPUT);
  digitalWrite(BoardConfig::rc_inverter_pin, HIGH);  // HIGH = SBUS (inverted)
#endif

  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::SBUS;
  config.baudrate = 100000;  // SBUS protocol baudrate (override BoardConfig default)
  config.timeout_ms = BoardConfig::rc_receiver.timeout_ms;
  config.idle_threshold_us = BoardConfig::rc_receiver.idle_threshold_us;
  config.invert_rx = true;   // SBUS uses inverted signal - enable hardware RXINV

  if (rc.begin(config)) {
    Serial.print("RC Receiver initialized (RX=0x");
    Serial.print(BoardConfig::rc_receiver.rx_pin, HEX);
    Serial.print(", TX=0x");
    Serial.print(BoardConfig::rc_receiver.tx_pin, HEX);
    Serial.print(", ");
    Serial.print(config.baudrate);
    Serial.println(" baud)");
    Serial.print("Software idle detection: ");
    Serial.print(config.idle_threshold_us > 0 ? "ENABLED" : "DISABLED");
    Serial.print(" (");
    Serial.print(config.idle_threshold_us);
    Serial.println(" us threshold)");

#if defined(USART_CR2_RXINV)
    Serial.println("RX signal inversion: ENABLED (hardware RXINV)");
#else
    #if defined(ARDUINO_OPEN_REVO)
    Serial.print("RX signal inversion: ENABLED (external inverter on pin 0x");
    Serial.print(BoardConfig::rc_inverter_pin, HEX);
    Serial.println(")");
    #else
    Serial.println("WARNING: Hardware RX inversion not supported on this MCU!");
    Serial.println("  External inverter circuit required for SBUS.");
    #endif
#endif

    Serial.println("Waiting for SBUS frames...\n");
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

        // Print first 4 channels as PWM microseconds (normalized)
        // channelToPWM() converts SBUS 0-2047 to PWM ~988-2012 µs
        Serial.print("Ail:");
        Serial.print(rc.channelToPWM(msg.channels[0]));
        Serial.print(" Ele:");
        Serial.print(rc.channelToPWM(msg.channels[1]));
        Serial.print(" Thr:");
        Serial.print(rc.channelToPWM(msg.channels[2]));
        Serial.print(" Rud:");
        Serial.print(rc.channelToPWM(msg.channels[3]));

        // Print flags if present
        if (msg.error_flags) {
          Serial.print(" Flags:0x");
          Serial.print(msg.error_flags, HEX);
        }

        // Print all 16 channels as PWM
        Serial.print(" | ");
        for (int i = 0; i < 16; i++) {
          Serial.print(rc.channelToPWM(msg.channels[i]));
          Serial.print(" ");
        }
        Serial.print("| Rx:");
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
  // SBUS sends at ~70 Hz, UART buffer is only 64 bytes (2-3 frames)
}
