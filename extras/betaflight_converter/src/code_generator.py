"""
C++ BoardConfig code generator.
"""

from dataclasses import dataclass
from typing import List, Dict, Optional
from pathlib import Path
import datetime

from betaflight_config import BetaflightConfig
from validator import ConfigValidator, ValidatedMotor


class BoardConfigGenerator:
    """Generates C++ BoardConfig header from validated configuration."""

    def __init__(self, bf_config: BetaflightConfig, validator: ConfigValidator):
        """Initialize generator with validated config."""
        self.bf_config = bf_config
        self.validator = validator

    def generate(self) -> str:
        """Generate complete BoardConfig C++ header."""
        lines = []

        # Header
        lines.append(self._generate_header())

        # Include guard and includes
        lines.append("#pragma once")
        lines.append("")
        lines.append("// Include ConfigTypes.h from targets/config directory")
        lines.append('#include "../../../targets/config/ConfigTypes.h"')
        lines.append("")

        # Comment with source info
        lines.append(self._generate_source_comment())

        # Namespace
        lines.append("namespace BoardConfig {")

        # Storage
        storage_code = self._generate_storage()
        if storage_code:
            lines.append(storage_code)

        # IMU
        imu_code = self._generate_imu()
        if imu_code:
            lines.append(imu_code)

        # I2C
        i2c_code = self._generate_i2c()
        if i2c_code:
            lines.append(i2c_code)

        # UARTs
        uart_code = self._generate_uarts()
        if uart_code:
            lines.append(uart_code)

        # ADC
        adc_code = self._generate_adc()
        if adc_code:
            lines.append(adc_code)

        # LEDs
        led_code = self._generate_leds()
        if led_code:
            lines.append(led_code)

        # RC Receiver
        rc_receiver_code = self._generate_rc_receiver()
        if rc_receiver_code:
            lines.append(rc_receiver_code)

        # Servos
        servo_code = self._generate_servos()
        if servo_code:
            lines.append(servo_code)

        # Motors
        motor_code = self._generate_motors()
        if motor_code:
            lines.append(motor_code)

        lines.append("}")  # End namespace

        return "\n".join(lines)

    def _generate_header(self) -> str:
        """Generate file header comment."""
        return f"""/*
 * Auto-generated BoardConfig from Betaflight target config
 * Generated: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
 * Generator: betaflight_target_converter.py
 */
"""

    def _generate_source_comment(self) -> str:
        """Generate comment with source file info."""
        lines = [
            f"// Board: {self.bf_config.board_name}",
            f"// Manufacturer: {self.bf_config.manufacturer_id}",
            f"// MCU: {self.bf_config.mcu_type}",
        ]

        # Add detected sensors
        gyro_chips = self.bf_config.get_gyro_chips()
        if gyro_chips:
            lines.append(f"// Gyro: {', '.join(gyro_chips)}")

        return "\n".join(lines)

    def _generate_storage(self) -> Optional[str]:
        """Generate StorageConfig."""
        # Check for flash or SD card
        flash_cs = self.bf_config.get_resources('FLASH_CS')
        sdcard_cs = self.bf_config.get_resources('SDCARD_CS')

        if not flash_cs and not sdcard_cs:
            return None

        # Get SPI bus for storage
        blackbox_device = self.bf_config.settings.get('blackbox_device')

        if flash_cs and (blackbox_device == 'SPIFLASH' or not sdcard_cs):
            # Use SPI flash
            cs_pin = self.bf_config.convert_pin_format(flash_cs[0].pin)
            backend = "StorageBackend::LITTLEFS"
            comment = "W25Q128FV SPI flash"

            # Get SPI bus number
            flash_spi_bus = int(self.bf_config.settings.get('flash_spi_bus', '2'))
            spi_buses = self.validator.validate_spi_buses()
            spi_bus = next((b for b in spi_buses if b.bus_num == flash_spi_bus), None)

            if not spi_bus:
                return f"  // ERROR: Could not find SPI{flash_spi_bus} for flash"

        elif sdcard_cs:
            # Use SD card
            cs_pin = self.bf_config.convert_pin_format(sdcard_cs[0].pin)
            backend = "StorageBackend::SDFS"
            comment = "SD card"

            # Get SPI bus number
            sdcard_spi_bus = int(self.bf_config.settings.get('sdcard_spi_bus', '3'))
            spi_buses = self.validator.validate_spi_buses()
            spi_bus = next((b for b in spi_buses if b.bus_num == sdcard_spi_bus), None)

            if not spi_bus:
                return f"  // ERROR: Could not find SPI{sdcard_spi_bus} for SD card"
        else:
            return None

        lines = [
            f"  // Storage: {comment} on {spi_bus.bus_name}",
            f"  static constexpr StorageConfig storage{{{backend}, {spi_bus.mosi}, {spi_bus.miso}, {spi_bus.sclk}, {cs_pin}, 8000000}};",
            ""
        ]

        return "\n".join(lines)

    def _generate_imu(self) -> Optional[str]:
        """Generate IMUConfig."""
        gyro_cs = self.bf_config.get_resources('GYRO_CS')
        if not gyro_cs:
            return None

        cs_pin = self.bf_config.convert_pin_format(gyro_cs[0].pin)

        # Get interrupt pin
        gyro_exti = self.bf_config.get_resources('GYRO_EXTI')
        int_pin = self.bf_config.convert_pin_format(gyro_exti[0].pin) if gyro_exti else "0"

        # Get SPI bus
        gyro_spi_bus = int(self.bf_config.settings.get('gyro_1_spibus', '1'))
        spi_buses = self.validator.validate_spi_buses()
        spi_bus = next((b for b in spi_buses if b.bus_num == gyro_spi_bus), None)

        if not spi_bus:
            return f"  // ERROR: Could not find SPI{gyro_spi_bus} for IMU"

        # Get sensor chips
        chips = self.bf_config.get_gyro_chips()
        chip_comment = ", ".join(chips) if chips else "IMU"

        lines = [
            f"  // IMU: {chip_comment} on {spi_bus.bus_name}",
            f"  static constexpr SPIConfig imu_spi{{{spi_bus.mosi}, {spi_bus.miso}, {spi_bus.sclk}, {cs_pin}, 8000000}};",
            f"  static constexpr IMUConfig imu{{imu_spi, {int_pin}, 1000000}};",
            ""
        ]

        return "\n".join(lines)

    def _generate_i2c(self) -> Optional[str]:
        """Generate I2CConfig."""
        i2c_buses = self.validator.validate_i2c_buses()
        if not i2c_buses:
            return None

        lines = []

        # Common I2C usage patterns (based on typical flight controller configurations)
        i2c_usage = {
            1: ("Airspeed sensor, external compass", "airspeed"),
            2: ("Barometer, compass", "baro"),
            3: ("External sensors", "i2c3"),
            4: ("External sensors", "i2c4"),
        }

        for bus in i2c_buses:
            usage_desc, var_name = i2c_usage.get(bus.bus_num, ("External sensors", f"i2c{bus.bus_num}"))

            # If only one I2C bus, use generic name
            if len(i2c_buses) == 1:
                var_name = "sensors"
                usage_desc = "Environmental sensors"

            lines.append(f"  // {bus.bus_name}: {usage_desc}")
            lines.append(f"  static constexpr I2CConfig {var_name}{{{bus.sda}, {bus.scl}, 400000}};")
            lines.append("")

        return "\n".join(lines)

    def _generate_uarts(self) -> Optional[str]:
        """Generate UARTConfig."""
        uarts = self.validator.validate_uarts()
        if not uarts:
            return None

        lines = []
        for uart in uarts:
            # Determine usage from serial config
            usage = f"UART{uart.uart_num}"
            lines.append(f"  // {uart.uart_name}: Serial port")
            lines.append(f"  static constexpr UARTConfig uart{uart.uart_num}{{{uart.tx}, {uart.rx}, 115200}};")
            lines.append("")

        return "\n".join(lines)

    def _generate_adc(self) -> Optional[str]:
        """Generate ADC config for battery monitoring."""
        adc_batt = self.bf_config.get_resources('ADC_BATT')
        adc_curr = self.bf_config.get_resources('ADC_CURR')

        if not adc_batt and not adc_curr:
            return None

        vbat_pin = self.bf_config.convert_pin_format(adc_batt[0].pin) if adc_batt else "0"
        curr_pin = self.bf_config.convert_pin_format(adc_curr[0].pin) if adc_curr else "0"

        # Get scales from settings
        vbat_scale = self.bf_config.settings.get('vbat_scale', '110')
        ibata_scale = self.bf_config.settings.get('ibata_scale', '170')

        lines = [
            "  // ADC: Battery voltage and current monitoring",
            f"  static constexpr ADCConfig battery{{{vbat_pin}, {curr_pin}, {vbat_scale}, {ibata_scale}}};",
            ""
        ]

        return "\n".join(lines)

    def _generate_leds(self) -> Optional[str]:
        """Generate LEDConfig for status LEDs."""
        led_resources = self.bf_config.get_resources('LED')

        if not led_resources:
            return None

        # Get up to 2 LED pins
        led_pins = []
        for led in sorted(led_resources, key=lambda r: r.index)[:2]:
            pin = self.bf_config.convert_pin_format(led.pin)
            led_pins.append(pin)

        # Construct LEDConfig
        if len(led_pins) == 1:
            config_line = f"  static constexpr LEDConfig status_leds{{{led_pins[0]}}};"
        else:
            config_line = f"  static constexpr LEDConfig status_leds{{{led_pins[0]}, {led_pins[1]}}};"

        lines = [
            "  // Status LEDs",
            config_line,
            ""
        ]

        return "\n".join(lines)

    def _generate_rc_receiver(self) -> Optional[str]:
        """Generate RCReceiverConfig for dRehmFlight compatibility."""
        # Use USART1 by default (adjust if needed)
        uarts = self.validator.validate_uarts()
        uart1 = next((u for u in uarts if u.uart_num == 1), None)
        if not uart1:
            return None

        lines = [
            "  // RC Receiver: IBus/SBUS (adjust protocol based on actual wiring)",
            f"  static constexpr RCReceiverConfig rc_receiver{{{uart1.rx}, {uart1.tx}, 115200, 1000, 300}};",
            ""
        ]
        return "\n".join(lines)

    def _generate_servos(self) -> Optional[str]:
        """Generate Servo namespace with servo array (ServoManager compatible).

        Always generates the namespace even if no servos configured, so sketches
        can check BoardConfig::Servo::num_servos at compile time.
        """
        servos = self.validator.validate_servos()

        if not servos:
            # Generate empty Servo namespace for targets without servos
            lines = [
                "  // Servo outputs - none configured",
                "  namespace Servo {",
                "    static constexpr uint32_t frequency_hz = 50;",
                "",
                "    struct ServoConfig {",
                "      TIM_TypeDef* timer;",
                "      uint32_t pin;",
                "      uint32_t channel;",
                "      uint32_t min_us;",
                "      uint32_t max_us;",
                "    };",
                "",
                "    static constexpr ServoConfig servos[] = {};",
                "    static constexpr int num_servos = 0;",
                "  };",
            ]
            return "\n".join(lines)

        # Servos use standard PWM (50 Hz, 1000-2000 µs)
        frequency_hz = 50
        min_us, max_us = 1000, 2000

        lines = [
            "  // Servo outputs - 50 Hz PWM for standard servos",
            "  namespace Servo {",
            f"    static constexpr uint32_t frequency_hz = {frequency_hz};",
            ""
        ]

        # Add ServoConfig struct definition
        lines.append("    struct ServoConfig {")
        lines.append("      TIM_TypeDef* timer;")
        lines.append("      uint32_t pin;")
        lines.append("      uint32_t channel;")
        lines.append("      uint32_t min_us;")
        lines.append("      uint32_t max_us;")
        lines.append("    };")
        lines.append("")

        # Generate servo array
        lines.append("    static constexpr ServoConfig servos[] = {")

        for servo in sorted(servos, key=lambda s: s.index):
            lines.append(f"      {{{servo.timer}, {servo.pin_arduino}, {servo.channel}, {min_us}, {max_us}}},  // Servo {servo.index}: {servo.timer}_CH{servo.channel}")

        lines.append("    };")
        lines.append("")
        lines.append("    static constexpr int num_servos = sizeof(servos) / sizeof(servos[0]);")
        lines.append("  };")  # End Servo namespace

        return "\n".join(lines)

    def _generate_motors(self) -> Optional[str]:
        """Generate Motor namespace with motor array (runtime timer discovery)."""
        motors = self.validator.validate_motors()
        if not motors:
            return None

        # Get protocol from config, but always use OneShot125 (DSHOT not implemented)
        bf_protocol = self.bf_config.settings.get('motor_pwm_protocol', 'ONESHOT125')

        # Force OneShot125 - DSHOT is not implemented in this core
        # 2 kHz is a practical rate that works with most ESCs (max theoretical is 4 kHz)
        protocol = 'ONESHOT125'
        frequency_hz = 2000
        min_us, max_us = (125, 250)

        lines = [
            f"  // Motors: {protocol} protocol (125-250 µs)",
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
        lines.append("  };")  # End Motor namespace

        return "\n".join(lines)

    def _get_protocol_frequency(self, protocol: str) -> int:
        """Get PWM frequency for protocol."""
        protocol_map = {
            'PWM': 50,
            'ONESHOT125': 1000,
            'ONESHOT42': 2000,
            'MULTISHOT': 8000,
            'DSHOT150': 1000,  # Placeholder
            'DSHOT300': 1000,  # Placeholder
            'DSHOT600': 1000,  # Placeholder
        }
        return protocol_map.get(protocol, 1000)

    def _get_protocol_pulse_range(self, protocol: str) -> tuple:
        """Get min/max pulse widths for protocol."""
        protocol_map = {
            'PWM': (1000, 2000),
            'ONESHOT125': (125, 250),
            'ONESHOT42': (42, 84),
            'MULTISHOT': (5, 25),
            'DSHOT150': (0, 0),  # Digital
            'DSHOT300': (0, 0),  # Digital
            'DSHOT600': (0, 0),  # Digital
        }
        return protocol_map.get(protocol, (125, 250))

    def save(self, output_path: Path):
        """Generate and save to file."""
        code = self.generate()
        with open(output_path, 'w') as f:
            f.write(code)
