# JHEF-JHEF411 to NUCLEO-F411RE Compatibility Analysis

Analysis of whether NUCLEO-F411RE can be rewired to match JHEF-JHEF411 (NOXE V3) configuration for HIL testing.

## Configuration Comparison

### Current NUCLEO-F411RE HIL Rig
| Peripheral | Pins | Location | Notes |
|------------|------|----------|-------|
| SPI Flash (SPI3) | PC12 (MOSI), PC11 (MISO), PC10 (SCK), PD2 (CS) | Morpho CN7/CN6 | Breadboard jumpers |
| IMU (SPI1) | PA7 (MOSI), PA6 (MISO), PA5 (SCK), PA4 (CS) | Arduino D11/D12/D13/A2 | Available |
| IMU INT | PC4 | Morpho CN5-34 | Available |
| Servo PWM | PB4 (TIM3_CH1) | Arduino D5 | Hardware validated |
| ESC1 PWM | PB6 (TIM4_CH1) | Arduino D10 | Hardware validated |
| ESC2 PWM | PB7 (TIM4_CH2) | Morpho CN7-21 | Hardware validated |

### Target JHEF-JHEF411 Configuration
| Peripheral | Pins | Notes |
|------------|------|-------|
| SPI Flash (SPI2) | **PB15 (MOSI), PB14 (MISO), PB13 (SCK), PB2 (CS)** | 8 MHz |
| IMU (SPI1) | PA7 (MOSI), PA6 (MISO), PA5 (SCK), PA4 (CS) | **Same as current** ✅ |
| IMU INT | **PB3** | Different from current (PC4) |
| Motor 1 (TIM1_CH1) | **PA8** | New |
| Motor 2 (TIM1_CH2) | **PA9** | New |
| Motor 3 (TIM1_CH3) | **PA10** | New |
| Motor 4 (TIM3_CH3) | **PB0_ALT1** | New |
| Motor 5 (TIM3_CH1) | **PB4** | **Conflict with current servo** ⚠️ |

## Pin Availability on NUCLEO-F411RE

### ✅ All JHEF-JHEF411 Pins Available

Checking pinout reference (`doc/NUCLEO_F411RE_PINOUT.md`):

| JHEF Pin | STM32 | Arduino | Nucleo Location | Available? |
|----------|-------|---------|-----------------|------------|
| **SPI Flash (SPI2)** |
| PB15 | MOSI | - | CN5-26 (Morpho) | ✅ Yes |
| PB14 | MISO | - | CN5-28 (Morpho) | ✅ Yes |
| PB13 | SCK | - | CN5-30 (Morpho) | ✅ Yes |
| PB2 | CS | - | CN5-22 (Morpho) | ✅ Yes |
| **IMU (SPI1)** |
| PA7 | MOSI | D11 | CN9-23/24 | ✅ Yes (Arduino) |
| PA6 | MISO | D12 | CN9-25/26 | ✅ Yes (Arduino) |
| PA5 | SCK | D13 | CN9-27/28 | ✅ Yes (Arduino) |
| PA4 | CS | A2 | CN8-5/6 | ✅ Yes (Arduino) |
| PB3 | INT | D3 | CN9-7/8 | ✅ Yes (Arduino) |
| **Motors (TIM1/TIM3)** |
| PA8 | Motor1 | D7 | CN9-15/16 | ✅ Yes (Arduino) |
| PA9 | Motor2 | D8 | CN9-17/18 | ✅ Yes (Arduino) |
| PA10 | Motor3 | D2 | CN9-5/6 | ✅ Yes (Arduino) |
| PB0 | Motor4 | A3 | CN8-7/8 | ✅ Yes (Arduino) |
| PB4 | Motor5 | D5 | CN9-11/12 | ✅ Yes (Arduino) |

**Result: ✅ ALL JHEF-JHEF411 pins are physically available on NUCLEO-F411RE**

## Peripheral Validation

### SPI2 Flash (Target Config)

**Current HIL Rig**: SPI3 (PC12/PC11/PC10/PD2) - Morpho only, requires 4 jumper wires
**JHEF-JHEF411**: SPI2 (PB15/PB14/PB13/PB2) - Morpho only, requires 4 jumper wires

**Pin Check**:
```
PB15 = SPI2_MOSI (CN5-26) ✅
PB14 = SPI2_MISO (CN5-28) ✅
PB13 = SPI2_SCK  (CN5-30) ✅
PB2  = GPIO CS   (CN5-22) ✅
```

**Compatibility**: ✅ **Same complexity** - Both require 4 morpho jumper wires. JHEF config uses SPI2 which is fully supported.

### SPI1 IMU (Target Config)

**Current HIL Rig**: PA7/PA6/PA5/PA4 (D11/D12/D13/A2)
**JHEF-JHEF411**: PA7/PA6/PA5/PA4 (D11/D12/D13/A2)

**Compatibility**: ✅ **IDENTICAL** - No rewiring needed for IMU SPI pins!

### IMU Interrupt Pin

**Current HIL Rig**: PC4 (Morpho CN5-34)
**JHEF-JHEF411**: PB3 (Arduino D3)

**Pin Check**:
```
PB3 = TIM2_CH2, SPI1_SCK (CN9-7/8, Arduino D3) ✅
```

**Compatibility**: ✅ **Arduino header** - Easier access than current (PC4 on morpho). **Improvement!**

### Motor Pins (5 Motors on TIM1 + TIM3)

**JHEF-JHEF411 Motor Layout**:
```
TIM1 Bank (3 motors):
  Motor 1: PA8  (TIM1_CH1) - Arduino D7
  Motor 2: PA9  (TIM1_CH2) - Arduino D8
  Motor 3: PA10 (TIM1_CH3) - Arduino D2

TIM3 Bank (2 motors):
  Motor 4: PB0_ALT1 (TIM3_CH3) - Arduino A3 (requires ALT variant)
  Motor 5: PB4       (TIM3_CH1) - Arduino D5
```

**Pin Check**:
```
PA8  = TIM1_CH1 (CN9-15/16, D7)  ✅
PA9  = TIM1_CH2 (CN9-17/18, D8)  ✅
PA10 = TIM1_CH3 (CN9-5/6, D2)    ✅
PB0  = TIM3_CH3 (CN8-7/8, A3)    ✅ (via ALT1)
PB4  = TIM3_CH1 (CN9-11/12, D5)  ✅
```

**Compatibility**: ✅ **All pins available on Arduino headers** - No morpho jumpers needed!

### Current HIL Rig Conflicts

**CONFLICT**: Current servo PWM uses **PB4 (D5)**, which JHEF uses for Motor 5.

**Resolution Options**:
1. **Remove servo PWM validation** - Focus on motor PWM only (matches JHEF usage)
2. **Move servo to unused pin** - Use different TIM3 channel for servo testing
3. **Dual configuration** - Maintain both configs (current + JHEF)

## Recommended Wiring Plan

### Option A: Full JHEF-JHEF411 Match (Recommended)

**Advantages**:
- ✅ Tests actual flight controller pinout
- ✅ Validates Betaflight converter output
- ✅ Most Arduino header pins (less jumper wires)
- ✅ Enables motor PWM validation for 5 motors

**Wiring Changes Needed**:

| Component | From (Current) | To (JHEF) | Connector | Difficulty |
|-----------|----------------|-----------|-----------|------------|
| SPI Flash MOSI | PC12 (Morpho) | PB15 (Morpho CN5-26) | 1 jumper move | Easy |
| SPI Flash MISO | PC11 (Morpho) | PB14 (Morpho CN5-28) | 1 jumper move | Easy |
| SPI Flash SCK | PC10 (Morpho) | PB13 (Morpho CN5-30) | 1 jumper move | Easy |
| SPI Flash CS | PD2 (Morpho) | PB2 (Morpho CN5-22) | 1 jumper move | Easy |
| IMU INT | PC4 (Morpho CN5-34) | PB3 (Arduino D3) | 1 jumper move | **Easier!** |
| **IMU SPI** | **No change** | **No change** | - | ✅ |

**New Connections for Motors** (5 total):
- Motor 1: PA8 (D7) - Arduino header ✅
- Motor 2: PA9 (D8) - Arduino header ✅
- Motor 3: PA10 (D2) - Arduino header ✅
- Motor 4: PB0 (A3) - Arduino header ✅
- Motor 5: PB4 (D5) - Arduino header ✅ (replaces current servo)

**Total Jumper Count**:
- SPI Flash: 4 jumpers (MOSI, MISO, SCK, CS) - **Morpho to breadboard**
- IMU SPI: 4 jumpers (MOSI, MISO, SCK, CS) - **Arduino to breadboard** (no change)
- IMU INT: 1 jumper (PB3) - **Arduino D3 to breadboard** (easier than before!)
- Motors: 5 jumpers (PA8, PA9, PA10, PB0, PB4) - **Arduino to test points**
- **Total: 14 jumpers** (vs current 12)

### Option B: Hybrid Config (Keep Current + Add JHEF)

Maintain current NUCLEO config for storage/IMU validation, add separate JHEF config for motor testing.

**Advantages**:
- ✅ Preserves existing validated setup
- ✅ Adds JHEF motor validation
- ✅ Flexible testing

**Disadvantages**:
- ❌ More complex test rig
- ❌ Requires config switching

## ALT Variant Requirement

**CRITICAL**: Motor 4 uses **PB0_ALT1** to select TIM3_CH3 (not default TIM1_CH2N).

**Check PeripheralPins.c**:
```cpp
// Default: PB_0 → TIM1_CH2N (complementary output)
{PB_0,      TIM1, STM_PIN_DATA_EXT(..., 2, 1)}

// ALT1: PB_0_ALT1 → TIM3_CH3 (normal output)
{PB_0_ALT1, TIM3, STM_PIN_DATA_EXT(..., 3, 0)}
```

**BoardConfig must use**:
```cpp
static constexpr Channel motor4 = {PB0_ALT1, 3, 0, 0};  // ✅ Correct
// NOT: {PB0, 3, 0, 0};  // ❌ Wrong - would get TIM1_CH2N instead
```

**This is already correct in JHEF-JHEF411.h** ✅

## Pin Conflict Analysis

### No Blocking Conflicts

| Pin | JHEF Usage | Current Usage | Conflict? |
|-----|------------|---------------|-----------|
| PB4 (D5) | Motor 5 (TIM3_CH1) | Servo PWM (TIM3_CH1) | ⚠️ Same usage, different purpose |
| PA5 (D13) | IMU SCK | IMU SCK | ✅ Same |
| PA6 (D12) | IMU MISO | IMU MISO | ✅ Same |
| PA7 (D11) | IMU MOSI | IMU MOSI | ✅ Same |
| PA4 (A2) | IMU CS | IMU CS | ✅ Same |

**Result**: ✅ **NO blocking conflicts** - PB4 conflict is functional (servo vs motor), easily resolved.

## Validation Test Strategy

### Phase 1: Storage Migration
1. Rewire SPI flash from SPI3→SPI2 (4 jumper moves)
2. Update config to JHEF storage settings
3. Run LittleFS_IT to validate
4. **Expected**: All 8 tests pass

### Phase 2: IMU Migration
1. Move IMU INT from PC4→PB3 (1 jumper move, easier access!)
2. Update config to JHEF IMU settings (INT pin only)
3. Run IMU examples to validate
4. **Expected**: Interrupt-driven data acquisition works

### Phase 3: Motor PWM Validation
1. Wire 5 motor outputs (PA8, PA9, PA10, PB0, PB4)
2. Create JHEF motor test sketch using existing PWMOutputBank
3. Validate TIM1 bank (3 motors) + TIM3 bank (2 motors)
4. Use input capture for frequency validation
5. **Expected**: All 5 channels generate correct PWM

## Recommendation

**✅ YES - Rewire to JHEF-JHEF411 configuration**

**Rationale**:
1. ✅ All pins physically available on Nucleo
2. ✅ IMU SPI identical (no rewiring needed)
3. ✅ SPI flash same complexity (morpho jumpers)
4. ✅ IMU INT easier (Arduino D3 vs morpho)
5. ✅ Motors all on Arduino headers (better than morpho)
6. ✅ Validates real flight controller pinout
7. ✅ Tests Betaflight converter output
8. ✅ Enables 5-motor PWM validation

**Trade-off**: Lose current servo PWM test (PB4 D5 repurposed for motor), but gain realistic 5-motor flight controller validation.

## Next Steps

1. **Create JHEF-JHEF411 Nucleo config** - `targets/NUCLEO_F411RE_JHEF411.h`
2. **Update wiring** - Move 5 jumpers (SPI flash + IMU INT)
3. **Validate storage** - Run LittleFS tests
4. **Validate IMU** - Run IMU tests with new INT pin
5. **Add motor tests** - Create 5-motor PWM validation sketch
6. **Document test rig** - Update HW_CONFIG.md with JHEF layout

**Estimated effort**: 30 minutes rewiring + 1 hour testing = **1.5 hours total**

## References

- JHEF-JHEF411 Config: `extras/betaflight_converter/output/JHEF-JHEF411.h`
- Current Config: `targets/NUCLEO_F411RE_LITTLEFS.h`
- Pinout Reference: `doc/NUCLEO_F411RE_PINOUT.md`
- PeripheralPins.c: `Arduino_Core_STM32/variants/STM32F4xx/F411R(C-E)T/PeripheralPins.c`
