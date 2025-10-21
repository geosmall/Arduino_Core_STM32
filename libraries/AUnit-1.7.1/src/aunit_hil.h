// aunit_hil.h - AUnit HIL Integration Shim
// Integrates AUnit v1.7.1 with existing ci_log.h RTT/Serial abstraction and HIL workflow
#pragma once

#include <ci_log.h>
#include <AUnit.h>

// Custom printer that redirects AUnit output to ci_log system
class HILPrinter : public Print {
private:
  static int callCount;
  static char lineBuffer[256];
  static int bufferPos;

public:
  size_t write(uint8_t c) override {
    callCount++;

    // Buffer characters until we get a newline
    if (c == '\n') {
      lineBuffer[bufferPos] = '\0';
      CI_LOG(lineBuffer);
      CI_LOG("\n");
      bufferPos = 0;
    } else if (bufferPos < 255) {
      lineBuffer[bufferPos++] = c;
    }

    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    for (size_t i = 0; i < size; i++) {
      write(buffer[i]);
    }
    return size;
  }

  static int getCallCount() { return callCount; }
};

// HIL-specific TestRunner wrapper that adds exit wildcard functionality
namespace aunit_hil {

class HILTestRunner {
private:
  static HILPrinter hilPrinter;
  static bool isSetup;

public:
  // Initialize HIL testing environment
  static void setup() {
    if (isSetup) return;

    CI_LOG("=== AUnit HIL Test Runner ===\n");
    CI_BUILD_INFO();
    CI_READY_TOKEN();

    // Set AUnit to use our HIL printer
    aunit::TestRunner::setPrinter(&hilPrinter);

    // Enable all verbosity for comprehensive test output
    aunit::TestRunner::setVerbosity(aunit::Verbosity::kAll);

    isSetup = true;
  }

  // Run tests with HIL integration
  static void run() {
    setup();

    // Run AUnit tests for sufficient iterations to complete all tests
    // TestRunner::run() is designed to be called in loop() - each call advances the state machine
    // We use a fixed iteration approach to ensure all output is captured
    for (int i = 0; i < 100; i++) {
      aunit::TestRunner::run();
      delay(10);  // Small delay to prevent tight loop issues and allow proper test execution
    }

    // Emit completion after tests run
    emitTestCompletion();

    // In Serial mode, add delay to allow user to observe output before next iteration
    // In RTT mode, J-Run will exit on *STOP* so this delay won't affect HIL testing
#ifndef USE_RTT
    delay(1000);  // 1 second delay for Serial mode observation
#endif
  }


  // Emit HIL-compatible test completion signal
  static void emitTestCompletion() {
    CI_LOG("=== AUnit HIL Test Complete ===\n");

    // Get test statistics from TestRunner internals
    // Note: We'll need to access private members via friendship or public accessors
    CI_LOG("Tests completed - check output above for results\n");

    // Emit exit wildcard for deterministic HIL completion
    CI_LOG("*STOP*\n");
  }

  // Convenience methods that delegate to AUnit TestRunner
  static void exclude(const char* pattern) {
    aunit::TestRunner::exclude(pattern);
  }

  static void include(const char* pattern) {
    aunit::TestRunner::include(pattern);
  }

  static void setTimeout(uint16_t seconds) {
    aunit::TestRunner::setTimeout(seconds);
  }
};

// Static member definitions
HILPrinter HILTestRunner::hilPrinter;
bool HILTestRunner::isSetup = false;

} // namespace aunit_hil

// Static member definitions (outside namespace)
int HILPrinter::callCount = 0;
char HILPrinter::lineBuffer[256];
int HILPrinter::bufferPos = 0;

// Convenience macros for HIL testing
#define HIL_TEST_SETUP() aunit_hil::HILTestRunner::setup()
#define HIL_TEST_RUN() aunit_hil::HILTestRunner::run()
#define HIL_TEST_EXCLUDE(pattern) aunit_hil::HILTestRunner::exclude(pattern)
#define HIL_TEST_INCLUDE(pattern) aunit_hil::HILTestRunner::include(pattern)
#define HIL_TEST_TIMEOUT(seconds) aunit_hil::HILTestRunner::setTimeout(seconds)