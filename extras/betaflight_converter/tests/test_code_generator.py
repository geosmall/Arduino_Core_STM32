"""
Tests for BoardConfig code generator.
"""

import unittest
import tempfile
from pathlib import Path
import sys
import re

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'src'))

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap
from validator import ConfigValidator
from code_generator import BoardConfigGenerator


class TestCodeGenerator(unittest.TestCase):
    """Test code generator with real JHEF411 data."""

    @classmethod
    def setUpClass(cls):
        """Load JHEF411 config and F411CE pinmap."""
        # Load Betaflight config
        config_path = Path(__file__).parent.parent / "bf_configs/JHEF411"
        if not (config_path / 'config.h').exists():
            raise FileNotFoundError(f"JHEF411 config not found at {config_path}")
        cls.bf_config = BetaflightConfig(config_path)

        # Load PeripheralPins.c
        arduino_root = Path(__file__).parents[3]
        pinmap_path = arduino_root / "variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c"
        if not pinmap_path.exists():
            raise FileNotFoundError(f"PeripheralPins.c not found at {pinmap_path}")
        cls.pinmap = PeripheralPinMap(pinmap_path)

        # Resolve timers before validation
        cls.bf_config.resolve_timers(cls.pinmap)

        # Create validator
        cls.validator = ConfigValidator(cls.bf_config, cls.pinmap)
        cls.validator.validate_all()

        # Create generator
        cls.generator = BoardConfigGenerator(cls.bf_config, cls.validator)

    def test_generate_complete(self):
        """Test that generator produces valid C++ code."""
        code = self.generator.generate()

        # Should not be empty
        self.assertGreater(len(code), 100)

        # Should have header guard
        self.assertIn("#pragma once", code)

        # Should include ConfigTypes (resolved via targets/ on include path)
        self.assertIn('#include "config/ConfigTypes.h"', code)

        # Should have namespace
        self.assertIn("namespace BoardConfig {", code)

    def test_generate_header(self):
        """Test header generation."""
        code = self.generator.generate()

        # Should have auto-generated comment
        self.assertIn("Auto-generated", code)
        self.assertIn("BoardConfig", code)

        # Should have board info
        self.assertIn("Board: JHEF411", code)
        self.assertIn("Manufacturer: JHEF", code)
        self.assertIn("MCU: STM32F411", code)

    def test_generate_storage(self):
        """Test storage config generation."""
        code = self.generator.generate()

        # Should have storage config
        self.assertIn("StorageConfig storage", code)
        self.assertIn("StorageBackend::LITTLEFS", code)

        # Should have SPI2 pins (PB15, PB14, PB13)
        self.assertIn("PB15", code)
        self.assertIn("PB14", code)
        self.assertIn("PB13", code)

        # Should have CS pin (PB2)
        self.assertIn("PB2", code)

    def test_generate_imu(self):
        """Test IMU config generation."""
        code = self.generator.generate()

        # Should have IMU config
        self.assertIn("IMUConfig imu", code)
        self.assertIn("SPIConfig imu_spi", code)

        # Should have SPI1 pins (PA7, PA6, PA5)
        self.assertIn("PA7", code)
        self.assertIn("PA6", code)
        self.assertIn("PA5", code)

        # Should have CS pin (PA4)
        self.assertIn("PA4", code)

        # Should have interrupt pin (PB3)
        self.assertIn("PB3", code)

    def test_generate_imu_alignment(self):
        """JHEF411 has GYRO_1_ALIGN CW180_DEG — must appear as 4th IMUConfig arg."""
        code = self.generator.generate()
        self.assertIn("IMUAlignment::CW180_DEG", code)
        # And the 4th-arg position specifically
        match = re.search(
            r"IMUConfig imu\{imu_spi,\s*\w+,\s*\d+,\s*IMUAlignment::(\w+)\}",
            code,
        )
        self.assertIsNotNone(match, "IMUConfig must emit 4 positional args incl. alignment")
        self.assertEqual(match.group(1), "CW180_DEG")

    def test_generate_i2c(self):
        """Test I2C config generation."""
        code = self.generator.generate()

        # Should have I2C config
        self.assertIn("I2CConfig sensors", code)

        # Should have I2C1 pins (PB8, PB9)
        self.assertIn("PB8", code)
        self.assertIn("PB9", code)

    def test_generate_uarts(self):
        """Test UART config generation."""
        code = self.generator.generate()

        # Should have UART configs
        self.assertIn("UARTConfig uart1", code)
        self.assertIn("UARTConfig uart2", code)

        # UART1: PB6/PB7
        self.assertIn("PB6", code)
        self.assertIn("PB7", code)

        # UART2: PA2/PA3
        self.assertIn("PA2", code)
        self.assertIn("PA3", code)

    def test_generate_adc(self):
        """Test ADC config generation."""
        code = self.generator.generate()

        # Should have ADC config
        self.assertIn("ADCConfig battery", code)

        # Should have voltage/current pins
        self.assertIn("PA0", code)  # VBAT
        self.assertIn("PA1", code)  # CURR

    def test_generate_motors(self):
        """Test motor namespace generation."""
        code = self.generator.generate()

        # Should have Motor namespace
        self.assertIn("namespace Motor {", code)
        self.assertIn("frequency_hz", code)

        # Should have motor array declaration (the emitter inlines the struct
        # type from ConfigTypes.h, doesn't redeclare it)
        self.assertIn("MotorConfig motors[]", code)
        self.assertIn("num_motors", code)

        # Should have correct timer references
        self.assertIn("TIM1", code)
        self.assertIn("TIM3", code)

    def test_motor_timer_grouping(self):
        """Test motors are listed in array with correct timer assignments."""
        code = self.generator.generate()

        # Motors should be in array format with explicit timer assignments
        motor_array_section = re.search(r'motors\[\] = \{(.+?)\n    \};', code, re.DOTALL)
        self.assertIsNotNone(motor_array_section)
        motors_code = motor_array_section.group(1)

        # TIM1 motors (motors 1-3 in JHEF411 config)
        self.assertIn("{TIM1, PA8, 1,", motors_code)   # Motor 1
        self.assertIn("{TIM1, PA9, 2,", motors_code)   # Motor 2
        self.assertIn("{TIM1, PA10, 3,", motors_code)  # Motor 3

        # TIM3 motors (motors 4-5 in JHEF411 config)
        self.assertIn("{TIM3, PB0_ALT1, 3,", motors_code)  # Motor 4
        self.assertIn("{TIM3, PB4, 1,", motors_code)       # Motor 5

    def test_protocol_detection(self):
        """Test motor protocol detection."""
        code = self.generator.generate()

        # Always use OneShot125 @ 2kHz (DSHOT not implemented)
        self.assertIn("frequency_hz = 2000", code)
        self.assertIn("ONESHOT125 protocol", code)
        self.assertIn("125, 250", code)  # OneShot125 pulse range

    def test_valid_cpp_syntax(self):
        """Test generated code has valid C++ syntax."""
        code = self.generator.generate()

        # Should have matching braces
        self.assertEqual(code.count('{'), code.count('}'))

        # Should end with closing brace
        self.assertTrue(code.strip().endswith('}'))

        # Should have proper semicolons after config lines (but not array declarations)
        config_lines = [line for line in code.split('\n') if 'Config' in line and '=' in line]
        for line in config_lines:
            # Skip array declarations (motors[]) and comments
            if not line.strip().startswith('//') and 'motors[]' not in line:
                self.assertTrue(line.strip().endswith(';'), f"Missing semicolon: {line}")

    def test_generate_servos_matekh743(self):
        """Test servo array generation with MATEKH743 (has 2 servos)."""
        config_path = Path(__file__).parent.parent / "bf_configs/MATEKH743"
        if not (config_path / 'config.h').exists():
            self.skipTest("MATEKH743 config not found")

        bf_config = BetaflightConfig(config_path)

        # Load H743 pinmap
        arduino_root = Path(__file__).parents[3]
        pinmap_path = arduino_root / "variants/STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)_H750VBT_H753VI(H-T)/PeripheralPins.c"
        if not pinmap_path.exists():
            self.skipTest("H743 PeripheralPins.c not found")

        pinmap = PeripheralPinMap(pinmap_path)
        bf_config.resolve_timers(pinmap)
        validator = ConfigValidator(bf_config, pinmap)
        validator.validate_all()
        generator = BoardConfigGenerator(bf_config, validator)

        code = generator.generate()

        # Should have Servo namespace
        self.assertIn("namespace Servo {", code)
        self.assertIn("frequency_hz = 50", code)

        # Should have servo array structure
        self.assertIn("ServoConfig servos[]", code)
        self.assertIn("servos[]", code)
        self.assertIn("num_servos", code)

    def test_save_to_file(self):
        """Test saving generated code to file."""
        # tempfile.gettempdir() honors $TMPDIR; /tmp is not always writable
        # (e.g. when running under a sandbox that allowlists $TMPDIR only).
        output_path = Path(tempfile.gettempdir()) / "test_generated_JHEF411.h"

        # Generate and save
        self.generator.save(output_path)

        # Verify file exists
        self.assertTrue(output_path.exists())

        # Verify content
        with open(output_path, 'r') as f:
            content = f.read()

        self.assertIn("namespace BoardConfig", content)
        self.assertIn("StorageConfig storage", content)

        # Cleanup
        output_path.unlink()


if __name__ == '__main__':
    unittest.main()
