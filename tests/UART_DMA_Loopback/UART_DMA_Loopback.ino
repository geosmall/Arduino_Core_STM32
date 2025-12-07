/**
 * UART_DMA_Loopback.ino - Stress test for UART DMA RX implementation
 *
 * Tests:
 * 1. Mode switching: INT -> DMA -> INT -> DMA
 * 2. Stress test: Multiple iterations, larger payloads
 * 3. Buffer wraparound in circular DMA mode
 *
 * Hardware setup:
 * - NUCLEO_F411RE: Jumper D1 (PB6/TX) to D0 (PB7/RX) on CN9 Arduino header
 * - NUCLEO_H753ZI: Jumper D1 (PB6/TX) to D0 (PB7/RX) on CN8 Arduino header
 *   Note: H7 uses ALT pins (PB6_ALT2/PB7_ALT1) for USART1 instead of LPUART1
 *
 * Run with: ./system/ci/aflash.sh tests/UART_DMA_Loopback --use-rtt --build-id
 */

/*******************************************************************************
 * SERIAL RX BUFFER SIZE
 *
 * Default SERIAL_RX_BUFFER_SIZE is 128 bytes (127 usable, one slot reserved
 * for ring buffer head/tail distinction). This handles GPS NMEA sentences
 * (max 82 bytes) and most serial protocols.
 *
 * To increase for high-throughput streams, add to build flags:
 *   arduino-cli compile --build-property "build.extra_flags=-DSERIAL_RX_BUFFER_SIZE=256"
 ******************************************************************************/

#include <ci_log.h>
#include <HardwareSerial.h>

// DMA buffer - 256 bytes for circular reception
SERIAL_DMA_BUFFER uint8_t dmaRxBuffer[256];

// Use USART1 for loopback testing
// H7: PB6/PB7 default to LPUART1, need ALT pins for USART1
#if defined(STM32H7xx)
HardwareSerial SerialTest(PB7_ALT1, PB6_ALT2);  // RX=USART1, TX=USART1
#else
HardwareSerial SerialTest(PB7, PB6);  // RX, TX (USART1 on F4)
#endif

// Test patterns
const char *shortStr = "Hello";
const char *longStr = "The quick brown fox jumps over the lazy dog. 0123456789!";  // 57 bytes

bool testLoopback(const char *data, size_t len, const char *label) {
  // Clear any garbage
  while (SerialTest.available()) {
    SerialTest.read();
  }
  delay(5);

  // Send test data
  SerialTest.write((const uint8_t*)data, len);
  SerialTest.flush();
  delay(20 + len/10);  // Scale wait with data size

  int avail = SerialTest.available();

  if ((size_t)avail >= len) {
    char received[128] = {0};
    for (size_t i = 0; i < len && i < sizeof(received)-1; i++) {
      received[i] = (char)SerialTest.read();
    }

    if (memcmp(received, data, len) == 0) {
      return true;
    } else {
      CI_LOGF("  %s FAIL: Data mismatch at byte\n", label);
      return false;
    }
  } else {
    CI_LOGF("  %s FAIL: Expected %d, got %d bytes\n", label, len, avail);
    return false;
  }
}

void setup()
{
#ifdef USE_RTT
  SEGGER_RTT_Init();
#endif

  CI_LOG("=== UART DMA Stress Test ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  int passed = 0;
  int failed = 0;

  // ========== Phase 1: Interrupt Mode ==========
  CI_LOG("\n--- Phase 1: Interrupt Mode ---\n");
  SerialTest.begin(115200);
  CI_LOGF("  isDMAListening() = %s\n", SerialTest.isDMAListening() ? "true" : "false");

  // Stress test: 10 iterations with short string
  CI_LOG("  Testing 10x short (5 bytes)...\n");
  for (int i = 0; i < 10; i++) {
    if (testLoopback(shortStr, 5, "INT-short")) {
      passed++;
    } else {
      failed++;
    }
  }

  // 5 iterations with long string
  CI_LOG("  Testing 5x long (57 bytes)...\n");
  for (int i = 0; i < 5; i++) {
    if (testLoopback(longStr, 57, "INT-long")) {
      passed++;
    } else {
      failed++;
    }
  }

  // ========== H7 Buffer Validation Test ==========
#if defined(STM32H7xx)
  CI_LOG("\n--- H7 Buffer Validation Test ---\n");
  {
    // Test 1: Buffer NOT in D2 SRAM3 should fail with error -8
    uint8_t badBuffer[64];  // Stack buffer - NOT in .dmabuf section
    CI_LOG("  Testing buffer validation (stack buffer should fail)...\n");
    if (SerialTest.beginDMA(115200, badBuffer, sizeof(badBuffer))) {
      CI_LOG("  FAIL: beginDMA() should have rejected stack buffer\n");
      failed++;
    } else {
      int err = SerialTest.getLastDMAError();
      if (err == -8) {
        CI_LOG("  PASS: Stack buffer correctly rejected (error=-8)\n");
        passed++;
      } else {
        CI_LOGF("  FAIL: Expected error -8, got %d\n", err);
        failed++;
      }
    }

    // Test 2: Buffer in D2 SRAM3 (SERIAL_DMA_BUFFER) should succeed
    CI_LOG("  Testing buffer validation (SERIAL_DMA_BUFFER should pass)...\n");
    if (SerialTest.beginDMA(115200, dmaRxBuffer, sizeof(dmaRxBuffer))) {
      CI_LOG("  PASS: SERIAL_DMA_BUFFER accepted\n");
      passed++;
      SerialTest.endDMA();  // Clean up for next test
    } else {
      CI_LOGF("  FAIL: SERIAL_DMA_BUFFER rejected, error=%d\n", SerialTest.getLastDMAError());
      failed++;
    }
  }
#endif

  // ========== Phase 2: DMA Mode ==========
  CI_LOG("\n--- Phase 2: DMA Mode ---\n");
  if (!SerialTest.beginDMA(115200, dmaRxBuffer, sizeof(dmaRxBuffer))) {
    CI_LOGF("  ERROR: beginDMA() failed, error=%d\n", SerialTest.getLastDMAError());
    CI_LOG("*STOP*\n");
    while(1);
  }
  CI_LOGF("  isDMAListening() = %s\n", SerialTest.isDMAListening() ? "true" : "false");

  // Stress test: 10 iterations with short string
  CI_LOG("  Testing 10x short (5 bytes)...\n");
  for (int i = 0; i < 10; i++) {
    if (testLoopback(shortStr, 5, "DMA-short")) {
      passed++;
    } else {
      failed++;
    }
  }

  // 5 iterations with long string
  CI_LOG("  Testing 5x long (57 bytes)...\n");
  for (int i = 0; i < 5; i++) {
    if (testLoopback(longStr, 57, "DMA-long")) {
      passed++;
    } else {
      failed++;
    }
  }

  // GPS NMEA-sized payload test (82 bytes - max NMEA sentence length)
  CI_LOG("  Testing 5x NMEA-sized (82 bytes)...\n");
  char nmeaTest[82];
  for (int i = 0; i < 82; i++) {
    nmeaTest[i] = '0' + (i % 10);
  }
  for (int i = 0; i < 5; i++) {
    if (testLoopback(nmeaTest, 82, "DMA-NMEA")) {
      passed++;
    } else {
      failed++;
    }
  }

  // DMA circular buffer wraparound test
  // Sends 635 bytes total through 256-byte DMA buffer (wraps ~2.5x)
  // Max per transfer: 127 bytes (ring buffer is 128, one slot reserved)
  CI_LOG("  Testing DMA wraparound (5x 127 bytes = 635 bytes through 256-byte DMA buf)...\n");
  char wrapTest[127];
  for (int i = 0; i < 127; i++) {
    wrapTest[i] = 'A' + (i % 26);
  }
  for (int i = 0; i < 5; i++) {
    if (testLoopback(wrapTest, 127, "DMA-wrap")) {
      passed++;
    } else {
      failed++;
    }
  }

  // ========== Phase 3: Back to Interrupt Mode ==========
  CI_LOG("\n--- Phase 3: Back to Interrupt Mode ---\n");
  SerialTest.endDMA();
  CI_LOGF("  isDMAListening() = %s\n", SerialTest.isDMAListening() ? "true" : "false");

  CI_LOG("  Testing 5x short (5 bytes)...\n");
  for (int i = 0; i < 5; i++) {
    if (testLoopback(shortStr, 5, "INT2-short")) {
      passed++;
    } else {
      failed++;
    }
  }

  // ========== Phase 4: DMA Mode again ==========
  CI_LOG("\n--- Phase 4: DMA Mode again ---\n");
  if (!SerialTest.beginDMA(115200, dmaRxBuffer, sizeof(dmaRxBuffer))) {
    CI_LOGF("  ERROR: beginDMA() failed, error=%d\n", SerialTest.getLastDMAError());
    CI_LOG("*STOP*\n");
    while(1);
  }
  CI_LOGF("  isDMAListening() = %s\n", SerialTest.isDMAListening() ? "true" : "false");

  CI_LOG("  Testing 5x short (5 bytes)...\n");
  for (int i = 0; i < 5; i++) {
    if (testLoopback(shortStr, 5, "DMA2-short")) {
      passed++;
    } else {
      failed++;
    }
  }

  // ========== Results ==========
  CI_LOG("\n=== STRESS TEST RESULTS ===\n");
  CI_LOGF("Passed: %d\n", passed);
  CI_LOGF("Failed: %d\n", failed);
  CI_LOGF("Total:  %d\n", passed + failed);

  if (failed == 0) {
    CI_LOG("SUCCESS: All tests passed!\n");
  } else {
    CI_LOG("FAILURE: Some tests failed\n");
  }

  CI_LOG("*STOP*\n");
}

void loop()
{
  // Nothing
}
