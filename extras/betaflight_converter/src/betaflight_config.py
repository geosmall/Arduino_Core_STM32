"""
Parser for Betaflight unified target configuration files.
"""

import re
from dataclasses import dataclass, field
from typing import Dict, List, Optional
from pathlib import Path


class Patterns:
    """Compiled regex patterns for Betaflight config parsing."""
    MCU_TYPE = re.compile(r'STM32\w+')
    RESOURCE = re.compile(r'resource\s+(\w+)\s+(\d+)\s+(\w+)')
    TIMER = re.compile(r'timer\s+(\w+)\s+AF(\d+)')
    TIMER_COMMENT = re.compile(r'#\s+pin\s+(\w+):\s+(\w+)\s+CH(\d+)')
    DMA = re.compile(r'dma\s+(pin\s+)?(\w+)\s+(\d+)\s+(\d+)')
    FEATURE = re.compile(r'feature\s+(\w+)')
    SETTING = re.compile(r'set\s+(\w+)\s+=\s+(.+)')
    PIN_LEADING_ZERO = re.compile(r'0(\d)')
    DEFINE_CHECK = re.compile(r'#define\s+USE_GYRO_SPI_(\w+)')


@dataclass
class ResourcePin:
    """Resource pin assignment."""
    resource_type: str  # e.g., "MOTOR", "SERIAL_TX"
    index: int          # 1-based index
    pin: str            # Betaflight format: "B04", "A08"


@dataclass
class TimerAssignment:
    """Timer assignment for a pin."""
    pin: str            # Betaflight format: "B04"
    af: int             # Alternate function number
    timer: Optional[str] = None    # e.g., "TIM3" (parsed from comment)
    channel: Optional[int] = None  # e.g., 1 for CH1 (parsed from comment)


@dataclass
class DMAAssignment:
    """DMA assignment for a pin or peripheral."""
    target: str         # Pin name or peripheral name
    stream: int         # DMA stream number
    channel: Optional[int] = None  # DMA channel number (from comment)


class BetaflightConfig:
    """Parser for Betaflight unified target .config files."""

    def __init__(self, filepath: Path):
        """Initialize parser with path to .config file."""
        self.filepath = filepath

        # Parsed data
        self.mcu_type: Optional[str] = None
        self.board_name: Optional[str] = None
        self.manufacturer_id: Optional[str] = None

        self.defines: List[str] = []
        self.resources: Dict[str, List[ResourcePin]] = {}
        self.timers: Dict[str, TimerAssignment] = {}
        self.dma: Dict[str, DMAAssignment] = {}
        self.features: List[str] = []
        self.settings: Dict[str, str] = {}

        if filepath.exists():
            self._parse()

    def _parse(self):
        """Parse Betaflight .config file."""
        with open(self.filepath, 'r') as f:
            lines = f.readlines()

        for line in lines:
            line = line.strip()

            # Skip empty lines
            if not line:
                continue

            # Parse different line types (including comments for header/timer info)
            self._parse_header(line)
            self._parse_define(line)
            self._parse_board_info(line)
            self._parse_resource(line)
            self._parse_timer(line)  # Parses both timer lines and comments
            self._parse_dma(line)
            self._parse_feature(line)
            self._parse_setting(line)

    def _parse_header(self, line: str):
        """Parse header line for MCU type."""
        # # Betaflight / STM32F411 (S411) 4.2.0 ...
        if line.startswith('#') and 'Betaflight' in line:
            match = Patterns.MCU_TYPE.search(line)
            if match:
                self.mcu_type = match.group(0)

    def _parse_define(self, line: str):
        """Parse #define statements."""
        if line.startswith('#define'):
            self.defines.append(line)

    def _parse_board_info(self, line: str):
        """Parse board_name and manufacturer_id."""
        if line.startswith('board_name'):
            self.board_name = line.split()[1]
        elif line.startswith('manufacturer_id'):
            self.manufacturer_id = line.split()[1]

    def _parse_resource(self, line: str):
        """Parse resource definitions."""
        # resource MOTOR 1 B04
        match = Patterns.RESOURCE.match(line)
        if match:
            resource_type = match.group(1)
            index = int(match.group(2))
            pin = match.group(3)

            if resource_type not in self.resources:
                self.resources[resource_type] = []

            self.resources[resource_type].append(ResourcePin(
                resource_type=resource_type,
                index=index,
                pin=pin
            ))

    def _parse_timer(self, line: str):
        """Parse timer assignments."""
        # timer B04 AF2
        match = Patterns.TIMER.match(line)
        if match:
            pin = match.group(1)
            af = int(match.group(2))
            self.timers[pin] = TimerAssignment(pin=pin, af=af)

        # Parse comment for timer/channel info
        # # pin B04: TIM3 CH1 (AF2)
        # # pin A08: TIM1 CH1 (AF1)
        comment_match = Patterns.TIMER_COMMENT.match(line)
        if comment_match:
            pin = comment_match.group(1)
            timer = comment_match.group(2)
            channel = int(comment_match.group(3))

            if pin in self.timers:
                self.timers[pin].timer = timer
                self.timers[pin].channel = channel
            else:
                # Comment came before timer line, create entry
                self.timers[pin] = TimerAssignment(pin=pin, af=0, timer=timer, channel=channel)

    def _parse_dma(self, line: str):
        """Parse DMA assignments."""
        # dma pin B04 0
        # dma ADC 1 1
        match = Patterns.DMA.match(line)
        if match:
            is_pin = match.group(1) is not None
            target = match.group(2)
            if is_pin:
                # dma pin B04 0
                stream = int(match.group(3))
            else:
                # dma ADC 1 1
                target = f"{target}_{match.group(3)}"  # ADC_1
                stream = int(match.group(4))

            self.dma[target] = DMAAssignment(target=target, stream=stream)

    def _parse_feature(self, line: str):
        """Parse feature flags."""
        # feature RX_SERIAL
        match = Patterns.FEATURE.match(line)
        if match:
            self.features.append(match.group(1))

    def _parse_setting(self, line: str):
        """Parse set commands."""
        # set gyro_1_spibus = 1
        match = Patterns.SETTING.match(line)
        if match:
            key = match.group(1)
            value = match.group(2).strip()
            self.settings[key] = value

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

        for resource_type in ['SPI_MOSI', 'SPI_MISO', 'SPI_SCK']:
            resources = self.get_resources(resource_type)
            for res in resources:
                if res.index == bus_num:
                    signal = resource_type.replace('SPI_', '').replace('SCK', 'SCLK')
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

    def convert_pin_format(self, bf_pin: str) -> str:
        """
        Convert Betaflight pin format to Arduino macro format.

        Args:
            bf_pin: Betaflight format (e.g., "B04", "A08")

        Returns:
            Arduino macro format (e.g., "PB4", "PA8") - no underscore
            This format matches Arduino pin macros defined in variant headers.
        """
        # Remove leading zero: B04 -> B4
        pin = Patterns.PIN_LEADING_ZERO.sub(r'\1', bf_pin)
        # Add P prefix (no underscore): B4 -> PB4
        return f"P{pin[0]}{pin[1:]}"

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
