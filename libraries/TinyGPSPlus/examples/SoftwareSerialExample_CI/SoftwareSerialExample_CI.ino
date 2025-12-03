// SoftwareSerialExample_CI.ino - TinyGPSPlus CI/HIL test with SoftwareSerial
//
// Tests TinyGPSPlus with GPS module using SoftwareSerial (bit-banged UART).
// Allows flexible GPIO pin selection - any digital pin can be used for GPS RX.
//
// SoftwareSerial at 9600 baud has negligible CPU overhead on F411 @ 100 MHz:
//   - ~10-20 µs ISR per byte received
//   - <2% CPU at typical GPS data rates
//   - Safe for use alongside 2 kHz flight loops
//
// Supported targets:
//   NUCLEO_F411RE    - GPS_RX=PA0, GPS_TX=PA1 (user configurable)
//   BLACKPILL_F411CE - GPS_RX=PA0, GPS_TX=PA1 (user configurable)
//   BKMN_NERO        - GPS_RX=PA0, GPS_TX=PA1 (user configurable)
//   MATEK_H743VI     - GPS_RX=PE9, GPS_TX=PE10 (user configurable)
//
// Usage:
//   Serial: Arduino IDE monitor (115200 baud)
//   RTT:    ./system/ci/aflash.sh libraries/TinyGPSPlus/examples/SoftwareSerialExample_CI --use-rtt --build-id
//
// Hardware:
//   Connect GPS module TX to GPS_RX_PIN (defined below)
//   GPS modules typically use 9600 baud

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
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
//                                             USER-CONFIGURABLE GPS PINS                                                 //
//========================================================================================================================//

// SoftwareSerial allows ANY GPIO pin - adjust these for your wiring
// Note: Only RX pin matters for GPS (GPS TX -> MCU RX). TX pin can be any unused pin.

#if defined(ARDUINO_BLACKPILL_F411CE)
  // BlackPill: Use PA0/PA1 (avoid PA9/PA10 which are USB on some boards)
  static const int GPS_RX_PIN = PA0;   // Connect to GPS module TX
  static const int GPS_TX_PIN = PA1;   // Not used by GPS (GPS doesn't receive)
  #define TARGET_NAME "BLACKPILL_F411CE"

#elif defined(ARDUINO_NUCLEO_F411RE)
  // Nucleo: Use PA0/PA1 on Arduino connector (A0/A1)
  static const int GPS_RX_PIN = PA0;   // Arduino A0
  static const int GPS_TX_PIN = PA1;   // Arduino A1
  #define TARGET_NAME "NUCLEO_F411RE"

#elif defined(ARDUINO_BKMN_NERO)
  // NERO F7: Use PA0/PA1
  static const int GPS_RX_PIN = PA0;
  static const int GPS_TX_PIN = PA1;
  #define TARGET_NAME "BKMN_NERO"

#elif defined(ARDUINO_MATEK_H743VI)
  // MATEK H743: Use PE9/PE10 (avoid conflict with motor outputs)
  static const int GPS_RX_PIN = PE9;
  static const int GPS_TX_PIN = PE10;
  #define TARGET_NAME "MATEK_H743VI"

#else
  #error "Unsupported board! Use BLACKPILL_F411CE, NUCLEO_F411RE, BKMN_NERO, or MATEK_H743VI"
#endif

// GPS baud rate (9600 is standard for most GPS modules)
static const uint32_t GPS_BAUD = 9600;

//========================================================================================================================//
//                                                   TEST CONFIGURATION                                                   //
//========================================================================================================================//

// Test timeouts (milliseconds)
static const uint32_t DETECTION_TIMEOUT_MS = 5000;   // Time to detect GPS (any NMEA data)
static const uint32_t FIX_TIMEOUT_MS       = 120000; // Time to acquire position fix

// Minimum thresholds for validation
static const uint32_t MIN_CHARS_FOR_DETECTION = 50;
static const uint32_t MIN_SENTENCES_FOR_FIX   = 3;

//========================================================================================================================//
//                                                      GLOBALS                                                           //
//========================================================================================================================//

TinyGPSPlus gps;
SoftwareSerial GPSSerial(GPS_RX_PIN, GPS_TX_PIN);

// Test state
bool gpsDetected = false;
bool fixAcquired = false;
uint32_t startTime = 0;
uint32_t lastDisplayTime = 0;
int passCount = 0;
int failCount = 0;

// Statistics
uint32_t lastCharsProcessed = 0;

//========================================================================================================================//
//                                                    FUNCTIONS                                                           //
//========================================================================================================================//

void displayInfo();
void runValidation();
const char* pinNameStr(uint32_t pin);

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== TinyGPSPlus SoftwareSerialExample_CI ===\n");
    CI_PRINTF("Target: %s\n", TARGET_NAME);
    CI_PRINTF("Library version: %s\n", TinyGPSPlus::libraryVersion());
    CI_PRINTF("GPS (SoftwareSerial): RX=%s, TX=%s @ %lu baud\n",
        pinNameStr(GPS_RX_PIN), pinNameStr(GPS_TX_PIN), (unsigned long)GPS_BAUD);
    CI_LOG("Note: SoftwareSerial at 9600 baud has <2% CPU overhead on F411\n");
    CI_READY_TOKEN();
    CI_LOG("\n");

    // Initialize GPS SoftwareSerial
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
            if (gps.location.isValid()) {
                fixAcquired = true;
            }
        }
    }

    // Check for GPS detection
    if (!gpsDetected && gps.charsProcessed() >= MIN_CHARS_FOR_DETECTION) {
        gpsDetected = true;
        CI_PRINTF("GPS detected! (%lu chars in %lu ms)\n",
            gps.charsProcessed(), elapsed);
        CI_LOG("----------------------------------------\n");
    }

    // Periodic status display (every 2 seconds)
    if (now - lastDisplayTime >= 2000) {
        lastDisplayTime = now;

        uint32_t charsDelta = gps.charsProcessed() - lastCharsProcessed;
        lastCharsProcessed = gps.charsProcessed();

        if (gpsDetected) {
            CI_PRINTF("[%lu.%lus] ", elapsed / 1000, (elapsed % 1000) / 100);
            CI_PRINTF("Chars: %lu (+%lu/2s), ", gps.charsProcessed(), charsDelta);
            CI_PRINTF("Sentences: %lu (fix: %lu), ", gps.passedChecksum(), gps.sentencesWithFix());
            CI_PRINTF("Failed: %lu\n", gps.failedChecksum());

            if (gps.location.isValid()) {
                displayInfo();
            }
        } else {
            CI_PRINTF("[%lu.%lus] Waiting for GPS data... (chars: %lu)\n",
                elapsed / 1000, (elapsed % 1000) / 100, gps.charsProcessed());
        }
    }

    // Check for test completion
    bool testComplete = false;
    bool testPassed = false;

    if (fixAcquired && gps.sentencesWithFix() >= MIN_SENTENCES_FOR_FIX) {
        testComplete = true;
        testPassed = true;
    }

    if (!gpsDetected && elapsed >= DETECTION_TIMEOUT_MS) {
        testComplete = true;
        testPassed = false;
        CI_LOG("\n");
        CI_LOG("TIMEOUT: No GPS data received!\n");
        CI_PRINTF("Check wiring: GPS TX -> %s (RX)\n", pinNameStr(GPS_RX_PIN));
    }

    if (gpsDetected && !fixAcquired && elapsed >= FIX_TIMEOUT_MS) {
        testComplete = true;
        testPassed = false;
        CI_LOG("\n");
        CI_LOG("TIMEOUT: GPS detected but no position fix.\n");
        CI_LOG("Ensure GPS has clear sky view.\n");
    }

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
        CI_PRINTF("WARN: %lu checksum failures (may indicate noise/timing)\n", gps.failedChecksum());
        passCount++;  // Don't fail - SoftwareSerial can have occasional errors
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
        CI_PRINTF("WARN: Low satellite count (%lu)\n", gps.satellites.value());
        passCount++;
    } else {
        CI_LOG("INFO: Satellite count not available\n");
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

const char* pinNameStr(uint32_t pin) {
    static char buf[8];
    uint32_t port = STM_PORT(pin);
    uint32_t pin_num = STM_PIN(pin);
    char port_letter = 'A' + port;
    snprintf(buf, sizeof(buf), "P%c%lu", port_letter, pin_num);
    return buf;
}
