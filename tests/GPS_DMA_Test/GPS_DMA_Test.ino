/**
 * GPS_DMA_Test.ino - GPS test with UART DMA reception
 *
 * Supports two modes:
 *   1. GPS_LOOPBACK_TEST defined: Loopback test with simulated NMEA (no GPS hardware)
 *   2. GPS_LOOPBACK_TEST undefined: Real GPS module test
 *
 * Hardware setup:
 *   Loopback mode: Jumper PB6 (TX) to PB7 (RX)
 *   GPS mode:      GPS TX -> PB7, GPS VCC -> 3.3V, GPS GND -> GND
 *
 * Supported boards:
 *   BlackPill F411CE (8MHz crystal):
 *     ./system/ci/aflash.sh tests/GPS_DMA_Test STMicroelectronics:stm32:FlightCtr:pnum=JHEF_JHEF411 --use-rtt --build-id
 *
 *   BlackPill F411CE (25MHz crystal):
 *     ./system/ci/aflash.sh tests/GPS_DMA_Test STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --use-rtt --build-id
 *
 *   Nucleo H753ZI:
 *     ./system/ci/aflash.sh tests/GPS_DMA_Test STMicroelectronics:stm32:Nucleo_144:pnum=NUCLEO_H753ZI --use-rtt --build-id
 */

//==============================================================================
// Configuration: Uncomment to enable loopback test mode (no GPS hardware needed)
//==============================================================================
#define GPS_LOOPBACK_TEST

#include <ci_log.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// GPS on USART1: PB7 (RX), PB6 (TX)
// H7: PB6/PB7 default to LPUART1, need ALT pins for USART1
#if defined(STM32H7xx)
HardwareSerial SerialGPS(PB7_ALT1, PB6_ALT2);
#else
HardwareSerial SerialGPS(PB7, PB6);
#endif

// DMA buffer for GPS UART
SERIAL_DMA_BUFFER uint8_t gpsDmaBuffer[256];

// TinyGPSPlus parser
TinyGPSPlus gps;

// Stats
uint32_t lastPrintTime = 0;
uint32_t bytesReceived = 0;
uint32_t sentencesDecoded = 0;

#ifdef GPS_LOOPBACK_TEST
//==============================================================================
// Loopback Test Mode - Simulated NMEA sentences
//==============================================================================

// NMEA test sentences (checksums verified with XOR calculation)
// Location: 53.3613 N, 6.5056 W (Ireland), 8 sats, alt 545.4m
const char* testGGA = "$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,0.95,545.4,M,47.0,M,,*49\r\n";
const char* testRMC = "$GPRMC,092750.000,A,5321.6802,N,00630.3372,W,0.02,31.66,280511,,,A*43\r\n";
const char* testGLL = "$GPGLL,5321.6802,N,00630.3372,W,092750.000,A,A*4B\r\n";

int testsPassed = 0;
int testsFailed = 0;

bool sendAndParse(const char* sentence, const char* label) {
  // Clear any pending data
  while (SerialGPS.available()) SerialGPS.read();
  delay(10);

  // Send NMEA sentence
  SerialGPS.print(sentence);
  SerialGPS.flush();
  delay(50);

  // Read and parse
  int bytes = 0;
  bool decoded = false;
  while (SerialGPS.available()) {
    char c = SerialGPS.read();
    bytes++;
    bytesReceived++;
    if (gps.encode(c)) {
      decoded = true;
      sentencesDecoded++;
    }
  }

  CI_LOGF("  %s: %d bytes, decoded=%s\n", label, bytes, decoded ? "yes" : "no");
  return bytes > 0 && decoded;
}

void setup() {
#ifdef USE_RTT
  SEGGER_RTT_Init();
#else
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== GPS DMA Test (Loopback Mode) ===\n");
  CI_LOG("Hardware: Jumper PB6 (TX) to PB7 (RX)\n\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize with DMA
  CI_LOG("--- Init UART DMA ---\n");
  if (SerialGPS.beginDMA(9600, gpsDmaBuffer, sizeof(gpsDmaBuffer))) {
    CI_LOG("  PASS: DMA mode initialized\n");
    testsPassed++;
  } else {
    CI_LOG("  FAIL: DMA init failed\n");
    testsFailed++;
    SerialGPS.begin(9600);
  }

  // Test 1: Send GGA sentence
  CI_LOG("\n--- Test 1: GGA Sentence ---\n");
  if (sendAndParse(testGGA, "GGA")) {
    testsPassed++;
  } else {
    testsFailed++;
  }

  // Test 2: Send RMC sentence
  CI_LOG("\n--- Test 2: RMC Sentence ---\n");
  if (sendAndParse(testRMC, "RMC")) {
    testsPassed++;
  } else {
    testsFailed++;
  }

  // Test 3: Send GLL sentence
  CI_LOG("\n--- Test 3: GLL Sentence ---\n");
  if (sendAndParse(testGLL, "GLL")) {
    testsPassed++;
  } else {
    testsFailed++;
  }

  // Test 4: Verify parsed data
  CI_LOG("\n--- Test 4: Parsed Data ---\n");
  CI_LOGF("  Chars processed: %lu\n", gps.charsProcessed());
  CI_LOGF("  Sentences with fix: %lu\n", gps.sentencesWithFix());
  CI_LOGF("  Failed checksums: %lu\n", gps.failedChecksum());

  if (gps.location.isValid()) {
    CI_LOG("  Location: VALID\n");
    CI_LOG_FLOAT("    Lat: ", gps.location.lat(), 4);
    CI_LOG("\n");
    CI_LOG_FLOAT("    Lon: ", gps.location.lng(), 4);
    CI_LOG("\n");

    // Verify approximate values (53.36 N, -6.51 W - Ireland)
    double lat = gps.location.lat();
    double lng = gps.location.lng();
    if (lat > 53.3 && lat < 53.4 && lng < -6.4 && lng > -6.6) {
      CI_LOG("  PASS: Location in expected range\n");
      testsPassed++;
    } else {
      CI_LOG("  FAIL: Location out of range\n");
      testsFailed++;
    }
  } else {
    CI_LOG("  FAIL: Location not valid\n");
    testsFailed++;
  }

  if (gps.altitude.isValid()) {
    CI_LOG_FLOAT("    Alt: ", gps.altitude.meters(), 1);
    CI_LOG(" m\n");
    testsPassed++;
  }

  if (gps.satellites.isValid() && gps.satellites.value() == 8) {
    CI_LOGF("    Sats: %lu\n", gps.satellites.value());
    CI_LOG("  PASS: Satellite count correct\n");
    testsPassed++;
  }

  // Results
  CI_LOG("\n=== RESULTS ===\n");
  CI_LOGF("Passed: %d\n", testsPassed);
  CI_LOGF("Failed: %d\n", testsFailed);

  if (testsFailed == 0) {
    CI_LOG("SUCCESS: GPS loopback test passed!\n");
  } else {
    CI_LOG("FAILURE: Some tests failed\n");
  }

  CI_LOG("*STOP*\n");
}

void loop() {
  // Nothing - loopback test runs once in setup()
}

#else
//==============================================================================
// Real GPS Mode - Continuous GPS data reception
//==============================================================================

void setup() {
#ifdef USE_RTT
  SEGGER_RTT_Init();
#else
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== GPS DMA Test (GPS Module Mode) ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Initialize GPS with DMA (9600 baud default for most GPS modules)
  if (SerialGPS.beginDMA(9600, gpsDmaBuffer, sizeof(gpsDmaBuffer))) {
    CI_LOG("GPS UART initialized (DMA mode)\n");
  } else {
    CI_LOG("DMA init failed, using interrupt mode\n");
    SerialGPS.begin(9600);
  }

  CI_LOG("Waiting for GPS data on USART1 (PB7)...\n\n");
}

void loop() {
  // Feed bytes to TinyGPSPlus parser
  while (SerialGPS.available()) {
    char c = SerialGPS.read();
    bytesReceived++;

    if (gps.encode(c)) {
      sentencesDecoded++;
    }
  }

  // Print status every 2 seconds
  if (millis() - lastPrintTime >= 2000) {
    lastPrintTime = millis();

    CI_LOGF("Bytes: %lu, Sentences: %lu, Chars: %lu, Failed: %lu\n",
            bytesReceived, sentencesDecoded,
            gps.charsProcessed(), gps.failedChecksum());

    if (gps.location.isValid()) {
      CI_LOG("  Fix: VALID\n");
      CI_LOG_FLOAT("  Lat: ", gps.location.lat(), 6);
      CI_LOG("\n");
      CI_LOG_FLOAT("  Lon: ", gps.location.lng(), 6);
      CI_LOG("\n");
      CI_LOG_FLOAT("  Alt: ", gps.altitude.meters(), 1);
      CI_LOG(" m\n");
      CI_LOGF("  Sats: %lu\n", gps.satellites.value());
      CI_LOGF("  Age: %lu ms\n", gps.location.age());
    } else {
      CI_LOG("  Fix: SEARCHING...\n");
      CI_LOGF("  Sats: %lu\n", gps.satellites.value());
    }
    CI_LOG("\n");
  }
}

#endif // GPS_LOOPBACK_TEST
