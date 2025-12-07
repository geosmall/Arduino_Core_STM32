# Plan: UART DMA Integration for GPS and SerialRX

## Goal
Integrate UART DMA RX into two key subsystems:
1. **GPS** - Add TinyGPSPlus-based GPS to dRehmFlight using UART DMA
2. **SerialRX** - Add optional DMA mode to SerialRx library for RC receivers

## Benefits
- **99%+ interrupt reduction** - DMA handles byte reception in hardware
- **Lower latency** - IDLE interrupt triggers immediate frame processing
- **CPU freed** - Background DMA while flight loop runs at 2kHz

---

## How DMA RX Works (Data Flow)

The key insight is that **DMA is transparent to the application layer**. The polling API (`available()`, `read()`) remains identical.

### Interrupt Mode (Traditional)
```
UART RX pin → RXNE interrupt (per byte) → ISR pushes to ring buffer → available()/read()
             ~11,500 IRQs/sec @ 115200 baud
```

### DMA Mode (New)
```
UART RX pin → DMA fills circular buffer (zero CPU) → IDLE interrupt → batch copy to ring buffer → available()/read()
             ~100 IRQs/sec (IDLE only)
```

### Why SerialRx Doesn't Need Callback Changes

The `_dma_rx_callback` in HardwareSerial.cpp (line 668) pushes received bytes to the **same ring buffer** used by interrupt mode:

```cpp
void HardwareSerial::_dma_rx_callback(serial_t *obj, uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        obj->rx_buff[obj->rx_head] = data[i];  // Same ring buffer as interrupt mode
        obj->rx_head = next;
    }
}
```

This means SerialRx's `update()` loop stays **exactly the same**:
```cpp
void SerialRx::update() {
    while (serial_->available()) {           // Works identically
        parser_->ParseByte(serial_->read()); // Works identically
    }
}
```

**The only change is in `begin()`** - routing to `beginDMA()` instead of `begin()`.

### GPS with TinyGPSPlus - Same Principle

TinyGPSPlus uses a simple byte-by-byte `encode()` pattern:
```cpp
void getGPSdata() {
    while (SerialGPS.available()) {
        gps.encode(SerialGPS.read());  // Feed NMEA bytes to parser
    }
}
```

With DMA, this code is **unchanged**. The DMA fills the ring buffer in the background, and `available()`/`read()` work identically. Benefits:

| Metric | Interrupt Mode | DMA Mode |
|--------|---------------|----------|
| IRQs per NMEA sentence (82 bytes) | 82 | 1 (IDLE) |
| CPU during byte reception | ISR per byte | Zero |
| Latency to process | Loop rate dependent | IDLE triggers immediate |

**GPS at 9600 baud**: ~960 bytes/sec = 960 IRQs/sec → reduced to ~12 IRQs/sec (one per sentence)

---

## Part 1: SerialRX DMA Integration

### Files to Modify

| File | Changes |
|------|---------|
| `libraries/SerialRx/src/SerialRx.h` | Add DMA fields to Config struct |
| `libraries/SerialRx/src/SerialRx.cpp` | Add beginDMA() support in begin() |

### Step 1.1: Extend SerialRx::Config

```cpp
// In SerialRx.h, add to Config struct
struct Config {
    HardwareSerial* serial;
    Protocol rx_protocol;
    uint32_t baudrate;
    uint32_t timeout_ms;
    uint32_t idle_threshold_us;
    // DMA mode (optional)
    bool use_dma;           // Enable DMA reception
    uint8_t* dma_rx_buf;    // User-provided DMA buffer
    size_t dma_rx_size;     // Buffer size (256 recommended)
};

// Default constructor sets use_dma = false for backwards compatibility
```

### Step 1.2: Update SerialRx::begin()

```cpp
// In SerialRx.cpp, modify begin()
bool SerialRx::begin(const Config& config) {
    serial_ = config.serial;
    // ... existing setup ...

    if (config.use_dma && config.dma_rx_buf != nullptr) {
        if (!serial_->beginDMA(config.baudrate, config.dma_rx_buf, config.dma_rx_size)) {
            // DMA failed, fall back to interrupt mode
            serial_->begin(config.baudrate);
        }
    } else {
        serial_->begin(config.baudrate);
    }

    return true;
}
```

### Step 1.3: Add end() DMA cleanup

```cpp
void SerialRx::end() {
    if (serial_ != nullptr) {
        if (serial_->isDMAListening()) {
            serial_->endDMA();
        }
        serial_->end();
    }
}
```

---

## Part 2: GPS Integration for dRehmFlight

### Files to Modify

| File | Changes |
|------|---------|
| `targets/BoardConfigTypes.h` | Add GPSConfig struct |
| `targets/NUCLEO_F411RE_JHEF411.h` | Add GPS config |
| `targets/MTKS-MATEKH743.h` | Add GPS config |
| `sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino` | Add GPS functions |

### Step 2.1: Add GPSConfig to BoardConfig

```cpp
// In BoardConfigTypes.h
struct GPSConfig {
    PinName tx_pin;
    PinName rx_pin;
    uint32_t baud_rate;
};
```

### Step 2.2: Add GPS Config to Target Headers

```cpp
// NUCLEO_F411RE: USART2 (PA2/PA3)
static constexpr GPSConfig gps = {
    .tx_pin = PA_2,
    .rx_pin = PA_3,
    .baud_rate = 9600
};

// MATEK_H743VI: USART2 (PD5/PD6)
static constexpr GPSConfig gps = {
    .tx_pin = PD_5,
    .rx_pin = PD_6,
    .baud_rate = 9600
};
```

### Step 2.3: Add GPS to dRehmFlight

**Global variables:**
```cpp
#include <TinyGPSPlus.h>

TinyGPSPlus gps;
HardwareSerial SerialGPS(BoardConfig::gps.rx_pin, BoardConfig::gps.tx_pin);
SERIAL_DMA_BUFFER uint8_t gpsRxBuffer[256];

// GPS data for flight controller
float gps_latitude, gps_longitude, gps_altitude;
float gps_speed_mps;
bool gps_fix_valid;
```

**gpsSetup():**
```cpp
void gpsSetup() {
    if (!SerialGPS.beginDMA(BoardConfig::gps.baud_rate, gpsRxBuffer, sizeof(gpsRxBuffer))) {
        SerialGPS.begin(BoardConfig::gps.baud_rate);  // Fallback
    }
}
```

**getGPSdata():**
```cpp
void getGPSdata() {
    while (SerialGPS.available()) {
        gps.encode(SerialGPS.read());
    }
    if (gps.location.isUpdated()) {
        gps_latitude = gps.location.lat();
        gps_longitude = gps.location.lng();
        gps_altitude = gps.altitude.meters();
        gps_speed_mps = gps.speed.mps();
        gps_fix_valid = gps.location.isValid();
    }
}
```

---

## Part 3: Update dRehmFlight to Use DMA for RC Receiver

dRehmFlight already uses SerialRx. Update to enable DMA mode:

```cpp
// In dRehmFlight, update RC receiver setup
SERIAL_DMA_BUFFER uint8_t rcRxBuffer[256];  // Add DMA buffer

void radioSetup() {
    SerialRx::Config config;
    config.serial = &SerialRC;
    config.rx_protocol = SerialRx::SBUS;
    config.baudrate = 100000;
    config.idle_threshold_us = 300;
    // Enable DMA
    config.use_dma = true;
    config.dma_rx_buf = rcRxBuffer;
    config.dma_rx_size = sizeof(rcRxBuffer);

    rc.begin(config);
}
```

---

## UART Assignments by Board

| Board | RC UART | RC Pins | GPS UART | GPS Pins |
|-------|---------|---------|----------|----------|
| NUCLEO_F411RE | USART1 | PB7/PB6 | USART2 | PA2/PA3 |
| MATEK_H743VI | UART7 | PE7/PE8 | USART2 | PD5/PD6 |

---

## Implementation Order

1. **SerialRx DMA** (Part 1) - Add use_dma option
2. **BoardConfig GPS** (Steps 2.1-2.2) - Add GPSConfig struct
3. **dRehmFlight GPS** (Step 2.3) - Add GPS functions
4. **dRehmFlight RC DMA** (Part 3) - Enable DMA for RC receiver

---

## Test Plan

### SerialRx DMA Test
1. Modify existing `tests/IBus_Loopback_Test` to use DMA mode
2. Run on F4 and H7 - verify frame parsing works

### GPS Test
1. Build dRehmFlight for NUCLEO_F411RE
2. Connect GPS module to USART2 (PA2=TX, PA3=RX)
3. Verify NMEA parsing via RTT output
4. Verify no impact on 2kHz flight loop timing

### RC DMA Test
1. Enable DMA mode in dRehmFlight RC setup
2. Connect SBUS receiver
3. Verify channel data via RTT output
4. Verify frame rate maintained (~7ms SBUS frames)
