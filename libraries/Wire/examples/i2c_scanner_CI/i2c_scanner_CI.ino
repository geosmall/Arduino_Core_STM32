// i2c_scanner_CI.ino - I2C bus scanner for CI/HIL testing
//
// Scans I2C bus once and reports all detected devices.
// Uses default Wire pins (D14=SDA, D15=SCL on NUCLEO_F411RE)

#include <Wire.h>
#include <ci_log.h>

void setup() {
#ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial && millis() < 3000);
#endif

    CI_BUILD_INFO();
    CI_LOG("\n=== I2C Scanner CI ===\n");
    CI_READY_TOKEN();
    CI_LOG("\n");

    Wire.begin();
    CI_LOG("Scanning I2C bus (0x01-0x7F)...\n\n");

    int nDevices = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            CI_LOGF("Found device at 0x%02X\n", address);
            nDevices++;
        } else if (error == 4) {
            CI_LOGF("Error at 0x%02X\n", address);
        }
    }

    CI_LOG("\n");
    if (nDevices == 0) {
        CI_LOG("No I2C devices found\n");
    } else {
        CI_LOGF("Found %d device(s)\n", nDevices);
    }

    CI_LOG("\n*STOP*\n");
}

void loop() {
    // Scan complete
}
