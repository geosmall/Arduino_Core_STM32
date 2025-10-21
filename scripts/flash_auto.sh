#!/bin/sh
#
# J-Link flash script with auto-detection for any STM32
# Usage: ./scripts/flash_auto.sh [--quick] <path_to_binary.bin>
#
# --quick: Fast upload without erase/verify (development)
# default: Full cycle with erase/verify (production)
#

set -eu

# Parse arguments
QUICK_MODE=false
if [ "${1:-}" = "--quick" ]; then
    QUICK_MODE=true
    shift
fi

# Check arguments
if [ $# -ne 1 ]; then
    echo "J-Link Flash Tool - Auto-Detecting STM32"
    echo "Usage: $0 [--quick] <path_to_binary.bin>"
    echo
    echo "Modes:"
    echo "  --quick    Fast upload (halt → load → reset → go) ~1s"
    echo "  default    Full cycle (halt → erase → load → verify → reset → go) ~9s"
    echo
    echo "Examples:"
    echo "  $0 --quick HIL_RTT_Test.ino.bin    # Fast development cycle"
    echo "  $0 HIL_RTT_Test.ino.bin             # Production flash cycle"
    exit 1
fi

BINARY_PATH="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Check if binary exists
if [ ! -f "$BINARY_PATH" ]; then
    echo "Error: Binary file not found: $BINARY_PATH"
    exit 1
fi

# Auto-detect device
echo "Step 1: Auto-detecting STM32 device..."
STM32_DEVICE_ID=""
eval $("$SCRIPT_DIR/detect_device.sh" | grep STM32_DEVICE_ID || true)

if [ -z "${STM32_DEVICE_ID:-}" ]; then
    echo "ERROR: Could not detect STM32 device"
    exit 1
fi

# Map device ID to optimal J-Link device name
case "$(printf "%03X" $((STM32_DEVICE_ID & 0xFFF)))" in
    413) JLINK_DEVICE="STM32F405RG" ;;  # F405/407 flight controllers
    431) JLINK_DEVICE="STM32F411RE" ;;  # F411 like yours
    450) JLINK_DEVICE="STM32H743ZI" ;;  # H7 high-performance
    468) JLINK_DEVICE="STM32G431RB" ;;  # G4 series
    *) JLINK_DEVICE="CORTEX-M4" ;;      # Generic fallback
esac

echo "Using J-Link device: $JLINK_DEVICE"

# Get binary info
BINARY_SIZE=$(stat -c%s "$BINARY_PATH")
BINARY_NAME=$(basename "$BINARY_PATH")

if [ "$QUICK_MODE" = true ]; then
    echo "Quick Upload: $BINARY_NAME ($BINARY_SIZE bytes)"
    echo "Operations: halt → load → reset → go"
    OPERATION_TYPE="Quick Upload"
else
    echo "Full Flash: $BINARY_NAME ($BINARY_SIZE bytes)"
    echo "Operations: halt → erase → load → verify → reset → go"
    OPERATION_TYPE="Full Flash"
fi

# Create J-Link script
JLINK_SCRIPT=$(mktemp /tmp/jlink_flash_XXXXXX.jlink)
trap "rm -f $JLINK_SCRIPT" EXIT

if [ "$QUICK_MODE" = true ]; then
    # Quick mode: no erase, no verify
    cat > "$JLINK_SCRIPT" << EOF
h
loadfile $BINARY_PATH 0x8000000
r
g
qc
EOF
else
    # Full mode: erase + verify
    cat > "$JLINK_SCRIPT" << EOF
h
erase
loadfile $BINARY_PATH 0x8000000
verifybin $BINARY_PATH 0x8000000
r
g
qc
EOF
fi

# Execute flash operation with detected device
START_TIME=$(date +%s)
JLinkExe -AutoConnect 1 -Device "$JLINK_DEVICE" -If SWD -Speed 4000 -CommandFile "$JLINK_SCRIPT"
END_TIME=$(date +%s)
FLASH_TIME=$((END_TIME - START_TIME))

if [ $? -eq 0 ]; then
    echo "✓ $OPERATION_TYPE successful: $BINARY_SIZE bytes in ${FLASH_TIME}s"
    echo "✓ Device reset and running"
else
    echo "✗ $OPERATION_TYPE failed"
    exit 1
fi