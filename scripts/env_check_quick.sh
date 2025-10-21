#!/bin/bash
#
# env_check_quick.sh - Fast environment validation for build workflows
# Validates critical components without full environment probe overhead
#

set -euo pipefail

# Expected locked versions (from Phase 0)
EXPECTED_ARDUINO_CLI="1.3.0"
EXPECTED_STM32_CORE="2.7.1"
EXPECTED_GCC_MAJOR="12"

# Configuration
VERBOSE=${1:-false}
CANONICAL_FQBN="STMicroelectronics:stm32:Nucleo_64:pnum=NUCLEO_F411RE"

# Quick validation function
quick_check() {
    local component="$1"
    local check_cmd="$2"
    local expected="$3"
    
    if ! command -v $(echo "$check_cmd" | cut -d' ' -f1) >/dev/null 2>&1; then
        echo "✗ $component: Not found in PATH"
        return 1
    fi
    
    local actual
    actual=$(eval "$check_cmd" 2>/dev/null || echo "unknown")
    
    if [[ "$actual" == *"$expected"* ]]; then
        if [[ "$VERBOSE" == "true" ]]; then
            echo "✓ $component: $actual"
        fi
        return 0
    else
        echo "⚠ $component: Expected $expected, got $actual"
        return 1
    fi
}

# Main validation
ERRORS=0

# Arduino CLI check
if ! quick_check "Arduino CLI" "arduino-cli version --format json | grep VersionString | cut -d'\"' -f4" "$EXPECTED_ARDUINO_CLI"; then
    ((ERRORS++))
fi

# STM32 Core check  
if ! quick_check "STM32 Core" "arduino-cli core list | grep STMicroelectronics | awk '{print \$2}'" "$EXPECTED_STM32_CORE"; then
    ((ERRORS++))
fi

# GCC Toolchain check (optional - Arduino CLI manages its own toolchain)
if command -v arm-none-eabi-gcc >/dev/null 2>&1; then
    if ! quick_check "ARM GCC" "arm-none-eabi-gcc --version | head -1 | grep -o '[0-9]*\.[0-9]*\.[0-9]*' | head -1 | cut -d. -f1" "$EXPECTED_GCC_MAJOR"; then
        echo "⚠ ARM GCC version mismatch (non-critical - Arduino CLI manages toolchain)"
    fi
else
    if [[ "$VERBOSE" == "true" ]]; then
        echo "ℹ ARM GCC: Not in PATH (Arduino CLI manages toolchain)"
    fi
fi

# FQBN validation (critical for build success)
if command -v arduino-cli >/dev/null 2>&1; then
    if arduino-cli board details --fqbn "$CANONICAL_FQBN" >/dev/null 2>&1; then
        if [[ "$VERBOSE" == "true" ]]; then
            echo "✓ FQBN: $CANONICAL_FQBN"
        fi
    else
        echo "✗ FQBN: $CANONICAL_FQBN invalid or core not installed"
        ((ERRORS++))
    fi
else
    echo "✗ Cannot validate FQBN - arduino-cli not available"
    ((ERRORS++))
fi

# Summary
if [[ $ERRORS -eq 0 ]]; then
    if [[ "$VERBOSE" == "true" ]]; then
        echo "✓ Environment: All critical components validated"
    fi
    exit 0
else
    echo "✗ Environment: $ERRORS validation errors found"
    echo "Run './scripts/env_probe.sh' for detailed diagnostics"
    exit 1
fi