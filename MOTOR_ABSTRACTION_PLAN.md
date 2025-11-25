# Motor Abstraction and dRehmFlight STM32 Refactoring Plan

## Executive Summary

**Problem**: dRehmFlight STM32 port has broken motor abstraction with hardcoded timer bank assumptions that prevent supporting boards with different timer assignments (NERO F7 uses TIM5/TIM3/TIM8, not TIM1/TIM3).

**Solution**: Implement clean motor array abstraction where:
- BoardConfig defines motor array with hardware timer assignments
- Sketch has ZERO knowledge of timer banks
- Code generator produces motor array format from Betaflight configs
- MotorManager handles runtime timer grouping automatically

**Scope**:
- A) Motor abstraction model supporting any board configuration
- B) Refactor Betaflight-to-BoardConfig converter for motor array generation
- C) Refactor dRehmFlight_STM32_BETA_1.3 to use new abstraction

---

## Part A: Motor Abstraction Architecture

### Design: Motor Array with Runtime Discovery

**BoardConfig Structure** (generated from .config files):
```cpp
namespace BoardConfig {
  namespace Motor {
    static constexpr uint32_t frequency_hz = 8000;  // 8 kHz for OneShot125

    struct MotorConfig {
      TIM_TypeDef* timer;   // Hardware timer (TIM1, TIM3, TIM5, etc.)
      uint32_t pin;         // Arduino pin with ALT suffix if needed
      uint32_t channel;     // Timer channel (1-4)
      uint32_t min_us;      // Min pulse width
      uint32_t max_us;      // Max pulse width
    };

    // Motor array - index = logical motor number
    static constexpr MotorConfig motors[] = {
      {TIM5, PA0_ALT1, 1, 125, 250},  // Motor 0 (called motor1 in sketch)
      {TIM5, PA1_ALT1, 2, 125, 250},  // Motor 1
      {TIM5, PA2_ALT1, 3, 125, 250},  // Motor 2
      {TIM5, PA3_ALT1, 4, 125, 250},  // Motor 3
      {TIM3, PB1_ALT1, 4, 125, 250},  // Motor 4
      {TIM8, PC8_ALT1, 3, 125, 250},  // Motor 5
    };

    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);
  };
}
```

**MotorManager Class** (in `targets/config/ConfigTypes.h`, NOT sketch directory):
```cpp
class MotorManager {
  // Part of BoardConfig system alongside StorageConfig, IMUConfig, etc.
  // Automatically discovers which timers are used
  // Groups motors by timer at Init() time
  // Provides simple SetMotor(index, pulse_width) API
};
```

**Key Benefits**:
- ✅ Sketch only knows motor indices (0, 1, 2, ...) - NO timer knowledge
- ✅ Works for any board (NERO: TIM5/3/8, Nucleo: TIM1/3, BlackPill: TIM2/3)
- ✅ No sketch changes when adding new boards
- ✅ Auto-generated from Betaflight configs

---

## Part B: Refactor Betaflight-to-BoardConfig Converter

### Current Issues
1. ❌ Generates timer bank namespaces (TIM1_Bank, TIM3_Bank, etc.)
2. ❌ Uses nested `struct Channel` inside each bank
3. ❌ Hardcodes frequency (1000 Hz instead of 8000 Hz for OneShot125)
4. ❌ Doesn't generate RCReceiverConfig (required by dRehmFlight)

### Required Changes to `extras/betaflight_converter/src/code_generator.py`

**File**: `code_generator.py`, function `_generate_motors()` (line 337-384)

**Replace with**:
```python
def _generate_motors(self) -> Optional[str]:
    """Generate Motor namespace with motor array (runtime timer discovery)."""
    motors = self.validator.validate_motors()
    if not motors:
        return None

    # Get protocol
    protocol = self.bf_config.settings.get('motor_pwm_protocol', 'ONESHOT125')

    # CRITICAL FIX: Use 8000 Hz for OneShot125 (not 1000 Hz)
    frequency_hz = 8000 if protocol == 'ONESHOT125' else self._get_protocol_frequency(protocol)
    min_us, max_us = self._get_protocol_pulse_range(protocol)

    lines = [
        f"  // Motors: {protocol} protocol",
        "  namespace Motor {",
        f"    static constexpr uint32_t frequency_hz = {frequency_hz};",
        ""
    ]

    # Add MotorConfig struct definition
    lines.append("    struct MotorConfig {")
    lines.append("      TIM_TypeDef* timer;")
    lines.append("      uint32_t pin;")
    lines.append("      uint32_t channel;")
    lines.append("      uint32_t min_us;")
    lines.append("      uint32_t max_us;")
    lines.append("    };")
    lines.append("")

    # Generate motor array
    lines.append("    // Motor array - hardware timer assignments from Betaflight config")
    lines.append("    static constexpr MotorConfig motors[] = {")

    for motor in sorted(motors, key=lambda m: m.index):
        lines.append(f"      {{{motor.timer}, {motor.pin_arduino}, {motor.channel}, {min_us}, {max_us}}},  // Motor {motor.index}: {motor.timer}_CH{motor.channel}")

    lines.append("    };")
    lines.append("")
    lines.append("    static constexpr int num_motors = sizeof(motors) / sizeof(motors[0]);")
    lines.append("  };")  // End Motor namespace

    return "\n".join(lines)
```

**Add RCReceiverConfig generation** (new function after `_generate_leds()`):
```python
def _generate_rc_receiver(self) -> Optional[str]:
    """Generate RCReceiverConfig for dRehmFlight compatibility."""
    # Use USART1 by default (adjust if needed)
    uart1 = next((u for u in self.validator.validate_uarts() if u.uart_num == 1), None)
    if not uart1:
        return None

    lines = [
        "  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)",
        f"  static constexpr RCReceiverConfig rc_receiver{{{uart1.rx}, {uart1.tx}, 115200, 1000, 300}};",
        ""
    ]
    return "\n".join(lines)
```

Insert call in `generate()` after LEDs, before Motors.

### Files Generated from .config
```
extras/betaflight_converter/data/
├── BKMN-NERO.config       → targets/BKMN-NERO.h
├── JHEF-JHEF411.config    → targets/NUCLEO_F411RE_JHEF411.h
├── MTKS-MATEKH743.config  → targets/MTKS-MATEKH743.h
└── OPEN-REVO.config       → targets/OPEN-REVO.h
```

### Manual Target Files (Not Auto-Generated)
These need manual updates to motor array format:
```
targets/BLACKPILL_F411CE.h         # Development board
targets/NUCLEO_F411RE_LITTLEFS.h   # Test config
targets/NUCLEO_F411RE_SDFS.h       # Test config
```

---

## Part C: Refactor dRehmFlight_STM32_BETA_1.3

### Current Broken Architecture
```cpp
// ❌ Hardcoded timer bank references
PWMOutputBank motors_tim1;
PWMOutputBank motors_tim3;

// ❌ Hardcoded assumptions about which motors on which timer
motors_tim1.Init(BoardConfig::Motor::TIM1_Bank::timer, 8000);
motors_tim1.AttachChannel(BoardConfig::Motor::TIM1_Bank::motor1.ch, m1Pin, 125, 250);
motors_tim3.AttachChannel(BoardConfig::Motor::TIM3_Bank::motor4.ch, m4Pin, 125, 250);

// ❌ Hardcoded timer/channel knowledge
motors_tim1.SetPulseWidth(1, m1_command_PWM);  // Knows motor1 is on TIM1 channel 1
motors_tim3.SetPulseWidth(3, m4_command_PWM);  // Knows motor4 is on TIM3 channel 3
```

### New Clean Architecture

**File Location**: `targets/config/ConfigTypes.h` (add to existing file)

MotorManager will be added to ConfigTypes.h alongside existing config types (StorageConfig, IMUConfig, etc.)

```cpp
// Add to targets/config/ConfigTypes.h after existing config structs

#include <PWMOutputBank.h>  // CRITICAL: Use correct include (not TimerPWM.h)

class MotorManager {
private:
  static constexpr int MAX_TIMERS = 4;
  static constexpr int MAX_MOTORS = 8;

  struct TimerBank {
    TIM_TypeDef* timer;
    PWMOutputBank pwm;
    bool initialized;
  };

  TimerBank banks[MAX_TIMERS];
  int num_banks;

  struct MotorInfo {
    int bank_index;
    uint32_t channel;
  };
  MotorInfo motor_info[MAX_MOTORS];
  int num_motors;

  int FindOrCreateBank(TIM_TypeDef* timer) {
    for (int i = 0; i < num_banks; i++) {
      if (banks[i].timer == timer) return i;
    }
    if (num_banks < MAX_TIMERS) {
      banks[num_banks].timer = timer;
      banks[num_banks].initialized = false;
      return num_banks++;
    }
    return -1;
  }

public:
  MotorManager() : num_banks(0), num_motors(0) {}

  template<typename MotorArray>
  bool Init(const MotorArray& motors, int count, uint32_t frequency_hz) {
    if (count > MAX_MOTORS) return false;
    num_motors = count;

    for (int i = 0; i < count; i++) {
      const auto& motor = motors[i];
      int bank_idx = FindOrCreateBank(motor.timer);
      if (bank_idx < 0) return false;

      if (!banks[bank_idx].initialized) {
        banks[bank_idx].pwm.Init(motor.timer, frequency_hz);
        banks[bank_idx].initialized = true;
      }

      banks[bank_idx].pwm.AttachChannel(motor.channel, motor.pin, motor.min_us, motor.max_us);
      motor_info[i].bank_index = bank_idx;
      motor_info[i].channel = motor.channel;
    }
    return true;
  }

  void SetMotor(int motor_idx, uint32_t pulse_width_us) {
    if (motor_idx < 0 || motor_idx >= num_motors) return;
    banks[motor_info[motor_idx].bank_index].pwm.SetPulseWidth(
      motor_info[motor_idx].channel, pulse_width_us
    );
  }

  void ArmAll(uint32_t min_pulse_us = 125) {
    for (int i = 0; i < num_motors; i++) {
      SetMotor(i, min_pulse_us);
    }
  }

  int GetNumMotors() const { return num_motors; }
};

#endif // MOTOR_MANAGER_H
```

### Sketch Changes

**Includes** (line ~82):
```cpp
#include <IMU.h>
#include <SerialRx.h>
// MotorManager available from ConfigTypes.h (included via BoardConfig header)
#include <ci_log.h>
```

**Pin Declarations** (line ~182): **REMOVE pin declarations**
```cpp
// OLD: Remove these
const int m1Pin = BoardConfig::Motor::TIM1_Bank::motor1.pin;
const int m2Pin = BoardConfig::Motor::TIM1_Bank::motor2.pin;
// ... etc

// NEW: No pin declarations needed - MotorManager handles everything
```

**Motor Objects** (line ~257):
```cpp
// OLD: Remove these
PWMOutputBank motors_tim1;
PWMOutputBank motors_tim3;

// NEW: Single motor manager
MotorManager motors;
```

**setup()** Motor Initialization (line ~312):
```cpp
// OLD: Remove all this
pinMode(m1Pin, OUTPUT);
pinMode(m2Pin, OUTPUT);
motors_tim1.Init(BoardConfig::Motor::TIM1_Bank::timer, 8000);
motors_tim1.AttachChannel(BoardConfig::Motor::TIM1_Bank::motor1.ch, m1Pin, 125, 250);
// ... etc

// NEW: One-line initialization
if (!motors.Init(BoardConfig::Motor::motors, BoardConfig::Motor::num_motors, BoardConfig::Motor::frequency_hz)) {
  CI_LOG("ERROR: Motor initialization failed!\n");
  while(1);
}
```

**commandMotors()** (line ~1206):
```cpp
// OLD: Remove this
motors_tim1.SetPulseWidth(1, m1_command_PWM);
motors_tim1.SetPulseWidth(2, m2_command_PWM);
motors_tim3.SetPulseWidth(3, m4_command_PWM);

// NEW: Simple index-based API (0-based indexing)
motors.SetMotor(0, m1_command_PWM);  // Motor 0 (sketch calls it motor1)
motors.SetMotor(1, m2_command_PWM);  // Motor 1 (sketch calls it motor2)
motors.SetMotor(2, m3_command_PWM);
motors.SetMotor(3, m4_command_PWM);
motors.SetMotor(4, m5_command_PWM);
if (BoardConfig::Motor::num_motors >= 6) motors.SetMotor(5, m6_command_PWM);
```

---

## Implementation Steps

### Phase 1: Clean Repository
```bash
# Navigate to root
cd /home/geo/Arduino/Arduino_Core_STM32

# CRITICAL: Remove all temporary/broken files
rm sketches/dRehmFlight_STM32_BETA_1.3/motor_*.cpp
rm sketches/dRehmFlight_STM32_BETA_1.3/MotorManager.h
rm sketches/dRehmFlight_STM32_BETA_1.3/MOTOR_ABSTRACTION_OPTIONS.md

# Reset modified files to clean state
git checkout targets/BKMN-NERO.h
git checkout targets/BLACKPILL_F411CE.h
git checkout targets/NUCLEO_F411RE_JHEF411.h
git checkout sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino

# Verify clean state
git status
```

### Phase 2: Refactor Code Generator
```bash
# Edit code_generator.py _generate_motors() function
# Add _generate_rc_receiver() function
# Fix frequency_hz calculation (8000 Hz for OneShot125)

# Test regeneration
cd extras/betaflight_converter
python3 convert.py data/BKMN-NERO.config
cat output/BKMN-NERO.h  # Verify motor array format
```

### Phase 3: Regenerate All Auto-Generated Targets
```bash
cd extras/betaflight_converter

# Regenerate from .config files
python3 convert.py data/BKMN-NERO.config
python3 convert.py data/JHEF-JHEF411.config
python3 convert.py data/MTKS-MATEKH743.config
python3 convert.py data/OPEN-REVO.config

# Copy to targets directory
cp output/BKMN-NERO.h ../../targets/
cp output/JHEF-JHEF411.h ../../targets/NUCLEO_F411RE_JHEF411.h
cp output/MTKS-MATEKH743.h ../../targets/
cp output/OPEN-REVO.h ../../targets/
```

### Phase 4: Manually Update Non-Generated Targets
Update these files to motor array format:
- `targets/BLACKPILL_F411CE.h`
- `targets/NUCLEO_F411RE_LITTLEFS.h` (if has Motor section)
- `targets/NUCLEO_F411RE_SDFS.h` (if has Motor section)

### Phase 5: Add MotorManager to ConfigTypes.h
Add MotorManager class to `targets/config/ConfigTypes.h` (after existing config structs).

### Phase 6: Refactor Sketch
Edit `sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino`:
1. Remove MotorManager.h include (already available via ConfigTypes.h)
2. Remove pin declarations
3. Replace PWMOutputBank with MotorManager
4. Update setup() motor init
5. Update commandMotors()

### Phase 7: Build and Verify
```bash
cd /home/geo/Arduino/Arduino_Core_STM32

# Build for all three boards
./system/ci/build.sh sketches/dRehmFlight_STM32_BETA_1.3 --build-id  # NUCLEO_F411RE
./system/ci/build.sh sketches/dRehmFlight_STM32_BETA_1.3 STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE --build-id
./system/ci/build.sh sketches/dRehmFlight_STM32_BETA_1.3 STMicroelectronics:stm32:FlightCtr:pnum=BKMN_NERO --build-id

# All should compile successfully with no timer bank references
```

---

## Validation Criteria

### Code Generator Validation
- ✅ Generates `struct MotorConfig` with timer/pin/channel/min/max fields
- ✅ Generates motor array `motors[]` indexed by motor number
- ✅ Generates `num_motors` constant
- ✅ NO timer bank namespaces (TIM1_Bank, TIM3_Bank, etc.)
- ✅ Frequency is 8000 Hz for OneShot125 (not 1000 Hz)
- ✅ Generates RCReceiverConfig

### Sketch Validation
- ✅ NO includes of `<PWMOutputBank.h>` directly
- ✅ NO includes of `"MotorManager.h"` (available via ConfigTypes.h)
- ✅ NO PWMOutputBank object declarations
- ✅ NO timer bank references (TIM1_Bank, TIM3_Bank, etc.)
- ✅ Uses `motors.Init()` with BoardConfig array
- ✅ Uses `motors.SetMotor(index, pwm)` API

### Build Validation
- ✅ NUCLEO_F411RE compiles (5 motors: TIM1×3 + TIM3×2)
- ✅ BLACKPILL_F411CE compiles (5 motors: TIM2×3 + TIM3×2)
- ✅ BKMN_NERO compiles (6+ motors: TIM5×4 + TIM3×1 + TIM8×2)
- ✅ Binary sizes reasonable (~47-50KB)

---

## Critical Success Factors

### 1. MotorManager Location
**CRITICAL**: MotorManager class must be:
- Added to `targets/config/ConfigTypes.h` (NOT sketch directory)
- Part of BoardConfig system alongside StorageConfig, IMUConfig, etc.
- Automatically available when BoardConfig header is included
- Use `#include <PWMOutputBank.h>` internally (not `<TimerPWM.h>`)

### 2. Code Generator Accuracy
**CRITICAL**: Must use correct ValidatedMotor field names:
- `motor.timer` (NOT `motor.timer_name`)
- `motor.pin_arduino`
- `motor.channel`

### 3. Frequency Configuration
**CRITICAL**: OneShot125 requires 8000 Hz (not 1000 Hz from Betaflight config)

### 4. Zero Sketch Knowledge of Timers
**CRITICAL**: Sketch must NOT:
- Reference timer bank namespaces
- Know which motor is on which timer
- Hardcode timer/channel associations

---

## Files to Modify

### BoardConfig System
- `targets/config/ConfigTypes.h` (ADD MotorManager class)

### Code Generator
- `extras/betaflight_converter/src/code_generator.py` (lines 337-410)

### Auto-Generated Targets (regenerate from .config)
- `targets/BKMN-NERO.h`
- `targets/NUCLEO_F411RE_JHEF411.h`
- `targets/MTKS-MATEKH743.h`
- `targets/OPEN-REVO.h`

### Manual Targets (hand-edit)
- `targets/BLACKPILL_F411CE.h`

### Sketch Files
- `sketches/dRehmFlight_STM32_BETA_1.3/dRehmFlight_STM32_BETA_1.3.ino` (REFACTOR)

---

## Benefits

### For dRehmFlight Port
- ✅ Supports NERO F7 (TIM5/TIM3/TIM8) without sketch changes
- ✅ Supports BlackPill (TIM2/TIM3) without sketch changes
- ✅ Supports any future board by just adding BoardConfig
- ✅ Cleaner, more maintainable code

### For Code Generator
- ✅ Simpler output format (array vs nested namespaces)
- ✅ Matches Betaflight's motor index semantics
- ✅ Easier to validate (linear array vs nested structure)

### For Architecture
- ✅ True abstraction - sketch agnostic to hardware
- ✅ Runtime efficiency (timer grouping optimized once at init)
- ✅ Compile-time validation (motor count, timer assignments)
- ✅ Future-proof (supports new timer configurations)

---

## Post-Implementation Cleanup

After successful build verification:
```bash
# Commit changes
git add targets/config/ConfigTypes.h
git add extras/betaflight_converter/src/code_generator.py
git add targets/*.h
git add sketches/dRehmFlight_STM32_BETA_1.3/

git commit -m "Refactor motor abstraction: array-based config with runtime timer discovery

- Added MotorManager class to BoardConfig system (ConfigTypes.h)
- Code generator produces motor array format (not timer banks)
- MotorManager handles runtime timer grouping
- dRehmFlight sketch now board-agnostic (no timer knowledge)
- Supports NERO F7 (TIM5/3/8), Nucleo (TIM1/3), BlackPill (TIM2/3)
- Regenerated all targets from Betaflight configs"
```
