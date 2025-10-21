#!/bin/bash
# cleanup_repo.sh - Clean repository of build artifacts before commit
# Usage: ./scripts/cleanup_repo.sh

set -e

echo "=== Repository Cleanup ==="

# Remove sketch build directories (excluding cmake examples)
echo "Removing sketch build directories..."
find tests/ libraries/ extras/ -name "build" -type d -exec rm -rf {} + 2>/dev/null || true

# Remove auto-generated build_id.h files
echo "Removing auto-generated build_id.h files..."
find . -name "build_id.h" -delete 2>/dev/null || true

# Remove binary artifacts (excluding library documentation)
echo "Removing binary artifacts..."
find . -name "*.bin" -delete 2>/dev/null || true
find . -name "*.hex" -delete 2>/dev/null || true
find . -name "*.elf" -delete 2>/dev/null || true
# Remove .map files but exclude library docs (e.g., Doxygen HTML imagemaps)
find . -name "*.map" -not -path "*/libraries/*/docs/*" -delete 2>/dev/null || true

# Remove Arduino CLI cache artifacts (if any leaked into repo)
echo "Removing Arduino CLI artifacts..."
find . -name ".arduino" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "sketches" -type d -path "*/.cache/*" -exec rm -rf {} + 2>/dev/null || true

# Remove Python cache directories and bytecode
echo "Removing Python cache directories..."
find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name "*.pyc" -delete 2>/dev/null || true
find . -name "*.pyo" -delete 2>/dev/null || true

echo "✓ Repository cleanup complete"
echo ""
echo "Clean files ready for commit:"
git status --porcelain 2>/dev/null || echo "  (git status not available)"