#!/bin/bash
#
# build_archive.sh - Build precompiled libraries for STM32 targets
#
# Builds .a archives for:
#   - cortex-m4: F4 (F411, F405) and G4 (G431, G474) families
#   - cortex-m7: F7 (F722, F746) and H7 (H743, H753) families
#
# Usage:
#   ./scripts/build_archive.sh
#   ./scripts/build_archive.sh --clean   # Clean and rebuild
#
# Output:
#   output/cortex-m4/fpv4-sp-d16-hard/libPrecompLib.a
#   output/cortex-m7/fpv4-sp-d16-hard/libPrecompLib.a

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$ROOT_DIR/src"
OUT_DIR="$ROOT_DIR/output"

# Clean if requested
if [[ "$1" == "--clean" ]]; then
    echo "Cleaning output directory..."
    rm -rf "$OUT_DIR"
fi

# Target configurations
# All families use fpv4-sp-d16 with hard float
TARGETS=(
    "cortex-m4"   # F4 + G4
    "cortex-m7"   # F7 + H7
)

FPU="fpv4-sp-d16"
FLOAT_ABI="hard"
FPU_DIR="${FPU}-${FLOAT_ABI}"

# Compiler settings (match Arduino Core platform.txt)
CC="arm-none-eabi-gcc"
AR="arm-none-eabi-gcc-ar"
CFLAGS="-mfpu=${FPU} -mfloat-abi=${FLOAT_ABI} -mthumb"
CFLAGS+=" -Os -ffunction-sections -fdata-sections"
CFLAGS+=" -fno-exceptions -fno-rtti"
CFLAGS+=" -Wall -Wextra"

echo "=== PrecompLib Build Script ==="
echo "Source: $SRC_DIR"
echo "Output: $OUT_DIR"
echo ""

# Verify toolchain
if ! command -v $CC &> /dev/null; then
    echo "ERROR: $CC not found in PATH"
    echo "Install ARM toolchain or add to PATH"
    exit 1
fi

# Verify source files exist
if [[ ! -f "$SRC_DIR/PrecompLib.cpp" ]]; then
    echo "ERROR: Source file not found: $SRC_DIR/PrecompLib.cpp"
    exit 1
fi

echo "Toolchain: $(which $CC)"
echo ""

# Build for each target
for MCU in "${TARGETS[@]}"; do
    echo "Building for ${MCU}..."

    TARGET_DIR="$OUT_DIR/${MCU}/${FPU_DIR}"
    mkdir -p "$TARGET_DIR"

    OBJ_FILE="$TARGET_DIR/PrecompLib.o"
    LIB_FILE="$TARGET_DIR/libPrecompLib.a"

    # Compile
    $CC -mcpu=${MCU} $CFLAGS \
        -c "$SRC_DIR/PrecompLib.cpp" \
        -o "$OBJ_FILE"

    # Create archive
    $AR rcs "$LIB_FILE" "$OBJ_FILE"

    # Remove intermediate object file
    rm -f "$OBJ_FILE"

    # Show result
    SIZE=$(stat --printf="%s" "$LIB_FILE" 2>/dev/null || stat -f%z "$LIB_FILE" 2>/dev/null)
    echo "  Created: $LIB_FILE ($SIZE bytes)"
done

echo ""
echo "=== Build Complete ==="
echo ""
echo "Output files:"
find "$OUT_DIR" -name "*.a" -exec ls -la {} \;
