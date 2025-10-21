"""
Tests for Betaflight config parser.
"""

import unittest
from pathlib import Path
import sys

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'src'))

from betaflight_config import BetaflightConfig, ResourcePin, TimerAssignment


class TestBetaflightConfig(unittest.TestCase):
    """Test Betaflight config parser with real JHEF411 data."""

    @classmethod
    def setUpClass(cls):
        """Load real JHEF411 config file."""
        config_path = Path(__file__).parent.parent / "data/JHEF-JHEF411.config"

        if not config_path.exists():
            raise FileNotFoundError(f"JHEF411 config not found at {config_path}")

        cls.config = BetaflightConfig(config_path)

    def test_header_parsing(self):
        """Test MCU type parsing from header."""
        self.assertEqual(self.config.mcu_type, "STM32F411")

    def test_board_info_parsing(self):
        """Test board name and manufacturer parsing."""
        self.assertEqual(self.config.board_name, "JHEF411")
        self.assertEqual(self.config.manufacturer_id, "JHEF")

    def test_define_parsing(self):
        """Test #define parsing."""
        self.assertGreater(len(self.config.defines), 0)
        self.assertTrue(self.config.has_define("USE_GYRO_SPI_ICM42688P"))
        self.assertTrue(self.config.has_define("USE_FLASH_W25Q128FV"))
        self.assertTrue(self.config.has_define("USE_MAX7456"))

    def test_gyro_chip_detection(self):
        """Test gyro chip detection from #defines."""
        chips = self.config.get_gyro_chips()
        self.assertIn("MPU6000", chips)
        self.assertIn("ICM42688P", chips)

    def test_motor_resources(self):
        """Test motor resource parsing."""
        motors = self.config.get_motors()
        self.assertEqual(len(motors), 5)

        # Verify motor 1: PA08
        motor1 = next((m for m in motors if m.index == 1), None)
        self.assertIsNotNone(motor1)
        self.assertEqual(motor1.pin, "A08")

        # Verify motor 5: PB04
        motor5 = next((m for m in motors if m.index == 5), None)
        self.assertIsNotNone(motor5)
        self.assertEqual(motor5.pin, "B04")

    def test_spi_resources(self):
        """Test SPI resource parsing."""
        # SPI1 (IMU)
        spi1_pins = self.config.get_spi_pins(1)
        self.assertIsNotNone(spi1_pins)
        self.assertEqual(spi1_pins['MOSI'], 'A07')
        self.assertEqual(spi1_pins['MISO'], 'A06')
        self.assertEqual(spi1_pins['SCLK'], 'A05')

        # SPI2 (Flash/OSD)
        spi2_pins = self.config.get_spi_pins(2)
        self.assertIsNotNone(spi2_pins)
        self.assertEqual(spi2_pins['MOSI'], 'B15')
        self.assertEqual(spi2_pins['MISO'], 'B14')
        self.assertEqual(spi2_pins['SCLK'], 'B13')

    def test_i2c_resources(self):
        """Test I2C resource parsing."""
        # I2C1 (Sensors)
        i2c1_pins = self.config.get_i2c_pins(1)
        self.assertIsNotNone(i2c1_pins)
        self.assertEqual(i2c1_pins['SCL'], 'B08')
        self.assertEqual(i2c1_pins['SDA'], 'B09')

    def test_uart_resources(self):
        """Test UART resource parsing."""
        # UART1
        uart1_pins = self.config.get_uart_pins(1)
        self.assertIsNotNone(uart1_pins)
        self.assertEqual(uart1_pins['TX'], 'B06')
        self.assertEqual(uart1_pins['RX'], 'B07')

        # UART2
        uart2_pins = self.config.get_uart_pins(2)
        self.assertIsNotNone(uart2_pins)
        self.assertEqual(uart2_pins['TX'], 'A02')
        self.assertEqual(uart2_pins['RX'], 'A03')

    def test_timer_parsing(self):
        """Test timer assignment parsing."""
        self.assertGreater(len(self.config.timers), 0)

        # Verify motor 1: PA08 = TIM1_CH1 on AF1
        self.assertIn('A08', self.config.timers)
        timer_a08 = self.config.timers['A08']
        self.assertEqual(timer_a08.af, 1)
        self.assertEqual(timer_a08.timer, 'TIM1')
        self.assertEqual(timer_a08.channel, 1)

        # Verify motor 5: PB04 = TIM3_CH1 on AF2
        self.assertIn('B04', self.config.timers)
        timer_b04 = self.config.timers['B04']
        self.assertEqual(timer_b04.af, 2)
        self.assertEqual(timer_b04.timer, 'TIM3')
        self.assertEqual(timer_b04.channel, 1)

    def test_feature_parsing(self):
        """Test feature flag parsing."""
        self.assertIn('OSD', self.config.features)
        self.assertIn('RX_SERIAL', self.config.features)

    def test_setting_parsing(self):
        """Test setting parsing."""
        self.assertGreater(len(self.config.settings), 0)

        # Verify key settings
        self.assertEqual(self.config.settings['gyro_1_spibus'], '1')
        self.assertEqual(self.config.settings['gyro_1_sensor_align'], 'CW180')
        self.assertEqual(self.config.settings['blackbox_device'], 'SPIFLASH')
        self.assertEqual(self.config.settings['flash_spi_bus'], '2')
        self.assertEqual(self.config.settings['motor_pwm_protocol'], 'DSHOT300')

    def test_pin_format_conversion(self):
        """Test Betaflight to Arduino pin format conversion."""
        self.assertEqual(self.config.convert_pin_format('B04'), 'PB4')
        self.assertEqual(self.config.convert_pin_format('A08'), 'PA8')
        self.assertEqual(self.config.convert_pin_format('A10'), 'PA10')
        self.assertEqual(self.config.convert_pin_format('B15'), 'PB15')

    def test_storage_cs_pins(self):
        """Test storage chip select pin parsing."""
        # FLASH_CS
        flash_cs = self.config.get_resources('FLASH_CS')
        self.assertEqual(len(flash_cs), 1)
        self.assertEqual(flash_cs[0].pin, 'B02')

        # OSD_CS
        osd_cs = self.config.get_resources('OSD_CS')
        self.assertEqual(len(osd_cs), 1)
        self.assertEqual(osd_cs[0].pin, 'B12')

    def test_gyro_resources(self):
        """Test gyro resource parsing."""
        # GYRO_CS
        gyro_cs = self.config.get_resources('GYRO_CS')
        self.assertEqual(len(gyro_cs), 1)
        self.assertEqual(gyro_cs[0].pin, 'A04')

        # GYRO_EXTI
        gyro_exti = self.config.get_resources('GYRO_EXTI')
        self.assertEqual(len(gyro_exti), 1)
        self.assertEqual(gyro_exti[0].pin, 'B03')

    def test_adc_resources(self):
        """Test ADC resource parsing."""
        # ADC_BATT
        adc_batt = self.config.get_resources('ADC_BATT')
        self.assertEqual(len(adc_batt), 1)
        self.assertEqual(adc_batt[0].pin, 'A00')

        # ADC_CURR
        adc_curr = self.config.get_resources('ADC_CURR')
        self.assertEqual(len(adc_curr), 1)
        self.assertEqual(adc_curr[0].pin, 'A01')

    def test_led_resources(self):
        """Test LED resource parsing."""
        # LED (status LED)
        led = self.config.get_resources('LED')
        self.assertEqual(len(led), 1)
        self.assertEqual(led[0].pin, 'C13')

        # LED_STRIP
        led_strip = self.config.get_resources('LED_STRIP')
        self.assertEqual(len(led_strip), 1)
        self.assertEqual(led_strip[0].pin, 'A15')


class TestPinConversion(unittest.TestCase):
    """Test pin format conversion edge cases."""

    def setUp(self):
        """Create config instance for conversion testing."""
        # Create empty config just for conversion method
        self.config = BetaflightConfig(Path("nonexistent.config"))

    def test_single_digit_pins(self):
        """Test single-digit pin conversion."""
        self.assertEqual(self.config.convert_pin_format('A00'), 'PA0')
        self.assertEqual(self.config.convert_pin_format('B01'), 'PB1')
        self.assertEqual(self.config.convert_pin_format('C04'), 'PC4')

    def test_double_digit_pins(self):
        """Test double-digit pin conversion."""
        self.assertEqual(self.config.convert_pin_format('A10'), 'PA10')
        self.assertEqual(self.config.convert_pin_format('B15'), 'PB15')
        self.assertEqual(self.config.convert_pin_format('E11'), 'PE11')


if __name__ == '__main__':
    unittest.main()
