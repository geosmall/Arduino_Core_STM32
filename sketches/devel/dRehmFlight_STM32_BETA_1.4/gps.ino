//Arduino/Teensy Flight Controller - dRehmFlight
//Author: Nicholas Rehm
//Project Start: 1/6/2020
//Last Updated: 7/29/2022
//Version: Beta 1.3 (original Teensy)
//
//STM32 Port: BETA 1.4 - GPS integration using TinyGPSPlus and UART DMA
//Target: STM32F4/H7 (NUCLEO_F411RE, NOXE V3, MATEK H743)

//========================================================================================================================//

//This file contains GPS functions using TinyGPSPlus library with optional UART DMA for reduced interrupt overhead.
//GPS data is updated each loop iteration and made available to flight controller.

#ifdef USE_GPS

#include <TinyGPSPlus.h>

// GPS serial port using BoardConfig pins
HardwareSerial SerialGPS(BoardConfig::gps.rx_pin, BoardConfig::gps.tx_pin);

// DMA buffer for GPS UART - reduces interrupt overhead from ~960/sec to ~12/sec (IDLE only)
// On H7: Must use SERIAL_DMA_BUFFER macro for D2 SRAM3 placement
SERIAL_DMA_BUFFER uint8_t gpsDmaBuffer[256];

// TinyGPSPlus parser instance
TinyGPSPlus gps;

// GPS data - updated by getGPSdata(), available to flight controller
float gps_latitude = 0.0f;
float gps_longitude = 0.0f;
float gps_altitude_m = 0.0f;
float gps_speed_mps = 0.0f;
float gps_course_deg = 0.0f;
uint32_t gps_satellites = 0;
bool gps_fix_valid = false;
uint32_t gps_fix_age_ms = 0;

void gpsSetup() {
  //DESCRIPTION: Initialize GPS serial with optional DMA mode

#ifdef USE_GPS_DMA
  // DMA mode - fail loudly if DMA initialization fails
  if (!SerialGPS.beginDMA(BoardConfig::gps.baud_rate, gpsDmaBuffer, sizeof(gpsDmaBuffer))) {
    CI_LOG("ERROR: GPS DMA init failed! Check:\n");
    CI_LOG("  - UART supports DMA (USART1/2/6 on F4, USART1/2/3/4/6 on H7)\n");
    CI_LOG("  - Buffer uses SERIAL_DMA_BUFFER on H7\n");
    while (1) { delay(1000); }  // Halt - don't silently degrade
  }
  CI_LOG("GPS initialized (DMA mode)\n");
#else
  // Standard interrupt mode
  SerialGPS.begin(BoardConfig::gps.baud_rate);
  CI_LOG("GPS initialized (interrupt mode)\n");
#endif
}

void getGPSdata() {
  //DESCRIPTION: Read available GPS data and update global variables
  //Feed all available bytes to TinyGPSPlus parser

  while (SerialGPS.available()) {
    gps.encode(SerialGPS.read());
  }

  // Update GPS data variables when new data available
  if (gps.location.isUpdated()) {
    gps_latitude = gps.location.lat();
    gps_longitude = gps.location.lng();
    gps_fix_valid = gps.location.isValid();
    gps_fix_age_ms = gps.location.age();
  }

  if (gps.altitude.isUpdated()) {
    gps_altitude_m = gps.altitude.meters();
  }

  if (gps.speed.isUpdated()) {
    gps_speed_mps = gps.speed.mps();
  }

  if (gps.course.isUpdated()) {
    gps_course_deg = gps.course.deg();
  }

  if (gps.satellites.isUpdated()) {
    gps_satellites = gps.satellites.value();
  }
}

void printGPSdata() {
  //DESCRIPTION: Print GPS data for debugging (call at ~1Hz, not every loop)

  if (gps_fix_valid) {
    CI_LOGF("GPS: %.6f, %.6f, Alt: %.1fm, Spd: %.1fm/s, Sats: %lu\n",
            gps_latitude, gps_longitude, gps_altitude_m, gps_speed_mps, gps_satellites);
  } else {
    CI_LOGF("GPS: No fix (sats: %lu, age: %lums)\n", gps_satellites, gps_fix_age_ms);
  }
}

#endif // USE_GPS
