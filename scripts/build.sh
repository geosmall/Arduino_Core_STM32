#!/bin/bash
#
# Arduino CLI build script with optional environment validation
# Usage: ./scripts/build.sh <sketch_directory> [FQBN] [--env-check] [--build-id] [--use-rtt] [--clean-cache]
#

set -euo pipefail

# Default configuration
DEFAULT_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"

# Parse arguments
SKETCH_DIR=""
FQBN="$DEFAULT_FQBN"
ENV_CHECK=false
BUILD_ID=false
USE_RTT=false
CLEAN_CACHE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --env-check)
            ENV_CHECK=true
            shift
            ;;
        --build-id)
            BUILD_ID=true
            shift
            ;;
        --use-rtt)
            USE_RTT=true
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
            else
                FQBN="$1"
            fi
            shift
            ;;
    esac
done

# Check arguments
if [[ -z "$SKETCH_DIR" ]]; then
    echo "Arduino CLI Build Script with Environment Validation"
    echo "Usage: $0 <sketch_directory> [FQBN] [--env-check] [--build-id] [--use-rtt] [--clean-cache]"
    echo
    echo "Examples:"
    echo "  $0 HIL_RTT_Test"
    echo "  $0 HIL_RTT_Test --env-check"
    echo "  $0 HIL_RTT_Test --build-id"
    echo "  $0 HIL_RTT_Test --use-rtt"
    echo "  $0 HIL_RTT_Test --env-check --build-id --use-rtt"
    echo "  $0 HIL_RTT_Test STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE"
    echo
    echo "Default FQBN: $DEFAULT_FQBN"
    exit 1
fi

# Optional environment validation
if [[ "$ENV_CHECK" == true ]]; then
    echo "=== Environment Validation ==="
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if ! "$SCRIPT_DIR/env_check_quick.sh" true; then
        echo "Environment validation failed. Run './scripts/env_probe.sh' for detailed diagnostics."
        exit 1
    fi
    echo "✓ Environment validated"
    echo
fi

# Optional build-ID header generation
if [[ "$BUILD_ID" == true ]]; then
    echo "=== Build ID Generation ==="
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    if "$SCRIPT_DIR/generate_build_id.sh" "$SKETCH_DIR"; then
        echo "✓ Build ID header generated"
    else
        echo "⚠ Build ID generation failed (continuing with build)"
    fi
    echo
fi

# Validate sketch directory
if [ ! -d "$SKETCH_DIR" ]; then
    echo "Error: Sketch directory not found: $SKETCH_DIR"
    exit 1
fi

# Find the .ino file
INO_FILE=$(find "$SKETCH_DIR" -name "*.ino" | head -1)
if [ -z "$INO_FILE" ]; then
    echo "Error: No .ino file found in $SKETCH_DIR"
    exit 1
fi

SKETCH_NAME=$(basename "$INO_FILE" .ino)
echo "Building sketch: $SKETCH_NAME"
echo "FQBN: $FQBN"
echo "Directory: $SKETCH_DIR"

# Clean cache if requested
if [[ "$CLEAN_CACHE" == true ]]; then
    echo "Cleaning Arduino CLI cache..."
    rm -rf ~/.cache/arduino/sketches/* 2>/dev/null || true
    echo "✓ Cache cleaned"
fi

# Build with export binaries for J-Link upload
echo "Starting arduino-cli compile..."
START_TIME=$(date +%s)

# Prepare build command with individual --build-property flags
BUILD_CMD="arduino-cli compile --fqbn \"$FQBN\" --export-binaries"
BUILD_CMD="$BUILD_CMD --build-property \"compiler.cpp.extra_flags=-DBUILD_TIMESTAMP=$(date +%s)\""

if [[ "$USE_RTT" == true ]]; then
    BUILD_CMD="$BUILD_CMD --build-property \"compiler.cpp.extra_flags=-DBUILD_TIMESTAMP=$(date +%s) -DUSE_RTT\""
    echo "Mode: J-Run/RTT (USE_RTT enabled)"
else
    echo "Mode: Arduino IDE (Serial output)"
fi

BUILD_CMD="$BUILD_CMD \"$SKETCH_DIR\""

eval $BUILD_CMD

END_TIME=$(date +%s)
BUILD_TIME=$((END_TIME - START_TIME))

# Find the generated binary
BINARY_PATH=$(find "$SKETCH_DIR/build" -name "*.bin" | head -1)
if [ -z "$BINARY_PATH" ]; then
    echo "Error: No binary file found after build"
    exit 1
fi

BINARY_SIZE=$(stat -c%s "$BINARY_PATH")
echo
echo "✓ Build successful:"
echo "  Binary: $BINARY_PATH"
echo "  Size: $BINARY_SIZE bytes"
echo "  Build time: ${BUILD_TIME}s"
echo
echo "Ready for upload with: ./scripts/flash.sh --quick $BINARY_PATH"