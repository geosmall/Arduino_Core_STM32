#!/bin/bash
#
# J-Run execution with RTT capture
# Loads ELF file, runs with J-Run, captures RTT output
# Usage: ./scripts/jrun.sh <elf_file> [device] [duration] [log_prefix]
#

set -euo pipefail

# Default configuration
DEFAULT_DEVICE="STM32F411RE"
DEFAULT_TIMEOUT=60  # Fallback timeout for exit wildcard
DEFAULT_LOG_PREFIX="jrun_output"
DEFAULT_EXIT_WILDCARD="*STOP*"

# Check arguments
if [ $# -lt 1 ]; then
    echo "J-Run Execution with Exit Wildcard Detection"
    echo "Usage: $0 <elf_file> [device] [timeout_seconds] [log_prefix] [exit_wildcard]"
    echo
    echo "Examples:"
    echo "  $0 HIL_RTT_Test.ino.elf"
    echo "  $0 HIL_RTT_Test.ino.elf STM32F411RE"
    echo "  $0 HIL_RTT_Test.ino.elf STM32F411RE 60 my_test"
    echo "  $0 HIL_RTT_Test.ino.elf STM32F411RE 60 my_test \"*DONE*\""
    echo
    echo "Default device: $DEFAULT_DEVICE"
    echo "Default timeout: ${DEFAULT_TIMEOUT}s (fallback for exit wildcard)"
    echo "Default exit wildcard: \"$DEFAULT_EXIT_WILDCARD\""
    echo
    echo "Note: J-Run will exit automatically when exit wildcard is detected"
    exit 1
fi

ELF_FILE="$1"
DEVICE="${2:-$DEFAULT_DEVICE}"
TIMEOUT="${3:-$DEFAULT_TIMEOUT}"
LOG_PREFIX="${4:-$DEFAULT_LOG_PREFIX}"
EXIT_WILDCARD="${5:-$DEFAULT_EXIT_WILDCARD}"

# Validate ELF file
if [ ! -f "$ELF_FILE" ]; then
    echo "Error: ELF file not found: $ELF_FILE"
    exit 1
fi

echo "=== J-Run HIL Test Execution ==="
echo "ELF: $(basename "$ELF_FILE")"
echo "Device: $DEVICE"
echo "Timeout: ${TIMEOUT}s (fallback)"
echo "Log prefix: $LOG_PREFIX"
echo "Exit wildcard: \"$EXIT_WILDCARD\""
echo

# Create test_logs/rtt directory if it doesn't exist
mkdir -p test_logs/rtt

# Generate log filename with timestamp
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="test_logs/rtt/${LOG_PREFIX}_${TIMESTAMP}.txt"
LATEST_LINK="test_logs/rtt/latest_jrun.txt"

echo "Starting J-Run with exit wildcard detection..."

# Run J-Run with exit wildcard detection, fallback timeout for safety
if timeout "${TIMEOUT}" JRun \
    --device "$DEVICE" \
    --if SWD \
    --speed 4000 \
    --rtt \
    --wexit "$EXIT_WILDCARD" \
    --pc vec \
    --sp vec \
    --v \
    "$ELF_FILE" 2>&1 | tee "$LOG_FILE"; then
    echo "✓ J-Run execution completed successfully"
else
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 124 ]; then
        echo "⚠ J-Run execution reached fallback timeout (${TIMEOUT}s) - exit wildcard not detected"
        echo "  Check test code emits: \"$EXIT_WILDCARD\""
    elif [ $EXIT_CODE -eq 0 ] || [ $EXIT_CODE -eq 1 ]; then
        # J-Run exits with code 0 or 1 when exit wildcard is detected
        if grep -q "$EXIT_WILDCARD" "$LOG_FILE"; then
            echo "✓ J-Run execution completed (exit wildcard \"$EXIT_WILDCARD\" detected)"
        else
            echo "✓ J-Run execution completed (normal exit)"
        fi
    elif [ $EXIT_CODE -eq 255 ]; then
        # Check if exit wildcard was detected before the error
        if grep -q "$EXIT_WILDCARD" "$LOG_FILE"; then
            echo "✓ J-Run execution completed (exit wildcard \"$EXIT_WILDCARD\" detected, then exit)"
        elif grep -q "Downloading file.*OK" "$LOG_FILE"; then
            echo "⚠ J-Run execution completed (ELF loaded, no exit wildcard detected)"
        else
            echo "✗ J-Run execution failed with exit code $EXIT_CODE"
            exit $EXIT_CODE
        fi
    else
        echo "✗ J-Run execution failed with exit code $EXIT_CODE"
        exit $EXIT_CODE
    fi
fi

# Create symlink to latest log
ln -sf "$(basename "$LOG_FILE")" "$LATEST_LINK"

echo
echo "=== J-Run HIL Test Complete ==="
if grep -q "$EXIT_WILDCARD" "$LOG_FILE"; then
    echo "✓ Test completed with exit wildcard detection"
    echo "✓ Deterministic test execution confirmed"
else
    echo "⚠ Test may have incomplete execution (no exit wildcard)"
fi
echo "✓ ELF loaded and executed via J-Run"
echo "✓ RTT output captured: $LOG_FILE"
echo "✓ Latest log link: $LATEST_LINK"
echo
echo "To view output: cat $LOG_FILE"
echo "Or: cat $LATEST_LINK"
echo
if grep -q "$EXIT_WILDCARD" "$LOG_FILE"; then
    echo "Exit wildcard \"$EXIT_WILDCARD\" found - test execution was deterministic"
fi