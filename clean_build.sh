#!/usr/bin/env bash
# ESPHome Clean Build Script
# Cleans all build caches to prevent compilation issues from corrupted files
#
# USAGE:
#   ./clean_build.sh        (recommended)
#   bash clean_build.sh     (alternative)
#
# DO NOT run with: python3 clean_build.sh
# This is a bash script, not a Python script!
#
# Enhanced for WSL and cross-platform compatibility

set -e

# Check if we have bash available
if ! command -v bash >/dev/null 2>&1; then
    echo "❌ ERROR: bash is not available in your environment!"
    echo "Please install bash or use an environment where bash is available."
    exit 1
fi

# Detect WSL environment and warn about potential issues
if grep -qi microsoft /proc/version 2>/dev/null || grep -qi wsl /proc/version 2>/dev/null; then
    echo "🔍 WSL Environment Detected"
    if [ ! -d "/bin" ] || [ ! -x "/bin/bash" ]; then
        echo "⚠️  WARNING: Your WSL environment may have compatibility issues."
        echo "   If this script fails, try running it from Windows Command Prompt instead."
    fi
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"  # We're already in the project root

echo ""
echo "🧹 ESPHome Clean Build Script"
echo "=============================="

# Function to safely remove directory if it exists
safe_remove() {
    local dir="$1"
    if [ -d "$dir" ]; then
        echo "Removing: $dir"
        if rm -rf "$dir" 2>/dev/null; then
            echo "✅ Successfully removed: $dir"
        else
            echo "⚠️  Warning: Could not remove $dir (may require permissions)"
        fi
    else
        echo "Already clean: $dir"
    fi
}

echo ""
echo "📁 Cleaning ESPHome build directories..."

# Ensure we're in the project root
if ! cd "$PROJECT_ROOT"; then
    echo "❌ ERROR: Cannot access project root directory: $PROJECT_ROOT"
    exit 1
fi

echo "Working in: $(pwd)"

# Clean ESPHome build directories with better error handling
echo "Searching for .esphome directories..."
find . -name ".esphome" -type d -exec rm -rf {} + 2>/dev/null || true

echo "Searching for .pioenvs directories..."
find . -name ".pioenvs" -type d -exec rm -rf {} + 2>/dev/null || true

echo "Searching for .pio directories..."
find . -name ".pio" -type d -exec rm -rf {} + 2>/dev/null || true

# Clean PlatformIO cache directories that might be in extras/ESPHome
if [ -d "extras/ESPHome" ]; then
    echo "Cleaning extras/ESPHome directory..."
    cd extras/ESPHome 2>/dev/null || {
        echo "⚠️  Warning: Cannot access extras/ESPHome directory"
        cd "$PROJECT_ROOT"
    }
else
    echo "No extras/ESPHome directory found - staying in project root"
fi

safe_remove ".esphome"
safe_remove ".pioenvs"
safe_remove ".pio"

# Return to project root for final cleanup
cd "$PROJECT_ROOT"

# Clean any temporary files
echo ""
echo "🗑️  Cleaning temporary files..."
echo "Removing *.tmp files..."
find . -name "*.tmp" -delete 2>/dev/null || echo "No .tmp files found"

echo "Removing *.pyc files..."
find . -name "*.pyc" -delete 2>/dev/null || echo "No .pyc files found"

echo "Removing __pycache__ directories..."
find . -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || echo "No __pycache__ directories found"

echo ""
echo "✅ Build directories cleaned successfully!"
echo ""
echo "🔄 You can now run a fresh ESPHome compilation:"
echo "   cd extras/ESPHome"
echo "   esphome compile YourConfig.yaml"
echo ""

# Final status check
if [ -d "extras/ESPHome" ]; then
    echo "📂 ESPHome directory structure:"
    ls -la extras/ESPHome/ 2>/dev/null || echo "Could not list ESPHome directory contents"
else
    echo "ℹ️  Note: No extras/ESPHome directory found in this project"
fi