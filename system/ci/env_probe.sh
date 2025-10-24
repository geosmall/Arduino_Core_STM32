#!/bin/bash

# env_probe.sh - Capture build environment state for Phase 0
# Part of Build Workflow project - deterministic environment snapshot

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ARDUINO_DIR="$(dirname "$SCRIPT_DIR")"
LOG_DIR="$ARDUINO_DIR/test_logs/env"

# Ensure log directory exists
mkdir -p "$LOG_DIR"

# Generate timestamp for this probe session
TIMESTAMP=$(date -u +"%Y%m%d_%H%M%S_UTC")
PROBE_LOG="$LOG_DIR/env_probe_$TIMESTAMP.txt"

echo "=== Arduino Build Environment Probe ===" | tee "$PROBE_LOG"
echo "Timestamp: $(date -u --iso-8601=seconds)" | tee -a "$PROBE_LOG"
echo "Working Directory: $ARDUINO_DIR" | tee -a "$PROBE_LOG"
echo "" | tee -a "$PROBE_LOG"

# Arduino CLI Version
echo "=== Arduino CLI ===" | tee -a "$PROBE_LOG"
if command -v arduino-cli >/dev/null 2>&1; then
    arduino-cli version | tee -a "$PROBE_LOG"
else
    echo "ERROR: arduino-cli not found in PATH" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# STM32 Core Version
echo "=== Installed Cores ===" | tee -a "$PROBE_LOG"
if command -v arduino-cli >/dev/null 2>&1; then
    arduino-cli core list | tee -a "$PROBE_LOG"
    echo "" | tee -a "$PROBE_LOG"
    
    # Check for STM32 core specifically
    echo "STM32 Core Details:" | tee -a "$PROBE_LOG"
    arduino-cli core list | grep -i stm32 || echo "No STM32 core found" | tee -a "$PROBE_LOG"
else
    echo "Cannot check cores - arduino-cli not available" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Canonical FQBN
echo "=== Target Configuration ===" | tee -a "$PROBE_LOG"
CANONICAL_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"
echo "Canonical FQBN: $CANONICAL_FQBN" | tee -a "$PROBE_LOG"

# Verify FQBN is valid
if command -v arduino-cli >/dev/null 2>&1; then
    echo "FQBN Validation:" | tee -a "$PROBE_LOG"
    if arduino-cli board details --fqbn "$CANONICAL_FQBN" >/dev/null 2>&1; then
        echo "✓ FQBN valid" | tee -a "$PROBE_LOG"
    else
        echo "✗ FQBN invalid or core not installed" | tee -a "$PROBE_LOG"
    fi
else
    echo "Cannot validate FQBN - arduino-cli not available" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Git Status
echo "=== Git Repository Status ===" | tee -a "$PROBE_LOG"
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Git SHA: $(git rev-parse HEAD)" | tee -a "$PROBE_LOG"
    echo "Git Branch: $(git rev-parse --abbrev-ref HEAD)" | tee -a "$PROBE_LOG"
    echo "Git Status:" | tee -a "$PROBE_LOG"
    git status --porcelain | tee -a "$PROBE_LOG"
    if [[ -z $(git status --porcelain) ]]; then
        echo "✓ Working directory clean" | tee -a "$PROBE_LOG"
    else
        echo "⚠ Working directory has changes" | tee -a "$PROBE_LOG"
    fi
else
    echo "Not in a git repository" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Arduino_Core_STM32 Symlink Check
echo "=== Arduino Core STM32 Integration ===" | tee -a "$PROBE_LOG"
CORE_PATH="$ARDUINO_DIR/Arduino_Core_STM32"
if [[ -L "$CORE_PATH" ]]; then
    echo "✓ Arduino_Core_STM32 is a symlink" | tee -a "$PROBE_LOG"
    echo "Symlink target: $(readlink -f "$CORE_PATH")" | tee -a "$PROBE_LOG"
elif [[ -d "$CORE_PATH" ]]; then
    echo "⚠ Arduino_Core_STM32 is a directory (not symlinked)" | tee -a "$PROBE_LOG"
else
    echo "✗ Arduino_Core_STM32 not found" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Available Boards for STM32
echo "=== Available STM32 Boards ===" | tee -a "$PROBE_LOG"
if command -v arduino-cli >/dev/null 2>&1; then
    arduino-cli board listall | grep -i stm32 | head -10 | tee -a "$PROBE_LOG"
    echo "..." | tee -a "$PROBE_LOG"
else
    echo "Cannot list boards - arduino-cli not available" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Connected Hardware
echo "=== Connected Hardware ===" | tee -a "$PROBE_LOG"
if command -v arduino-cli >/dev/null 2>&1; then
    arduino-cli board list | tee -a "$PROBE_LOG"
else
    echo "Cannot detect connected boards - arduino-cli not available" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# USB Device List (for debugging connection issues)
echo "=== USB Devices (for debug) ===" | tee -a "$PROBE_LOG"
if command -v lsusb >/dev/null 2>&1; then
    lsusb | grep -E "(STM|ST-LINK|Debug|Serial)" | tee -a "$PROBE_LOG" || echo "No relevant USB devices found" | tee -a "$PROBE_LOG"
else
    echo "lsusb not available" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# Toolchain Info
echo "=== Toolchain ===" | tee -a "$PROBE_LOG"
if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    echo "GCC Version: $(arm-none-eabi-gcc --version | head -1)" | tee -a "$PROBE_LOG"
else
    echo "arm-none-eabi-gcc not found in PATH" | tee -a "$PROBE_LOG"
fi
echo "" | tee -a "$PROBE_LOG"

# System Info
echo "=== System Environment ===" | tee -a "$PROBE_LOG"
echo "OS: $(uname -a)" | tee -a "$PROBE_LOG"
echo "User: $(whoami)" | tee -a "$PROBE_LOG"
echo "Home: $HOME" | tee -a "$PROBE_LOG"
echo "PATH: $PATH" | tee -a "$PROBE_LOG"
echo "" | tee -a "$PROBE_LOG"

echo "=== Probe Complete ===" | tee -a "$PROBE_LOG"
echo "Log saved to: $PROBE_LOG" | tee -a "$PROBE_LOG"

# Create a symlink to the latest probe for easy access
ln -sf "$(basename "$PROBE_LOG")" "$LOG_DIR/latest_probe.txt"

echo ""
echo "Environment probe complete. Latest results:"
echo "  Log: $PROBE_LOG"
echo "  Link: $LOG_DIR/latest_probe.txt"