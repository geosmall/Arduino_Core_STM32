# dRehmFlight STM32 Port - BETA 1.4 (Development)

Extended port of [dRehmFlight](https://github.com/nickrehm/dRehmFlight) BETA 1.3 with GPS integration and optional UART DMA support.

**Location**: `sketches/devel/` - Development/experimental version

## What's New in BETA 1.4

Building on the validated BETA 1.3 port, this version adds:

| Feature | Description |
|---------|-------------|
| **GPS Support** | TinyGPSPlus library integration for position data |
| **GPS DMA Mode** | Optional UART DMA reduces GPS IRQ overhead (~960/sec to ~12/sec) |
| **RC DMA Mode** | Optional UART DMA for RC receiver (reduced interrupt load) |
| **Modular Files** | Separate `gps.ino` and `radioComm.ino` for cleaner organization |
| **H7 DMA Buffers** | `SERIAL_DMA_BUFFER` macro for D2 SRAM3 placement on H7 |

## Configuration Options

Enable features in the main `.ino` file:

```cpp
// RC Receiver (required - choose one protocol)
#define USE_SERIAL_RX
#define USE_IBUS_RX    // IBus protocol (FlySky)
// #define USE_SBUS_RX // SBUS protocol (FrSky, etc.)
// #define USE_RC_DMA  // Optional: UART DMA for RC (reduces IRQ overhead)

// GPS (optional)
// #define USE_GPS      // Enable TinyGPSPlus GPS support
// #define USE_GPS_DMA  // Optional: UART DMA for GPS
```

## Target Hardware

Same 4 boards as BETA 1.3:

| Board | MCU | IMU | GPS UART |
|-------|-----|-----|----------|
| NUCLEO_F411RE | STM32F411RE | ICM42688P (breadboard) | USART6 (BoardConfig) |
| BLACKPILL_F411CE | STM32F411CE | MPU-9250 | USART1 (BoardConfig) |
| BKMN_NERO | STM32F722RET6 | ICM-20602 | UART4 (BoardConfig) |
| MATEK_H743VI | STM32H743VIT6 | ICM42688P | UART7 (BoardConfig) |

## GPS Integration

GPS data is parsed via TinyGPSPlus and exposed as global variables:

```cpp
float gps_latitude;      // Degrees
float gps_longitude;     // Degrees
float gps_altitude_m;    // Meters
float gps_speed_mps;     // Meters per second
float gps_course_deg;    // Heading in degrees
uint32_t gps_satellites; // Satellite count
bool gps_fix_valid;      // Fix status
```

**DMA Mode Benefits**:
- Standard mode: ~960 UART interrupts/sec (byte-by-byte at 9600 baud)
- DMA mode: ~12 interrupts/sec (IDLE line detection only)
- Critical for maintaining 2kHz flight loop timing

## Build

```bash
# From Arduino_Core_STM32 repository root
./system/ci/build.sh sketches/devel/dRehmFlight_STM32_BETA_1.4 --build-id

# Flash with RTT for testing
./system/ci/aflash.sh sketches/devel/dRehmFlight_STM32_BETA_1.4 --use-rtt --build-id

# For other boards (specify FQBN)
./system/ci/aflash.sh sketches/devel/dRehmFlight_STM32_BETA_1.4 \
    STMicroelectronics:stm32:FlightCtr:pnum=MATEK_H743VI --use-rtt --build-id
```

## Build Options

The `build.opt` file configures:
- `SERIAL_RX_BUFFER_SIZE=256` - Increased buffer for GPS NMEA bursts

## Status

**Development Version** - Extends validated BETA 1.3 with experimental features.

| Component | Status |
|-----------|--------|
| Base flight control | Inherited from BETA 1.3 (validated) |
| GPS parsing | Implemented (TinyGPSPlus) |
| GPS DMA mode | Implemented (untested on flight hardware) |
| RC DMA mode | Implemented (untested on flight hardware) |
| Flight testing | Pending |

## Relationship to BETA 1.3

```
sketches/dRehmFlight_STM32_BETA_1.3/  <- Production-ready, validated
sketches/devel/dRehmFlight_STM32_BETA_1.4/  <- Development, adds GPS + DMA
```

Use BETA 1.3 for flight testing. Use BETA 1.4 to experiment with GPS integration.

## STM32 Libraries Used

- **IMU** - ICM42688P/MPU9250 high-level wrapper
- **SerialRx** - IBus/SBUS protocol parser (with optional DMA)
- **TimerPWM** - Hardware timer PWM (PWMOutputBank)
- **TinyGPSPlus** - NMEA GPS parser
- **BoardConfig** - Multi-board pin abstraction
- **ci_log** - HIL testing and logging

## License

MIT License - Same as original dRehmFlight

## Attribution

Original work by Nicholas Rehm: https://github.com/nickrehm/dRehmFlight
