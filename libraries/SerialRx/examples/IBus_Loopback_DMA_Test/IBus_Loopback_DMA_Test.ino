/*
 * SerialRx IBus Loopback DMA Test
 *
 * Validates SerialRx DMA mode using loopback between two USART peripherals.
 *
 * HARDWARE SETUP:
 *   Jumper wire: PA11 (USART6 TX) → PA10 (USART1 RX)
 *
 * TEST APPROACH:
 *   - USART6 (SerialTx): Generates IBus frames at 115200 baud
 *   - USART1 (SerialRx): Receives via DMA mode with SerialRx library
 *   - Validates channel values, frame rate, and DMA operation
 *
 * DMA BENEFITS:
 *   - Interrupt mode: ~11,500 IRQs/sec @ 115200 baud (per byte)
 *   - DMA mode: ~100 IRQs/sec (IDLE line detection only)
 *
 * EXIT CRITERIA:
 *   - "*TEST_PASS*" - All validations successful
 *   - "*TEST_FAIL*" - Validation failure or timeout
 *
 * OUTPUT:
 *   - 5-second test duration
 *   - Compatible with Arduino IDE Serial Monitor and CI automation
 */

#include <SerialRx.h>

// DMA buffer for SerialRx - must use SERIAL_DMA_BUFFER on H7
SERIAL_DMA_BUFFER uint8_t rcDmaBuffer[256];

// Hardware configuration
// TX: USART6 for IBus frame generation
HardwareSerial SerialTx(NC, PA11);  // TX-only on PA11 (USART6 TX)

// RX: USART1 for SerialRx library (DMA mode)
HardwareSerial SerialRC(PA10, PA9);  // USART1 (RX=PA10, TX=PA9)

// SerialRx instance
SerialRx rc;

// Test configuration
constexpr uint32_t TEST_DURATION_MS = 5000;  // 5 second test
constexpr uint32_t FRAME_INTERVAL_MS = 10;   // 100 Hz IBus frame rate
constexpr uint8_t MIN_FRAMES_EXPECTED = 400; // Expect ~500 frames in 5 seconds

// Test state
uint32_t test_start_time = 0;
uint32_t frames_sent = 0;
uint32_t last_frame_time = 0;
bool test_failed = false;
bool dma_mode_active = false;

// IBus frame generation
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
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n=== SerialRx IBus Loopback DMA Test ===");
  Serial.println("\nHardware Setup:");
  Serial.println("  Jumper: PA11 (USART6 TX) -> PA10 (USART1 RX)");
  Serial.println("  TX: USART6 @ 115200 baud (IBus generator)");
  Serial.println("  RX: USART1 @ 115200 baud (SerialRx DMA mode)\n");

  // Initialize transmitter (USART6)
  SerialTx.begin(115200);

  // Initialize receiver (USART1 via SerialRx with DMA)
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = 115200;
  config.timeout_ms = 100;  // 100ms failsafe
  // Enable DMA mode
  config.use_dma = true;
  config.dma_rx_buf = rcDmaBuffer;
  config.dma_rx_size = sizeof(rcDmaBuffer);

  if (!rc.begin(config)) {
    Serial.println("ERROR: SerialRx initialization failed!");
    Serial.println("*TEST_FAIL*");
    Serial.println("*STOP*");
    while (1);
  }

  // Check if DMA mode was actually enabled
  dma_mode_active = SerialRC.isDMAListening();
  if (dma_mode_active) {
    Serial.println("DMA mode: ENABLED");
  } else {
    Serial.println("DMA mode: FALLBACK to interrupt (DMA not available for this UART)");
  }

  Serial.println("Test starting...");
  delay(100);

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

  // Update SerialRx (polls Serial.available() - works same for DMA and interrupt)
  rc.update();

  // Process received messages
  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      // Validate channel 1 value (simple sanity check)
      uint16_t received_ch1 = msg.channels[0];

      // Allow some tolerance due to timing
      if (received_ch1 < 1000 || received_ch1 > 2999) {
        Serial.print("FAIL: Invalid Ch1 value: ");
        Serial.println(received_ch1);
        test_failed = true;
      }

      // Log every 100 frames using built-in statistics
      uint32_t frames_received = rc.getFramesReceived();
      if (frames_received % 100 == 0) {
        Serial.print("Progress: ");
        Serial.print(frames_received);
        Serial.print(" frames RX, ");
        Serial.print(frames_sent);
        Serial.println(" frames TX");
      }
    }
  }

  // Check for timeout (failsafe detection test)
  if (rc.timeout(200)) {
    Serial.println("FAIL: Unexpected timeout detected");
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
        rc.getMessage(&msg);
      }
    }

    // Get final statistics from SerialRx
    uint32_t frames_received = rc.getFramesReceived();
    uint32_t frames_failed = rc.getFramesFailed();
    float loss_percent = rc.getFrameLossPercent();

    Serial.println("\n=== Test Complete ===");
    Serial.print("DMA Mode: "); Serial.println(dma_mode_active ? "ACTIVE" : "FALLBACK");
    Serial.print("Frames Sent: "); Serial.println(frames_sent);
    Serial.print("Frames Received: "); Serial.println(frames_received);
    Serial.print("Frames Failed (checksum): "); Serial.println(frames_failed);
    Serial.print("Checksum Loss Rate: "); Serial.print(loss_percent); Serial.println("%");
    float total_loss = (float)(frames_sent - frames_received) * 100.0f / frames_sent;
    Serial.print("Total Loss: "); Serial.print(frames_sent - frames_received);
    Serial.print(" ("); Serial.print(total_loss); Serial.println("%)");

    // Validation
    bool pass = true;

    if (frames_received < MIN_FRAMES_EXPECTED) {
      Serial.print("FAIL: Too few frames received (");
      Serial.print(frames_received);
      Serial.print(" < ");
      Serial.print(MIN_FRAMES_EXPECTED);
      Serial.println(")");
      pass = false;
    }

    float total_loss_rate = (float)(frames_sent - frames_received) * 100.0f / frames_sent;
    if (total_loss_rate > 2.0f) {
      Serial.print("FAIL: Frame loss rate too high (");
      Serial.print(total_loss_rate);
      Serial.println("% > 2.0%)");
      pass = false;
    }

    if (test_failed) {
      Serial.println("FAIL: Validation errors detected");
      pass = false;
    }

    // Test end() cleanup
    rc.end();
    Serial.println("SerialRx::end() called - cleanup complete");

    if (pass) {
      Serial.println("\n*TEST_PASS*");
    } else {
      Serial.println("\n*TEST_FAIL*");
    }

    Serial.println("*STOP*");
    while (1);  // Halt
  }
}
