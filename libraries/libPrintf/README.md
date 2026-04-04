# libPrintf - Embedded Printf Library for Arduino STM32

A lightweight, embedded-friendly printf implementation that eliminates nanofp confusion and provides reliable float formatting for STM32 Arduino projects.

## Overview

libPrintf is an **optional Arduino library** based on [eyalroz/printf v6.2.0](https://github.com/eyalroz/printf) that provides embedded printf functionality for STM32 Arduino sketches. This library eliminates nanofp configuration complexity and provides reliable float formatting with significant binary size reductions.

## Key Benefits

- ✅ **Eliminates nanofp confusion**: No more complex FQBN configurations or rtlib settings
- ✅ **Reduces binary size**: ~20% reduction compared to nanofp (typically 8KB+ savings)
- ✅ **Reliable float formatting**: Works without build configuration complexity
- ✅ **Factory code compatible**: Seamless integration with existing printf/fprintf calls
- ✅ **Thread-safe**: Suitable for embedded real-time applications
- ✅ **Complete printf family**: Supports printf, sprintf, fprintf, snprintf, vprintf, etc.
- ✅ **Flexible output routing**: Custom putchar_() implementation for Serial, RTT, etc.

## Quick Start

### Installation

Copy the `libPrintf` directory to your Arduino `libraries/` folder, or place it in your project's libraries directory.

### Basic Usage

```cpp
#include <libPrintf.h>

void setup() {
  Serial.begin(115200);  // Required: libPrintf routes to Serial by default

  // All standard printf functions now work with float support
  printf("Pi = %.6f\n", 3.14159265);
  printf("Integer: %d, String: %s\n", 42, "Hello");

  // Works with sprintf too
  char buffer[64];
  sprintf(buffer, "Formatted: %.2f%%", 85.75);
  printf("Buffer: %s\n", buffer);
}
```

### Build Requirements

- **Arduino IDE**: 1.8.x or Arduino CLI
- **STM32 Core**: 2.7.1 or newer
- **FQBN**: Standard configuration (no rtlib needed!)
  ```
  STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE
  ```

## Technical Details

### Function Aliasing

libPrintf uses `PRINTF_ALIAS_STANDARD_FUNCTION_NAMES_SOFT` to automatically replace standard printf functions when the library is included:

- `printf()` → `printf_()`
- `sprintf()` → `sprintf_()`
- `fprintf()` → `fprintf_()`
- `snprintf()` → `snprintf_()`
- And all other printf family functions...

This aliasing is completely transparent - your existing code works unchanged once you include `<libPrintf.h>`.

### Binary Size Comparison

| Configuration | Binary Size | Savings |
|---------------|-------------|---------|
| Standard newlib | ~42KB | - |
| newlib nano + nanofp | ~42KB | 0% |
| newlib nano + libPrintf | ~34KB | **17%** |

### Library Architecture

**Library Files**:
- `src/libPrintf.h`: Main library header with function aliasing
- `src/printf.c`: eyalroz/printf v6.2.0 implementation
- `src/printf.h`: Function declarations and configuration
- `examples/BasicUsage/`: Demonstration sketch

**Output Flow**:
```
printf() → printf_() → putchar_() → Serial.print() → Hardware UART
```

## Output Routing & Customization

### Default Output

By default, libPrintf routes output to Arduino's Serial port. To customize, override `putchar_()`:

```cpp
#include <libPrintf.h>
#include <SEGGER_RTT.h>
#define LIBPRINTF_CUSTOM_PUTCHAR

extern "C" void putchar_(char c) {
    // Choose your output method:
    Serial1.write(c);                          // Different Serial port

    // OR for RTT output:
    char buf[2] = {c, '\0'};
    SEGGER_RTT_WriteString(0, buf);            // RTT debugging

    // OR conditional routing:
    #ifdef USE_RTT
        SEGGER_RTT_WriteString(0, buf);
    #else
        Serial.write(c);
    #endif

    // OR dual output:
    Serial.write(c);                           // IDE Serial Monitor
    SEGGER_RTT_WriteString(0, buf);            // HIL testing
}

void setup() {
    Serial.begin(115200);      // Initialize your chosen output
    SEGGER_RTT_Init();         // If using RTT
    printf("Custom output: %.2f\n", 3.14159);
}
```

## Supported Format Specifiers

libPrintf supports all standard printf format specifiers:

- **Integers**: `%d`, `%i`, `%o`, `%x`, `%X`, `%u`
- **Floats**: `%f`, `%F`, `%e`, `%E`, `%g`, `%G`
- **Characters**: `%c`, `%s`
- **Pointers**: `%p`
- **Width/Precision**: `%10s`, `%.2f`, `%*.*f`
- **Flags**: `%-`, `%+`, `%#`, `%0`, `% ` (space)

## Advanced Configuration

### Memory Optimization

Configure printf features to reduce binary size:

```cpp
#define PRINTF_SUPPORT_LONG_LONG 0           // Disable long long support
#define PRINTF_SUPPORT_DECIMAL_SPECIFIERS 1   // Keep %d, %i, %u
#define PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS 0  // Disable %e, %E, %g, %G
```

### Format String Validation

Enable compile-time format string checking:

```cpp
#define PRINTF_SUPPORT_RUNTIME_EVALUATION 0  // Disable for smaller code size
#define PRINTF_CHECK_FOR_NULL_POINTERS 1     // Enable null pointer checks
```

## Examples

### Real-World Integration

The ICM42688P self-test example demonstrates libPrintf integration with factory code:

```cpp
#include <libPrintf.h>
#include "factory_driver.h"  // Uses fprintf(stderr, ...)

void setup() {
  // Factory code automatically uses libPrintf
  run_factory_selftest();  // fprintf calls work seamlessly
}
```

### Production Build Integration

```bash
# Standard FQBN - no rtlib complexity!
arduino-cli compile --fqbn STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE <sketch>

# HIL testing with RTT output
./system/ci/aflash.sh <sketch> --use-rtt

# CMake builds
cmake -S <sketch> -B build && cmake --build build
```

## Comparison with Standard Printf

| Feature | Standard (nano) | Standard (nanofp) | libPrintf |
|---------|----------------|-------------------|-----------|
| **Float Support** | ❌ None | ✅ Full | ✅ Full |
| **Binary Size** | Small (~2KB) | Large (~16KB) | Optimized (~6KB) |
| **FQBN Dependency** | N/A | `:rtlib=nanofp` required | ❌ None |
| **Include Required** | None | None | `#include <libPrintf.h>` |
| **Output Routing** | Serial only | Serial only | Flexible putchar_() |
| **Configuration** | N/A | Complex FQBN | Simple include |

## Troubleshooting

| Problem | Quick Fix |
|---------|-----------|
| No output in Serial Monitor | Add `Serial.begin(115200);` in setup() |
| Float shows "0.000000" | Include `#include <libPrintf.h>` in all printf files |
| `'printf' was not declared` | Add `#include <libPrintf.h>` at top of sketch |
| Binary size larger than expected | Remove `:rtlib=nanofp` from FQBN |
| Custom putchar_() not working | Initialize Serial1/RTT before using printf |

### Verification Test

To confirm libPrintf is active:

```cpp
#include <libPrintf.h>
void setup() {
    Serial.begin(115200);
    printf("Float test: %.2f\n", 3.14);  // Should show "3.14", not "0.00"
}
```

## Version Information

- **libPrintf Library**: v1.0.0 (Arduino wrapper library)
- **Based on**: eyalroz/printf v6.2.0 (embedded implementation)
- **Compatible STM32 Core**: 2.7.1+
- **Upstream Repository**: [github.com/eyalroz/printf](https://github.com/eyalroz/printf)
- **License**: MIT (inherited from eyalroz/printf)

## Contributing

This library is a wrapper around eyalroz/printf. For core printf functionality improvements, contribute to the upstream repository. For Arduino integration improvements, contribute to this wrapper.

## License

MIT License (inherited from eyalroz/printf)