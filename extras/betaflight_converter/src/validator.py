"""
Validator that combines Betaflight config with PeripheralPins.c data.
"""

from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple
from pathlib import Path

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap


@dataclass
class ValidationError:
    """Validation error information."""
    severity: str  # "error" or "warning"
    message: str
    resource: Optional[str] = None
    pin: Optional[str] = None


@dataclass
class ValidatedMotor:
    """Validated motor configuration."""
    index: int
    pin_bf: str      # Betaflight pin: "PB4"
    pin_arduino: str # Arduino format with ALT if needed: "PB0_ALT1"
    timer: str       # e.g., "TIM3"
    channel: int     # Timer channel (1-4)
    af: int          # Alternate function


@dataclass
class ValidatedSPIBus:
    """Validated SPI bus configuration."""
    bus_num: int
    bus_name: str    # e.g., "SPI1"
    mosi: str        # Arduino format
    miso: str
    sclk: str


@dataclass
class ValidatedI2CBus:
    """Validated I2C bus configuration."""
    bus_num: int
    bus_name: str    # e.g., "I2C1"
    scl: str         # Arduino format
    sda: str


@dataclass
class ValidatedUART:
    """Validated UART configuration."""
    uart_num: int
    uart_name: str   # e.g., "USART1"
    tx: str          # Arduino format
    rx: str


class ConfigValidator:
    """Validates Betaflight config against PeripheralPins.c data."""

    def __init__(self, bf_config: BetaflightConfig, pinmap: PeripheralPinMap):
        """Initialize validator with parsed configs."""
        self.bf_config = bf_config
        self.pinmap = pinmap
        self.errors: List[ValidationError] = []
        self.warnings: List[ValidationError] = []

    def validate_all(self) -> bool:
        """
        Run all validations.

        Returns:
            True if no errors, False if errors found (warnings are OK)
        """
        self.errors.clear()
        self.warnings.clear()

        self.validate_motors()
        self.validate_spi_buses()
        self.validate_i2c_buses()
        self.validate_uarts()

        return len(self.errors) == 0

    def validate_motors(self) -> List[ValidatedMotor]:
        """Validate motor timer assignments."""
        validated_motors = []

        for motor in self.bf_config.get_motors():
            pin_bf = motor.pin
            pin_arduino = self.bf_config.convert_pin_format(pin_bf)

            # Get timer assignment
            if pin_bf not in self.bf_config.timers:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Motor {motor.index} pin {pin_bf} has no timer assignment",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))
                continue

            timer_assignment = self.bf_config.timers[pin_bf]

            # Validate against PeripheralPins.c
            channel = self.pinmap.validate_timer(
                pin=pin_arduino,
                timer=timer_assignment.timer,
                af=timer_assignment.af
            )

            if channel is None:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Motor {motor.index}: {pin_arduino} does not support {timer_assignment.timer} on AF{timer_assignment.af}",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))
                continue

            # Verify channel matches
            if timer_assignment.channel and channel != timer_assignment.channel:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"Motor {motor.index}: Expected CH{timer_assignment.channel}, found CH{channel}",
                    resource=f"MOTOR_{motor.index}",
                    pin=pin_bf
                ))

            # Get correct pin format with ALT suffix if needed
            correct_pin = self.pinmap.get_pin_for_timer(pin_arduino, timer_assignment.timer, timer_assignment.af)
            if not correct_pin:
                correct_pin = pin_arduino  # Fallback to base pin

            validated_motors.append(ValidatedMotor(
                index=motor.index,
                pin_bf=pin_bf,
                pin_arduino=correct_pin,
                timer=timer_assignment.timer,
                channel=channel,
                af=timer_assignment.af
            ))

        return validated_motors

    def validate_servos(self) -> List[ValidatedMotor]:
        """Validate servo timer assignments (reuses ValidatedMotor dataclass)."""
        validated_servos = []

        for servo in self.bf_config.get_servos():
            pin_bf = servo.pin
            pin_arduino = self.bf_config.convert_pin_format(pin_bf)

            # Get timer assignment
            if pin_bf not in self.bf_config.timers:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Servo {servo.index} pin {pin_bf} has no timer assignment",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))
                continue

            timer_assignment = self.bf_config.timers[pin_bf]

            # Validate against PeripheralPins.c
            channel = self.pinmap.validate_timer(
                pin=pin_arduino,
                timer=timer_assignment.timer,
                af=timer_assignment.af
            )

            if channel is None:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"Servo {servo.index}: {pin_arduino} does not support {timer_assignment.timer} on AF{timer_assignment.af}",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))
                continue

            # Verify channel matches
            if timer_assignment.channel and channel != timer_assignment.channel:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"Servo {servo.index}: Expected CH{timer_assignment.channel}, found CH{channel}",
                    resource=f"SERVO_{servo.index}",
                    pin=pin_bf
                ))

            # Get correct pin format with ALT suffix if needed
            correct_pin = self.pinmap.get_pin_for_timer(pin_arduino, timer_assignment.timer, timer_assignment.af)
            if not correct_pin:
                correct_pin = pin_arduino  # Fallback to base pin

            validated_servos.append(ValidatedMotor(
                index=servo.index,
                pin_bf=pin_bf,
                pin_arduino=correct_pin,
                timer=timer_assignment.timer,
                channel=channel,
                af=timer_assignment.af
            ))

        return validated_servos

    def validate_spi_buses(self) -> List[ValidatedSPIBus]:
        """Validate SPI bus pin assignments."""
        validated_buses = []

        # Find all SPI buses in config
        spi_resources = self.bf_config.get_resources('SPI_SCK')
        bus_numbers = [res.index for res in spi_resources]

        for bus_num in bus_numbers:
            pins_bf = self.bf_config.get_spi_pins(bus_num)
            if not pins_bf:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"SPI{bus_num} has incomplete pin assignments",
                    resource=f"SPI{bus_num}"
                ))
                continue

            # Convert to Arduino format (base pins, no ALT)
            pins_arduino_base = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Get correct pin format (with ALT if needed) for target bus
            target_bus = f"SPI{bus_num}"
            pins_with_alt = {}

            for signal, base_pin in pins_arduino_base.items():
                correct_pin = self.pinmap.get_pin_for_spi_bus(base_pin, signal, target_bus)
                if not correct_pin:
                    self.errors.append(ValidationError(
                        severity="error",
                        message=f"SPI{bus_num} {signal} pin {base_pin} cannot reach {target_bus}",
                        resource=f"SPI{bus_num}"
                    ))
                    break
                pins_with_alt[signal] = correct_pin
            else:
                # All pins validated successfully
                validated_buses.append(ValidatedSPIBus(
                    bus_num=bus_num,
                    bus_name=target_bus,
                    mosi=pins_with_alt['MOSI'],
                    miso=pins_with_alt['MISO'],
                    sclk=pins_with_alt['SCLK']
                ))

        return validated_buses

    def validate_i2c_buses(self) -> List[ValidatedI2CBus]:
        """Validate I2C bus pin assignments."""
        validated_buses = []

        # Find all I2C buses in config
        i2c_resources = self.bf_config.get_resources('I2C_SCL')
        bus_numbers = [res.index for res in i2c_resources]

        for bus_num in bus_numbers:
            pins_bf = self.bf_config.get_i2c_pins(bus_num)
            if not pins_bf:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"I2C{bus_num} has incomplete pin assignments",
                    resource=f"I2C{bus_num}"
                ))
                continue

            # Convert to Arduino format (base pins, no ALT)
            pins_arduino_base = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Get correct pin format (with ALT if needed) for target bus
            target_bus = f"I2C{bus_num}"
            pins_with_alt = {}

            for signal, base_pin in pins_arduino_base.items():
                correct_pin = self.pinmap.get_pin_for_i2c_bus(base_pin, signal, target_bus)
                if not correct_pin:
                    self.errors.append(ValidationError(
                        severity="error",
                        message=f"I2C{bus_num} {signal} pin {base_pin} cannot reach {target_bus}",
                        resource=f"I2C{bus_num}"
                    ))
                    break
                pins_with_alt[signal] = correct_pin
            else:
                # All pins validated successfully
                validated_buses.append(ValidatedI2CBus(
                    bus_num=bus_num,
                    bus_name=target_bus,
                    scl=pins_with_alt['SCL'],
                    sda=pins_with_alt['SDA']
                ))

        return validated_buses

    def validate_uarts(self) -> List[ValidatedUART]:
        """Validate UART pin assignments."""
        validated_uarts = []

        # Find all UARTs in config
        uart_resources = self.bf_config.get_resources('SERIAL_TX')
        uart_numbers = [res.index for res in uart_resources]

        for uart_num in uart_numbers:
            pins_bf = self.bf_config.get_uart_pins(uart_num)
            if not pins_bf:
                self.warnings.append(ValidationError(
                    severity="warning",
                    message=f"UART{uart_num} has incomplete pin assignments (TX only?)",
                    resource=f"UART{uart_num}"
                ))
                continue

            # Convert to Arduino format (base pins, no ALT)
            pins_arduino_base = {
                signal: self.bf_config.convert_pin_format(pin)
                for signal, pin in pins_bf.items()
            }

            # Betaflight UART numbers don't always match hardware UART numbers
            # We need to find which UART these pins connect to
            # Try to get UART name from TX pin first
            first_uart = self.pinmap.get_uart(pins_arduino_base['TX'], 'TX')
            if not first_uart:
                self.errors.append(ValidationError(
                    severity="error",
                    message=f"UART{uart_num} TX pin {pins_arduino_base['TX']} not found in pinmap",
                    resource=f"UART{uart_num}"
                ))
                continue

            # Get correct pin format (with ALT if needed) for target UART
            pins_with_alt = {}

            for signal, base_pin in pins_arduino_base.items():
                correct_pin = self.pinmap.get_pin_for_uart(base_pin, signal, first_uart)
                if not correct_pin:
                    self.errors.append(ValidationError(
                        severity="error",
                        message=f"UART{uart_num} {signal} pin {base_pin} cannot reach {first_uart}",
                        resource=f"UART{uart_num}"
                    ))
                    break
                pins_with_alt[signal] = correct_pin
            else:
                # All pins validated successfully
                validated_uarts.append(ValidatedUART(
                    uart_num=uart_num,
                    uart_name=first_uart,
                    tx=pins_with_alt['TX'],
                    rx=pins_with_alt['RX']
                ))

        return validated_uarts

    def group_motors_by_timer(self, motors: List[ValidatedMotor]) -> Dict[str, List[ValidatedMotor]]:
        """Group motors by timer for bank generation."""
        timer_banks = {}

        for motor in motors:
            if motor.timer not in timer_banks:
                timer_banks[motor.timer] = []
            timer_banks[motor.timer].append(motor)

        return timer_banks

    def get_validation_summary(self) -> str:
        """Get human-readable validation summary."""
        lines = []
        lines.append(f"Validation Summary:")
        lines.append(f"  Errors: {len(self.errors)}")
        lines.append(f"  Warnings: {len(self.warnings)}")

        if self.errors:
            lines.append("\nErrors:")
            for err in self.errors:
                lines.append(f"  ❌ {err.message}")

        if self.warnings:
            lines.append("\nWarnings:")
            for warn in self.warnings:
                lines.append(f"  ⚠️  {warn.message}")

        return "\n".join(lines)
