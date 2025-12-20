#!/bin/sh
# UF2 Upload Script for UF2 Bootloader
# Usage: uf2_upload.sh <binary.bin> <flash_offset>
#
# This script:
# 1. Converts .bin to .uf2 format using uf2conv.py
# 2. Copies .uf2 file to the UF2 bootloader drive
#
# Requires: Device must already be in bootloader mode
# Compatible with: bash, dash, ash, BusyBox sh

BIN_FILE="$1"
FLASH_OFFSET="$2"

# STM32F4 family ID for UF2
UF2_FAMILY="0x57755a57"

# Detect OS
detect_os() {
    case "$(uname -s)" in
        Linux*)  echo "linux" ;;
        Darwin*) echo "macos" ;;
        MINGW*|MSYS*|CYGWIN*) echo "windows" ;;
        *)       echo "unknown" ;;
    esac
}

OS_TYPE=$(detect_os)

# Find UF2 drive based on OS
find_uf2_drive() {
    case "$OS_TYPE" in
        linux)
            for mount in /media/$USER/*BOOT* /media/*BOOT* /run/media/$USER/*BOOT* /mnt/*BOOT*; do
                if [ -d "$mount" ] && [ -f "$mount/INFO_UF2.TXT" ]; then
                    echo "$mount"
                    return 0
                fi
            done
            ;;
        macos)
            for mount in /Volumes/*BOOT*; do
                if [ -d "$mount" ] && [ -f "$mount/INFO_UF2.TXT" ]; then
                    echo "$mount"
                    return 0
                fi
            done
            ;;
        windows)
            for letter in C D E F G H I J K; do
                drive="/$letter"
                if [ -f "$drive/INFO_UF2.TXT" ]; then
                    echo "$drive"
                    return 0
                fi
                drive="/$(echo $letter | tr 'A-Z' 'a-z')"
                if [ -f "$drive/INFO_UF2.TXT" ]; then
                    echo "$drive"
                    return 0
                fi
            done
            ;;
    esac
    return 1
}

# Get file size portably
get_file_size() {
    wc -c < "$1" | tr -d ' '
}

# Validate input
if [ ! -f "$BIN_FILE" ]; then
    echo "Error: Binary file not found: $BIN_FILE"
    exit 1
fi

# Generate .uf2 filename
UF2_FILE="${BIN_FILE%.bin}.uf2"

# Find uf2conv.py
UF2CONV=""
SCRIPT_DIR="$(dirname "$0")"
for path in \
    "$SCRIPT_DIR/uf2conv.py" \
    "$SCRIPT_DIR/../uf2conv.py" \
    "/usr/local/bin/uf2conv.py" \
    "$HOME/.local/bin/uf2conv.py"; do
    if [ -f "$path" ]; then
        UF2CONV="$path"
        break
    fi
done

if [ -z "$UF2CONV" ]; then
    echo "Error: uf2conv.py not found"
    exit 1
fi

echo ""
echo "Converting to UF2 format..."
echo "  Input:  $(basename "$BIN_FILE")"
echo "  Offset: $FLASH_OFFSET"

# Convert offset to absolute address for STM32 flash (base = 0x08000000)
FLASH_BASE=0x08000000
FLASH_ADDR=$(printf "0x%08X" $((FLASH_BASE + FLASH_OFFSET)))
echo "  Address: $FLASH_ADDR"

# Convert to UF2
python3 "$UF2CONV" -f "$UF2_FAMILY" -b "$FLASH_ADDR" -c -o "$UF2_FILE" "$BIN_FILE" > /dev/null 2>&1
CONV_RESULT=$?

if [ $CONV_RESULT -ne 0 ] || [ ! -f "$UF2_FILE" ]; then
    echo "Error: UF2 conversion failed"
    exit 1
fi

UF2_SIZE=$(get_file_size "$UF2_FILE")
echo "  Output: $(basename "$UF2_FILE") ($UF2_SIZE bytes)"
echo ""

# Check if UF2 drive exists
UF2_DRIVE=$(find_uf2_drive) || true

if [ -z "$UF2_DRIVE" ]; then
    echo "Error: UF2 drive not found. Enter bootloader first."
    exit 1
fi

echo "Uploading to $UF2_DRIVE..."

# Copy UF2 to drive
if ! cp "$UF2_FILE" "$UF2_DRIVE/"; then
    echo "Error: Failed to copy UF2 file to drive"
    exit 1
fi

# Sync to ensure write completes
sync 2>/dev/null || true

# Wait for bootloader to finish and reset
echo "  Waiting for flash write to complete..."
WAIT_COUNT=0
while [ $WAIT_COUNT -lt 10 ]; do
    sleep 0.5
    if [ ! -d "$UF2_DRIVE" ]; then
        break
    fi
    WAIT_COUNT=$((WAIT_COUNT + 1))
done

echo ""
echo "Upload complete!"
echo ""
exit 0
