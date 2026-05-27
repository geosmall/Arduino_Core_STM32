"""
Parser for Betaflight native target configuration files (config.h).

Parses C header files with #define statements from the Betaflight source tree
(betaflight/src/config/configs/<TARGET>/config.h).
"""

import re
from dataclasses import dataclass, field
from typing import Dict, List, Optional
from pathlib import Path


class Patterns:
    """Compiled regex patterns for Betaflight config.h parsing."""
    # Board identity
    FC_TARGET_MCU = re.compile(r'#define\s+FC_TARGET_MCU\s+(\w+)')
    BOARD_NAME = re.compile(r'#define\s+BOARD_NAME\s+(\w+)')
    MANUFACTURER_ID = re.compile(r'#define\s+MANUFACTURER_ID\s+(\w+)')

    # Pin definitions — numbered resources
    MOTOR_PIN = re.compile(r'#define\s+MOTOR(\d+)_PIN\s+(P\w+)')
    SERVO_PIN = re.compile(r'#define\s+SERVO(\d+)_PIN\s+(P\w+)')
    UART_TX_PIN = re.compile(r'#define\s+UART(\d+)_TX_PIN\s+(P\w+)')
    UART_RX_PIN = re.compile(r'#define\s+UART(\d+)_RX_PIN\s+(P\w+)')
    SPI_SCK_PIN = re.compile(r'#define\s+SPI(\d+)_SCK_PIN\s+(P\w+)')
    SPI_SDI_PIN = re.compile(r'#define\s+SPI(\d+)_SDI_PIN\s+(P\w+)')  # SDI = MISO
    SPI_SDO_PIN = re.compile(r'#define\s+SPI(\d+)_SDO_PIN\s+(P\w+)')  # SDO = MOSI
    I2C_SCL_PIN = re.compile(r'#define\s+I2C(\d+)_SCL_PIN\s+(P\w+)')
    I2C_SDA_PIN = re.compile(r'#define\s+I2C(\d+)_SDA_PIN\s+(P\w+)')
    LED_PIN = re.compile(r'#define\s+LED(\d+)_PIN\s+(P\w+)')

    # Pin definitions — singleton resources
    ADC_VBAT_PIN = re.compile(r'#define\s+ADC_VBAT_PIN\s+(P\w+)')
    ADC_CURR_PIN = re.compile(r'#define\s+ADC_CURR_PIN\s+(P\w+)')
    FLASH_CS_PIN = re.compile(r'#define\s+FLASH_CS_PIN\s+(P\w+)')
    SDCARD_SPI_CS_PIN = re.compile(r'#define\s+SDCARD_SPI_CS_PIN\s+(P\w+)')
    BEEPER_PIN = re.compile(r'#define\s+BEEPER_PIN\s+(P\w+)')

    # Pin definitions — gyro (indexed)
    GYRO_CS_PIN = re.compile(r'#define\s+GYRO_(\d+)_CS_PIN\s+(P\w+)')
    GYRO_EXTI_PIN = re.compile(r'#define\s+GYRO_(\d+)_EXTI_PIN\s+(P\w+)')

    # Timer pin mapping
    TIMER_PIN_MAP = re.compile(
        r'TIMER_PIN_MAP\(\s*(\d+)\s*,\s*(P\w+)\s*,\s*(\d+)\s*,\s*(-?\d+)\s*\)'
    )

    # SPI instance assignments
    SPI_INSTANCE = re.compile(r'#define\s+(\w+)_SPI_INSTANCE\s+SPI(\d+)')

    # Receiver UART assignment
    SERIALRX_UART = re.compile(r'#define\s+SERIALRX_UART\s+SERIAL_PORT_USART(\d+)')

    # Settings as #defines
    DEFAULT_CURRENT_METER_SCALE = re.compile(
        r'#define\s+DEFAULT_CURRENT_METER_SCALE\s+(\d+)'
    )
    DEFAULT_VOLTAGE_METER_SCALE = re.compile(
        r'#define\s+DEFAULT_VOLTAGE_METER_SCALE\s+(\d+)'
    )
    DEFAULT_BLACKBOX_DEVICE = re.compile(
        r'#define\s+DEFAULT_BLACKBOX_DEVICE\s+BLACKBOX_DEVICE_(\w+)'
    )
    GYRO_ALIGN = re.compile(r'#define\s+GYRO_(\d+)_ALIGN\s+(\w+)')

    # Feature/sensor defines
    USE_DEFINE = re.compile(r'#define\s+(USE_\w+)')
    DEFINE_CHECK = re.compile(r'#define\s+USE_GYRO_SPI_(\w+)')


@dataclass
class ResourcePin:
    """Resource pin assignment."""
    resource_type: str  # e.g., "MOTOR", "SERIAL_TX"
    index: int          # 1-based index
    pin: str            # Arduino format: "PA8", "PB0"


@dataclass
class TimerAssignment:
    """Timer assignment for a pin."""
    pin: str            # Arduino format: "PA8"
    af: int             # Alternate function number
    timer: Optional[str] = None    # e.g., "TIM3"
    channel: Optional[int] = None  # e.g., 1 for CH1


@dataclass
class TimerPinMapEntry:
    """Raw TIMER_PIN_MAP entry before resolution."""
    index: int          # Timer map index
    pin: str            # Pin name (Arduino format)
    occurrence: int     # 1-based occurrence in timer hardware table
    dma_opt: int        # DMA option (-1 = none)


@dataclass
class DMAAssignment:
    """DMA assignment for a pin or peripheral."""
    target: str         # Pin name or peripheral name
    stream: int         # DMA stream number
    channel: Optional[int] = None  # DMA channel number


class BetaflightConfig:
    """Parser for Betaflight native target config.h files."""

    def __init__(self, filepath: Path):
        """Initialize parser with path to config.h file.

        Args:
            filepath: Path to config.h file, or a directory containing config.h
        """
        # Handle directory input
        if filepath.is_dir():
            filepath = filepath / 'config.h'

        self.filepath = filepath

        # Parsed data
        self.mcu_type: Optional[str] = None
        self.board_name: Optional[str] = None
        self.manufacturer_id: Optional[str] = None

        self.defines: List[str] = []
        self.resources: Dict[str, List[ResourcePin]] = {}
        self.timers: Dict[str, TimerAssignment] = {}
        self.timer_pin_map: Dict[str, TimerPinMapEntry] = {}  # Raw entries
        self.dma: Dict[str, DMAAssignment] = {}
        self.features: List[str] = []
        self.settings: Dict[str, str] = {}

        if filepath.exists():
            self._parse()

    def _parse(self):
        """Parse Betaflight config.h file."""
        with open(self.filepath, 'r') as f:
            content = f.read()

        # Parse line by line for most patterns
        for line in content.splitlines():
            line = line.strip()
            if not line:
                continue

            self._parse_identity(line)
            self._parse_defines(line)
            self._parse_pin_resources(line)
            self._parse_spi_instances(line)
            self._parse_settings(line)

        # Parse TIMER_PIN_MAPPING (may span multiple lines via backslash continuation)
        self._parse_timer_pin_mapping(content)

    def _parse_identity(self, line: str):
        """Parse board identity defines."""
        match = Patterns.FC_TARGET_MCU.match(line)
        if match:
            self.mcu_type = match.group(1)
            return

        match = Patterns.BOARD_NAME.match(line)
        if match:
            self.board_name = match.group(1)
            return

        match = Patterns.MANUFACTURER_ID.match(line)
        if match:
            self.manufacturer_id = match.group(1)

    def _parse_defines(self, line: str):
        """Parse USE_* and other #define statements."""
        match = Patterns.USE_DEFINE.match(line)
        if match:
            self.defines.append(line)

    def _add_resource(self, resource_type: str, index: int, pin: str):
        """Add a resource pin assignment."""
        if resource_type not in self.resources:
            self.resources[resource_type] = []
        self.resources[resource_type].append(ResourcePin(
            resource_type=resource_type,
            index=index,
            pin=pin
        ))

    def _parse_pin_resources(self, line: str):
        """Parse all pin definition #defines into resources."""
        # Motors
        match = Patterns.MOTOR_PIN.match(line)
        if match:
            self._add_resource('MOTOR', int(match.group(1)), match.group(2))
            return

        # Servos
        match = Patterns.SERVO_PIN.match(line)
        if match:
            self._add_resource('SERVO', int(match.group(1)), match.group(2))
            return

        # UART TX/RX → SERIAL_TX/SERIAL_RX (validator compatibility)
        match = Patterns.UART_TX_PIN.match(line)
        if match:
            self._add_resource('SERIAL_TX', int(match.group(1)), match.group(2))
            return

        match = Patterns.UART_RX_PIN.match(line)
        if match:
            self._add_resource('SERIAL_RX', int(match.group(1)), match.group(2))
            return

        # SPI pins: SDI→SPI_MISO, SDO→SPI_MOSI, SCK→SPI_SCK
        match = Patterns.SPI_SCK_PIN.match(line)
        if match:
            self._add_resource('SPI_SCK', int(match.group(1)), match.group(2))
            return

        match = Patterns.SPI_SDI_PIN.match(line)
        if match:
            self._add_resource('SPI_MISO', int(match.group(1)), match.group(2))
            return

        match = Patterns.SPI_SDO_PIN.match(line)
        if match:
            self._add_resource('SPI_MOSI', int(match.group(1)), match.group(2))
            return

        # I2C
        match = Patterns.I2C_SCL_PIN.match(line)
        if match:
            self._add_resource('I2C_SCL', int(match.group(1)), match.group(2))
            return

        match = Patterns.I2C_SDA_PIN.match(line)
        if match:
            self._add_resource('I2C_SDA', int(match.group(1)), match.group(2))
            return

        # LEDs (0-based in config.h → 1-based internally)
        match = Patterns.LED_PIN.match(line)
        if match:
            self._add_resource('LED', int(match.group(1)) + 1, match.group(2))
            return

        # ADC (singleton resources)
        match = Patterns.ADC_VBAT_PIN.match(line)
        if match:
            self._add_resource('ADC_BATT', 1, match.group(1))
            return

        match = Patterns.ADC_CURR_PIN.match(line)
        if match:
            self._add_resource('ADC_CURR', 1, match.group(1))
            return

        # Flash CS
        match = Patterns.FLASH_CS_PIN.match(line)
        if match:
            self._add_resource('FLASH_CS', 1, match.group(1))
            return

        # SD card CS
        match = Patterns.SDCARD_SPI_CS_PIN.match(line)
        if match:
            self._add_resource('SDCARD_CS', 1, match.group(1))
            return

        # Gyro CS and EXTI (indexed)
        match = Patterns.GYRO_CS_PIN.match(line)
        if match:
            self._add_resource('GYRO_CS', int(match.group(1)), match.group(2))
            return

        match = Patterns.GYRO_EXTI_PIN.match(line)
        if match:
            self._add_resource('GYRO_EXTI', int(match.group(1)), match.group(2))
            return

        # Beeper
        match = Patterns.BEEPER_PIN.match(line)
        if match:
            self._add_resource('BEEPER', 1, match.group(1))
            return

    def _parse_timer_pin_mapping(self, content: str):
        """Parse TIMER_PIN_MAPPING macro (may span multiple lines)."""
        for match in Patterns.TIMER_PIN_MAP.finditer(content):
            index = int(match.group(1))
            pin = match.group(2)
            occurrence = int(match.group(3))
            dma_opt = int(match.group(4))

            self.timer_pin_map[pin] = TimerPinMapEntry(
                index=index,
                pin=pin,
                occurrence=occurrence,
                dma_opt=dma_opt
            )

    def _parse_spi_instances(self, line: str):
        """Parse SPI instance assignments into settings dict."""
        match = Patterns.SPI_INSTANCE.match(line)
        if match:
            device = match.group(1)   # e.g., "GYRO_1", "FLASH", "SDCARD"
            bus_num = match.group(2)  # e.g., "1", "2", "3"

            # Map to settings keys for validator/generator compatibility
            instance_map = {
                'GYRO_1': 'gyro_1_spibus',
                'GYRO_2': 'gyro_2_spibus',
                'FLASH': 'flash_spi_bus',
                'SDCARD': 'sdcard_spi_bus',
                'MAX7456': 'max7456_spi_bus',
            }

            settings_key = instance_map.get(device)
            if settings_key:
                self.settings[settings_key] = bus_num

    def _parse_settings(self, line: str):
        """Parse settings from #define statements."""
        # Current meter scale
        match = Patterns.DEFAULT_CURRENT_METER_SCALE.match(line)
        if match:
            self.settings['ibata_scale'] = match.group(1)
            return

        # Voltage meter scale
        match = Patterns.DEFAULT_VOLTAGE_METER_SCALE.match(line)
        if match:
            self.settings['vbat_scale'] = match.group(1)
            return

        # Blackbox device
        match = Patterns.DEFAULT_BLACKBOX_DEVICE.match(line)
        if match:
            device = match.group(1)
            # Map Betaflight names to old settings format
            device_map = {
                'FLASH': 'SPIFLASH',
                'SDCARD': 'SDCARD',
                'NONE': 'NONE',
            }
            self.settings['blackbox_device'] = device_map.get(device, device)
            return

        # Gyro alignment — preserve full Betaflight identifier so all 8
        # cardinal orientations (CW{0,90,180,270}_DEG and _FLIP variants)
        # survive end-to-end and match IMUAlignment enum names verbatim.
        match = Patterns.GYRO_ALIGN.match(line)
        if match:
            gyro_num = match.group(1)
            align = match.group(2)
            self.settings[f'gyro_{gyro_num}_sensor_align'] = align
            return

        # Receiver UART
        match = Patterns.SERIALRX_UART.match(line)
        if match:
            self.settings['serialrx_uart'] = match.group(1)

    def resolve_timers(self, pinmap):
        """
        Resolve TIMER_PIN_MAP occurrences to actual timer/channel/AF assignments
        using PeripheralPins.c data.

        The occurrence parameter is a 1-based index into the timer options for
        each pin, matching the ordering in PeripheralPins.c's PinMap_TIM array.

        Args:
            pinmap: PeripheralPinMap instance with parsed PinMap_TIM data
        """
        for pin, entry in self.timer_pin_map.items():
            # Get all timer entries for this pin from PeripheralPins.c
            pin_timers = [tp for tp in pinmap.timer_pins if tp.pin == pin]

            if not pin_timers:
                continue

            # Occurrence is 1-based index into the list
            occ_idx = entry.occurrence - 1
            if occ_idx < 0 or occ_idx >= len(pin_timers):
                continue

            tp = pin_timers[occ_idx]
            self.timers[pin] = TimerAssignment(
                pin=pin,
                af=tp.af,
                timer=tp.timer,
                channel=tp.channel
            )

    # --- Public API (compatible with validator.py and code_generator.py) ---

    def get_resources(self, resource_type: str) -> List[ResourcePin]:
        """Get all resources of a specific type."""
        return self.resources.get(resource_type, [])

    def get_motors(self) -> List[ResourcePin]:
        """Get motor resources."""
        return self.get_resources('MOTOR')

    def get_servos(self) -> List[ResourcePin]:
        """Get servo resources."""
        return self.get_resources('SERVO')

    def get_spi_pins(self, bus_num: int) -> Optional[Dict[str, str]]:
        """
        Get SPI pins for a specific bus.

        Returns:
            Dict with keys 'MOSI', 'MISO', 'SCLK' or None if incomplete
        """
        pins = {}

        for resource_type, signal in [('SPI_MOSI', 'MOSI'), ('SPI_MISO', 'MISO'),
                                       ('SPI_SCK', 'SCLK')]:
            resources = self.get_resources(resource_type)
            for res in resources:
                if res.index == bus_num:
                    pins[signal] = res.pin

        if len(pins) == 3:
            return pins
        return None

    def get_i2c_pins(self, bus_num: int) -> Optional[Dict[str, str]]:
        """
        Get I2C pins for a specific bus.

        Returns:
            Dict with keys 'SCL', 'SDA' or None if incomplete
        """
        pins = {}

        for resource_type in ['I2C_SCL', 'I2C_SDA']:
            resources = self.get_resources(resource_type)
            for res in resources:
                if res.index == bus_num:
                    signal = resource_type.replace('I2C_', '')
                    pins[signal] = res.pin

        if len(pins) == 2:
            return pins
        return None

    def get_uart_pins(self, uart_num: int) -> Optional[Dict[str, str]]:
        """
        Get UART pins for a specific port.

        Returns:
            Dict with keys 'TX', 'RX' or None if incomplete
        """
        pins = {}

        for resource_type in ['SERIAL_TX', 'SERIAL_RX']:
            resources = self.get_resources(resource_type)
            for res in resources:
                if res.index == uart_num:
                    signal = resource_type.replace('SERIAL_', '')
                    pins[signal] = res.pin

        if len(pins) == 2:
            return pins
        return None

    def get_serialrx_uart(self) -> Optional[int]:
        """Get SERIALRX_UART number, or None if not defined."""
        val = self.settings.get('serialrx_uart')
        return int(val) if val else None

    def convert_pin_format(self, pin: str) -> str:
        """
        Convert pin format to Arduino macro format.

        For native config.h files, pins are already in Arduino format (PA8, PB0).
        This method is an identity function for API compatibility with the
        validator and code generator.
        """
        return pin

    def has_define(self, define_name: str) -> bool:
        """Check if a specific #define exists."""
        pattern = f"#define\\s+{define_name}"
        for define in self.defines:
            if re.search(pattern, define):
                return True
        return False

    def get_gyro_chips(self) -> List[str]:
        """Get list of supported gyro chips from #defines."""
        chips = []
        for define in self.defines:
            match = Patterns.DEFINE_CHECK.match(define)
            if match:
                chips.append(match.group(1))
        return chips
