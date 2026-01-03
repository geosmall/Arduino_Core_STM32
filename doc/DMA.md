# STM32 DMA Reference

Technical reference for Direct Memory Access (DMA) on STM32 microcontrollers in the Arduino STM32 Core.

## Overview

DMA enables data transfers between memory and peripherals without CPU intervention. This dramatically reduces interrupt overhead for continuous data streams like UART, SPI, and ADC.

**Key Benefits:**
- Zero CPU involvement during transfers
- Reduced interrupt load (per-transfer vs per-byte)
- Background operation while main loop executes
- Essential for high-throughput or timing-critical applications

## DMA Architecture by Family

### STM32F4xx (Fixed Channel Assignment)

DMA1 and DMA2, each with 8 streams. Each stream has fixed channel mappings.

```
DMA Controller → Stream (0-7) → Channel (0-7) → Peripheral
```

**UART DMA Channels (F4):**

| UART | DMA | Stream | Channel |
|------|-----|--------|---------|
| USART1 RX | DMA2 | Stream 2 | Channel 4 |
| USART1 TX | DMA2 | Stream 7 | Channel 4 |
| USART2 RX | DMA1 | Stream 5 | Channel 4 |
| USART2 TX | DMA1 | Stream 6 | Channel 4 |
| USART6 RX | DMA2 | Stream 1 | Channel 5 |
| USART6 TX | DMA2 | Stream 6 | Channel 5 |

### STM32F7xx (Similar to F4)

Same fixed stream/channel architecture as F4. DMA channel assignments are MCU-specific - consult reference manual.

### STM32H7xx (DMAMUX - Flexible Assignment)

H7 introduces DMAMUX for flexible DMA request routing:

```
Peripheral → DMAMUX → DMA Stream → Memory
```

**Benefits:**
- Any DMA request can be routed to any stream
- No fixed channel conflicts
- Uses `DMA_REQUEST_USARTx_RX` instead of fixed channel numbers

**Example (H7 UART DMA):**
```c
hdma_rx.Init.Request = DMA_REQUEST_USART2_RX;  // DMAMUX routing
```

## Common DMA Use Cases

### Flight Controller DMA Usage Analysis

| Peripheral | Betaflight | iNav | Recommended |
|------------|------------|------|-------------|
| **ADC** | ✅ DMA | ✅ DMA | DMA (continuous sampling) |
| **DSHOT (motors)** | ✅ DMA | ✅ DMA | DMA (timing-critical) |
| **LED Strip (WS2812)** | ✅ DMA | ✅ DMA | DMA (bit-bang timing) |
| **UART RX** | ✅ DMA | ❌ IRQ | DMA (GPS, telemetry) |
| **UART TX** | ✅ DMA | ❌ IRQ | Optional |
| **SPI (IMU)** | ✅ DMA | ❌ Polling | Polling (short transfers) |
| **I2C** | ❌ Blocking | ❌ Blocking | Blocking (short transfers) |

**Recommendation:** Use DMA for continuous streams (UART RX, ADC) and timing-critical outputs (DSHOT, LED). Polling is acceptable for short, on-demand transfers (SPI IMU reads, I2C sensors).

### UART RX DMA (Implemented)

**Interrupt Mode (Traditional):**
```
UART RX → RXNE interrupt (per byte) → ISR → ring buffer
         ~11,500 IRQs/sec @ 115200 baud
```

**DMA Mode:**
```
UART RX → DMA circular buffer → IDLE interrupt → batch to ring buffer
         ~100 IRQs/sec (IDLE only)
```

**Interrupt Reduction:** 99%+ for continuous streams like GPS NMEA.

## UART DMA Implementation

### API (HardwareSerial)

```cpp
// Start DMA circular RX
// Buffer must use SERIAL_DMA_BUFFER on H7 for cache coherency
bool beginDMA(unsigned long baud, uint8_t *rxBuffer, size_t rxBufferSize);

// Stop DMA, revert to interrupt mode
void endDMA(void);

// Check if DMA mode is active
bool isDMAListening(void);
```

### Usage Example (GPS)

```cpp
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

// DMA buffer - must be in non-cached memory on H7
SERIAL_DMA_BUFFER uint8_t gpsRxBuffer[256];

HardwareSerial SerialGPS(PA3, PA2);
TinyGPSPlus gps;

void setup() {
    // Initialize with DMA - buffer fills automatically in background
    SerialGPS.beginDMA(9600, gpsRxBuffer, sizeof(gpsRxBuffer));
}

void loop() {
    // Read API unchanged - but no per-byte interrupts occurred
    while (SerialGPS.available()) {
        gps.encode(SerialGPS.read());
    }
}
```

### Circular DMA + IDLE Detection

The implementation uses circular DMA with UART IDLE line detection:

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
         (packet/frame complete)
```

1. **DMA runs continuously** in circular mode
2. **IDLE interrupt** fires when line goes idle (gap between packets)
3. **CPU reads from tail**, DMA writes at head - no conflicts
4. **Zero-copy** to DMA buffer, then batch copy to ring buffer

### Supported UARTs

**STM32F4xx:** USART1, USART2, USART6 (fixed DMA stream/channel)

**STM32H7xx:** USART1, USART2, USART3, UART4, USART6 (DMAMUX)

## STM32H7 Cache Coherency

### The Problem

H7 has a D-Cache that can cause DMA coherency issues:
- CPU writes may be cached, not visible to DMA
- DMA writes may be overwritten by cached CPU data

### Solution: Non-Cached Memory Region

DMA buffers on H7 must be placed in non-cached memory. The core provides D2 SRAM3 (0x30040000-0x30047FFF, 32KB) configured as non-cached via MPU.

**Buffer Placement Macro:**

```cpp
#if defined(STM32H7xx)
  #define SERIAL_DMA_BUFFER __attribute__((section(".dmabuf")))
#else
  #define SERIAL_DMA_BUFFER  // No-op on F4/F7
#endif
```

**Usage:**
```cpp
SERIAL_DMA_BUFFER uint8_t rxBuffer[256];  // Placed in D2 SRAM3 on H7
```

**Runtime Validation:** `beginDMA()` returns error (-8) if buffer is not in the valid D2 SRAM3 range on H7.

### Alternative: Cache Invalidation

For read-only DMA buffers, invalidate cache before reading:
```c
SCB_InvalidateDCache_by_Addr(buffer, size);
```

For write buffers, clean cache before DMA starts:
```c
SCB_CleanDCache_by_Addr(buffer, size);
```

The non-cached region approach is simpler and recommended.

## HAL/LL Functions Reference

### DMA Initialization

```c
// Configure DMA handle
DMA_HandleTypeDef hdma_rx;
hdma_rx.Instance = DMA1_Stream5;
hdma_rx.Init.Channel = DMA_CHANNEL_4;           // F4/F7
hdma_rx.Init.Request = DMA_REQUEST_USART2_RX;   // H7 DMAMUX
hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
hdma_rx.Init.Mode = DMA_CIRCULAR;               // Key: circular mode
hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;
HAL_DMA_Init(&hdma_rx);

// Link to UART
__HAL_LINKDMA(&huart, hdmarx, hdma_rx);
```

### Starting DMA Reception

```c
// Start circular DMA receive
HAL_UART_Receive_DMA(&huart, buffer, size);

// Enable IDLE interrupt for packet detection
__HAL_UART_ENABLE_IT(&huart, UART_IT_IDLE);
```

### IDLE Interrupt Handling

```c
void USARTx_IRQHandler(void) {
    if (__HAL_UART_GET_FLAG(&huart, UART_FLAG_IDLE)) {
        __HAL_UART_CLEAR_IDLEFLAG(&huart);

        // Calculate bytes received
        uint32_t head = bufferSize - __HAL_DMA_GET_COUNTER(huart.hdmarx);

        // Process data from tail to head
        // ...
    }
    HAL_UART_IRQHandler(&huart);
}
```

### DMA Position Tracking

```c
// Get current DMA write position (head)
size_t dma_head = buffer_size - __HAL_DMA_GET_COUNTER(&hdma_rx);

// Track last processed position (tail)
static size_t last_pos = 0;

// Calculate new data
if (dma_head >= last_pos) {
    new_bytes = dma_head - last_pos;
} else {
    // Wrapped around
    new_bytes = (buffer_size - last_pos) + dma_head;
}
last_pos = dma_head;
```

## Hardware Validation Status

| Platform | Status | Notes |
|----------|--------|-------|
| **NUCLEO_F411RE** | ✅ Validated | 50/50 stress tests, mode switching verified |
| **NUCLEO_H753ZI** | ✅ Validated | 50/50 stress tests, D2 SRAM3 buffer verified |

**Tests Performed:**
- Mode switching: INT → DMA → INT → DMA
- NMEA-sized payloads (82 bytes)
- Buffer wraparound (635 bytes through 256-byte buffer)
- H7 buffer placement validation

## Troubleshooting

### DMA Not Receiving Data

1. **Check DMA clock enabled:** `__HAL_RCC_DMA1_CLK_ENABLE()`
2. **Verify stream/channel mapping** matches peripheral (see reference manual)
3. **Check NVIC priorities:** DMA IRQ must be enabled
4. **H7: Verify buffer in D2 SRAM3** using `SERIAL_DMA_BUFFER` macro

### Data Corruption on H7

1. **Buffer not in non-cached region:** Use `SERIAL_DMA_BUFFER` macro
2. **Cache not invalidated:** Call `SCB_InvalidateDCache_by_Addr()` before reading
3. **Buffer address:** Must be in 0x30040000-0x30047FFF range

### IDLE Interrupt Not Firing

1. **IDLE IT not enabled:** Call `__HAL_UART_ENABLE_IT(&huart, UART_IT_IDLE)`
2. **Flag not cleared:** Call `__HAL_UART_CLEAR_IDLEFLAG()` in handler
3. **No gap in data:** IDLE requires ~1 byte-time of silence

## References

- **STM32 AN3109:** "Communication peripheral FIFO emulation with DMA"
- **STM32F4 Reference Manual:** DMA controller chapter
- **STM32H7 Reference Manual:** DMAMUX chapter
- **Betaflight:** `src/main/drivers/serial_uart.c` - production UART DMA
