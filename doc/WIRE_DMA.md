# WIRE_DMA.md - DMA Read Capability for Wire Library

## Overview

Add non-blocking DMA read capability to the Wire library for efficient multi-byte I2C sensor reads.

**Status**: Phase 1 Implementation Complete (pending hardware validation)
**Reference**: UVOS_Duino Wire library DMA implementation

## Implementation Phases

| Phase | Target | Scope |
|-------|--------|-------|
| **Phase 1** | STM32F4xx | Core DMA implementation, no cache concerns |
| **Phase 2** | STM32H7xx | Add non-cached memory requirement + runtime check |

## Motivation

Current Wire library uses blocking reads - CPU polls until I2C transfer completes. For slow multi-byte reads (barometers, EEPROMs, multi-register sensor bursts), DMA frees the CPU during transfer.

## API Design

```cpp
// New TwoWire methods (polling-based, non-blocking)
bool requestFromDMA(uint8_t address, uint8_t* buf, uint32_t len, bool stopBit = true);
bool dmaTransferDone() const;
```

**Usage Pattern (F4):**
```cpp
uint8_t sensorData[16];
Wire.requestFromDMA(0x77, sensorData, 16);
while (!Wire.dmaTransferDone()) {
    // Do other work...
}
// Data now in sensorData[]
```

**Usage Pattern (H7 - Phase 2):**
```cpp
WIRE_DMA_BUFFER uint8_t sensorData[16];  // Must be in non-cached memory
Wire.requestFromDMA(0x77, sensorData, 16);
while (!Wire.dmaTransferDone()) {
    // Do other work...
}
```

## Architecture

### Current Wire Stack
```
TwoWire (Wire.h/cpp)     ← Arduino API
    ↓
twi.c/h                  ← HAL wrapper (blocking)
    ↓
HAL_I2C_Master_Receive_IT()  ← Interrupt-based, but code blocks waiting
```

### With DMA Addition
```
TwoWire (Wire.h/cpp)     ← Arduino API + new DMA methods
    ↓
twi.c/h                  ← HAL wrapper + DMA functions
    ↓
HAL_I2C_Master_Receive_DMA() ← True non-blocking with DMA
    ↓
DMA IRQ → Callback       ← Signals completion
```

---

## Phase 1: STM32F4xx Implementation

### Files to Modify

| File | Changes |
|------|---------|
| `libraries/Wire/src/utility/twi.h` | Add DMA state to `i2c_s` struct |
| `libraries/Wire/src/utility/twi.c` | Add DMA init, read, IRQ, callbacks |
| `libraries/Wire/src/Wire.h` | Declare requestFromDMA(), dmaTransferDone() |
| `libraries/Wire/src/Wire.cpp` | Implement new methods |

### 1. DMA State (twi.h)

Add to `struct i2c_s`:
```c
// DMA RX state
DMA_HandleTypeDef hdma_rx;
volatile uint8_t *dma_rx_buf;
volatile uint16_t dma_rx_len;
volatile bool dma_rx_busy;
volatile i2c_status_e dma_rx_result;
```

### 2. F4xx DMA Channel Mapping

| I2C | DMA | Stream | Channel |
|-----|-----|--------|---------|
| I2C1 | DMA1 | Stream 0 | Channel 1 |
| I2C2 | DMA1 | Stream 2 | Channel 7 |
| I2C3 | DMA1 | Stream 2 | Channel 3 |

### 3. Core Functions (twi.c)

```c
// Initialize DMA for I2C RX (lazy init on first use)
static void i2c_dma_rx_init(i2c_t *obj);

// Start DMA read - returns immediately
i2c_status_e i2c_master_read_dma(i2c_t *obj, uint8_t dev_address,
                                  uint8_t *data, uint16_t size);

// Check if DMA transfer complete
bool i2c_dma_rx_done(i2c_t *obj);

// Get result of last DMA transfer
i2c_status_e i2c_dma_rx_result(i2c_t *obj);
```

### 4. IRQ Handlers

```c
// DMA stream IRQ handler
void DMA1_Stream0_IRQHandler(void);  // I2C1 RX

// HAL completion callbacks (weak, we override)
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
```

### 5. Callback Implementation (F4 - Simple)

```c
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_t *obj = get_i2c_obj(hi2c);
    obj->dma_rx_busy = false;
    obj->dma_rx_result = I2C_OK;
    // No cache ops needed on F4
}
```

### 6. Arduino API Layer

```cpp
// Wire.h additions
bool requestFromDMA(uint8_t address, uint8_t* buf, uint32_t len, bool stopBit = true);
bool dmaTransferDone() const;

// Wire.cpp implementation
bool TwoWire::requestFromDMA(uint8_t address, uint8_t* buf,
                              uint32_t len, bool stopBit) {
    if (!i2c_dma_rx_done(&_i2c)) return false;  // Previous DMA still running
    return i2c_master_read_dma(&_i2c, address << 1, buf, len) == I2C_OK;
}

bool TwoWire::dmaTransferDone() const {
    return i2c_dma_rx_done(&_i2c);
}
```

### Phase 1 Testing (NUCLEO_F411RE)

1. DPS3xx barometer - 6-byte temperature/pressure burst read
2. Timing comparison: blocking vs DMA
3. Data integrity verification
4. Stress test: rapid sequential reads
5. Interleave DMA and blocking reads

---

## Phase 2: STM32H7xx Implementation

### Additional Requirements

H7 has D-Cache - DMA writes to RAM bypass cache, causing coherency issues.

**Solution**: Require buffers in non-cached memory region (no cache ops needed).

### Additional Files to Modify

| File | Changes |
|------|---------|
| `system/STM32H7xx/ldscript.ld` | Add `.dmabuf` section in D2 SRAM |
| `cores/arduino/stm32/system_stm32h7xx.c` | MPU config for non-cached region |
| `libraries/Wire/src/Wire.h` | Add `WIRE_DMA_BUFFER` macro |
| `libraries/Wire/src/utility/twi.c` | Add buffer address validation |

### H7xx DMA Channel Mapping

| I2C | DMA | Stream | Request |
|-----|-----|--------|---------|
| I2C1 | DMA1 | Stream 0 | DMA_REQUEST_I2C1_RX |
| I2C2 | DMA1 | Stream 2 | DMA_REQUEST_I2C2_RX |
| I2C4 | BDMA | Channel 1 | BDMA_REQUEST_I2C4_RX |

### Buffer Macro

```c
// In Wire.h
#if defined(STM32H7xx)
  #define WIRE_DMA_BUFFER __attribute__((section(".dmabuf")))
#else
  #define WIRE_DMA_BUFFER  // No-op on F4
#endif
```

### Runtime Buffer Validation (H7 only)

```c
#if defined(STM32H7xx)
// D2 SRAM1 (non-cached region): 0x30000000 - 0x30007FFF (32KB)
#define IS_DMA_SAFE_BUFFER(buf) \
    ((uint32_t)(buf) >= 0x30000000 && (uint32_t)(buf) < 0x30008000)
#endif

i2c_status_e i2c_master_read_dma(i2c_t *obj, uint8_t addr, uint8_t *data, uint16_t size)
{
#if defined(STM32H7xx)
    if (!IS_DMA_SAFE_BUFFER(data)) {
        return I2C_ERROR;  // Buffer not in non-cached region
    }
#endif
    // ... proceed with DMA
}
```

### Callback (H7 - Also Simple)

```c
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    i2c_t *obj = get_i2c_obj(hi2c);
    obj->dma_rx_busy = false;
    obj->dma_rx_result = I2C_OK;
    // No cache ops needed - buffer is in non-cached memory
}
```

### Phase 2 Testing (MATEK_H743VI)

1. Verify runtime check rejects normal buffers
2. DPS3xx with `WIRE_DMA_BUFFER` - same tests as Phase 1
3. Confirm no data corruption

---

## Risks & Mitigations

| Risk | Mitigation |
|------|------------|
| DMA stream conflicts | Use streams not typically claimed by SPI/UART |
| H7 wrong buffer | Runtime address check returns error (not silent corruption) |
| IRQ priority issues | Match existing I2C IRQ priorities |
| Breaking existing code | Additive API only, no changes to existing functions |

## Estimated Effort

| Component | Lines | Effort |
|-----------|-------|--------|
| twi.h additions | ~15 | Small |
| twi.c DMA functions (F4) | ~150 | Medium |
| Wire.h/cpp additions | ~40 | Small |
| **Phase 1 Total** | **~200** | **Medium** |
| H7 additions (twi.c) | ~30 | Small |
| Linker script + MPU | ~20 | Small |
| **Phase 2 Total** | **~50** | **Small** |

## Reference Implementation

UVOS_Duino Wire library: `/home/geo/src/UVOS_Duino/libraries/Wire`
- `Wire.h:44-55` - API declarations
- `Wire.cpp:138-161` - requestFromDMA implementation
- `cores/arduino/per/i2c.cpp:562-632` - HAL DMA integration
- `cores/arduino/sys/dma.c` - Cache helpers
- `cores/arduino/uvos_core.h:28` - DMA_BUFFER_MEM_SECTION macro

## Future Extensions

- TX DMA for large writes (EEPROM programming)
- Callback-based completion notification
