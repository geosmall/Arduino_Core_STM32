# SEGGER RTT Library for STM32 Arduino Core

This directory contains the **SEGGER Real-Time Transfer (RTT) v8.62** implementation for STM32 Arduino Core, providing high-speed debugging output without requiring UART/serial connections.

## Library Overview

RTT enables real-time terminal output and input via the debug probe (J-Link/ST-Link) using existing SWD/JTAG connections. This eliminates the need for additional serial connections and provides faster, more reliable debugging output compared to traditional UART-based methods.

### Key Features
- **Real-time debugging output** via debug probe connection
- **High-speed data transfer** (much faster than UART)
- **No additional hardware required** (uses existing SWD connection)
- **Bi-directional communication** (output and input capable)
- **Multiple virtual terminals** (up to 3 channels by default)
- **Printf-style formatting** with full SEGGER RTT printf implementation
- **Optimized assembly routines** for ARM Cortex-M4/M7

## Source Files and Origin

This implementation is based on **SEGGER RTT v8.62** (August 2025 release).

### File Sources
All source files were copied from a working Arduino example project:
**Primary Source**: `/home/geo/Arduino/Segger_RTT_PrintfTest_Lib_V862/src/SEGGER_RTT/`

**Official Archive**: `/opt/SEGGER/JLink_V862/Samples/RTT/SEGGER_RTT_V862.tgz`
- The working example files are based on the official SEGGER RTT v8.62 archive
- Files have been tested and validated in an Arduino STM32 environment
- Minor differences exist in copyright years (2019 vs 2021) and formatting

### Core Files
- **SEGGER_RTT.h** - Main header with API declarations and configuration
- **SEGGER_RTT.c** - Core RTT implementation with buffer management  
- **SEGGER_RTT_Conf.h** - Configuration file with buffer sizes and platform settings
- **SEGGER_RTT_printf.c** - Printf implementation for formatted output
- **SEGGER_RTT_ASM_ARMv7M.S** - Optimized assembly routines for ARM Cortex-M4/M7
- **SEGGER_RTT_Syscalls_GCC.c** - GCC syscalls integration (optional)

### Previous Implementation
The original Arduino_Core_STM32 contained a simplified RTT implementation:
- **SEGGER_RTT.cpp** (removed) - Simplified C++ wrapper
- **SEGGER_RTT.h** (replaced) - Basic API declarations only

## Configuration

The library is configured via `SEGGER_RTT_Conf.h`:

```c
// Buffer configuration
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS     (3)     // Target->Host buffers
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS   (3)     // Host->Target buffers  
#define BUFFER_SIZE_UP                    (1024)  // Output buffer size
#define BUFFER_SIZE_DOWN                  (16)    // Input buffer size
#define SEGGER_RTT_PRINTF_BUFFER_SIZE     (64u)   // Printf buffer size

// Operating mode
#define SEGGER_RTT_MODE_DEFAULT           SEGGER_RTT_MODE_NO_BLOCK_SKIP
```

## Usage Example

```cpp
#include "SEGGER_RTT.h"

void setup() {
    // Initialize RTT (optional - done automatically)
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    
    // Basic output
    SEGGER_RTT_WriteString(0, "RTT Debug Output\r\n");
    
    // Printf-style formatting  
    SEGGER_RTT_printf(0, "Counter: %d, Hex: 0x%08X\r\n", counter, value);
}

void loop() {
    static int count = 0;
    SEGGER_RTT_printf(0, "Loop %d at %lu ms\r\n", count++, millis());
    delay(1000);
}
```

## Hardware Requirements

### Supported Debug Probes
- **J-Link** (all models) - Preferred, native RTT support
- **ST-Link** (V2/V3) - Requires ST-Link to J-Link firmware reflash using SEGGER STLinkReflash utility

### Target Compatibility  
- **STM32F4xx** series (tested on STM32F411RE)
- **STM32F7xx**, **STM32G4xx**, **STM32H7xx** series (should work)
- Any ARM Cortex-M3/M4/M7 with SWD interface

## Connection and Testing

### 1. Hardware Setup
- Connect debug probe to target SWD pins (SWDIO, SWCLK, GND, 3V3)
- For Nucleo boards: ST-Link is integrated, reflash to J-Link if desired

### 2. Upload Sketch
```bash
# Standard Arduino upload (if ST-Link works)
arduino-cli upload --fqbn "STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE" <sketch>

# J-Link upload (if reflashed or using external J-Link)  
./system/ci/jlink_upload.sh <path_to_compiled.bin>
```

### 3. Connect RTT Client
```bash
# Start J-Link GDB Server with RTT
JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 &

# Connect RTT client for real-time output
JLinkRTTClient
```

### 4. Using RTT Test Framework
```bash
# Automated RTT testing
./system/ci/rtt-test.sh <sketch_path>

# Interactive RTT session  
./system/ci/rtt-test.sh -i
```

## Updating the Library

### Update Procedure

1. **Get Latest RTT Source**
   ```bash
   # Option 1: Extract from official SEGGER J-Link installation
   cd /tmp
   tar -xf /opt/SEGGER/JLink_V*/Samples/RTT/SEGGER_RTT_V*.tgz
   
   # Option 2: Use existing working example (if available)
   # Source: /home/geo/Arduino/Segger_RTT_PrintfTest_Lib_V862/src/SEGGER_RTT/
   
   # Option 3: Download from SEGGER website
   # https://www.segger.com/downloads/jlink -> Samples/RTT
   ```

2. **Backup Current Implementation**
   ```bash
   cd Arduino_Core_STM32/libraries/
   cp -r SEGGER_RTT SEGGER_RTT_backup_$(date +%Y%m%d)
   ```

3. **Replace Source Files**
   ```bash
   # From official archive (Option 1):
   cp /tmp/SEGGER_RTT_V*/RTT/SEGGER_RTT.h src/
   cp /tmp/SEGGER_RTT_V*/RTT/SEGGER_RTT.c src/  
   cp /tmp/SEGGER_RTT_V*/Config/SEGGER_RTT_Conf.h src/
   cp /tmp/SEGGER_RTT_V*/RTT/SEGGER_RTT_printf.c src/
   cp /tmp/SEGGER_RTT_V*/RTT/SEGGER_RTT_ASM_ARMv7M.S src/
   cp /tmp/SEGGER_RTT_V*/Syscalls/SEGGER_RTT_Syscalls_GCC.c src/
   
   # From working example (Option 2):
   cp /home/geo/Arduino/Segger_RTT_PrintfTest_Lib_V862/src/SEGGER_RTT/* src/
   ```

4. **Test Compilation**
   ```bash
   cd HIL_RTT_Test/
   make clean
   make  # Should compile without errors
   ```

5. **Test RTT Functionality**  
   ```bash
   # Upload and test RTT output
   ./system/ci/jlink_upload.sh <compiled_binary>
   ./system/ci/rtt-test.sh -i  # Interactive test
   ```

6. **Update Documentation**
   - Update version number in this README.md
   - Document any breaking changes or new features
   - Update usage examples if API changed

### Version History
- **v8.62** (August 2025) - Current version, full implementation with printf support
- **Previous** (Unknown version) - Simplified C++ wrapper implementation

## Troubleshooting

### Common Issues

**RTT Connection Failed**
- Verify debug probe connection (SWD pins)
- Check target power supply
- Ensure J-Link/ST-Link firmware is compatible
- Try different connection speeds (4000, 1000, 500 kHz)

**No RTT Output**
- Verify RTT buffer initialization in sketch
- Check buffer configuration (SEGGER_RTT_Conf.h)  
- Ensure RTT client connects before target starts outputting
- Try different RTT modes (blocking vs non-blocking)

**Build Errors**
- Clean build cache: `make clean` or `arduino-cli cache clean`
- Verify all 6 source files are present in src/ directory
- Check for Arduino IDE vs arduino-cli compatibility issues

### Debug Tips
- Use `SEGGER_RTT_printf(0, "Debug: %s:%d\r\n", __FILE__, __LINE__);` for tracing
- RTT channel 0 is for normal output, channels 1-2 for specialized use
- Monitor RTT buffer overflow with `SEGGER_RTT_HasData()` functions
- For high-throughput applications, increase buffer sizes in SEGGER_RTT_Conf.h

## Integration Notes

This RTT implementation integrates seamlessly with the STM32 Arduino Core build system:
- **Automatic detection**: Arduino IDE/CLI automatically includes the library when `#include "SEGGER_RTT.h"` is used
- **Build integration**: All source files compile automatically during sketch build
- **Memory usage**: Default configuration uses ~1KB RAM for buffers (configurable)
- **Flash usage**: Adds ~2-3KB flash overhead for full printf implementation

## References

- [SEGGER RTT Documentation](https://www.segger.com/products/debug-probes/j-link/technology/about-real-time-transfer/)
- [J-Link Software Pack](https://www.segger.com/downloads/jlink)
- [STM32 Arduino Core Documentation](https://github.com/stm32duino/Arduino_Core_STM32)

---
*Documentation updated: August 2025*  
*RTT Version: v8.62*  
*Maintainer: Arduino STM32 Build Workflow Project*