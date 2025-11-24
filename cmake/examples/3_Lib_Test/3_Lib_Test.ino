/*
 * Simple SPI library test sketch
 * Demonstrates SPI initialization and basic transaction
 */

#include <SPI.h>

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("SPI Test Starting...");

  // Initialize SPI with default settings
  SPI.begin();

  // Configure SPI settings: 1MHz, MSB first, Mode 0
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  Serial.println("SPI initialized successfully");
  Serial.print("MOSI: "); Serial.println(MOSI);
  Serial.print("MISO: "); Serial.println(MISO);
  Serial.print("SCK:  "); Serial.println(SCK);

  SPI.endTransaction();
  SPI.end();

  Serial.println("SPI Test Complete");
}

void loop() {
  // Nothing to do in loop
  delay(1000);
}
