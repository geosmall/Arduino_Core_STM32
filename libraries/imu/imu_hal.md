# IMU Hardware Abstraction — Filter Presets for Flight Controllers

**Target audience:** Flight controller firmware developers
**Scope:** Unified configuration for ICM-42688-P, MPU-6000, MPU-9250
**Goal:** Intent-based presets with single-source-of-truth LUT

---

## Design Philosophy

Four **intent-based presets** abstract hardware differences across three IMU families:

| Preset | Intent | Use Case |
|--------|--------|----------|
| **SAFE** | Very tame for unknown/noisy frames | Initial bring-up, troubleshooting |
| **SMOOTH** | Extra on-chip filtering, moderate latency | Smooth video, GPS hold |
| **BALANCED** | Strong default for 2 kHz PID loops | General flight, racing |
| **ACRO** | Lowest latency, rely on software filters | Aggressive acro, freestyle |

**Common settings (all presets, all chips):**
- Gyro FSR: **±2000 dps**
- Accel FSR: **±16 g**
- Target rates: Gyro **4 kHz**, Accel **1 kHz** (ACRO may use 8 kHz gyro where feasible)

---

## ICM-42688-P Configuration

**Architecture:** Dual-stage filtering (AAF → UI filter)
- **AAF (Anti-Alias Filter):** Primary hardware filter, sets actual corner frequency
- **UI (User Interface) Filter:** Secondary digital filter, can add smoothing or stay wide

**Initialization requirement:** Disable Auto-FSR to prevent stalls
```c
uint8_t val = readReg(BANK0, 0x4D);  // INTF_CONFIG1
val = (val & ~0xC0) | 0x40;          // Set bits[7:6] = 01 (AFSR_DISABLE)
writeReg(BANK0, 0x4D, val);
```

### Preset Table

| Preset | Gyro ODR | Accel ODR | Gyro AAF (Hz) | Accel AAF (Hz) | UI Order | UI BW Code |
|--------|----------|-----------|---------------|----------------|----------|------------|
| **SAFE** | 1 kHz | 1 kHz | 126 | 84 | 2nd | 1 |
| **SMOOTH** | 4 kHz | 1 kHz | 213 | 126 | 2nd | 1 |
| **BALANCED** | 4 kHz | 1 kHz | 258 | 170 | 1st | 15 |
| **ACRO** | 8 kHz | 1 kHz | 303 | 170 | 1st | 15 |

**AAF register values:**

| Corner (Hz) | DELT | DELTSQR | BITSHIFT | Notes |
|-------------|------|---------|----------|-------|
| 84 | 3 | 0x0009 | 12 | SAFE accel |
| 126 | 4 | 0x0010 | 12 | SAFE gyro, SMOOTH accel |
| 170 | 4 | 0x0010 | 11 | BALANCED/ACRO accel |
| 213 | 5 | 0x0019 | 11 | SMOOTH gyro |
| 258 | 6 | 0x0024 | 10 | BALANCED gyro |
| 303 | 7 | 0x0031 | 10 | ACRO gyro |

**Register locations:**
- **ODR (BANK0):**
  - GYRO_CONFIG0 (0x4F): GYRO_ODR bits[3:0] (0x06=1kHz, 0x05=4kHz, 0x04=8kHz)
  - ACCEL_CONFIG0 (0x50): ACCEL_ODR bits[3:0] (0x06=1kHz)
- **Gyro AAF (BANK1):**
  - 0x0B (enable), 0x0C (DELT), 0x0D (DELTSQR[7:0]), 0x0E (BITSHIFT[7:4] + DELTSQR[11:8])
- **Accel AAF (BANK2):**
  - 0x03 (DELT bits[6:1] + enable bit[0]), 0x04 (DELTSQR[7:0]), 0x05 (BITSHIFT[7:4] + DELTSQR[11:8])
- **UI filter (BANK0):**
  - 0x52 (GYRO_ACCEL_CONFIG0 - BW codes: gyro bits[3:0], accel bits[7:4])
  - 0x51 (GYRO_CONFIG1 - gyro order bits[3:2])
  - 0x53 (ACCEL_CONFIG1 - accel order bits[4:3])

**Guardrail:** Keep AAF ≤ 0.45×ODR (absolute max). For control loops, target 0.05–0.20×ODR.

---

## MPU-6000 Configuration

**Architecture:** Single-stage DLPF (Digital Low-Pass Filter)

### Rate Calculation Rules

**DLPF_CFG determines base rate:**
- `DLPF_CFG = 0` → Internal rate: **8 kHz** → `Gyro SR = 8 kHz / (1 + SMPLRT_DIV)`
- `DLPF_CFG = 1..6` → Internal rate: **1 kHz** → `Gyro SR = 1 kHz / (1 + SMPLRT_DIV)`

**Accel path:** Fixed 1 kHz update rate (gyro rates >1 kHz will re-read same accel sample)

### Preset Table

| Preset | DLPF_CFG | Gyro BW (Hz) | SMPLRT_DIV | Gyro SR | Accel BW (Hz) |
|--------|----------|--------------|------------|---------|---------------|
| **SAFE** | 2 | 98 | 0 | 1 kHz | 94 |
| **SMOOTH** | 1 | 188 | 0 | 1 kHz | 184 |
| **BALANCED** | 0 | 256 | 1 | 4 kHz | 260 |
| **ACRO** | 0 | 256 | 0 | 8 kHz | 260 |

**Register locations:**
- CONFIG (0x1A): DLPF_CFG bits[2:0]
- SMPLRT_DIV (0x19): Sample rate divider (0-255)
- GYRO_CONFIG (0x1B): FS_SEL bits[4:3] (3 = ±2000 dps)
- ACCEL_CONFIG (0x1C): AFS_SEL bits[4:3] (3 = ±16 g)

**Key feature:** DLPF_CFG=0 works correctly with SMPLRT_DIV for flexible rate control.

---

## MPU-9250 Configuration

**Architecture:** Separate gyro/accel DLPF paths (MPU-6500-class gyro + AK8963 mag)

### Rate Calculation Rules

**DLPF_CFG determines base rate:**
- `DLPF_CFG = 0` → Internal rate: **8 kHz** → `Gyro SR = 8 kHz / (1 + SMPLRT_DIV)` ⚠️ **See critical issue below**
- `DLPF_CFG = 1..6` → Internal rate: **1 kHz** → `Gyro SR = 1 kHz / (1 + SMPLRT_DIV)`

**Accel path:** Configured separately via ACCEL_CONFIG2.A_DLPF_CFG (1 kHz update when DLPF active)

### ⚠️ CRITICAL HARDWARE LIMITATION

**SMPLRT_DIV prerequisites (MPU-9250 Register Map, page 12):**

The SMPLRT_DIV register is **only effective when**:
1. `FCHOICE_B[1:0] = 00` (DLPF enabled), **AND**
2. **`0 < DLPF_CFG < 7`** (DLPF_CFG must be 1-6, **NOT 0**)

**Hardware-validated behavior:**
| Configuration | Expected Rate | Actual Rate | Result |
|---------------|---------------|-------------|--------|
| DLPF_CFG=0, SMPLRT_DIV=7 | 1 kHz (8kHz/8) | ~8 kHz | ❌ DIVIDER IGNORED |
| DLPF_CFG=1, SMPLRT_DIV=0 | 1 kHz (1kHz/1) | ~1 kHz | ✅ WORKS CORRECTLY |

**Root cause:** Register write-verify succeeds (reads back 0x07), but hardware ignores SMPLRT_DIV when DLPF_CFG=0.

**Impact:** Unlike MPU-6000, you **cannot use DLPF_CFG=0 with SMPLRT_DIV** for rate control on MPU-9250.

### Preset Table (Hardware-Validated)

| Preset | Gyro DLPF_CFG | Gyro BW (Hz) | SMPLRT_DIV | On-Sensor Rate | Effective Rate | Accel DLPF_CFG | Accel BW (Hz) |
|--------|---------------|--------------|------------|----------------|----------------|----------------|---------------|
| **SAFE** | 2 | 92 | 0 | 1 kHz | 1 kHz | 2 | 92 |
| **SMOOTH** | 1 | 184 | 0 | 1 kHz | 1 kHz | 1 | 184 |
| **BALANCED** | 0 | 250 | 0 | 8 kHz | 4 kHz (SW decimate ×2) | 1 | 184 |
| **ACRO** | 0 | 250 | 0 | 8 kHz | 8 kHz | 1 | 184 |

**Implementation notes:**
- **SAFE/SMOOTH:** Use DLPF_CFG=1-6 to enable SMPLRT_DIV functionality (hardware divides to 1 kHz)
- **BALANCED:** Uses DLPF_CFG=0 (250 Hz wide filter) at 8 kHz on-sensor. **Driver must read every 2nd sample** to achieve 4 kHz effective rate.
- **ACRO:** Uses DLPF_CFG=0 (250 Hz wide filter) at 8 kHz, driver reads every sample for full 8 kHz effective rate.
- **Software decimation:** SMPLRT_DIV is non-functional with DLPF_CFG=0, so rate control must be implemented in software for BALANCED.

**Register locations:**
- CONFIG (0x1A): DLPF_CFG bits[2:0]
- SMPLRT_DIV (0x19): Sample rate divider (0-255) - only functional when DLPF_CFG=1-6
- GYRO_CONFIG (0x1B): FCHOICE_B bits[1:0] (must be 00 for DLPF), FS_SEL bits[4:3] (3 = ±2000 dps)
- ACCEL_CONFIG (0x1C): AFS_SEL bits[4:3] (3 = ±16 g)
- ACCEL_CONFIG2 (0x1D): ACCEL_FCHOICE_B bit[3] (must be 0), A_DLPF_CFG bits[2:0]

### Software Decimation Example (BALANCED Preset)

Since SMPLRT_DIV is non-functional with DLPF_CFG=0, implement rate control in the driver:

```c
// MPU-9250 BALANCED: Software decimation from 8 kHz to 4 kHz
static uint8_t decimate_counter = 0;

void imu_drdy_interrupt_handler(void) {
  // Hardware DRDY fires at 8 kHz (DLPF_CFG=0)
  decimate_counter++;

  if (decimate_counter >= 2) {  // Process every 2nd sample
    decimate_counter = 0;

    // Read and process IMU data (4 kHz effective rate)
    mpu9250_read_gyro_accel();
    // ... feed to PID loop running at 4 kHz (or 2 kHz with 2× decimation)
  }
  // Sample at 8 kHz but don't read: ignored, next interrupt overwrites
}

// Alternative: Polled mode at lower frequency
void main_loop_4khz(void) {  // Called at 4 kHz by scheduler
  if (mpu9250_data_ready()) {
    mpu9250_read_gyro_accel();  // Read latest sample, ignore intermediate
  }
}
```

**Rationale:** Using DLPF_CFG=0 provides the wide 250 Hz filter bandwidth needed for BALANCED/ACRO presets, matching the intent across all three IMU families. The 8 kHz on-sensor rate is decimated in software to achieve the target effective rate.

---

## Chip Comparison Summary

| Feature | ICM-42688-P | MPU-6000 | MPU-9250 |
|---------|-------------|----------|----------|
| **Filter architecture** | AAF + UI (dual-stage) | DLPF (single-stage) | DLPF gyro/accel (split) |
| **DLPF_CFG=0 + divider** | N/A (no DLPF_CFG) | ✅ Works correctly | ❌ Divider ignored (use SW decimation) |
| **Max gyro ODR** | 8 kHz (ACRO) | 8 kHz (ACRO) | 8 kHz (ACRO with DLPF_CFG=0) |
| **Accel ODR** | Configurable (1 kHz typical) | Fixed 1 kHz | 1 kHz (when DLPF active) |
| **Separate accel filter** | Yes (AAF) | No (shares DLPF) | Yes (A_DLPF_CFG) |
| **Flexible rate control** | Yes (via ODR regs) | Yes (HW: DLPF_CFG + DIV) | Hybrid (HW for 1kHz, SW for 4k/8k) |
| **Wide filter + 4kHz** | Yes (AAF 258 Hz @ 4kHz ODR) | Yes (DLPF_CFG=0, DIV=1) | Requires SW decimation from 8kHz |

---

## Implementation: Single-Source-of-Truth LUT

```c
typedef enum { FILTER_SAFE, FILTER_SMOOTH, FILTER_BALANCED, FILTER_ACRO } ImuPreset;
typedef enum { IMU_ICM42688P, IMU_MPU6000, IMU_MPU9250 } ImuModel;

typedef struct {
  ImuPreset preset;
  ImuModel  model;

  // Common
  uint16_t gyro_odr_hz;
  uint16_t accel_odr_hz;
  uint16_t gyro_fsr_dps;   // Always 2000
  uint8_t  accel_fsr_g;    // Always 16

  // ICM-42688-P
  struct {
    uint8_t  gyro_aaf_delt, gyro_aaf_bitshift;  uint16_t gyro_aaf_deltsqr;
    uint8_t  accel_aaf_delt, accel_aaf_bitshift; uint16_t accel_aaf_deltsqr;
    uint8_t  ui_bw_code_gyro, ui_bw_code_accel;  // 0-15 (15 = wide/low-latency)
    uint8_t  ui_order_gyro, ui_order_accel;      // 1 or 2
  } icm;

  // MPU-6000 / MPU-9250
  struct {
    uint8_t  dlpf_cfg_gyro;    // CONFIG.DLPF_CFG (0-6)
    uint8_t  dlpf_cfg_accel;   // MPU-9250: ACCEL_CONFIG2.A_DLPF_CFG; MPU-6000: N/A (shares gyro DLPF)
    uint8_t  smplrt_div;       // SMPLRT_DIV register (0-255)
  } mpu;

} ImuConfig;

// Example LUT entries
const ImuConfig IMU_PRESETS[] = {
  // ICM-42688-P
  {FILTER_SAFE, IMU_ICM42688P, 1000, 1000, 2000, 16,
   {4,12,0x0010, 3,12,0x0009, 1,1, 2,2}, {0,0,0}},

  {FILTER_SMOOTH, IMU_ICM42688P, 4000, 1000, 2000, 16,
   {5,11,0x0019, 4,12,0x0010, 1,1, 2,2}, {0,0,0}},

  {FILTER_BALANCED, IMU_ICM42688P, 4000, 1000, 2000, 16,
   {6,10,0x0024, 4,11,0x0010, 15,15, 1,1}, {0,0,0}},

  {FILTER_ACRO, IMU_ICM42688P, 8000, 1000, 2000, 16,
   {7,10,0x0031, 4,11,0x0010, 15,15, 1,1}, {0,0,0}},

  // MPU-6000
  {FILTER_SAFE, IMU_MPU6000, 1000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {2,0,0}},  // DLPF_CFG=2 (98Hz gyro, 94Hz accel), DIV=0

  {FILTER_SMOOTH, IMU_MPU6000, 1000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {1,0,0}},  // DLPF_CFG=1 (188Hz gyro, 184Hz accel), DIV=0

  {FILTER_BALANCED, IMU_MPU6000, 4000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {0,0,1}},  // DLPF_CFG=0 (256Hz gyro, 260Hz accel), DIV=1

  {FILTER_ACRO, IMU_MPU6000, 8000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {0,0,0}},  // DLPF_CFG=0 (256Hz gyro, 260Hz accel), DIV=0

  // MPU-9250
  {FILTER_SAFE, IMU_MPU9250, 1000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {2,2,0}},  // Gyro DLPF=2 (92Hz), Accel DLPF=2 (92Hz), DIV=0 → 1kHz

  {FILTER_SMOOTH, IMU_MPU9250, 1000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {1,1,0}},  // Gyro DLPF=1 (184Hz), Accel DLPF=1 (184Hz), DIV=0 → 1kHz

  {FILTER_BALANCED, IMU_MPU9250, 8000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {0,1,0}},  // Gyro DLPF=0 (250Hz wide), Accel DLPF=1 (184Hz), DIV=0 → 8kHz (SW decimate to 4kHz)

  {FILTER_ACRO, IMU_MPU9250, 8000, 1000, 2000, 16,
   {0,0,0, 0,0,0, 0,0, 0,0}, {0,1,0}},  // Gyro DLPF=0 (250Hz wide), Accel DLPF=1 (184Hz), DIV=0 → 8kHz

  // NOTE: MPU-9250 BALANCED/ACRO use DLPF_CFG=0 for wide 250 Hz filter bandwidth.
  //       SMPLRT_DIV is non-functional with DLPF_CFG=0 (hardware-validated).
  //       BALANCED requires software decimation (×2) to achieve 4 kHz effective rate.
};
```

---

## Validation Checklist

### Basic Sanity
- [ ] WHO_AM_I matches expected value (0x47=ICM-42688-P, 0x68=MPU-6000, 0x71=MPU-9250)
- [ ] FSRs configured correctly (±2000 dps gyro, ±16 g accel)
- [ ] No SPI communication errors during init

### Rate Verification
- [ ] **ICM-42688-P:** ODR registers match preset (measure with scope or verify via register read-back)
- [ ] **MPU-6000:** DRDY interrupt rate matches expected (measure with scope/input-capture)
- [ ] **MPU-9250:**
  - SAFE/SMOOTH (DLPF_CFG≥1): Verify 1 kHz DRDY rate
  - BALANCED/ACRO (DLPF_CFG=0): **Expect 8 kHz DRDY rate** (this is correct behavior)
  - Verify software decimation achieves target effective rate (4 kHz for BALANCED, 8 kHz for ACRO)

### Register Read-Back (MPU-9250 Debug)
For MPU-9250, read back critical registers to verify configuration:
```c
uint8_t config = readReg(0x1A);          // Should show DLPF_CFG in bits[2:0]
uint8_t smplrt_div = readReg(0x19);      // Should show divider value
uint8_t gyro_config = readReg(0x1B);     // FCHOICE_B[1:0] should be 00
uint8_t int_enable = readReg(0x38);      // DATA_RDY_EN (bit 0) should be 1
```

**Key insight:** For BALANCED/ACRO with DLPF_CFG=0, CONFIG should read 0x00 and SMPLRT_DIV should read 0x00. The hardware correctly produces 8 kHz output - this is expected behavior. SMPLRT_DIV is only functional with DLPF_CFG=1-6.

### Filter Performance
- [ ] FFT shows expected filter corner (e.g., ~258 Hz for BALANCED on ICM-42688-P)
- [ ] No hard aliasing at Nyquist frequency
- [ ] Noise floor acceptable for D-term

### Flight Feel
- [ ] Not sluggish (if so: raise AAF/DLPF corner or open UI filter)
- [ ] No excessive motor noise/oscillation (if so: lower AAF/DLPF corner)

---

## Tuning Tips

**If sluggish but clean:**
- ICM-42688-P: Increase gyro AAF (213→258→303 Hz) or change UI from code 1 → code 15
- MPU: Increase DLPF_CFG (2→1→0 for wider bandwidth)

**If noisy / D-term oscillation:**
- ICM-42688-P: Decrease gyro AAF (258→213→170 Hz)
- MPU: Decrease DLPF_CFG (0→1→2 for tighter filtering)

**Betaflight parity (ICM-42688-P):**
- Gyro/Accel AAF: 258 Hz / 258 Hz
- UI BW: code 15 (wide)
- UI order: 2nd (reset default)

**Betaflight parity (MPU-9250):**
- **SMOOTH preset:** Gyro DLPF_CFG=1 (184 Hz), Accel DLPF_CFG=1 (184 Hz), SMPLRT_DIV=0 → 1 kHz
- **BALANCED/ACRO:** Use DLPF_CFG=0 (250 Hz wide) with software decimation for 4kHz/8kHz rates
- Note: Betaflight typically runs MPU-9250 at 1 kHz (SMOOTH-equivalent), not 4kHz/8kHz

---

## References

- ICM-42688-P Datasheet (TDK/InvenSense DS-000347, Rev 1.7)
- MPU-6000 Register Map (InvenSense RM-MPU-6000A, Rev 4.2)
- MPU-9250 Register Map (InvenSense RM-MPU-9250A-00, Rev 1.6) - **Page 12: SMPLRT_DIV prerequisites**
- Betaflight firmware (gyro initialization sequences)
- dRehmFlight BETA 1.3 (Teensy reference implementation)

**Hardware validation platform:** BLACKPILL_F411CE with J-Link RTT, interrupt rate measurement via timer input capture
