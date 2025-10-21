#!/bin/bash
#
# RTT logger with timestamps - captures RTT output to log files
# Usage: ./scripts/rtt_cat.sh [duration_seconds] [log_prefix]
#
# ⚠️  DEPRECATED: This script is deprecated in favor of jrun.sh
# jrun.sh provides superior RTT integration with J-Run execution
# Use: ./scripts/jrun.sh <elf> [timeout] [exit_wildcard]
#

set -euo pipefail

# Default configuration
DEFAULT_DURATION=30
DEFAULT_LOG_PREFIX="rtt_log"

# Parse arguments
DURATION="${1:-$DEFAULT_DURATION}"
LOG_PREFIX="${2:-$DEFAULT_LOG_PREFIX}"

# Create log directory
LOG_DIR="test_logs/rtt"
mkdir -p "$LOG_DIR"

# Generate log filename with timestamp
TIMESTAMP=$(date +%Y%m%d_%H%M%S_UTC)
LOG_FILE="$LOG_DIR/${LOG_PREFIX}_${TIMESTAMP}.txt"

echo "RTT Logger starting..."
echo "Duration: ${DURATION}s"
echo "Log file: $LOG_FILE"
echo "Press Ctrl+C to stop early"
echo

# Start J-Link GDB Server in background if not running
if ! pgrep -f "JLinkGDBServer.*RTTTelnetPort" > /dev/null; then
    echo "Starting J-Link GDB Server..."
    JLinkGDBServer -Device STM32F411RE -If SWD -Speed 4000 -RTTTelnetPort 19021 > /dev/null 2>&1 &
    GDBSERVER_PID=$!
    echo "GDB Server PID: $GDBSERVER_PID"
    sleep 3  # Wait for server to initialize
    STARTED_GDBSERVER=1
else
    echo "J-Link GDB Server already running"
    STARTED_GDBSERVER=0
fi

# Cleanup function
cleanup() {
    echo
    echo "Stopping RTT capture..."
    if [ "$STARTED_GDBSERVER" -eq 1 ] && [ -n "${GDBSERVER_PID:-}" ]; then
        kill $GDBSERVER_PID 2>/dev/null || true
        echo "Stopped GDB Server"
    fi
    if [ -f "$LOG_FILE" ]; then
        LOG_SIZE=$(stat -c%s "$LOG_FILE")
        echo "Log saved: $LOG_FILE ($LOG_SIZE bytes)"
        
        # Create symlink to latest log
        ln -sf "$(basename "$LOG_FILE")" "$LOG_DIR/latest_rtt.txt"
        echo "Latest log: $LOG_DIR/latest_rtt.txt"
        
        # Show last few lines
        echo
        echo "Last 5 lines of RTT output:"
        tail -5 "$LOG_FILE" | sed 's/^/  /'
    fi
}

trap cleanup EXIT INT TERM

# Start RTT capture with timeout and timestamping
echo "Connecting to RTT..."
{
    echo "=== RTT Capture Started: $(date -u +%Y-%m-%d\ %H:%M:%S\ UTC) ==="
    timeout "${DURATION}s" JLinkRTTClient 2>/dev/null | while IFS= read -r line; do
        # Skip RTT client header messages
        if [[ "$line" == *"###RTT Client:"* ]] || [[ "$line" == *"*****"* ]] || [[ "$line" == *"SEGGER"* ]] || [[ "$line" == *"www.segger.com"* ]] || [[ "$line" == *"Compiled"* ]] || [[ "$line" == *"Connecting"* ]] || [[ "$line" == *"Connected"* ]] || [[ "$line" == *"terminal output"* ]] || [[ "$line" == *"Process:"* ]]; then
            continue
        fi
        
        # Add timestamp to each line of actual RTT output
        printf "[%s] %s\n" "$(date -u +%H:%M:%S.%3N)" "$line"
    done
    echo "=== RTT Capture Ended: $(date -u +%Y-%m-%d\ %H:%M:%S\ UTC) ==="
} | tee "$LOG_FILE"

echo "RTT capture completed."