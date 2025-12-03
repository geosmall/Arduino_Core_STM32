// BasicExample_CI.ino - TinyGPSPlus CI/HIL test
//
// Tests TinyGPSPlus NMEA parsing using static test data (no GPS device needed).
// Validates location, date, time, altitude, and satellite parsing.
//
// Usage:
//   Serial: Arduino IDE monitor (115200 baud)
//   RTT:    ./system/ci/aflash.sh libraries/TinyGPSPlus/examples/BasicExample_CI --use-rtt --build-id

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

// Sample NMEA stream with multiple sentences
// Contains: 3x GPRMC (position/velocity), 3x GPGGA (position/altitude/satellites)
const char *gpsStream =
  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";

// Expected values for validation (from last complete fix)
const double EXPECTED_LAT = 30.240455;    // 30°14.4273'N
const double EXPECTED_LNG = -97.817713;   // 97°49.0628'W
const double EXPECTED_ALT = 206.9;        // meters
const uint32_t EXPECTED_SATS = 9;
const uint16_t EXPECTED_YEAR = 2013;
const uint8_t EXPECTED_MONTH = 9;
const uint8_t EXPECTED_DAY = 3;

// Tolerance for floating point comparisons
const double COORD_TOLERANCE = 0.0001;    // ~11 meters
const double ALT_TOLERANCE = 0.5;         // meters

TinyGPSPlus gps;
int sentenceCount = 0;
int passCount = 0;
int failCount = 0;

void displayInfo();
bool validateFinalFix();

void setup()
{
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n");
    CI_LOG("=== TinyGPSPlus BasicExample_CI ===\n");
    CI_PRINTF("Library version: %s\n", TinyGPSPlus::libraryVersion());
    CI_READY_TOKEN();
    CI_LOG("\n");

    // Process the NMEA stream character by character
    CI_LOG("Processing NMEA sentences...\n");
    CI_LOG("----------------------------------------\n");

    const char *p = gpsStream;
    while (*p) {
        if (gps.encode(*p++)) {
            sentenceCount++;
            displayInfo();
        }
    }

    CI_LOG("----------------------------------------\n");
    CI_PRINTF("Sentences processed: %d\n", sentenceCount);
    CI_PRINTF("Characters processed: %lu\n", gps.charsProcessed());
    CI_PRINTF("Sentences with fix: %lu\n", gps.sentencesWithFix());
    CI_PRINTF("Checksum passed: %lu\n", gps.passedChecksum());
    CI_PRINTF("Checksum failed: %lu\n", gps.failedChecksum());
    CI_LOG("\n");

    // Validate final parsed values
    CI_LOG("=== Validation ===\n");
    bool allPassed = validateFinalFix();

    CI_LOG("\n");
    CI_PRINTF("Tests passed: %d\n", passCount);
    CI_PRINTF("Tests failed: %d\n", failCount);
    CI_LOG("\n");

    if (allPassed && failCount == 0) {
        CI_LOG("*TEST_PASS*\n");
    } else {
        CI_LOG("*TEST_FAIL*\n");
    }

    CI_LOG("*STOP*\n");
}

void loop()
{
    // Test complete
}

void displayInfo()
{
    CI_PRINTF("[%d] ", sentenceCount);

    if (gps.location.isValid()) {
        CI_PRINTF("Loc: %.6f,%.6f  ", gps.location.lat(), gps.location.lng());
    } else {
        CI_LOG("Loc: INVALID  ");
    }

    if (gps.date.isValid()) {
        CI_PRINTF("Date: %d/%d/%d  ", gps.date.month(), gps.date.day(), gps.date.year());
    } else {
        CI_LOG("Date: INVALID  ");
    }

    if (gps.time.isValid()) {
        CI_PRINTF("Time: %02d:%02d:%02d.%02d  ",
            gps.time.hour(), gps.time.minute(),
            gps.time.second(), gps.time.centisecond());
    } else {
        CI_LOG("Time: INVALID  ");
    }

    if (gps.altitude.isValid()) {
        CI_PRINTF("Alt: %.1fm  ", gps.altitude.meters());
    }

    if (gps.satellites.isValid()) {
        CI_PRINTF("Sats: %lu", gps.satellites.value());
    }

    CI_LOG("\n");
}

bool validateFinalFix()
{
    bool allPassed = true;

    // Validate location
    if (gps.location.isValid()) {
        double latDiff = fabs(gps.location.lat() - EXPECTED_LAT);
        double lngDiff = fabs(gps.location.lng() - EXPECTED_LNG);

        if (latDiff < COORD_TOLERANCE && lngDiff < COORD_TOLERANCE) {
            CI_PRINTF("PASS: Location (%.6f, %.6f)\n", gps.location.lat(), gps.location.lng());
            passCount++;
        } else {
            CI_PRINTF("FAIL: Location (%.6f, %.6f) expected (%.6f, %.6f)\n",
                gps.location.lat(), gps.location.lng(), EXPECTED_LAT, EXPECTED_LNG);
            failCount++;
            allPassed = false;
        }
    } else {
        CI_LOG("FAIL: Location invalid\n");
        failCount++;
        allPassed = false;
    }

    // Validate altitude
    if (gps.altitude.isValid()) {
        double altDiff = fabs(gps.altitude.meters() - EXPECTED_ALT);
        if (altDiff < ALT_TOLERANCE) {
            CI_PRINTF("PASS: Altitude %.1f m\n", gps.altitude.meters());
            passCount++;
        } else {
            CI_PRINTF("FAIL: Altitude %.1f m, expected %.1f m\n",
                gps.altitude.meters(), EXPECTED_ALT);
            failCount++;
            allPassed = false;
        }
    } else {
        CI_LOG("FAIL: Altitude invalid\n");
        failCount++;
        allPassed = false;
    }

    // Validate satellites
    if (gps.satellites.isValid()) {
        if (gps.satellites.value() == EXPECTED_SATS) {
            CI_PRINTF("PASS: Satellites %lu\n", gps.satellites.value());
            passCount++;
        } else {
            CI_PRINTF("FAIL: Satellites %lu, expected %lu\n",
                gps.satellites.value(), EXPECTED_SATS);
            failCount++;
            allPassed = false;
        }
    } else {
        CI_LOG("FAIL: Satellites invalid\n");
        failCount++;
        allPassed = false;
    }

    // Validate date
    if (gps.date.isValid()) {
        if (gps.date.year() == EXPECTED_YEAR &&
            gps.date.month() == EXPECTED_MONTH &&
            gps.date.day() == EXPECTED_DAY) {
            CI_PRINTF("PASS: Date %d/%d/%d\n",
                gps.date.month(), gps.date.day(), gps.date.year());
            passCount++;
        } else {
            CI_PRINTF("FAIL: Date %d/%d/%d, expected %d/%d/%d\n",
                gps.date.month(), gps.date.day(), gps.date.year(),
                EXPECTED_MONTH, EXPECTED_DAY, EXPECTED_YEAR);
            failCount++;
            allPassed = false;
        }
    } else {
        CI_LOG("FAIL: Date invalid\n");
        failCount++;
        allPassed = false;
    }

    // Validate time (just check valid, since it varies with each sentence)
    if (gps.time.isValid()) {
        CI_PRINTF("PASS: Time %02d:%02d:%02d.%02d\n",
            gps.time.hour(), gps.time.minute(),
            gps.time.second(), gps.time.centisecond());
        passCount++;
    } else {
        CI_LOG("FAIL: Time invalid\n");
        failCount++;
        allPassed = false;
    }

    // Validate checksum stats
    if (gps.failedChecksum() == 0 && gps.passedChecksum() > 0) {
        CI_PRINTF("PASS: Checksum (0 failed, %lu passed)\n", gps.passedChecksum());
        passCount++;
    } else {
        CI_PRINTF("FAIL: Checksum (%lu failed, %lu passed)\n",
            gps.failedChecksum(), gps.passedChecksum());
        failCount++;
        allPassed = false;
    }

    return allPassed;
}
