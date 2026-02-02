/*
 * PinWiggle - UART Pin Activity Detector
 *
 * Diagnostic tool for identifying which MCU pin receives RC receiver data.
 * Monitors all UART TX/RX pins and reports transition counts per second.
 *
 * Protocol-agnostic: works with SBUS, iBus, DSM, or any serial signal.
 * Tests both inverter states (if hardware inverter present):
 * - Inverter ON (HIGH): For inverted protocols like SBUS
 * - Inverter OFF (LOW): For non-inverted protocols like iBus, DSM
 *
 * Usage:
 * 1. Flash to target board
 * 2. Connect RC receiver to suspected input pin
 * 3. Power receiver and bind to transmitter
 * 4. Monitor serial output - active pin shows high transition count (~27k/sec for SBUS)
 * 5. Press 'a' for auto-detect (tests both inverter states)
 *
 * Commands:
 *   'i' - Toggle inverter pin
 *   'r' - Reset counters
 *   'a' - Auto-detect (tests both inverter states)
 *
 * Board-specific: Modify pins[] array for target board's UART pinout.
 * Current config: OpenPilot Revolution F405 (all UARTs except USART3/debug)
 */

// Candidate RX pins to monitor
struct PinMonitor {
  const char* name;
  uint32_t pin;
  volatile uint32_t transitions;
  uint8_t lastState;
};

// Test ALL UART RX and TX pins (RC input could be on either)
// NOTE: PB10/PB11 (USART3) excluded - used for debug serial output
PinMonitor pins[] = {
  // USART1
  {"PA9  (USART1 TX)", PA9,  0, 0},
  {"PA10 (USART1 RX)", PA10, 0, 0},
  {"PB_6 (USART1 TX alt)", PB_6, 0, 0},
  {"PB_7 (USART1 RX alt)", PB_7, 0, 0},
  // USART2
  {"PA2  (USART2 TX)", PA2,  0, 0},  // Motor 4
  {"PA3  (USART2 RX)", PA3,  0, 0},  // Motor 3
  // USART6
  {"PC6  (USART6 TX)", PC6,  0, 0},
  {"PC7  (USART6 RX)", PC7,  0, 0},
  // UART4
  {"PA0  (UART4 TX)",  PA0,  0, 0},  // Motor 6
  {"PA1  (UART4 RX)",  PA1,  0, 0},  // Motor 5
  {"PC10 (UART4 TX alt)", PC10, 0, 0},  // SPI3 SCK
  {"PC11 (UART4 RX alt)", PC11, 0, 0},  // SPI3 MISO
  // PPM
  {"PB14 (PPM/PWM1)",  PB14, 0, 0},
};

const int NUM_PINS = sizeof(pins) / sizeof(pins[0]);

// Inverter control pin
#define INVERTER_PIN PC0

uint32_t lastReport = 0;
const uint32_t REPORT_INTERVAL = 2000;  // Report every 2 seconds

uint32_t sampleCount = 0;
bool inverterState = true;  // Start with inverter ON for SBUS
uint32_t inverterToggleTime = 0;
const uint32_t INVERTER_TOGGLE_INTERVAL = 6000;  // Toggle every 6 seconds

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("\n=== PinWiggle - UART Pin Activity Detector ===");
  Serial.println("Monitoring candidate RX pins for signal activity...\n");

  // Configure all monitor pins as inputs with pullup
  for (int i = 0; i < NUM_PINS; i++) {
    pinMode(pins[i].pin, INPUT_PULLUP);
    pins[i].lastState = digitalRead(pins[i].pin);
    pins[i].transitions = 0;
  }

  // Configure and test inverter control
  pinMode(INVERTER_PIN, OUTPUT);

  Serial.println("Inverter pin: PC0");
  Serial.println("  LOW  = non-inverted (iBus, DSM, etc.)");
  Serial.println("  HIGH = inverted (SBUS)");
  Serial.println("");
  digitalWrite(INVERTER_PIN, HIGH);  // Start with inverter ON

  Serial.println("Connect receiver and watch for activity.\n");
  Serial.println("Pin Name             | Transitions/sec | State | Activity");
  Serial.println("---------------------|-----------------|-------|----------");
}

void loop() {
  // Fast polling loop - sample all pins
  for (int i = 0; i < NUM_PINS; i++) {
    uint8_t state = digitalRead(pins[i].pin);
    if (state != pins[i].lastState) {
      pins[i].transitions++;
      pins[i].lastState = state;
    }
  }
  sampleCount++;

  // Periodic report
  if (millis() - lastReport >= REPORT_INTERVAL) {
    lastReport = millis();

    Serial.print("\033["); // ANSI escape to move cursor up
    Serial.print(NUM_PINS);
    Serial.println("A");

    for (int i = 0; i < NUM_PINS; i++) {
      uint32_t trans = pins[i].transitions;
      pins[i].transitions = 0;  // Reset counter

      // Determine activity level
      const char* activity;
      if (trans > 10000) {
        activity = "*** SERIAL ***";  // High activity = likely serial data
      } else if (trans > 1000) {
        activity = "** ACTIVE **";
      } else if (trans > 100) {
        activity = "* some *";
      } else if (trans > 0) {
        activity = "minimal";
      } else {
        activity = "none";
      }

      char buf[80];
      snprintf(buf, sizeof(buf), "%-20s | %15lu | %5s | %s",
               pins[i].name,
               trans,
               pins[i].lastState ? "HIGH" : "LOW",
               activity);
      Serial.println(buf);
    }

    Serial.print("\nSamples/sec: ");
    Serial.print(sampleCount);
    Serial.print("  | Inverter PC0 = ");
    Serial.println(inverterState ? "HIGH (inverted)" : "LOW (non-inverted)");
    Serial.println("'i'=toggle inverter, 'r'=reset, 'a'=auto-detect\n");
    sampleCount = 0;
  }

  // Check for serial commands
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'i' || c == 'I') {
      inverterState = !inverterState;
      digitalWrite(INVERTER_PIN, inverterState ? HIGH : LOW);
      Serial.print("\n>>> Inverter: ");
      Serial.println(inverterState ? "ON (inverted/SBUS)" : "OFF (non-inverted/iBus)");
    } else if (c == 'r' || c == 'R') {
      for (int i = 0; i < NUM_PINS; i++) {
        pins[i].transitions = 0;
      }
      Serial.println("\n>>> Counters reset");
    } else if (c == 'a' || c == 'A') {
      Serial.println("\n>>> Auto-detect: Testing both inverter states...");

      // Test inverter OFF (iBus/DSM)
      digitalWrite(INVERTER_PIN, LOW);
      inverterState = false;
      delay(500);
      for (int i = 0; i < NUM_PINS; i++) pins[i].transitions = 0;
      delay(1000);

      uint32_t maxOff = 0;
      int pinOff = -1;
      for (int i = 0; i < NUM_PINS; i++) {
        if (pins[i].transitions > maxOff) {
          maxOff = pins[i].transitions;
          pinOff = i;
        }
      }

      // Test inverter ON (SBUS)
      digitalWrite(INVERTER_PIN, HIGH);
      inverterState = true;
      delay(500);
      for (int i = 0; i < NUM_PINS; i++) pins[i].transitions = 0;
      delay(1000);

      uint32_t maxOn = 0;
      int pinOn = -1;
      for (int i = 0; i < NUM_PINS; i++) {
        if (pins[i].transitions > maxOn) {
          maxOn = pins[i].transitions;
          pinOn = i;
        }
      }

      Serial.println("\n>>> Auto-detect results:");
      Serial.print("    Inverter OFF: ");
      if (maxOff > 1000) {
        Serial.print(pins[pinOff].name);
        Serial.print(" = ");
        Serial.print(maxOff);
        Serial.println(" trans (iBus/DSM likely)");
      } else {
        Serial.println("No activity");
      }

      Serial.print("    Inverter ON:  ");
      if (maxOn > 1000) {
        Serial.print(pins[pinOn].name);
        Serial.print(" = ");
        Serial.print(maxOn);
        Serial.println(" trans (SBUS likely)");
      } else {
        Serial.println("No activity");
      }

      // Set to state with most activity
      if (maxOn > maxOff) {
        digitalWrite(INVERTER_PIN, HIGH);
        inverterState = true;
        Serial.println("    -> Using inverted mode (SBUS)");
      } else if (maxOff > 0) {
        digitalWrite(INVERTER_PIN, LOW);
        inverterState = false;
        Serial.println("    -> Using non-inverted mode (iBus/DSM)");
      }
    }
  }
}
