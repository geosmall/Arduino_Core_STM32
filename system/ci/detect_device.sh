#!/bin/bash
# Auto-detect STM32 device via J-Link for HIL CI/CD
# No prior device knowledge required

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JLINK_SCRIPT="$SCRIPT_DIR/detect_stm32.jlink"

echo "Auto-detecting STM32 device via J-Link..."

# Run J-Link detection script
output=$(JLinkExe -AutoConnect 1 -If SWD -Speed 4000 -CommandFile "$JLINK_SCRIPT" 2>&1)

# Extract device ID from DBGMCU_IDCODE register (handles both hex and decimal format)
device_id=$(echo "$output" | grep -E "E0042000.*=" | sed -E 's/.*E0042000 = ([0-9A-Fa-f]+).*/\1/')

if [ -z "$device_id" ]; then
    echo "ERROR: Could not detect device ID"
    echo "J-Link output:"
    echo "$output"
    exit 1
fi

echo "Device ID: 0x$device_id"

# Decode STM32 device IDs (extract device ID from bits [11:0])
device_only_id=$(printf "%03X" $((0x$device_id & 0xFFF)))
case "$device_only_id" in
    410) echo "Detected: STM32F1xx medium-density" ;;
    411) echo "Detected: STM32F2xx" ;;
    412) echo "Detected: STM32F1xx low-density" ;;
    413) echo "Detected: STM32F405xx/407xx/415xx/417xx" ;;
    414) echo "Detected: STM32F1xx high-density" ;;
    418) echo "Detected: STM32F1xx connectivity line" ;;
    419) echo "Detected: STM32F4xx high-density" ;;
    420) echo "Detected: STM32F1xx value line" ;;
    421) echo "Detected: STM32F446xx" ;;
    422) echo "Detected: STM32F3xx" ;;
    431) echo "Detected: STM32F411xC/E (like F411RE)" ;;
    440) echo "Detected: STM32F030x8" ;;
    441) echo "Detected: STM32F031xx" ;;
    442) echo "Detected: STM32F09xxx" ;;
    444) echo "Detected: STM32F03xx" ;;
    445) echo "Detected: STM32F04xx" ;;
    448) echo "Detected: STM32F07xxx" ;;
    449) echo "Detected: STM32F0x0 Value line" ;;
    450) echo "Detected: STM32H74x/H75xx (H742/H743/H750/H753)" ;;
    451) echo "Detected: STM32F76xxx/F77xxx" ;;
    452) echo "Detected: STM32F72xxx/F73xxx" ;;
    458) echo "Detected: STM32F410xx" ;;
    460) echo "Detected: STM32G07xxx/G08xxx" ;;
    463) echo "Detected: STM32F413xx/F423xx" ;;
    466) echo "Detected: STM32G03xxx/G04xxx" ;;
    468) echo "Detected: STM32G431xx/G441xx/G491xx/G4A1xx" ;;
    469) echo "Detected: STM32G471xx/G473xx/G474xx/G484xx" ;;
    479) echo "Detected: STM32G491xx/G4A1xx" ;;
    480) echo "Detected: STM32H7A3xx/H7B0xx/H7B3xx" ;;
    483) echo "Detected: STM32H72xxx/H73xxx" ;;
    *) echo "Unknown STM32 device (ID: 0x$device_only_id)" ;;
esac

# Export for use by other scripts
export STM32_DEVICE_ID="0x$device_id"
echo "STM32_DEVICE_ID=$STM32_DEVICE_ID"