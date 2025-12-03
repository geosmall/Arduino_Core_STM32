// DeviceExample_CI.ino - TinyGPSPlus CI/HIL test with live GPS module
//
// Tests TinyGPSPlus NMEA parsing with actual GPS hardware.
// Validates GPS module detection, NMEA reception, and position fix.
//
// Supported targets:
//   NUCLEO_F411RE   - GPS on USART1 (PB7/PB6)
//   BLACKPILL_F411CE - GPS on USART1 (PA10/PA9)
//   BKMN_NERO       - GPS on USART1 (PA10/PA9)
//   MATEK_H743VI    - GPS on USART2 (PD6/PD5)
//
// Usage:
//   Serial: Arduino IDE monitor (115200 baud)
//   RTT:    ./system/ci/aflash.sh libraries/TinyGPSPlus/examples/DeviceExample_CI --use-rtt --build-id
//
// Hardware:
//   Connect GPS module TX to board RX pin (see target config below)
//   GPS modules typically use 9600 baud (default) or 115200 baud

#include <TinyGPSPlus.h>
#include <ci_log.h>
#include <libPrintf.h>

// putchar_() implementation for CI_PRINTF (libPrintf output routing)
extern "C" void putchar_(char c) {
#ifdef USE_RTT
    SEGGER_RTT_PutChar(0, c);
#else
    Serial.write(c);
#endif
}

//========================================================================================================================//
//                                             BOARD-SPECIFIC CONFIGURATION                                               //
//========================================================================================================================//

// BoardConfig: Auto-detect from Arduino board selection
#if defined(ARDUINO_BLACKPILL_F411CE)
  #include "../../targets/BLACKPILL_F411CE.h"
  // GPS on USART1: RX=PA10, TX=PA9
  #define GPS_SERIAL Serial1
  #define GPS_RX_PIN PA10
  #define GPS_TX_PIN PA9
  #define GPS_BAUD   9600
  #define TARGET_NAME "BLACKPILL_F411CE"

#elif defined(ARDUINO_NUCLEO_F411RE)
  #include "../../targets/NUCLEO_F411RE_JHEF411.h"
  // GPS on USART1: RX=PB7, TX=PB6
  #define GPS_SERIAL Serial1
  #define GPS_RX_PIN PB7
  #define GPS_TX_PIN PB6
  #define GPS_BAUD   9600
  #define TARGET_NAME "NUCLEO_F411RE"

#elif defined(ARDUINO_BKMN_NERO)
  #include "../../targets/BKMN-NERO.h"
  // GPS on USART1: RX=PA10, TX=PA9
  #define GPS_SERIAL Serial1
  #define GPS_RX_PIN PA10
  #define GPS_TX_PIN PA9
  #define GPS_BAUD   9600
  #define TARGET_NAME "BKMN_NERO"

#elif defined(ARDUINO_MATEK_H743VI)
  #include "../../targets/MTKS-MATEKH743.h"
  // GPS on USART2: RX=PD6, TX=PD5
  #define GPS_SERIAL Serial2
  #define GPS_RX_PIN PD6
  #define GPS_TX_PIN PD5
  #define GPS_BAUD   9600
  #define TARGET_NAME "MATEK_H743VI"

#else
  #error "Unsupported board! Use BLACKPILL_F411CE, NUCLEO_F411RE, BKMN_NERO, or MATEK_H743VI"
#endif

//========================================================================================================================//
//                                                   TEST CONFIGURATION                                                   //
//========================================================================================================================//

// Test timeouts (milliseconds)
static const uint32_t DETECTION_TIMEOUT_MS = 5000;   // Time to detect GPS (any NMEA data)
static const uint32_t FIX_TIMEOUT_MS       = 120000; // Time to acquire position fix (cold start can take 30-60s)

// Minimum thresholds for validation
static const uint32_t MIN_CHARS_FOR_DETECTION = 50;  // Minimum chars to consider GPS detected
static const uint32_t MIN_SENTENCES_FOR_FIX   = 3;   // Minimum valid sentences with fix

//========================================================================================================================//
//                                                      GLOBALS                                                           //
//========================================================================================================================//

TinyGPSPlus gps;
HardwareSerial GPSSerial(GPS_RX_PIN, GPS_TX_PIN);

// Test state
bool gpsDetected = false;
bool fixAcquired = false;
uint32_t startTime = 0;
uint32_t lastDisplayTime = 0;
int passCount = 0;
int failCount = 0;

// Statistics
uint32_t validSentenceCount = 0;
uint32_t lastCharsProcessed = 0;

//========================================================================================================================//
//                                                    FUNCTIONS                                                           //
//========================================================================================================================//

void displayInfo();
void runValidation();

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== TinyGPSPlus DeviceExample_CI ===\n");
    CI_PRINTF("Target: %s\n", TARGET_NAME);
    CI_PRINTF("Library version: %s\n", TinyGPSPlus::libraryVersion());
    CI_PRINTF("GPS UART: RX=%s, TX=%s @ %lu baud\n",
        pinNameStr(GPS_RX_PIN), pinNameStr(GPS_TX_PIN), (unsigned long)GPS_BAUD);
    CI_READY_TOKEN();
    CI_LOG("\n");

    // Initialize GPS serial
    GPSSerial.begin(GPS_BAUD);

    CI_LOG("Waiting for GPS module...\n");
    CI_LOG("----------------------------------------\n");

    startTime = millis();
    lastDisplayTime = startTime;
}

void loop()
{
    uint32_t now = millis();
    uint32_t elapsed = now - startTime;

    // Process incoming GPS data
    while (GPSSerial.available() > 0) {
        char c = GPSSerial.read();
        if (gps.encode(c)) {
            validSentenceCount++;

            // Count sentences with valid fix
            if (gps.location.isValid()) {
                fixAcquired = true;
            }
        }
    }

    // Check for GPS detection (any data received)
    if (!gpsDetected && gps.charsProcessed() >= MIN_CHARS_FOR_DETECTION) {
        gpsDetected = true;
        CI_PRINTF("GPS detected! (%lu chars in %lu ms)\n",
            gps.charsProcessed(), elapsed);
        CI_LOG("----------------------------------------\n");
    }

    // Periodic status display (every 2 seconds)
    if (now - lastDisplayTime >= 2000) {
        lastDisplayTime = now;

        // Show data rate
        uint32_t charsDelta = gps.charsProcessed() - lastCharsProcessed;
        lastCharsProcessed = gps.charsProcessed();

        if (gpsDetected) {
            CI_PRINTF("[%lu.%lus] ", elapsed / 1000, (elapsed % 1000) / 100);
            CI_PRINTF("Chars: %lu (+%lu/2s), ", gps.charsProcessed(), charsDelta);
            CI_PRINTF("Sentences: %lu (fix: %lu), ", gps.passedChecksum(), gps.sentencesWithFix());
            CI_PRINTF("Failed: %lu\n", gps.failedChecksum());

            // Display position if available
            if (gps.location.isValid()) {
                displayInfo();
            }
        } else {
            CI_PRINTF("[%lu.%lus] Waiting for GPS data... (chars: %lu)\n",
                elapsed / 1000, (elapsed % 1000) / 100, gps.charsProcessed());
        }
    }

    // Check for test completion conditions
    bool testComplete = false;
    bool testPassed = false;

    // Success: Got a valid fix with enough sentences
    if (fixAcquired && gps.sentencesWithFix() >= MIN_SENTENCES_FOR_FIX) {
        testComplete = true;
        testPassed = true;
    }

    // Timeout: Detection phase
    if (!gpsDetected && elapsed >= DETECTION_TIMEOUT_MS) {
        testComplete = true;
        testPassed = false;
        CI_LOG("\n");
        CI_LOG("TIMEOUT: No GPS data received!\n");
        CI_LOG("Check wiring: GPS TX -> Board RX\n");
    }

    // Timeout: Fix acquisition phase
    if (gpsDetected && !fixAcquired && elapsed >= FIX_TIMEOUT_MS) {
        testComplete = true;
        testPassed = false;
        CI_LOG("\n");
        CI_LOG("TIMEOUT: GPS detected but no position fix acquired.\n");
        CI_LOG("Ensure GPS has clear sky view.\n");
    }

    // Run final validation and exit
    if (testComplete) {
        CI_LOG("\n");
        CI_LOG("========================================\n");
        runValidation();

        CI_LOG("\n");
        CI_PRINTF("Tests passed: %d\n", passCount);
        CI_PRINTF("Tests failed: %d\n", failCount);
        CI_LOG("\n");

        if (testPassed && failCount == 0) {
            CI_LOG("*TEST_PASS*\n");
        } else {
            CI_LOG("*TEST_FAIL*\n");
        }

        CI_LOG("*STOP*\n");
        while (1) { delay(1000); }
    }
}

void displayInfo()
{
    CI_LOG("  Location: ");
    if (gps.location.isValid()) {
        CI_PRINTF("%.6f, %.6f", gps.location.lat(), gps.location.lng());
    } else {
        CI_LOG("INVALID");
    }

    if (gps.altitude.isValid()) {
        CI_PRINTF("  Alt: %.1fm", gps.altitude.meters());
    }

    if (gps.satellites.isValid()) {
        CI_PRINTF("  Sats: %lu", gps.satellites.value());
    }

    if (gps.hdop.isValid()) {
        CI_PRINTF("  HDOP: %.1f", gps.hdop.hdop());
    }

    CI_LOG("\n");

    if (gps.date.isValid() && gps.time.isValid()) {
        CI_PRINTF("  DateTime: %d/%d/%d %02d:%02d:%02d UTC\n",
            gps.date.month(), gps.date.day(), gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
    }
}

void runValidation()
{
    CI_LOG("=== Validation Results ===\n");

    // Test 1: GPS Detection
    if (gpsDetected) {
        CI_PRINTF("PASS: GPS module detected (%lu chars received)\n", gps.charsProcessed());
        passCount++;
    } else {
        CI_LOG("FAIL: GPS module not detected\n");
        failCount++;
    }

    // Test 2: NMEA Parsing
    if (gps.passedChecksum() > 0) {
        CI_PRINTF("PASS: NMEA parsing OK (%lu valid sentences)\n", gps.passedChecksum());
        passCount++;
    } else {
        CI_LOG("FAIL: No valid NMEA sentences parsed\n");
        failCount++;
    }

    // Test 3: Checksum verification
    if (gps.failedChecksum() == 0) {
        CI_LOG("PASS: All checksums valid (0 failures)\n");
        passCount++;
    } else {
        CI_PRINTF("WARN: %lu checksum failures (may indicate noise)\n", gps.failedChecksum());
        // Don't fail on checksum errors - could be noise on long wires
        passCount++;
    }

    // Test 4: Position fix
    if (gps.location.isValid()) {
        CI_PRINTF("PASS: Position fix acquired (%.6f, %.6f)\n",
            gps.location.lat(), gps.location.lng());
        passCount++;
    } else {
        CI_LOG("FAIL: No position fix acquired\n");
        failCount++;
    }

    // Test 5: Satellite count
    if (gps.satellites.isValid() && gps.satellites.value() >= 4) {
        CI_PRINTF("PASS: Sufficient satellites (%lu)\n", gps.satellites.value());
        passCount++;
    } else if (gps.satellites.isValid()) {
        CI_PRINTF("WARN: Low satellite count (%lu) - may affect accuracy\n", gps.satellites.value());
        passCount++;
    } else {
        CI_LOG("INFO: Satellite count not available\n");
        // Don't fail - some GPS modules don't report this
        passCount++;
    }

    // Test 6: Date/Time
    if (gps.date.isValid() && gps.time.isValid()) {
        CI_PRINTF("PASS: Date/Time valid (%d/%d/%d %02d:%02d:%02d)\n",
            gps.date.month(), gps.date.day(), gps.date.year(),
            gps.time.hour(), gps.time.minute(), gps.time.second());
        passCount++;
    } else {
        CI_LOG("FAIL: Date/Time not valid\n");
        failCount++;
    }
}

// Helper function to convert pin number to string (for debug output)
const char* pinNameStr(uint32_t pin) {
    static char buf[8];

    // Extract port and pin number from STM32 pin encoding
    uint32_t port = STM_PORT(pin);
    uint32_t pin_num = STM_PIN(pin);

    char port_letter = 'A' + port;
    snprintf(buf, sizeof(buf), "P%c%lu", port_letter, pin_num);
    return buf;
}
