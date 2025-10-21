#!/bin/bash
#
# One-button build-flash-run harness
# Orchestrates: build → flash → RTT capture
# Usage: ./scripts/aflash.sh <sketch_directory> [FQBN] [rtt_duration]
#

set -euo pipefail

# Default configuration
DEFAULT_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
DEFAULT_TIMEOUT=60
DEFAULT_EXIT_WILDCARD="*STOP*"

# Parse arguments
SKETCH_DIR=""
FQBN="$DEFAULT_FQBN"
TIMEOUT="$DEFAULT_TIMEOUT"
EXIT_WILDCARD="$DEFAULT_EXIT_WILDCARD"
ENV_CHECK=false
USE_RTT=false
BUILD_ID=false
CLEAN_CACHE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --env-check)
            ENV_CHECK=true
            shift
            ;;
        --use-rtt)
            USE_RTT=true
            shift
            ;;
        --build-id)
            BUILD_ID=true
            shift
            ;;
        --clean-cache)
            CLEAN_CACHE=true
            shift
            ;;
        -*)
            echo "Unknown option $1"
            exit 1
            ;;
        *)
            if [[ -z "$SKETCH_DIR" ]]; then
                SKETCH_DIR="$1"
            elif [[ "$FQBN" == "$DEFAULT_FQBN" ]]; then
                FQBN="$1"
            elif [[ "$TIMEOUT" == "$DEFAULT_TIMEOUT" ]]; then
                TIMEOUT="$1"
            else
                EXIT_WILDCARD="$1"
            fi
            shift
            ;;
    esac
done

# Check arguments
if [[ -z "$SKETCH_DIR" ]]; then
    echo "One-Button Build-JRun-Test Harness with Environment Validation"
    echo "Usage: $0 <sketch_directory> [FQBN] [timeout_seconds] [exit_wildcard] [--env-check] [--use-rtt] [--build-id]"
    echo
    echo "Examples:"
    echo "  $0 HIL_RTT_Test"
    echo "  $0 HIL_RTT_Test --env-check"
    echo "  $0 HIL_RTT_Test --use-rtt --build-id"
    echo "  $0 HIL_RTT_Test STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE"
    echo "  $0 HIL_RTT_Test STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE 60 --env-check"
    echo "  $0 HIL_RTT_Test \"\" 60 \"*DONE*\" --env-check --use-rtt --build-id"
    echo
    echo "Default FQBN: $DEFAULT_FQBN"
    echo "Default timeout: ${DEFAULT_TIMEOUT}s (fallback for exit wildcard)"
    echo "Default exit wildcard: \"$DEFAULT_EXIT_WILDCARD\""
    echo
    echo "Options:"
    echo "  --env-check    Validate build environment before starting"
    echo "  --use-rtt      Enable RTT mode (USE_RTT compile flag)"
    echo "  --build-id     Generate build traceability header"
    echo
    echo "Note: Tests should emit exit wildcard for deterministic completion"
    exit 1
fi

# Validate sketch directory
if [ ! -d "$SKETCH_DIR" ]; then
    echo "Error: Sketch directory not found: $SKETCH_DIR"
    exit 1
fi

SKETCH_NAME=$(basename "$SKETCH_DIR")

# Optional environment validation
if [[ "$ENV_CHECK" == true ]]; then
    echo "=== Pre-Flight Environment Check ==="
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if ! "$SCRIPT_DIR/env_check_quick.sh" true; then
        echo "Environment validation failed. Run './scripts/env_probe.sh' for detailed diagnostics."
        exit 1
    fi
    echo "✓ Environment validated for HIL testing"
    echo
fi

echo "=== One-Button Build-JRun-Test: $SKETCH_NAME ==="
echo "FQBN: $FQBN"
echo "Timeout: ${TIMEOUT}s (fallback)"
echo "Exit wildcard: \"$EXIT_WILDCARD\""
if [[ "$ENV_CHECK" == true ]]; then
    echo "Environment check: ✓ Enabled"
fi
echo

# Step 1: Build (pass through env-check, use-rtt, and build-id flags if enabled)
echo "Step 1/2: Building..."
BUILD_ARGS="$SKETCH_DIR $FQBN"
if [[ "$ENV_CHECK" == true ]]; then
    BUILD_ARGS="$BUILD_ARGS --env-check"
fi
if [[ "$USE_RTT" == true ]]; then
    BUILD_ARGS="$BUILD_ARGS --use-rtt"
fi
if [[ "$BUILD_ID" == true ]]; then
    BUILD_ARGS="$BUILD_ARGS --build-id"
fi
if [[ "$CLEAN_CACHE" == true ]]; then
    BUILD_ARGS="$BUILD_ARGS --clean-cache"
fi
if ! ./scripts/build.sh $BUILD_ARGS; then
    echo "✗ Build failed"
    exit 1
fi
echo

# Find the ELF file (required for J-Run execution)
ELF_PATH=$(find /home/geo/.cache/arduino/sketches -name "${SKETCH_NAME}.ino.elf" | head -1)
if [ -z "$ELF_PATH" ]; then
    echo "✗ ELF file not found after build"
    echo ""
    echo "Expected location: /home/geo/.cache/arduino/sketches/${SKETCH_NAME}.ino.elf"
    echo ""
    echo "This indicates a build configuration issue. Possible causes:"
    echo "  • Arduino CLI not generating ELF files (check --export-binaries flag)"
    echo "  • Cache permissions issues (check ~/.cache/arduino/sketches/ access)"
    echo "  • Arduino CLI version mismatch (expected: 1.3.0)"
    echo ""
    echo "Troubleshooting steps:"
    echo "  1. Run: ./scripts/env_probe.sh (full environment diagnostics)"
    echo "  2. Check: ls -la /home/geo/.cache/arduino/sketches/"
    echo "  3. Verify: arduino-cli version (should be 1.3.0)"
    echo ""
    echo "Modern HIL testing requires ELF files for symbol-aware debugging"
    echo "and integrated J-Run execution with exit wildcard detection."
    exit 1
fi

echo "✓ Found ELF file: $(basename "$ELF_PATH")"

# Step 2: J-Run HIL test execution with exit wildcard detection
echo "Step 2/2: Executing HIL test with J-Run (exit wildcard detection)..."
LOG_PREFIX="${SKETCH_NAME}_aflash"
if ! ./scripts/jrun.sh "$ELF_PATH" STM32F411RE "$TIMEOUT" "$LOG_PREFIX" "$EXIT_WILDCARD"; then
    echo "✗ J-Run HIL test execution failed"
    exit 1
fi

echo
echo "=== Build-JRun-HIL-Test Complete ==="
echo "✓ Build: $(basename "$ELF_PATH") (ELF with symbols)"
echo "✓ J-Run: ELF loaded and executed with exit wildcard detection"

# Check if test completed deterministically
if grep -q "$EXIT_WILDCARD" test_logs/rtt/latest_jrun.txt 2>/dev/null; then
    echo "✓ HIL Test: Completed deterministically (exit wildcard detected)"
    echo "✓ Test Result: SUCCESS - deterministic test execution"
else
    echo "⚠ HIL Test: May be incomplete (exit wildcard not detected)"
    echo "⚠ Test Result: Check test implementation for exit wildcard emission"
fi
echo
echo "To view J-Run HIL test output: cat test_logs/rtt/latest_jrun.txt"