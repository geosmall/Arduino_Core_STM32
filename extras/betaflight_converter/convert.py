#!/usr/bin/env python3
"""
Betaflight native target config.h to BoardConfig converter.

Usage:
    python3 convert.py bf_configs/JHEF411/              # Convert single target directory
    python3 convert.py bf_configs/JHEF411/config.h      # Convert single config.h file
    python3 convert.py --all                             # Convert all targets in bf_configs/
    python3 convert.py bf_configs/JHEF411/ output/X.h   # Custom output path
    python3 convert.py bf_configs/JHEF411/ --force       # Skip validation errors
"""

import sys
from pathlib import Path

# Add src to path
sys.path.insert(0, str(Path(__file__).parent / 'src'))

from betaflight_config import BetaflightConfig
from peripheral_pins import PeripheralPinMap
from validator import ConfigValidator
from code_generator import BoardConfigGenerator


# Map MCU type to variant paths (try in order until found)
MCU_TO_VARIANTS = {
    'STM32F411': [
        'STM32F4xx/F411C(C-E)(U-Y)',
    ],
    'STM32F405': [
        'STM32F4xx/F405RGT_F415RGT',
        'STM32F4xx/F405VGT_F415VGT',
        'STM32F4xx/F405ZGT_F415ZGT',
        'STM32F4xx/F405O(E-G)Y_F415OGY',
    ],
    'STM32F7X2': [
        'STM32F7xx/F722Z(C-E)T_F732ZET',
        'STM32F7xx/F722V(C-E)T_F730V8T_F732VET',
        'STM32F7xx/F722I(C-E)(K-T)_F732IE(K-T)',
        'STM32F7xx/F722R(C-E)T_F730R8T_F732RET',
    ],
    'STM32F745': [
        'STM32F7xx/F74xZ(G-I)',
    ],
    'STM32H743': [
        'STM32H7xx/H742V(G-I)(H-T)_H743V(G-I)(H-T)_H750VBT_H753VI(H-T)',
        'STM32H7xx/H742Z(G-I)T_H743Z(G-I)T_H747A(G-I)I_H747I(G-I)T_H750ZBT_H753ZIT_H757AII_H757IIT',
        'STM32H7xx/H742I(G-I)(K-T)_H743I(G-I)(K-T)_H750IB(K-T)_H753II(K-T)',
    ],
    'STM32G47X': [
        'STM32G4xx/G473C(B-C-E)U_G474C(B-C-E)U_G483CEU_G484CEU',
        'STM32G4xx/G473C(B-C-E)T_G474C(B-C-E)T_G483CET_G484CET',
        'STM32G4xx/G473R(B-C-E)T_G474R(B-C-E)T_G483RET_G484RET',
        'STM32G4xx/G473V(B-C-E)(H-T)_G474V(B-C-E)(H-T)_G483VE(H-T)_G484VE(H-T)',
    ],
}


def find_pinmap(mcu_type: str, arduino_root: Path) -> Path:
    """Find PeripheralPins.c for the given MCU type."""
    variant_paths = MCU_TO_VARIANTS.get(mcu_type)
    if not variant_paths:
        print(f"Error: Unsupported MCU type: {mcu_type}")
        print(f"Supported MCUs: {', '.join(MCU_TO_VARIANTS.keys())}")
        sys.exit(1)

    for variant_subpath in variant_paths:
        candidate = arduino_root / f"variants/{variant_subpath}/PeripheralPins.c"
        if candidate.exists():
            return candidate

    print(f"Error: No PeripheralPins.c found for {mcu_type}")
    print(f"Tried paths:")
    for variant_subpath in variant_paths:
        print(f"  - variants/{variant_subpath}/PeripheralPins.c")
    sys.exit(1)


def convert_target(config_path: Path, output_path: Path = None,
                   force_generate: bool = False) -> bool:
    """
    Convert a single Betaflight target to BoardConfig header.

    Args:
        config_path: Path to config.h file or target directory
        output_path: Output .h file path (auto-derived if None)
        force_generate: Continue even if validation fails

    Returns:
        True if successful, False if errors
    """
    # Load Betaflight config
    print(f"Loading Betaflight config: {config_path}")
    bf_config = BetaflightConfig(config_path)
    print(f"  Board: {bf_config.board_name}")
    print(f"  Manufacturer: {bf_config.manufacturer_id}")
    print(f"  MCU: {bf_config.mcu_type}")

    # Auto-derive output filename: MANUFACTURER-BOARD.h
    if output_path is None:
        output_name = f"{bf_config.manufacturer_id}-{bf_config.board_name}.h"
        output_path = Path(__file__).parent / "output" / output_name

    # Find and load PeripheralPins.c
    arduino_root = Path(__file__).parents[2]
    pinmap_path = find_pinmap(bf_config.mcu_type, arduino_root)
    print(f"Loading PeripheralPins.c: {pinmap_path}")
    pinmap = PeripheralPinMap(pinmap_path)

    # Resolve timer occurrences to actual timer/channel/AF
    bf_config.resolve_timers(pinmap)
    print(f"  Resolved {len(bf_config.timers)} timer assignments")

    # Validate configuration
    print("\nValidating configuration...")
    validator = ConfigValidator(bf_config, pinmap)
    if not validator.validate_all():
        print("\n❌ Validation failed:")
        print(validator.get_validation_summary())
        if not force_generate:
            return False
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
    return True


def main():
    if len(sys.argv) < 2:
        print("Usage: ./convert.py <target_dir_or_config.h> [output_file] [--force]")
        print("       ./convert.py --all [--force]")
        print("")
        print("Examples:")
        print("  ./convert.py bf_configs/JHEF411/")
        print("  ./convert.py bf_configs/JHEF411/config.h")
        print("  ./convert.py bf_configs/JHEF411/ output/CUSTOM.h")
        print("  ./convert.py --all")
        print("  ./convert.py --all --force")
        sys.exit(1)

    # Check for flags
    force_generate = '--force' in sys.argv
    convert_all = '--all' in sys.argv
    args = [arg for arg in sys.argv[1:] if not arg.startswith('--')]

    if convert_all:
        # Convert all targets in bf_configs/
        bf_configs_dir = Path(__file__).parent / "bf_configs"
        if not bf_configs_dir.exists():
            print(f"Error: bf_configs/ directory not found at {bf_configs_dir}")
            sys.exit(1)

        targets = sorted([d for d in bf_configs_dir.iterdir()
                         if d.is_dir() and (d / 'config.h').exists()])

        if not targets:
            print("No targets found in bf_configs/")
            sys.exit(1)

        print(f"Converting {len(targets)} targets...\n")
        results = []
        for target_dir in targets:
            print(f"{'='*60}")
            success = convert_target(target_dir, force_generate=force_generate)
            results.append((target_dir.name, success))
            print()

        # Summary
        print(f"{'='*60}")
        print("Summary:")
        for name, success in results:
            status = "✅" if success else "❌"
            print(f"  {status} {name}")

        failures = sum(1 for _, s in results if not s)
        if failures:
            sys.exit(1)
    else:
        # Convert single target
        config_path = Path(args[0])

        if not config_path.exists():
            print(f"Error: Path not found: {config_path}")
            sys.exit(1)

        # Custom output path
        output_path = Path(args[1]) if len(args) >= 2 else None

        if not convert_target(config_path, output_path, force_generate):
            sys.exit(1)


if __name__ == '__main__':
    main()
