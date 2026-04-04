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

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("libPrintf Basic Usage Example");
  Serial.println("Testing basic printf functionality:");

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

  Serial.println("All printf functions working correctly!");
  Serial.println("*STOP*");
}

void loop() {
  // Nothing in loop
}

// Custom putchar implementation for libPrintf output routing
#ifdef __cplusplus
extern "C" {
#endif

void putchar_(char c) {
  Serial.print(c);
}

#ifdef __cplusplus
}
#endif
