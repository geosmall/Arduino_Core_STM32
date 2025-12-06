# UART_DMA.md - DMA-based UART RX for GPS and Serial Streams

## Overview

Analysis and implementation plan for adding DMA-based UART receive capability to the Arduino STM32 core, specifically targeting GPS receivers and other continuous serial data streams.

**Status**: Implementation Ready
**Priority**: Active development (uart-rx-dma branch)

## Motivation

Current Arduino STM32 `HardwareSerial` uses interrupt-based RX - each byte triggers an ISR. For continuous streams like GPS (NMEA at 9600-115200 baud, 10Hz update rate), this creates significant CPU overhead.

**DMA RX Benefits:**
- Zero CPU involvement during reception
- Circular buffer fills automatically in background
- IDLE line detection provides packet-complete notification
- Eliminates byte-by-byte interrupt overhead

## Flight Controller Firmware Analysis

### iNav DMA Usage

| Peripheral | DMA | Notes |
|------------|-----|-------|
| **ADC** | ✅ Yes | Continuous battery/current monitoring |
| **LED Strip (WS2811)** | ✅ Yes | Timer DMA for bit-banged protocol |
| **DSHOT (motors)** | ✅ Yes | Timer DMA burst for digital ESC protocol |
| **SPI (IMU)** | ❌ No | Blocking LL polling |
| **I2C (baro, mag)** | ❌ No | Blocking HAL |
| **UART (GPS, serial)** | ❌ No | Interrupt-based |

**Key Insight**: iNav uses DMA for continuous/timing-critical operations (ADC, LED, DSHOT) but NOT for communication buses (SPI, I2C, UART).

### Betaflight DMA Usage

| Peripheral | DMA | Notes |
|------------|-----|-------|
| **ADC** | ✅ Yes | Same as iNav |
| **LED Strip** | ✅ Yes | Same as iNav |
| **DSHOT** | ✅ Yes | Same as iNav |
| **SPI (IMU)** | ✅ Yes | Segment-based DMA with callbacks |
| **I2C** | ❌ No | Blocking |
| **UART** | ✅ Yes | Circular DMA RX, DMA TX |

**Key Insight**: Betaflight uses more aggressive DMA, including UART and SPI.

### UVOS_Duino DMA Usage

| Peripheral | DMA | Notes |
|------------|-----|-------|
| **I2C** | ✅ Yes | Non-blocking DMA reads |
| **UART** | ✅ Yes | "DmaListen" circular RX mode |

**Key Insight**: UVOS_Duino (our reference) uses UART DMA with a "listen" mode for circular reception.

## Why UART DMA is More Valuable than I2C DMA

| Aspect | I2C DMA | UART DMA |
|--------|---------|----------|
| **Transfer pattern** | Short, on-demand (~6 bytes) | Continuous stream (100+ bytes) |
| **Transfer duration** | ~200µs at 400kHz | Ongoing |
| **Interrupt load without DMA** | Low (one transaction) | High (every byte) |
| **CPU savings** | Marginal | Significant |
| **Use case** | Barometer @ 32Hz | GPS @ 10Hz continuous |

**Conclusion**: UART RX DMA provides more meaningful CPU savings than I2C DMA for flight controller applications.

## Recommended Implementation Approach

### Core Concept: Circular DMA + IDLE Line Detection

```
┌─────────────────────────────────────────┐
│           Circular DMA Buffer           │
│  [====DATA====|----empty----|====DATA]  │
│       ↑                          ↑      │
│    tail (CPU reads)         head (DMA)  │
└─────────────────────────────────────────┘
                    │
                    ▼
         IDLE Line Interrupt
         (packet complete)
```

1. **DMA runs continuously** in circular mode, filling buffer
2. **IDLE line interrupt** fires when line goes idle (gap between packets)
3. **CPU reads from tail**, DMA writes at head - no conflicts
4. **Zero-copy** - data goes directly to final buffer

### API Design

```cpp
// New HardwareSerial methods
class HardwareSerial {
public:
    // Enable DMA RX with user-provided buffer
    // Buffer MUST be in DMA-safe memory on H7 (use SERIAL_DMA_BUFFER macro)
    bool beginDMA(unsigned long baud, uint8_t* rxBuffer, size_t rxBufferSize);

    // Check bytes available (works same as regular available())
    int available();

    // Read works same as regular read()
    int read();

    // Optional: callback on IDLE (packet complete)
    void onReceiveComplete(void (*callback)(void));
};

// H7 buffer placement macro (similar to WIRE_DMA_BUFFER)
#if defined(STM32H7xx)
  #define SERIAL_DMA_BUFFER __attribute__((section(".dmabuf")))
#else
  #define SERIAL_DMA_BUFFER
#endif
```

### Usage Example (GPS)

```cpp
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// DMA buffer - must be in non-cached memory on H7
SERIAL_DMA_BUFFER uint8_t gpsRxBuffer[256];

HardwareSerial Serial2(PA3, PA2);  // GPS UART
TinyGPSPlus gps;

void setup() {
    // Initialize with DMA - buffer fills automatically in background
    Serial2.beginDMA(9600, gpsRxBuffer, sizeof(gpsRxBuffer));
}

void loop() {
    // Read works exactly like normal - but no per-byte interrupts occurred
    while (Serial2.available()) {
        gps.encode(Serial2.read());
    }

    if (gps.location.isUpdated()) {
        // Process GPS fix
    }
}
```

## Implementation Plan

### Reference Implementation: UVOS_Duino

**Source**: `/home/geo/src/UVOS_Duino/cores/arduino/per/uart.cpp`

Key patterns from UVOS:
- `DmaListenStart()` - Enables circular DMA + IDLE interrupt
- `UART_CheckRxListener()` - Calculates new data range on TC/HT/IDLE
- Uses `LL_DMA_GetDataLength()` for position: `head = bufSize - remaining`
- Handles wraparound: splits callback when `pos < old_pos`
- H7 cache: `SCB_InvalidateDCache_by_Addr()` before reading DMA buffer
- Uses `FIFO<uint8_t>` for lock-free software ring buffer

### Phase 1: STM32F4xx (No Cache Concerns)

**Files to Modify:**

| File | Changes |
|------|---------|
| `cores/arduino/util/FIFO.h` | **NEW** - Import from UVOS_Duino (with fixes) |
| `cores/arduino/stm32/uart.h` | Add DMA fields to `serial_s` struct |
| `cores/arduino/stm32/uart.c` | Add DMA init, circular RX, IDLE IRQ handler |
| `cores/arduino/HardwareSerial.h` | Add `beginDMA()`, `SERIAL_DMA_BUFFER` macro, FIFO member |
| `cores/arduino/HardwareSerial.cpp` | Implement `beginDMA()`, DMA-aware `available()`/`read()` |

**Step 1: Add DMA state to uart.h (`serial_s` struct)**
```c
#if defined(HAL_DMA_MODULE_ENABLED)
  DMA_HandleTypeDef hdma_rx;
  uint8_t *dma_rx_buf;
  size_t dma_rx_size;
  volatile size_t dma_rx_last_pos;  // Last processed position (UVOS pattern)
  volatile uint8_t dma_listen_mode; // 0=interrupt, 1=DMA listen
#endif
```

**Step 2: Add DMA functions to uart.c**
```c
// Start circular DMA RX with IDLE detection (UVOS DmaListenStart pattern)
int uart_dma_listen_start(serial_t *obj, uint8_t *buf, size_t size);

// Stop DMA listening
void uart_dma_listen_stop(serial_t *obj);

// Get current DMA write position (head)
static inline size_t uart_dma_get_head(serial_t *obj) {
  return obj->dma_rx_size - __HAL_DMA_GET_COUNTER(&obj->hdma_rx);
}

// Check for new data and push to FIFO (called from IRQ)
static void uart_dma_check_rx(serial_t *obj);
```

**Step 3: IDLE + TC + HT Interrupt Handling**
```c
// IDLE detection in UART IRQ
if (obj->dma_listen_mode && __HAL_UART_GET_FLAG(&huart, UART_FLAG_IDLE)) {
  uart_dma_check_rx(obj);
  __HAL_UART_CLEAR_IDLEFLAG(&huart);
}

// Also handle in HAL_UART_RxCpltCallback (TC) and HAL_UART_RxHalfCpltCallback (HT)
```

**DMA Channel Mapping (F4):**

| UART | DMA | Stream | Channel |
|------|-----|--------|---------|
| USART1 RX | DMA2 | Stream 2 | Channel 4 |
| USART2 RX | DMA1 | Stream 5 | Channel 4 |
| USART6 RX | DMA2 | Stream 1 | Channel 5 |

**Key Implementation Details:**

1. **Circular DMA Mode**: `hdma_rx.Init.Mode = DMA_CIRCULAR`
2. **IDLE Line Interrupt**: Enable `USART_IT_IDLE`, clear with `__HAL_UART_CLEAR_IDLEFLAG()`
3. **Head Position**: `bufSize - __HAL_DMA_GET_COUNTER(hdma_rx)`
4. **Tail Position**: Tracked in `dma_rx_last_pos`, advances after processing
5. **FIFO Push**: DMA callback pushes bytes to lock-free FIFO via `PutWithOverwrite()`

### Phase 2: STM32H7xx (D-Cache Coherency)

**Additional Requirements:**

1. **Buffer in D2 SRAM3**: Use existing `.dmabuf` section (already configured for Wire DMA)
2. **SERIAL_DMA_BUFFER Macro**: Same pattern as WIRE_DMA_BUFFER
3. **Buffer Validation**: Runtime check that buffer is in 0x30040000-0x30048000

**Note**: H7 UART DMA can reuse the MPU configuration already done for Wire DMA Phase 2.

### Phase 3: Optional Enhancements

1. **TX DMA**: Less critical but useful for high-throughput output
2. **Half-Transfer Interrupt**: Earlier notification for large buffers
3. **Error Handling**: Overrun detection, buffer overflow protection

## STM32 HAL Functions

```c
// Enable DMA circular receive
HAL_UART_Receive_DMA(&huart, buffer, size);  // Starts circular DMA

// Enable IDLE interrupt
__HAL_UART_ENABLE_IT(&huart, UART_IT_IDLE);

// In IDLE IRQ handler:
if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_IDLE)) {
    __HAL_UART_CLEAR_IDLEFLAG(&huart);
    // Calculate bytes received from DMA counter
    uint32_t dmaHead = bufferSize - __HAL_DMA_GET_COUNTER(huart.hdmarx);
    // Signal packet complete or process data
}
```

## Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| DMA channel conflicts | Use configurable DMA stream assignment |
| H7 cache coherency | Reuse existing D2 SRAM3 non-cached region |
| Buffer overflow | Size buffer for worst-case burst (GPS: ~256 bytes sufficient) |
| Breaking existing code | New `beginDMA()` method, regular `begin()` unchanged |
| IDLE detection timing | IDLE fires after 1 byte-time of silence - sufficient for NMEA |

## References

- **Betaflight UART DMA**: `src/main/drivers/serial_uart.c` - production implementation
- **UVOS_Duino UART DMA**: `cores/arduino/per/uart.cpp` - DmaListen pattern
- **STM32 AN3109**: "Communication peripheral FIFO emulation with DMA"
- **Wire DMA (this repo)**: `doc/WIRE_DMA.md` - similar pattern for I2C

## FIFO.h Utility - Import with Fixes

Import `FIFO.h` from UVOS_Duino (`/home/geo/src/UVOS_Duino/cores/arduino/util/FIFO.h`) with the following bug fixes:

### Bug 1: Copy Constructor (HIGH priority)

**Original** (Lines 171-175):
```cpp
// BUG: Calls operator= before base class initialized - undefined behavior
FIFO(const FIFO<T, otherCapacity>& other) { *this = other; }
```

**Fix**:
```cpp
FIFO(const FIFO<T, otherCapacity>& other)
    : FIFOBase<T>(buffer_, capacity + 1)
{
    FIFOBase<T>::operator=(other);
}
```

### Bug 2: `PutWithOverwrite()` Race Condition (Medium)

**Original** (Lines 75-102):
- Current order: advances tail → writes data → advances head
- Consumer could read stale data between tail advance and data write

**Fix** - Write data BEFORE advancing tail:
```cpp
bool PutWithOverwrite(T const el)
{
    size_t h = head_.load(std::memory_order_relaxed);
    size_t next_head = h + 1;
    if (next_head == end_) {
        next_head = 0;
    }

    size_t t = tail_.load(std::memory_order_acquire);

    buf_[h] = el;  // Write data FIRST

    if (next_head == t) {
        // Buffer full - advance tail to overwrite oldest
        size_t next_tail = t + 1;
        if (next_tail == end_) {
            next_tail = 0;
        }
        tail_.store(next_tail, std::memory_order_release);
    }

    head_.store(next_head, std::memory_order_release);
    return true;
}
```

### FIFO Usage in HardwareSerial

```cpp
#include "util/FIFO.h"
uvos::FIFO<uint8_t, SERIAL_RX_BUFFER_SIZE> dma_rx_fifo_;

// In DMA callback (uart_dma_check_rx):
dma_rx_fifo_.PutWithOverwrite(byte);  // Overwrites oldest if full

// In available():
return dma_rx_fifo_.GetNumElements();

// In read():
uint8_t b;
return dma_rx_fifo_.Get(b) ? b : -1;
```

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2025-12-06 | Prioritize UART RX DMA over TX | GPS/telemetry are receive-heavy |
| 2025-12-06 | Use circular DMA + IDLE detection | Industry standard (Betaflight, UVOS) |
| 2025-12-06 | Reuse H7 D2 SRAM3 region | Already configured for Wire DMA |
| 2025-12-06 | Document before implement | Learn from Wire DMA complexity |
| 2025-12-06 | Import FIFO.h with fixes | Lock-free atomics, same pattern as UVOS |
| 2025-12-06 | Fix FIFO copy constructor | Original has undefined behavior |
| 2025-12-06 | Fix PutWithOverwrite race | Write data before advancing tail |
