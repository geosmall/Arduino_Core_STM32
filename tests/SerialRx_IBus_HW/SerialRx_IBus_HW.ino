/*
 * IBus_PB7_Test - IBus RC receiver test on PB7 (USART1 RX)
 *
 * Hardware connections (NUCLEO_F411RE):
 *   RC Receiver IBus → PB7 (CN7-21, USART1 RX)
 *   RC Receiver GND  → GND
 *   RC Receiver VCC  → 5V (if receiver needs 5V power)
 *
 * Protocol: IBus @ 115200 baud
 * Expected: 14 RC channels (1000-2000 us typical range)
 */

#include <SerialRx.h>
#include <ci_log.h>

// USART1: PB7 = RX, PB6 = TX (not used for RX-only)
HardwareSerial SerialRC(PB7, PB6);

SerialRx rc;

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("IBus RC Receiver Test - PB7 (USART1)\n");
  CI_LOG("=====================================\n");

  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::IBUS;
  config.baudrate = 115200;
  config.timeout_ms = 1000;
  config.idle_threshold_us = 300;

  if (rc.begin(config)) {
    CI_LOG("RC Receiver initialized on PB7 @ 115200 baud\n");
    CI_LOG("Waiting for IBus frames...\n\n");
  } else {
    CI_LOG("ERROR: Failed to initialize RC receiver!\n");
    while (1);
  }

  CI_BUILD_INFO();
  CI_READY_TOKEN();
}

static bool signal_lost = false;
static uint32_t frame_count = 0;
static uint32_t last_print_time = 0;
static const uint32_t PRINT_INTERVAL_MS = 100;  // 10 Hz output throttle

#ifdef USE_RTT
static const uint32_t TEST_DURATION_MS = 15000;
static uint32_t test_start_time = 0;
#endif

void loop() {
#ifdef USE_RTT
  if (test_start_time == 0) {
    test_start_time = millis();
  }

  if (millis() - test_start_time >= TEST_DURATION_MS) {
    CI_LOG("\n=== 15-Second Test Complete ===\n");
    CI_LOGF("Frames received: %lu\n", rc.getFramesReceived());
    CI_LOGF("Frames failed:   %lu\n", rc.getFramesFailed());
    CI_LOG_FLOAT("Loss rate:       ", rc.getFrameLossPercent(), 2);
    CI_LOG("%\n");

    // Pass/fail criteria
    float loss = rc.getFrameLossPercent();
    uint32_t received = rc.getFramesReceived();

    if (received < 100) {
      CI_LOG("*FAIL* Insufficient frames received (minimum 100)\n");
    } else if (loss > 1.0f) {
      CI_LOG("*FAIL* Frame loss exceeds 1% threshold\n");
    } else {
      CI_LOG("*PASS* IBus reception validated\n");
    }

    CI_LOG("*STOP*\n");
    while (1);
  }
#endif

  // Call update() at full speed to drain serial buffer before overflow
  // IBus arrives at 143 Hz (32 bytes/frame), buffer is only 128 bytes
  rc.update();

  if (rc.available()) {
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      frame_count++;

      // Throttle output to 10 Hz (non-blocking)
      uint32_t now = millis();
      if (now - last_print_time >= PRINT_INTERVAL_MS) {
        last_print_time = now;

        // Print first 6 channels (AETR + 2 aux)
        CI_LOGF("[%4lu] Ch1:%4d Ch2:%4d Ch3:%4d Ch4:%4d Ch5:%4d Ch6:%4d\n",
                frame_count,
                msg.channels[0], msg.channels[1],
                msg.channels[2], msg.channels[3],
                msg.channels[4], msg.channels[5]);
      }

      signal_lost = false;
    }
  }

  if (rc.timeout(1000)) {
    if (!signal_lost) {
      CI_LOGF("WARNING: RC signal timeout (%lu ms)\n", rc.timeSinceLastMessage());
      signal_lost = true;
    }
  }
}
