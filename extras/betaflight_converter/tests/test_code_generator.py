"""
Tests for BoardConfig code generator.
"""

import unittest
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
        config_path = Path(__file__).parent.parent / "data/JHEF-JHEF411.config"
        if not config_path.exists():
            raise FileNotFoundError(f"JHEF411 config not found at {config_path}")
        cls.bf_config = BetaflightConfig(config_path)

        # Load PeripheralPins.c
        arduino_root = Path(__file__).parents[3]
        pinmap_path = arduino_root / "variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c"
        if not pinmap_path.exists():
            raise FileNotFoundError(f"PeripheralPins.c not found at {pinmap_path}")
        cls.pinmap = PeripheralPinMap(pinmap_path)

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

        # Should include ConfigTypes (with relative path)
        self.assertIn('#include "../../../../targets/config/ConfigTypes.h"', code)

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

        # Should have CS pin (PB02)
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

        # Should have CS pin (PA04)
        self.assertIn("PA4", code)

        # Should have interrupt pin (PB03)
        self.assertIn("PB3", code)

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

        # Should have timer banks
        self.assertIn("TIM1_Bank", code)
        self.assertIn("TIM3_Bank", code)

        # Should have motor definitions
        self.assertIn("motor1", code)
        self.assertIn("motor2", code)
        self.assertIn("motor3", code)
        self.assertIn("motor4", code)
        self.assertIn("motor5", code)

        # Should have correct timer references
        self.assertIn("TIM1", code)
        self.assertIn("TIM3", code)

    def test_motor_timer_grouping(self):
        """Test motors are grouped by timer correctly."""
        code = self.generator.generate()

        # TIM1 should have motors 1-3
        tim1_section = re.search(r'namespace TIM1_Bank \{(.+?)\n    \};', code, re.DOTALL)
        self.assertIsNotNone(tim1_section)
        tim1_code = tim1_section.group(1)

        self.assertIn("motor1", tim1_code)
        self.assertIn("motor2", tim1_code)
        self.assertIn("motor3", tim1_code)

        # TIM3 should have motors 4-5
        tim3_section = re.search(r'namespace TIM3_Bank \{(.+?)\n    \};', code, re.DOTALL)
        self.assertIsNotNone(tim3_section)
        tim3_code = tim3_section.group(1)

        self.assertIn("motor4", tim3_code)
        self.assertIn("motor5", tim3_code)

    def test_protocol_detection(self):
        """Test motor protocol detection."""
        code = self.generator.generate()

        # JHEF411 uses DSHOT300 (should fallback to 1kHz)
        self.assertIn("frequency_hz = 1000", code)

    def test_valid_cpp_syntax(self):
        """Test generated code has valid C++ syntax."""
        code = self.generator.generate()

        # Should have matching braces
        self.assertEqual(code.count('{'), code.count('}'))

        # Should end with closing brace
        self.assertTrue(code.strip().endswith('}'))

        # Should have proper semicolons after config lines
        config_lines = [line for line in code.split('\n') if 'Config' in line and '=' in line]
        for line in config_lines:
            if not line.strip().startswith('//'):
                self.assertTrue(line.strip().endswith(';'), f"Missing semicolon: {line}")

    def test_save_to_file(self):
        """Test saving generated code to file."""
        output_path = Path("/tmp/test_generated_NOXE_V3.h")

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
