#!/usr/bin/env python3
"""
Betaflight unified target to BoardConfig converter.

Usage: python3 convert.py <config_file> <output_file>
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / 'src'))

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap
from validator import ConfigValidator
from code_generator import BoardConfigGenerator


def main():
    if len(sys.argv) < 2:
        print("Usage: ./convert.py <config_file> [output_file] [--force]")
        print("Example: ./convert.py data/JHEF-JHEF411.config")
        print("         ./convert.py data/JHEF-JHEF411.config output/CUSTOM_NAME.h")
        print("         ./convert.py data/JHEF-JHEF411.config --force  # Skip validation")
        sys.exit(1)

    # Check for --force flag
    force_generate = '--force' in sys.argv
    args = [arg for arg in sys.argv[1:] if arg != '--force']

    config_path = Path(args[0])

    # If output path not specified, derive from board name
    if len(args) >= 2:
        output_path = Path(args[1])
    else:
        output_path = None  # Will be set after loading config

    if not config_path.exists():
        print(f"Error: Config file not found: {config_path}")
        sys.exit(1)

    # Load Betaflight config
    print(f"Loading Betaflight config: {config_path}")
    bf_config = BetaflightConfig(config_path)
    print(f"  Board: {bf_config.board_name}")
    print(f"  Manufacturer: {bf_config.manufacturer_id}")
    print(f"  MCU: {bf_config.mcu_type}")

    # Set default output path from config filename if not specified
    # Matches madflight convention: JHEF-JHEF411.config → JHEF-JHEF411.h
    if output_path is None:
        config_basename = config_path.stem  # Remove .config extension
        output_path = Path(__file__).parent / "output" / f"{config_basename}.h"

    # Detect Arduino variant path from MCU type
    arduino_root = Path(__file__).parents[2]

    # Map MCU type to variant paths (try in order until found)
    mcu_to_variants = {
        'STM32F411': [
            'STM32F4xx/F411C(C-E)(U-Y)',
        ],
        'STM32F405': [
            'STM32F4xx/F405RG',
        ],
        'STM32F745': [
            'STM32F7xx/F74xZ(G-I)',
        ],
        'STM32H743': [
            'STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)_H750VBT_H753VI(H-T)',  # V package (LQFP100)
            'STM32H7xx/H742Z(G-I)T_H743Z(G-I)T_H747A(G-I)I_H747I(G-I)T_H750ZBT_H753ZIT_H757AII_H757IIT',  # Z package (LQFP144)
            'STM32H7xx/H742I(G-I)(K-T)_H743I(G-I)(K-T)_H750IB(K-T)_H753II(K-T)',  # I package (BGA)
        ],
    }

    variant_paths = mcu_to_variants.get(bf_config.mcu_type)
    if not variant_paths:
        print(f"Error: Unsupported MCU type: {bf_config.mcu_type}")
        print(f"Supported MCUs: {', '.join(mcu_to_variants.keys())}")
        sys.exit(1)

    # Try each variant path until we find one that exists
    pinmap_path = None
    for variant_subpath in variant_paths:
        candidate = arduino_root / f"Arduino_Core_STM32/variants/{variant_subpath}/PeripheralPins.c"
        if candidate.exists():
            pinmap_path = candidate
            break

    if not pinmap_path:
        print(f"Error: No PeripheralPins.c found for {bf_config.mcu_type}")
        print(f"Tried paths:")
        for variant_subpath in variant_paths:
            print(f"  - Arduino_Core_STM32/variants/{variant_subpath}/PeripheralPins.c")
        sys.exit(1)

    print(f"Loading PeripheralPins.c: {pinmap_path}")
    pinmap = PeripheralPinMap(pinmap_path)

    # Validate configuration
    print("\nValidating configuration...")
    validator = ConfigValidator(bf_config, pinmap)
    if not validator.validate_all():
        print("\n❌ Validation failed:")
        print(validator.get_validation_summary())
        if not force_generate:
            sys.exit(1)
        else:
            print("⚠️  Continuing with --force flag...")

    if validator.validate_all():
        print("✅ Validation passed")
        print(validator.get_validation_summary())

    # Generate code
    print(f"\nGenerating BoardConfig: {output_path}")
    generator = BoardConfigGenerator(bf_config, validator)

    # Create output directory if needed
    output_path.parent.mkdir(parents=True, exist_ok=True)

    # Save generated code
    generator.save(output_path)

    print(f"✅ Successfully generated: {output_path}")


if __name__ == '__main__':
    main()
