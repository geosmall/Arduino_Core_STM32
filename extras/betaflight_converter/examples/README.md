# Betaflight BoardConfig Examples

These examples demonstrate how to use generated Betaflight BoardConfig headers in your Arduino sketches.

## Examples

### 1. basic_config_usage

**Purpose**: Learn how to access generated configuration data

Shows how to:
- Read storage, IMU, UART, ADC, and LED configurations
- Print configuration information to Serial
- Initialize status LEDs

**Hardware**: Any board (uses JHEF-JHEF411 config)

**Run this first** to understand the configuration structure.

---

### 2. example-icm42688p-minimal

**Purpose**: Verify IMU configuration from generated BoardConfig

Shows how to:
- Use generated IMU config (SPI pins, CS, frequency)
- Test SPI communication with ICM42688P sensor
- Verify WHO_AM_I device identification
- Validate pin assignments from Betaflight converter

**Hardware**: JHEF411 (NOXE V3) or any board with ICM42688P

**Requires**: ICM42688P library (`libraries/ICM42688P`)

**Expected Output**: 5 continuous WHO_AM_I reads returning 0x47

**Use Case**:
- Validate generated IMU configuration
- Confirm SPI bus and pin assignments
- Test sensor communication before full integration

---

### 3. motor_pwm_verification

**Purpose**: Hardware validation of motor PWM outputs using input capture

Shows how to:
- Verify auto-generated motor timer bank configuration
- Use TIM2 input capture to measure motor frequencies
- Validate timer separation (TIM1 vs TIM3)
- Implement hardware validation without oscilloscope

**Hardware**: JHEF411 (NOXE V3) or compatible F411 board

**Requires**: PWMOutputBank library (`libraries/TimerPWM`)

**Jumper Connections**:
- PA8 → PA0 (Motor1/TIM1 to TIM2_CH1 capture)
- PB0 → PA1 (Motor4/TIM3 to TIM2_CH2 capture)

**Validation Criteria**:
- Motor frequency: 1000 Hz ± 2% (980-1020 Hz)
- Demonstrates timer bank grouping from Betaflight converter
- Follows embedded hardware validation standards (see CLAUDE.md)

---

## Usage Pattern

1. **Generate BoardConfig** from Betaflight target config:
   ```bash
   cd extras/betaflight_converter
   python3 convert.py bf_configs/YOUR-BOARD/
   ```

2. **Include generated config** directly from output directory:
   ```cpp
   // Reference the generated file directly (always up-to-date)
   #include "../../output/JHEF-JHEF411.h"
   ```

   Or copy to your sketch folder if preferred:
   ```cpp
   #include "JHEF-JHEF411.h"
   ```

3. **Access configuration** via `BoardConfig` namespace:
   ```cpp
   auto pin = BoardConfig::storage.cs_pin;
   auto freq = BoardConfig::Motor::frequency_hz;
   ```

4. **Initialize hardware** using config values:
   ```cpp
   pinMode(BoardConfig::status_leds.led1_pin, OUTPUT);
   ```

## Configuration Structure

Generated configs include:

- **StorageConfig** - SPI flash or SD card
- **IMUConfig** - Gyro/accelerometer with interrupt
- **I2CConfig** - I2C peripherals (baro, mag, etc.)
- **UARTConfig** - Serial ports
- **ADCConfig** - Battery voltage/current monitoring
- **LEDConfig** - Status LEDs (GPIO)
- **Servo** namespace - PWM servo outputs (50 Hz)
- **Motor** namespace - High-speed motor outputs (DSHOT/OneShot)

## See Also

- Main converter README: `../README.md`
- Generated output files: `../output/`
- TimerPWM library: `../../../libraries/TimerPWM/`
