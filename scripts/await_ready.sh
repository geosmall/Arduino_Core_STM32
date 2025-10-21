#!/bin/bash
#
# await_ready.sh - Wait for HIL ready token with timeout/backoff
# Usage: ./scripts/await_ready.sh [log_file] [timeout] [ready_pattern]
#

set -euo pipefail

# Default configuration
DEFAULT_TIMEOUT=30
DEFAULT_READY_PATTERN="READY"
DEFAULT_LOG_FILE="test_logs/rtt/latest_jrun.txt"

# Parse arguments
LOG_FILE="${1:-$DEFAULT_LOG_FILE}"
TIMEOUT="${2:-$DEFAULT_TIMEOUT}"
READY_PATTERN="${3:-$DEFAULT_READY_PATTERN}"

# Check for help
if [[ "$LOG_FILE" == "-h" || "$LOG_FILE" == "--help" ]]; then
    echo "HIL Ready Token Detection with Latency Statistics"
    echo "Usage: $0 [log_file] [timeout_seconds] [ready_pattern]"
    echo
    echo "Examples:"
    echo "  $0                                    # Use latest J-Run log"
    echo "  $0 test_logs/rtt/my_test.txt         # Specific log file"
    echo "  $0 test_logs/rtt/my_test.txt 60      # Custom timeout"
    echo "  $0 test_logs/rtt/my_test.txt 60 READY  # Custom pattern"
    echo
    echo "Default log file: $DEFAULT_LOG_FILE"
    echo "Default timeout: ${DEFAULT_TIMEOUT}s"
    echo "Default pattern: \"$DEFAULT_READY_PATTERN\""
    echo
    echo "Enhanced Build-ID Token Format:"
    echo "  READY <board> <git_sha> <utc_timestamp>"
    echo "  Example: READY NUCLEO_F411RE 901dbd1 2025-09-09T09:33:40Z"
    echo
    echo "Returns:"
    echo "  0 - Ready token detected successfully"
    echo "  1 - Timeout waiting for ready token"
    echo "  2 - Log file not found or invalid"
    exit 0
fi

echo "=== HIL Ready Token Detection ==="
echo "Log file: $LOG_FILE"
echo "Timeout: ${TIMEOUT}s"
echo "Pattern: \"$READY_PATTERN\""
echo

# Record start time for latency measurement
START_TIME=$(date +%s.%N)
START_TIME_HUMAN=$(date +"%H:%M:%S.%3N")

echo "[$START_TIME_HUMAN] Waiting for ready token..."

# Wait for ready token with exponential backoff
BACKOFF=0.1
MAX_BACKOFF=2.0
ELAPSED=0

while [ $(echo "$ELAPSED < $TIMEOUT" | bc -l) -eq 1 ]; do
    # Check if log file exists and has content
    if [ -f "$LOG_FILE" ] && [ -s "$LOG_FILE" ]; then
        # Search for ready token
        if grep -q "$READY_PATTERN" "$LOG_FILE" 2>/dev/null; then
            END_TIME=$(date +%s.%N)
            END_TIME_HUMAN=$(date +"%H:%M:%S.%3N")
            
            # Calculate latency in milliseconds
            LATENCY=$(echo "($END_TIME - $START_TIME) * 1000" | bc -l)
            LATENCY_MS=$(printf "%.1f" "$LATENCY")
            
            echo "[$END_TIME_HUMAN] ✓ Ready token detected!"
            echo
            
            # Extract and parse the ready token line
            READY_LINE=$(grep "$READY_PATTERN" "$LOG_FILE" | head -1)
            echo "Ready token: $READY_LINE"
            
            # Parse enhanced build-ID token format: READY <board> <git_sha> <utc_timestamp>
            if [[ "$READY_LINE" =~ READY[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+)[[:space:]]+([^[:space:]]+) ]]; then
                BOARD="${BASH_REMATCH[1]}"
                GIT_SHA="${BASH_REMATCH[2]}"
                UTC_TIME="${BASH_REMATCH[3]}"
                echo "Build info: Board=$BOARD, Git=$GIT_SHA, Time=$UTC_TIME"
            fi
            echo "Latency: ${LATENCY_MS}ms (start→ready)"
            echo "Duration: $START_TIME_HUMAN → $END_TIME_HUMAN"
            echo
            echo "✓ HIL system ready for testing"
            exit 0
        fi
    elif [ ! -f "$LOG_FILE" ]; then
        # Log file doesn't exist yet - this is normal at startup
        :
    fi
    
    # Exponential backoff with jitter
    sleep "$BACKOFF"
    
    # Update elapsed time
    CURRENT_TIME=$(date +%s.%N)
    ELAPSED=$(echo "$CURRENT_TIME - $START_TIME" | bc -l)
    
    # Increase backoff (exponential with max cap)
    BACKOFF=$(echo "$BACKOFF * 1.2" | bc -l)
    if [ $(echo "$BACKOFF > $MAX_BACKOFF" | bc -l) -eq 1 ]; then
        BACKOFF=$MAX_BACKOFF
    fi
    
    # Progress indicator every 5 seconds
    ELAPSED_INT=$(printf "%.0f" "$ELAPSED")
    if [ $((ELAPSED_INT % 5)) -eq 0 ] && [ $ELAPSED_INT -gt 0 ]; then
        CURRENT_TIME_HUMAN=$(date +"%H:%M:%S")
        echo "[$CURRENT_TIME_HUMAN] Still waiting... (${ELAPSED_INT}s elapsed)"
    fi
done

# Timeout reached
END_TIME=$(date +%s.%N)
END_TIME_HUMAN=$(date +"%H:%M:%S.%3N")
ELAPSED_FINAL=$(echo "($END_TIME - $START_TIME) * 1000" | bc -l)
ELAPSED_FINAL_MS=$(printf "%.1f" "$ELAPSED_FINAL")

echo "[$END_TIME_HUMAN] ✗ Timeout waiting for ready token"
echo "Pattern: \"$READY_PATTERN\""
echo "Timeout: ${TIMEOUT}s"
echo "Elapsed: ${ELAPSED_FINAL_MS}ms"
echo

if [ -f "$LOG_FILE" ]; then
    echo "Log file exists: $LOG_FILE"
    LINE_COUNT=$(wc -l < "$LOG_FILE" 2>/dev/null || echo "0")
    echo "Log lines: $LINE_COUNT"
    
    if [ "$LINE_COUNT" -gt 0 ]; then
        echo "Last few lines:"
        tail -3 "$LOG_FILE" | sed 's/^/  /'
    fi
else
    echo "Log file not found: $LOG_FILE"
fi

echo
echo "✗ Ready token not detected within timeout"
exit 1