/*
 * UF2 Bootloader Test
 *
 * Demonstrates CLI command to enter bootloader from running application.
 *
 * Bootloader entry methods:
 * 1. Button hold: Hold KEY button while pressing RESET (bootloader level)
 * 2. CLI command: Type "bl" in Serial Monitor (app level)
 *
 * Build with: UF2 Bootloader upload method (enables -DBL_TINYUF2)
 */

#include <bootloader.h>

#define LED_PIN PC13  // BlackPill onboard LED (active low)

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED off initially

  Serial.begin(115200);

  // Wait for USB enumeration
  delay(1000);

  // Startup blink: 3 fast blinks
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, LOW);   // LED on
    delay(100);
    digitalWrite(LED_PIN, HIGH);  // LED off
    delay(100);
  }

  Serial.println();
  Serial.println("UF2 Bootloader Test");
  Serial.println("===================");
  Serial.println();
  Serial.println("Bootloader entry methods:");
  Serial.println("  1. Hold KEY + press RESET");
  Serial.println("  2. Type 'bl' here");
  Serial.println();

#ifdef BL_TINYUF2
  Serial.println("[OK] BL_TINYUF2 defined - enterBootloader() available");
#else
  Serial.println("[WARN] BL_TINYUF2 NOT defined!");
  Serial.println("       Select 'UF2 Bootloader' upload method");
#endif

  Serial.println();
  Serial.println("LED blinking. Type 'bl' to enter bootloader...");
  Serial.println();
}

void loop() {
  // Slow blink: 500ms on, 500ms off
  digitalWrite(LED_PIN, LOW);   // LED on
  delay(500);
  digitalWrite(LED_PIN, HIGH);  // LED off
  delay(500);

  // Check for CLI commands
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "bl") {
#ifdef BL_TINYUF2
      Serial.println("Entering bootloader...");
      delay(100);  // Let message flush
      enterBootloader();
#else
      Serial.println("Error: BL_TINYUF2 not defined");
#endif
    } else if (cmd.length() > 0) {
      Serial.print("Unknown: ");
      Serial.println(cmd);
      Serial.println("Type 'bl' to enter bootloader");
    }
  }
}
