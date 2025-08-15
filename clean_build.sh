#!/bin/bash
# ESPHome Clean Build Script
# Cleans all build caches to prevent compilation issues from corrupted files
#
# USAGE:
#   ./clean_build.sh        (recommended)
#   bash clean_build.sh     (alternative)
#
# DO NOT run with: python3 clean_build.sh
# This is a bash script, not a Python script!

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "🧹 ESPHome Clean Build Script"
echo "=============================="

# Function to safely remove directory if it exists
safe_remove() {
    local dir="$1"
    if [ -d "$dir" ]; then
        echo "Removing: $dir"
        rm -rf "$dir"
    else
        echo "Already clean: $dir"
    fi
}

echo ""
echo "📁 Cleaning ESPHome build directories..."

cd "$PROJECT_ROOT"

# Clean ESPHome build directories
find . -name ".esphome" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name ".pioenvs" -type d -exec rm -rf {} + 2>/dev/null || true
find . -name ".pio" -type d -exec rm -rf {} + 2>/dev/null || true

# Clean PlatformIO cache directories that might be in extras/ESPHome
cd extras/ESPHome 2>/dev/null || cd .

safe_remove ".esphome"
safe_remove ".pioenvs"
safe_remove ".pio"

# Clean any temporary files
echo ""
echo "🗑️  Cleaning temporary files..."
find . -name "*.tmp" -delete 2>/dev/null || true
find . -name "*.pyc" -delete 2>/dev/null || true
find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true

echo ""
echo "✅ Build directories cleaned successfully!"
echo ""
echo "🔄 You can now run a fresh ESPHome compilation:"
echo "   cd extras/ESPHome"
echo "   esphome compile YourConfig.yaml"
echo ""