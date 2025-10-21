#include <aunit_hil.h>

/*
 * AUnit Pilot Test - Single sketch supporting both Arduino IDE (Serial) and J-Run/RTT modes
 * Controlled via USE_RTT compile flag for deterministic HIL testing.
 *
 * Arduino IDE mode: Serial output with manual monitoring
 * J-Run/RTT mode:   RTT output with deterministic exit tokens
 */

// Basic functionality tests
test(basic_arithmetic) {
  assertEqual(2 + 2, 4);
  assertEqual(10 - 3, 7);
  assertEqual(3 * 4, 12);
}

test(boolean_logic) {
  assertTrue(true);
  assertFalse(false);
  assertEqual(true && true, true);
  assertEqual(true || false, true);
}

test(string_operations) {
  String test_string = "Hello";
  assertEqual((int)test_string.length(), 5);
  assertTrue(test_string.equals("Hello"));
}

// Test that should fail (commented out by default)
// test(intentional_failure) {
//   assertEqual(1, 2);  // This will fail
// }

void setup() {
#ifndef USE_RTT
  // Arduino IDE mode: Initialize Serial and wait
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
#else
  // RTT mode: Initialize RTT
  SEGGER_RTT_Init();
#endif

  // Initialize HIL testing environment
  HIL_TEST_SETUP();

  // Optional: Configure test filtering
  // HIL_TEST_EXCLUDE("intentional_failure");

  // Optional: Set timeout (default is 10 seconds)
  HIL_TEST_TIMEOUT(30);
}

void loop() {
  // Run tests with HIL integration
  HIL_TEST_RUN();
}