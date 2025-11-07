"""
Tests for configuration validator.
"""

import unittest
from pathlib import Path
import sys

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'src'))

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap
from validator import ConfigValidator, ValidatedMotor, ValidatedSPIBus, ValidatedI2CBus, ValidatedUART


class TestConfigValidator(unittest.TestCase):
    """Test configuration validator with real JHEF411 data."""

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

    def test_validate_all(self):
        """Test complete validation runs without errors."""
        result = self.validator.validate_all()
        if not result:
            print(self.validator.get_validation_summary())
        self.assertTrue(result, "Validation should pass for JHEF411")

    def test_validate_motors(self):
        """Test motor validation."""
        motors = self.validator.validate_motors()

        # Should have 5 motors
        self.assertEqual(len(motors), 5)

        # Verify motor 1: PA8 = TIM1_CH1 on AF1
        motor1 = next((m for m in motors if m.index == 1), None)
        self.assertIsNotNone(motor1)
        self.assertEqual(motor1.pin_bf, 'A08')
        self.assertEqual(motor1.pin_arduino, 'PA8')
        self.assertEqual(motor1.timer, 'TIM1')
        self.assertEqual(motor1.channel, 1)
        self.assertEqual(motor1.af, 1)

        # Verify motor 5: PB4 = TIM3_CH1 on AF2
        motor5 = next((m for m in motors if m.index == 5), None)
        self.assertIsNotNone(motor5)
        self.assertEqual(motor5.pin_bf, 'B04')
        self.assertEqual(motor5.pin_arduino, 'PB4')
        self.assertEqual(motor5.timer, 'TIM3')
        self.assertEqual(motor5.channel, 1)
        self.assertEqual(motor5.af, 2)

    def test_group_motors_by_timer(self):
        """Test motor grouping by timer."""
        motors = self.validator.validate_motors()
        timer_banks = self.validator.group_motors_by_timer(motors)

        # Should have 2 timer banks: TIM1 and TIM3
        self.assertEqual(len(timer_banks), 2)
        self.assertIn('TIM1', timer_banks)
        self.assertIn('TIM3', timer_banks)

        # TIM1 should have 3 motors (1-3)
        self.assertEqual(len(timer_banks['TIM1']), 3)
        tim1_indices = [m.index for m in timer_banks['TIM1']]
        self.assertEqual(sorted(tim1_indices), [1, 2, 3])

        # TIM3 should have 2 motors (4-5)
        self.assertEqual(len(timer_banks['TIM3']), 2)
        tim3_indices = [m.index for m in timer_banks['TIM3']]
        self.assertEqual(sorted(tim3_indices), [4, 5])

    def test_validate_spi_buses(self):
        """Test SPI bus validation."""
        spi_buses = self.validator.validate_spi_buses()

        # Should have 2 SPI buses
        self.assertEqual(len(spi_buses), 2)

        # Find SPI1 (IMU)
        spi1 = next((bus for bus in spi_buses if bus.bus_num == 1), None)
        self.assertIsNotNone(spi1)
        self.assertEqual(spi1.bus_name, 'SPI1')
        self.assertEqual(spi1.mosi, 'PA7')
        self.assertEqual(spi1.miso, 'PA6')
        self.assertEqual(spi1.sclk, 'PA5')

        # Find SPI2 (Flash/OSD)
        spi2 = next((bus for bus in spi_buses if bus.bus_num == 2), None)
        self.assertIsNotNone(spi2)
        self.assertEqual(spi2.bus_name, 'SPI2')
        self.assertEqual(spi2.mosi, 'PB15')
        self.assertEqual(spi2.miso, 'PB14')
        self.assertEqual(spi2.sclk, 'PB13')

    def test_validate_i2c_buses(self):
        """Test I2C bus validation."""
        i2c_buses = self.validator.validate_i2c_buses()

        # Should have 1 I2C bus
        self.assertEqual(len(i2c_buses), 1)

        i2c1 = i2c_buses[0]
        self.assertEqual(i2c1.bus_num, 1)
        self.assertEqual(i2c1.bus_name, 'I2C1')
        self.assertEqual(i2c1.scl, 'PB8')
        self.assertEqual(i2c1.sda, 'PB9')

    def test_validate_uarts(self):
        """Test UART validation."""
        uarts = self.validator.validate_uarts()

        # Should have 2 UARTs
        self.assertEqual(len(uarts), 2)

        # Find UART1
        uart1 = next((u for u in uarts if u.uart_num == 1), None)
        self.assertIsNotNone(uart1)
        self.assertEqual(uart1.uart_name, 'USART1')
        self.assertEqual(uart1.tx, 'PB6')
        self.assertEqual(uart1.rx, 'PB7')

        # Find UART2
        uart2 = next((u for u in uarts if u.uart_num == 2), None)
        self.assertIsNotNone(uart2)
        self.assertEqual(uart2.uart_name, 'USART2')
        self.assertEqual(uart2.tx, 'PA2')
        self.assertEqual(uart2.rx, 'PA3')

    def test_no_errors_for_jhef411(self):
        """Test that JHEF411 config validates without errors."""
        self.validator.validate_all()
        self.assertEqual(len(self.validator.errors), 0,
                        f"Expected no errors, got: {[e.message for e in self.validator.errors]}")

    def test_validation_summary(self):
        """Test validation summary generation."""
        self.validator.validate_all()
        summary = self.validator.get_validation_summary()

        self.assertIn("Validation Summary", summary)
        self.assertIn("Errors: 0", summary)


if __name__ == '__main__':
    unittest.main()
