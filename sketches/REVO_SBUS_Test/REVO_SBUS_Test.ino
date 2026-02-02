/*
 * REVO_SBUS_Test - SBUS RC receiver test for OpenPilot Revolution F405
 *
 * Hardware-validated configuration:
 *   - RC Input: PA10 (USART1 RX)
 *   - Inverter: PC0 (HIGH = SBUS inverted mode)
 *   - Protocol: SBUS @ 100000 baud, 8E2
 *
 * Build:
 *   ./ci/saflash.sh Arduino_Core_STM32/sketches/REVO_SBUS_Test STM32_Robotics:stm32:FlightCtr:pnum=OPEN_REVO
 */

#include <SerialRx.h>

// REVO F405 RC Input pins (hardware-validated)
#define RC_RX_PIN       PA10  // USART1 RX
#define RC_TX_PIN       PA9   // USART1 TX
#define RC_INVERTER_PIN PC0   // Hardware inverter control

// Create serial port for RC input
HardwareSerial SerialRC(RC_RX_PIN, RC_TX_PIN);

// SerialRx instance
SerialRx rc;

bool inverterState = true;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n=== REVO F405 SBUS Test ===");
  Serial.println("RC Input: PA10 (USART1 RX)");
  Serial.println("Inverter: PC0");

  // Enable hardware inverter for SBUS
  pinMode(RC_INVERTER_PIN, OUTPUT);
  digitalWrite(RC_INVERTER_PIN, HIGH);
  Serial.println("Inverter: HIGH (SBUS mode)");

  // Configure SerialRx for SBUS
  SerialRx::Config config;
  config.serial = &SerialRC;
  config.rx_protocol = SerialRx::SBUS;
  config.baudrate = 100000;
  config.timeout_ms = 500;

  if (rc.begin(config)) {
    Serial.println("SerialRx initialized");
    Serial.println("\nTest sequence:");
    Serial.println("  0-5s:  Inverter HIGH (should work)");
    Serial.println("  5-10s: Inverter LOW (should fail)");
    Serial.println("  10-15s: Inverter HIGH (should recover)");
    Serial.println("");
  } else {
    Serial.println("ERROR: SerialRx init failed!");
    Serial.println("*STOP*");
    while (1);
  }
}

void loop() {
  static uint32_t testStart = millis();
  static uint32_t lastPrint = 0;
  static uint32_t lastStatus = 0;
  static bool signalLost = false;
  static uint32_t framesAtToggle = 0;

  uint32_t elapsed = millis() - testStart;

  // Toggle inverter at 5s and 10s
  if (elapsed >= 5000 && elapsed < 10000 && inverterState) {
    framesAtToggle = rc.getFramesReceived();
    digitalWrite(RC_INVERTER_PIN, LOW);
    inverterState = false;
    Serial.println("\n>>> Inverter: LOW (disabled) - expecting signal loss");
  } else if (elapsed >= 10000 && !inverterState) {
    digitalWrite(RC_INVERTER_PIN, HIGH);
    inverterState = true;
    Serial.print("\n>>> Inverter: HIGH (enabled) - frames during LOW: ");
    Serial.println(rc.getFramesReceived() - framesAtToggle);
  }

  // End test at 15s
  if (elapsed >= 15000) {
    Serial.println("\n=== Test Complete ===");
    Serial.print("Total frames: ");
    Serial.println(rc.getFramesReceived());
    Serial.print("Failed: ");
    Serial.println(rc.getFramesFailed());
    Serial.println("*STOP*");
    while (1);
  }

  // Update RC receiver
  rc.update();

  // Status every second
  if (millis() - lastStatus >= 1000) {
    lastStatus = millis();
    Serial.print("[");
    Serial.print(elapsed / 1000);
    Serial.print("s] Inv=");
    Serial.print(inverterState ? "HIGH" : "LOW ");
    Serial.print(" Frames=");
    Serial.print(rc.getFramesReceived());
    Serial.print(" Timeout=");
    Serial.println(rc.timeout(500) ? "YES" : "no");
  }

  // Print channels at 10 Hz when receiving
  if (rc.available() && millis() - lastPrint >= 100) {
    lastPrint = millis();
    RCMessage msg;
    if (rc.getMessage(&msg)) {
      Serial.print("  CH: ");
      for (int i = 0; i < 4; i++) {
        Serial.print(msg.channels[i]);
        Serial.print(" ");
      }
      Serial.println();

      if (signalLost) {
        Serial.println("  Signal recovered!");
        signalLost = false;
      }
    }
  }

  // Track signal loss
  if (rc.timeout(500) && !signalLost) {
    Serial.println("  WARNING: Signal timeout!");
    signalLost = true;
  }
}
