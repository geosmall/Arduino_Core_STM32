"""
Tests for Betaflight config.h parser.
"""

import unittest
from pathlib import Path
import sys

# Add src to path
sys.path.insert(0, str(Path(__file__).parent.parent / 'src'))

from betaflight_config import BetaflightConfig, ResourcePin, TimerAssignment
from peripheral_pins import PeripheralPinMap


class TestBetaflightConfig(unittest.TestCase):
    """Test Betaflight config.h parser with real JHEF411 data."""

    @classmethod
    def setUpClass(cls):
        """Load real JHEF411 config.h file."""
        config_path = Path(__file__).parent.parent / "bf_configs/JHEF411"

        if not (config_path / 'config.h').exists():
            raise FileNotFoundError(f"JHEF411 config not found at {config_path}")

        cls.config = BetaflightConfig(config_path)

    def test_header_parsing(self):
        """Test MCU type parsing from #define."""
        self.assertEqual(self.config.mcu_type, "STM32F411")

    def test_board_info_parsing(self):
        """Test board name and manufacturer parsing."""
        self.assertEqual(self.config.board_name, "JHEF411")
        self.assertEqual(self.config.manufacturer_id, "JHEF")

    def test_define_parsing(self):
        """Test USE_* #define parsing."""
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

        # Verify motor 1: PA8 (already Arduino format)
        motor1 = next((m for m in motors if m.index == 1), None)
        self.assertIsNotNone(motor1)
        self.assertEqual(motor1.pin, "PA8")

        # Verify motor 5: PB4
        motor5 = next((m for m in motors if m.index == 5), None)
        self.assertIsNotNone(motor5)
        self.assertEqual(motor5.pin, "PB4")

    def test_spi_resources(self):
        """Test SPI resource parsing (SDI→MISO, SDO→MOSI mapping)."""
        # SPI1 (IMU)
        spi1_pins = self.config.get_spi_pins(1)
        self.assertIsNotNone(spi1_pins)
        self.assertEqual(spi1_pins['MOSI'], 'PA7')   # SDO_PIN → MOSI
        self.assertEqual(spi1_pins['MISO'], 'PA6')   # SDI_PIN → MISO
        self.assertEqual(spi1_pins['SCLK'], 'PA5')

        # SPI2 (Flash/OSD)
        spi2_pins = self.config.get_spi_pins(2)
        self.assertIsNotNone(spi2_pins)
        self.assertEqual(spi2_pins['MOSI'], 'PB15')
        self.assertEqual(spi2_pins['MISO'], 'PB14')
        self.assertEqual(spi2_pins['SCLK'], 'PB13')

    def test_i2c_resources(self):
        """Test I2C resource parsing."""
        i2c1_pins = self.config.get_i2c_pins(1)
        self.assertIsNotNone(i2c1_pins)
        self.assertEqual(i2c1_pins['SCL'], 'PB8')
        self.assertEqual(i2c1_pins['SDA'], 'PB9')

    def test_uart_resources(self):
        """Test UART resource parsing (UART→SERIAL mapping)."""
        # UART1
        uart1_pins = self.config.get_uart_pins(1)
        self.assertIsNotNone(uart1_pins)
        self.assertEqual(uart1_pins['TX'], 'PB6')
        self.assertEqual(uart1_pins['RX'], 'PB7')

        # UART2
        uart2_pins = self.config.get_uart_pins(2)
        self.assertIsNotNone(uart2_pins)
        self.assertEqual(uart2_pins['TX'], 'PA2')
        self.assertEqual(uart2_pins['RX'], 'PA3')

    def test_timer_pin_map_parsing(self):
        """Test TIMER_PIN_MAP macro parsing."""
        self.assertGreater(len(self.config.timer_pin_map), 0)

        # Motor 1: PA8, occurrence 1
        self.assertIn('PA8', self.config.timer_pin_map)
        entry = self.config.timer_pin_map['PA8']
        self.assertEqual(entry.occurrence, 1)

        # Motor 4: PB0, occurrence 2
        self.assertIn('PB0', self.config.timer_pin_map)
        entry = self.config.timer_pin_map['PB0']
        self.assertEqual(entry.occurrence, 2)

        # Motor 5: PB4, occurrence 1
        self.assertIn('PB4', self.config.timer_pin_map)
        entry = self.config.timer_pin_map['PB4']
        self.assertEqual(entry.occurrence, 1)

    def test_spi_instance_settings(self):
        """Test SPI instance #define → settings mapping."""
        self.assertEqual(self.config.settings['gyro_1_spibus'], '1')
        self.assertEqual(self.config.settings['flash_spi_bus'], '2')
        self.assertEqual(self.config.settings['max7456_spi_bus'], '2')

    def test_blackbox_device_setting(self):
        """Test DEFAULT_BLACKBOX_DEVICE → settings mapping."""
        self.assertEqual(self.config.settings['blackbox_device'], 'SPIFLASH')

    def test_current_scale_setting(self):
        """Test DEFAULT_CURRENT_METER_SCALE → ibata_scale mapping."""
        self.assertEqual(self.config.settings['ibata_scale'], '170')

    def test_gyro_align_setting(self):
        """Test GYRO_1_ALIGN → settings mapping (full identifier preserved)."""
        self.assertEqual(self.config.settings['gyro_1_sensor_align'], 'CW180_DEG')

    def test_pin_format_identity(self):
        """Test convert_pin_format is identity (pins already Arduino format)."""
        self.assertEqual(self.config.convert_pin_format('PA8'), 'PA8')
        self.assertEqual(self.config.convert_pin_format('PB0'), 'PB0')
        self.assertEqual(self.config.convert_pin_format('PC13'), 'PC13')

    def test_storage_cs_pins(self):
        """Test storage chip select pin parsing."""
        flash_cs = self.config.get_resources('FLASH_CS')
        self.assertEqual(len(flash_cs), 1)
        self.assertEqual(flash_cs[0].pin, 'PB2')

    def test_gyro_resources(self):
        """Test gyro resource parsing."""
        gyro_cs = self.config.get_resources('GYRO_CS')
        self.assertEqual(len(gyro_cs), 1)
        self.assertEqual(gyro_cs[0].pin, 'PA4')

        gyro_exti = self.config.get_resources('GYRO_EXTI')
        self.assertEqual(len(gyro_exti), 1)
        self.assertEqual(gyro_exti[0].pin, 'PB3')

    def test_adc_resources(self):
        """Test ADC resource parsing."""
        adc_batt = self.config.get_resources('ADC_BATT')
        self.assertEqual(len(adc_batt), 1)
        self.assertEqual(adc_batt[0].pin, 'PA0')

        adc_curr = self.config.get_resources('ADC_CURR')
        self.assertEqual(len(adc_curr), 1)
        self.assertEqual(adc_curr[0].pin, 'PA1')

    def test_led_resources(self):
        """Test LED resource parsing (0-based→1-based conversion)."""
        led = self.config.get_resources('LED')
        self.assertEqual(len(led), 1)
        self.assertEqual(led[0].index, 1)   # LED0 → index 1
        self.assertEqual(led[0].pin, 'PC13')

    def test_directory_input(self):
        """Test that parser accepts directory path."""
        config_path = Path(__file__).parent.parent / "bf_configs/JHEF411"
        config = BetaflightConfig(config_path)
        self.assertEqual(config.board_name, "JHEF411")


class TestGyroAlignFlipPreservation(unittest.TestCase):
    """MATEKH743 has CW0_DEG_FLIP + CW90_DEG_FLIP — the previous lossy parse
    stripped both _DEG and _FLIP and would have mangled these to 'CW0' and
    'CW90', silently dropping the FLIP information needed to reconstruct
    the chip-to-board rotation."""

    @classmethod
    def setUpClass(cls):
        config_path = Path(__file__).parent.parent / "bf_configs/MATEKH743"
        cls.config = BetaflightConfig(config_path)

    def test_gyro_1_align_flip_preserved(self):
        self.assertEqual(self.config.settings['gyro_1_sensor_align'], 'CW0_DEG_FLIP')

    def test_gyro_2_align_flip_preserved(self):
        self.assertEqual(self.config.settings['gyro_2_sensor_align'], 'CW90_DEG_FLIP')


class TestTimerResolution(unittest.TestCase):
    """Test timer occurrence resolution using PeripheralPins.c data."""

    @classmethod
    def setUpClass(cls):
        """Load JHEF411 config and F411CE pinmap for timer resolution."""
        config_path = Path(__file__).parent.parent / "bf_configs/JHEF411"
        cls.config = BetaflightConfig(config_path)

        arduino_root = Path(__file__).parents[3]
        pinmap_path = arduino_root / "variants/STM32F4xx/F411C(C-E)(U-Y)/PeripheralPins.c"
        if not pinmap_path.exists():
            raise FileNotFoundError(f"PeripheralPins.c not found at {pinmap_path}")
        cls.pinmap = PeripheralPinMap(pinmap_path)

        cls.config.resolve_timers(cls.pinmap)

    def test_timer_resolution_motor1(self):
        """Test PA8 occurrence=1 → TIM1_CH1."""
        self.assertIn('PA8', self.config.timers)
        timer = self.config.timers['PA8']
        self.assertEqual(timer.timer, 'TIM1')
        self.assertEqual(timer.channel, 1)
        self.assertEqual(timer.af, 1)

    def test_timer_resolution_motor4(self):
        """Test PB0 occurrence=2 → TIM3_CH3."""
        self.assertIn('PB0', self.config.timers)
        timer = self.config.timers['PB0']
        self.assertEqual(timer.timer, 'TIM3')
        self.assertEqual(timer.channel, 3)
        self.assertEqual(timer.af, 2)

    def test_timer_resolution_motor5(self):
        """Test PB4 occurrence=1 → TIM3_CH1."""
        self.assertIn('PB4', self.config.timers)
        timer = self.config.timers['PB4']
        self.assertEqual(timer.timer, 'TIM3')
        self.assertEqual(timer.channel, 1)
        self.assertEqual(timer.af, 2)

    def test_all_motor_timers_resolved(self):
        """Test all motor pins have timer assignments."""
        motors = self.config.get_motors()
        for motor in motors:
            self.assertIn(motor.pin, self.config.timers,
                         f"Motor {motor.index} pin {motor.pin} has no timer assignment")


class TestMultipleTargets(unittest.TestCase):
    """Test parser with multiple target configs."""

    def test_revo_f405(self):
        """Test REVO (F405) config parsing."""
        config_path = Path(__file__).parent.parent / "bf_configs/REVO"
        if not (config_path / 'config.h').exists():
            self.skipTest("REVO config not found")

        config = BetaflightConfig(config_path)
        self.assertEqual(config.mcu_type, "STM32F405")
        self.assertEqual(config.board_name, "REVO")
        self.assertEqual(config.manufacturer_id, "OPEN")
        self.assertEqual(len(config.get_motors()), 6)

    def test_nero_f7x2(self):
        """Test NERO (F7X2) config parsing."""
        config_path = Path(__file__).parent.parent / "bf_configs/NERO"
        if not (config_path / 'config.h').exists():
            self.skipTest("NERO config not found")

        config = BetaflightConfig(config_path)
        self.assertEqual(config.mcu_type, "STM32F7X2")
        self.assertEqual(config.board_name, "NERO")
        self.assertEqual(config.manufacturer_id, "BKMN")
        self.assertEqual(len(config.get_motors()), 8)

    def test_matekh743(self):
        """Test MATEKH743 (H743) config parsing."""
        config_path = Path(__file__).parent.parent / "bf_configs/MATEKH743"
        if not (config_path / 'config.h').exists():
            self.skipTest("MATEKH743 config not found")

        config = BetaflightConfig(config_path)
        self.assertEqual(config.mcu_type, "STM32H743")
        self.assertEqual(config.board_name, "MATEKH743")
        self.assertEqual(config.manufacturer_id, "MTKS")
        self.assertEqual(len(config.get_motors()), 8)
        self.assertEqual(len(config.get_servos()), 2)


if __name__ == '__main__':
    unittest.main()
