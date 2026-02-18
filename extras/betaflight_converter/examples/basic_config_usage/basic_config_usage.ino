/*
 * Basic BoardConfig Usage Example
 *
 * Demonstrates how to use generated Betaflight BoardConfig headers
 * to initialize peripherals on flight controller hardware.
 *
 * This example works with any generated config (JHEF411, MATEKH743, etc.)
 */

#include <SPI.h>
#include <Wire.h>

// Include generated BoardConfig header directly from output directory
// This ensures we always use the latest generated config
#include "../../output/JHEF-JHEF411.h"

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n=== BoardConfig Usage Example ===\n");

  // Storage Configuration
  Serial.println("Storage Config:");
  Serial.print("  Type: ");
  Serial.println((BoardConfig::storage.backend_type == StorageBackend::LITTLEFS) ? "LittleFS" : "SDFS");
  Serial.print("  CS Pin: ");
  Serial.println(BoardConfig::storage.cs_pin);
  Serial.print("  Frequency: ");
  Serial.print(BoardConfig::storage.freq_hz / 1000000.0);
  Serial.println(" MHz");

  // IMU Configuration
  Serial.println("\nIMU Config:");
  Serial.print("  CS Pin: ");
  Serial.println(BoardConfig::imu.spi.cs_pin);
  Serial.print("  Interrupt Pin: ");
  Serial.println(BoardConfig::imu.int_pin);
  Serial.print("  Frequency: ");
  Serial.print(BoardConfig::imu.spi.freq_hz / 1000000.0);
  Serial.println(" MHz");

  // UART Configuration
  Serial.println("\nUART1 Config:");
  Serial.print("  TX Pin: ");
  Serial.println(BoardConfig::uart1.tx_pin);
  Serial.print("  RX Pin: ");
  Serial.println(BoardConfig::uart1.rx_pin);
  Serial.print("  Baud Rate: ");
  Serial.println(BoardConfig::uart1.baud_rate);

  // ADC Configuration
  Serial.println("\nBattery Monitoring:");
  Serial.print("  Voltage Pin: ");
  Serial.println(BoardConfig::battery.voltage_pin);
  Serial.print("  Current Pin: ");
  Serial.println(BoardConfig::battery.current_pin);
  Serial.print("  Voltage Scale: ");
  Serial.println(BoardConfig::battery.voltage_scale);
  Serial.print("  Current Scale: ");
  Serial.println(BoardConfig::battery.current_scale);

  // LED Configuration
  Serial.println("\nStatus LEDs:");
  Serial.print("  LED1 Pin: ");
  Serial.println(BoardConfig::status_leds.led1_pin);
  if (BoardConfig::status_leds.led2_pin != 0) {
    Serial.print("  LED2 Pin: ");
    Serial.println(BoardConfig::status_leds.led2_pin);
  }

  // Initialize status LED
  pinMode(BoardConfig::status_leds.led1_pin, OUTPUT);

  // Motor Configuration
  Serial.println("\nMotor Config:");
  Serial.print("  Frequency: ");
  Serial.print(BoardConfig::Motor::frequency_hz);
  Serial.println(" Hz");
  Serial.print("  Number of motors: ");

  Serial.println(BoardConfig::Motor::num_motors);

  Serial.println("\n=== Configuration Complete ===");
}

void loop() {
  // Blink status LED
  static unsigned long last_blink = 0;
  if (millis() - last_blink > 500) {
    digitalWrite(BoardConfig::status_leds.led1_pin, !digitalRead(BoardConfig::status_leds.led1_pin));
    last_blink = millis();
  }
}
