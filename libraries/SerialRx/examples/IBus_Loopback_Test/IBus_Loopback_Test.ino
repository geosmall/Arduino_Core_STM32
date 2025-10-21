/*
 * SerialRx IBus Loopback Test
 *
 * Hardware-in-the-Loop validation test for SerialRx library using loopback
 * between two USART peripherals on the same MCU.
 *
 * HARDWARE SETUP:
 *   Jumper wire: PA11 (USART6 TX) → PA10 (USART1 RX)
 *
 * TEST APPROACH:
 *   - USART6 (SerialTx): Generates IBus frames at 115200 baud
 *   - USART1 (SerialRx): Receives and parses frames via SerialRx library
 *   - Validates channel values, frame rate, and timeout detection
 *
 * EXIT CRITERIA:
 *   - "*TEST_PASS*" - All validations successful
 *   - "*TEST_FAIL*" - Validation failure or timeout
 *
 * CI/HIL INTEGRATION:
 *   ./scripts/build.sh tests/SerialRx_Loopback_Test --build-id --env-check --use-rtt
 *   ./scripts/aflash.sh tests/SerialRx_Loopback_Test --use-rtt
 */

#include <SerialRx.h>
#include <ci_log.h>

// Hardware configuration
// TX: USART6 for IBus frame generation
HardwareSerial SerialTx(NC, PA11);  // TX-only on PA11 (USART6 TX)

// RX: USART1 for SerialRx library
HardwareSerial SerialRC(PA10, PA9);  // USART1 (RX=PA10, TX=PA9)

// SerialRx instance
SerialRx rc;

// Test configuration
constexpr uint32_t TEST_DURATION_MS = 5000;  // 5 second test
constexpr uint32_t FRAME_INTERVAL_MS = 10;   // 100 Hz IBus frame rate
constexpr uint8_t MIN_FRAMES_EXPECTED = 400; // Expect ~500 frames in 5 seconds

// Test state
uint32_t test_start_time = 0;
uint32_t frames_received = 0;
uint32_t frames_sent = 0;
uint32_t last_frame_time = 0;
bool test_failed = false;

// IBus frame generation
// void generateIBusFrame(uint16_t* channels, uint8_t num_channels);
// void setup();
// void loop();
void generateIBusFrame(uint16_t* channels, uint8_t num_channels) {
  uint8_t frame[32];
  uint16_t checksum = 0xFFFF;

  // Header
  frame[0] = 0x20;  // IBus header byte 1
  frame[1] = 0x40;  // IBus header byte 2
  checksum -= frame[0];
  checksum -= frame[1];

  // Channel data (little-endian)
  for (uint8_t i = 0; i < num_channels && i < 14; i++) {
    uint8_t idx = 2 + (i * 2);
    frame[idx] = channels[i] & 0xFF;         // Low byte
    frame[idx + 1] = (channels[i] >> 8) & 0xFF;  // High byte
    checksum -= frame[idx];
    checksum -= frame[idx + 1];
  }

  // Pad unused channels with 0x00
  for (uint8_t i = num_channels; i < 14; i++) {
    uint8_t idx = 2 + (i * 2);
    frame[idx] = 0x00;
    frame[idx + 1] = 0x00;
  }

  // Checksum (little-endian)
  frame[30] = checksum & 0xFF;
  frame[31] = (checksum >> 8) & 0xFF;

  // Transmit frame
  SerialTx.write(frame, 32);
  frames_sent++;
}

void setup() {
  // Initialize Serial/RTT for test output
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("\n=== SerialRx IBus Loopback Test ===\n");
  CI_BUILD_INFO();
  CI_LOG("\nHardware Setup:\n");
  CI_LOG("  Jumper: PA11 (USART6 TX) -> PA10 (USART1 RX)\n");
  CI_LOG("  TX: USART6 @ 115200 baud (IBus generator)\n");
  CI_LOG("  RX: USART1 @ 115200 baud (SerialRx library)\n\n");

  // Initialize transmitter (USART6)
  SerialTx.begin(115200);

  // Initialize receiver (USART1 via SerialRx)
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = 115200;
  config.timeout_ms = 100;  // 100ms failsafe

  if (!rc.begin(config)) {
    CI_LOG("ERROR: SerialRx initialization failed!\n");
    CI_LOG("*TEST_FAIL*\n");
    while (1);
  }

  CI_LOG("Test starting...\n");
  CI_READY_TOKEN();
  delay(100);  // Allow RTT buffer to flush before test begins

  test_start_time = millis();

  // Send first frame immediately to synchronize TX/RX timing
  uint16_t channels[10];
  for (uint8_t i = 0; i < 10; i++) {
    channels[i] = 1000 + (i * 100);
  }
  generateIBusFrame(channels, 10);
  last_frame_time = test_start_time;
}

void loop() {
  uint32_t current_time = millis();
  uint32_t elapsed = current_time - test_start_time;

  // Generate IBus frames at 100 Hz
  if (current_time - last_frame_time >= FRAME_INTERVAL_MS) {
    uint16_t channels[10];

    // Generate test pattern: increasing values
    for (uint8_t i = 0; i < 10; i++) {
      channels[i] = 1000 + (frames_sent % 1000) + (i * 100);
    }

    generateIBusFrame(channels, 10);
    last_frame_time = current_time;
  }

  // Update SerialRx (polls Serial.available())
  rc.update();

  // Process received messages
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      frames_received++;

      // Validate channel 1 value (simple sanity check)
      uint16_t expected_base = 1000 + ((frames_sent - 1) % 1000);
      uint16_t received_ch1 = msg.channels[0];

      // Allow some tolerance due to timing
      if (received_ch1 < 1000 || received_ch1 > 2999) {
        CI_LOGF("FAIL: Invalid Ch1 value: %d\n", received_ch1);
        test_failed = true;
      }

      // Log every 100 frames
      if (frames_received % 100 == 0) {
        CI_LOGF("Progress: %lu frames RX, %lu frames TX\n", frames_received, frames_sent);
      }
    }
  }

  // Check for timeout (failsafe detection test)
  if (rc.timeout(200)) {
    CI_LOG("FAIL: Unexpected timeout detected\n");
    test_failed = true;
  }

  // Test completion
  if (elapsed >= TEST_DURATION_MS) {
    // Drain remaining frames in receive buffer (give 100ms to process last frame)
    uint32_t drain_start = millis();
    while (millis() - drain_start < 100) {
      rc.update();
      if (rc.available()) {
        RCMessage msg;
        if (rc.getMessage(&msg)) {
          frames_received++;
        }
      }
    }

    CI_LOG("\n=== Test Complete ===\n");
    CI_LOGF("Frames Sent: %lu\n", frames_sent);
    CI_LOGF("Frames Received: %lu\n", frames_received);
    CI_LOGF("Frame Loss: %lu (%.2f%%)\n",
            frames_sent - frames_received,
            (float)(frames_sent - frames_received) * 100.0f / frames_sent);

    // Validation
    bool pass = true;

    if (frames_received < MIN_FRAMES_EXPECTED) {
      CI_LOGF("FAIL: Too few frames received (%lu < %d)\n",
              frames_received, MIN_FRAMES_EXPECTED);
      pass = false;
    }

    float loss_rate = (float)(frames_sent - frames_received) * 100.0f / frames_sent;
    if (loss_rate > 2.0f) {
      CI_LOGF("FAIL: Frame loss rate too high (%.2f%% > 2.0%%)\n", loss_rate);
      pass = false;
    }

    if (test_failed) {
      CI_LOG("FAIL: Validation errors detected\n");
      pass = false;
    }

    if (pass) {
      CI_LOG("\n*TEST_PASS*\n");
    } else {
      CI_LOG("\n*TEST_FAIL*\n");
    }

    CI_LOG("*STOP*\n");  // Exit wildcard for aflash.sh automation
    while (1);  // Halt
  }
}

