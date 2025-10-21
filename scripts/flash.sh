#!/bin/bash
#
# J-Link flash script for STM32F411RE
# Usage: ./scripts/flash.sh [--quick] <path_to_binary.bin>
#
# --quick: Fast upload without erase/verify (development)
# default: Full cycle with erase/verify (production)
#

set -euo pipefail

# Parse arguments
QUICK_MODE=false
if [[ "${1:-}" == "--quick" ]]; then
    QUICK_MODE=true
    shift
fi

# Check arguments
if [ $# -ne 1 ]; then
    echo "J-Link Flash Tool - STM32F411RE"
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

# Check if binary exists
if [ ! -f "$BINARY_PATH" ]; then
    echo "Error: Binary file not found: $BINARY_PATH"
    exit 1
fi

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

# Execute flash operation with automation flags
START_TIME=$(date +%s)
JLinkExe -AutoConnect 1 -Device STM32F411RE -If SWD -Speed 4000 -CommandFile "$JLINK_SCRIPT"
END_TIME=$(date +%s)
FLASH_TIME=$((END_TIME - START_TIME))

if [ $? -eq 0 ]; then
    echo "✓ $OPERATION_TYPE successful: $BINARY_SIZE bytes in ${FLASH_TIME}s"
    echo "✓ Device reset and running"
else
    echo "✗ $OPERATION_TYPE failed"
    exit 1
fi