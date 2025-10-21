/**
 * libPrintf - Embedded Printf Library for Arduino STM32
 *
 * A lightweight, embedded-friendly printf implementation that eliminates
 * the need for nanofp runtime libraries and provides reliable float formatting.
 *
 * Based on eyalroz/printf v6.2.0 (https://github.com/eyalroz/printf)
 *
 * KEY BENEFITS:
 * - Eliminates nanofp/nano confusion and FQBN complexity
 * - Reduces binary size by ~20% compared to nanofp
 * - Provides reliable float formatting without build configuration
 * - Compatible with factory code using standard printf/fprintf calls
 * - Single include for complete printf family replacement
 *
 * USAGE:
 *   #include <libPrintf.h>
 *
 * This automatically enables function aliasing so that all standard printf
 * family calls (printf, sprintf, fprintf, etc.) use the embedded implementation.
 *
 * TECHNICAL NOTES:
 * - Uses PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT for seamless integration
 * - Works with RTT, Serial, or any custom putchar_() implementation
 * - Thread-safe and suitable for embedded real-time applications
 * - Supports all standard printf format specifiers including %f, %g, %e
 */

#ifndef LIBPRINTF_H
#define LIBPRINTF_H

// Enable automatic aliasing of standard printf functions
#define PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT 1

// Include the embedded printf implementation
#include "printf.h"

// Version information
#define LIBPRINTF_VERSION_MAJOR 6
#define LIBPRINTF_VERSION_MINOR 2
#define LIBPRINTF_VERSION_PATCH 0
#define LIBPRINTF_VERSION "6.2.0"

#endif // LIBPRINTF_H