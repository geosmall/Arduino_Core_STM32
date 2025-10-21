/*
 * libPrintf Basic Usage Example
 *
 * Demonstrates the libPrintf library for embedded-friendly printf functionality
 * that eliminates nanofp confusion and reduces binary size.
 *
 * Key Features:
 * - Automatic function aliasing (printf, sprintf, fprintf work seamlessly)
 * - Reliable float formatting without build configuration
 * - ~20% binary size reduction compared to nanofp
 * - Compatible with factory code using standard printf calls
 */

#include <libPrintf.h>
#include <ci_log.h>

void setup() {
  #ifndef USE_RTT
    Serial.begin(115200);
    while (!Serial) delay(10);
  #endif

  CI_LOG("libPrintf Basic Usage Example\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Test basic string formatting
  CI_LOG("Testing basic printf functionality:\n");

  // Integer formatting
  printf("Integer: %d\n", 42);

  // Float formatting (this is where libPrintf shines!)
  printf("Float: %.6f\n", 3.14159265);
  printf("Scientific: %.2e\n", 1234.567);

  // String formatting
  printf("String: %s\n", "Hello libPrintf!");

  // Mixed formatting
  printf("Mixed: %s has %d characters and pi ≈ %.3f\n",
         "libPrintf", 9, 3.14159);

  // Buffer formatting with sprintf
  char buffer[128];
  sprintf(buffer, "Formatted to buffer: %.2f%% complete", 85.75);
  printf("Buffer result: %s\n", buffer);

  // Demonstrate stderr compatibility (important for factory code)
  fprintf(stderr, "Error message via fprintf to stderr\n");

  CI_LOG("All printf functions working correctly!\n");
  CI_LOG("*STOP*\n");
}

void loop() {
  // Nothing in loop
}

// Custom putchar implementation for RTT/Serial routing
#ifdef __cplusplus
extern "C" {
#endif

void putchar_(char c) {
#ifdef USE_RTT
  char buf[2] = {c, '\0'};
  SEGGER_RTT_WriteString(0, buf);
#else
  Serial.print(c);
#endif
}

#ifdef __cplusplus
}
#endif