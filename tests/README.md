# Tests Directory

This directory contains unit tests, integration tests, and hardware validation tests for the STM32 Arduino Core and its libraries.

## Test Categories

| Category | Description | CI Automatable | Example |
|----------|-------------|----------------|---------|
| **Unit Tests** | Test single component in isolation, no hardware dependencies | Yes | FIFO_UT |
| **Integration Tests** | Test multiple components working together, may need storage | Partial | LittleFS_IT |
| **Hardware Validation** | Verify hardware peripherals with physical connections | No | UART_DMA_HW |

---

## What Belongs in /tests

**SHOULD be in /tests:**
- Unit tests for core data structures (FIFO, queues, buffers)
- Unit tests for compile-time systems (BoardConfig)
- Integration tests for library functionality (Storage, minIni)
- Hardware validation tests for peripherals (UART DMA, GPS, SPI)
- Protocol validation tests (IBus, SBUS, CRSF)

**SHOULD NOT be in /tests:**
- Application sketches (belong in `/sketches`)
- Library usage examples (belong in `/libraries/*/examples`)
- One-off debugging scripts (use `/tmp`)
- Board bringup/verification (document separately)

---

## Current Tests

### CI-Automatable (No Hardware Required)

| Test | Description | Framework |
|------|-------------|-----------|
| **FIFO_UT** | Ring buffer data structure | AUnit (15 tests) |
| **BoardConfig_UT** | Compile-time board config | AUnit (3 tests) |
| **AUnit_HIL** | AUnit + HIL integration demo | AUnit (4 tests) |

### Hardware Required

| Test | Hardware | Description | Framework |
|------|----------|-------------|-----------|
| **LittleFS_IT** | SPI Flash (W25Q128JV) | Filesystem operations | AUnit (8 tests) |
| **SDFS_IT** | SD Card | FAT filesystem operations | AUnit (7 tests) |
| **Storage_LittleFS_IT** | SPI Flash | Storage abstraction layer | AUnit (5 tests) |
| **Storage_SDFS_IT** | SD Card | Storage abstraction layer | AUnit (5 tests) |
| **minIni_LittleFS_IT** | SPI Flash | INI config management | AUnit (5 tests) |
| **minIni_SDFS_IT** | SD Card | INI config management | AUnit (5 tests) |
| **UART_DMA_HW** | Loopback (PB6↔PB7) | UART DMA reception | ci_log.h |
| **GPS_DMA_HW** | Loopback or GPS module | GPS NMEA parsing | ci_log.h |
| **SerialRx_IBus_HW** | RC receiver (IBus) | RC channel parsing | ci_log.h |
| **SPI_BfBus_HW** | ICM-42688-P IMU | SPI device validation | ci_log.h |

---

## Hardware Requirements Reference

| Hardware | Tests |
|----------|-------|
| **None** | FIFO_UT, BoardConfig_UT, AUnit_HIL |
| **SPI Flash** | LittleFS_IT, Storage_LittleFS_IT, minIni_LittleFS_IT |
| **SD Card** | SDFS_IT, Storage_SDFS_IT, minIni_SDFS_IT |
| **UART Loopback (PB6↔PB7)** | UART_DMA_HW, GPS_DMA_HW (loopback mode) |
| **External IMU** | SPI_BfBus_HW (ICM-42688-P) |
| **GPS Module** | GPS_DMA_HW (GPS mode) |
| **RC Receiver** | SerialRx_IBus_HW (FlySky IBus) |

---

## Running Tests

All tests support the CI/HIL workflow via `aflash.sh`:

```bash
# Run a test with RTT output
./system/ci/aflash.sh tests/FIFO_UT --use-rtt --build-id

# Run with specific FQBN
./system/ci/aflash.sh tests/GPS_DMA_HW STMicroelectronics:stm32:FlightCtr:pnum=JHEF_JHEF411 --use-rtt --build-id
```

Default FQBN: `STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE`

---

## Creating New Tests

### Naming Convention

**Pattern**: `[Component]_[Qualifier?]_[TestType]`

#### Test Type Suffixes

| Suffix | Meaning | Hardware | Framework | Example |
|--------|---------|----------|-----------|---------|
| `_UT` | Unit Test - single component in isolation | None | AUnit | `FIFO_UT` |
| `_IT` | Integration Test - multiple components | May need storage | AUnit or ci_log | `Storage_LittleFS_IT` |
| `_HW` | Hardware Test - peripheral validation | Required | ci_log | `UART_DMA_HW` |
| `_HIL` | HIL Test - CI/HIL framework itself | None | AUnit | `AUnit_HIL` |

#### Optional Qualifiers

| Qualifier | When to Use | Example |
|-----------|-------------|---------|
| `_DMA_` | Tests DMA-specific functionality | `UART_DMA_HW` |
| `_LittleFS_` / `_SDFS_` | Tests with specific storage backend | `minIniStorage_LittleFS_UT` |
| `_IBus_` / `_SBUS_` | Tests specific protocol | `SerialRx_IBus_HW` |

#### Naming Guidelines

1. **Component first** - What is being tested (FIFO, UART, GPS, BoardConfig)
2. **Qualifier if needed** - Backend, protocol, or feature variant
3. **Type last** - Use compact suffix (_UT, _IT, _HW, _HIL)
4. **Use underscores** - `Snake_Case` with capital first letters

#### Name Should Answer

A good test name should answer: **"What component does this test, and what kind of testing does it do?"**

| Name | Component | Type | Clear? |
|------|-----------|------|--------|
| `FIFO_UT` | FIFO data structure | Unit test | ✅ Yes |
| `AUnit_Pilot_Test` | AUnit? Pilot? | Unknown | ❌ No |
| `AUnit_HIL` | AUnit HIL integration | HIL test | ✅ Yes |
| `bf_bus_test` | bf_bus | Unknown | ❌ No |
| `GPS_DMA_HW` | GPS with DMA | Hardware test | ✅ Yes |

#### Existing Tests vs Convention

Current names and what they would be under the naming convention:

| Current Name | Follows Convention? | Suggested Name (if renaming) |
|--------------|---------------------|------------------------------|
| `FIFO_UT` | ✅ Yes | - |
| `BoardConfig_UT` | ✅ Yes | - |
| `AUnit_HIL` | ✅ Yes | - |
| `LittleFS_IT` | ✅ Yes | - |
| `SDFS_IT` | ✅ Yes | - |
| `Storage_LittleFS_IT` | ✅ Yes | - |
| `Storage_SDFS_IT` | ✅ Yes | - |
| `minIni_LittleFS_IT` | ✅ Yes | - |
| `minIni_SDFS_IT` | ✅ Yes | - |
| `UART_DMA_HW` | ✅ Yes | - |
| `GPS_DMA_HW` | ✅ Yes | - |
| `SerialRx_IBus_HW` | ✅ Yes | - |
| `SPI_BfBus_HW` | ✅ Yes | - |

*All tests now follow the naming convention.*

### Documentation Template

Each test file should include a header comment:

```cpp
/**
 * TestName.ino - Brief description
 *
 * Category: Unit Test | Integration Test | Hardware Validation
 * Framework: AUnit | None (ci_log.h only)
 * Hardware: None | [list specific requirements]
 * CI Automatable: Yes | No
 *
 * Supported boards:
 *   ./system/ci/aflash.sh tests/TestName FQBN --use-rtt --build-id
 */
```

### Framework Guidelines

**Prefer AUnit for unit tests** (automatic test discovery, assertions, exit handling):

```cpp
#include <aunit_hil.h>

test(component_behavior_expected) {
  assertEqual(expected, actual);
  assertTrue(condition);
}

void setup() {
  HIL_TEST_SETUP();
}

void loop() {
  HIL_TEST_RUN();
}
```

**Use ci_log.h for hardware validation** (simpler, direct control):

```cpp
#include <ci_log.h>

void setup() {
#ifndef USE_RTT
  Serial.begin(115200);
  while (!Serial && millis() < 3000);
#endif

  CI_LOG("=== Test Name ===\n");
  CI_BUILD_INFO();
  CI_READY_TOKEN();

  // Test logic here...

  CI_LOG("*STOP*\n");  // Required for aflash.sh exit detection
}

void loop() {
  // Empty for single-run tests
}
```

---

## Exit Token Requirements

All tests must emit `*STOP*` for deterministic CI execution:

- **AUnit tests**: Framework handles this via `HIL_TEST_RUN()`
- **ci_log.h tests**: Explicitly call `CI_LOG("*STOP*\n")` before halting

Without `*STOP*`, `aflash.sh` will timeout after 60 seconds.

---

## Multi-Board Support

Tests that support multiple boards should use preprocessor conditionals:

```cpp
#if defined(STM32H7xx)
  HardwareSerial SerialTest(PB7_ALT1, PB6_ALT2);  // H7 ALT pins for USART1
#else
  HardwareSerial SerialTest(PB7, PB6);            // F4 standard pins
#endif
```

Document supported boards in the test header with their FQBNs.
