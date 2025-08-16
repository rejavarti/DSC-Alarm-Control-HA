#!/bin/bash
# DSC Panel Connection Monitoring Script
echo "🔍 DSC Panel Connection Monitor - $(date)"
echo "This script will flash the enhanced timing configuration and monitor the logs"
echo "Press Ctrl+C to stop monitoring"
echo ""

echo "📡 Flashing enhanced timing configuration..."
esphome run user_dsc_config_enhanced_timing.yaml --device /dev/ttyUSB0

echo ""
echo "🔍 Monitoring logs for connection status..."
echo "Watch for these key messages:"
echo "  ✅ SUCCESS: 'DSC Classic hardware initialization successful'"
echo "  ❌ FAILURE: 'DSC hardware initialization permanently failed'"
echo "  ⚠️  WARNING: 'Task watchdog got triggered'"
echo ""

# Monitor logs
esphome logs user_dsc_config_enhanced_timing.yaml --device /dev/ttyUSB0
