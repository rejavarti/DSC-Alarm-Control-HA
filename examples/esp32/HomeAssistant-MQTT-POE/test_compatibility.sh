#!/bin/bash

# Simple test script to verify ESP32-POE example structure and compatibility
# This script checks for common issues and verifies file integrity

echo "=== DSC ESP32-POE Example Verification ==="
echo

# Check if all required files exist
echo "Checking file structure..."
FILES=(
    "HomeAssistant-MQTT-POE.ino"
    "config.h"
    "config.cpp" 
    "webserver.h"
    "webserver.cpp"
    "README.md"
    "homeassistant-configuration-example.yaml"
)

for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file exists"
    else
        echo "✗ $file missing"
        exit 1
    fi
done

echo
echo "Checking MQTT topic compatibility..."

# Check for required MQTT topics in main sketch
TOPICS=(
    "dsc/Get/Partition"
    "dsc/Get/Zone"
    "dsc/Get/Fire"
    "dsc/Get/PGM"
    "dsc/Get/Trouble"
    "dsc/Status"
    "dsc/Set"
)

for topic in "${TOPICS[@]}"; do
    if grep -q "$topic" HomeAssistant-MQTT-POE.ino config.h; then
        echo "✓ Topic '$topic' found"
    else
        echo "✗ Topic '$topic' missing"
    fi
done

echo
echo "Checking Home Assistant compatibility..."

# Check for key Home Assistant compatible functions
HA_FUNCTIONS=(
    "mqttCallback"
    "publishMessage"
    "appendPartition"
    "armed_home"
    "armed_away" 
    "armed_night"
    "disarmed"
    "triggered"
)

for func in "${HA_FUNCTIONS[@]}"; do
    if grep -q "$func" HomeAssistant-MQTT-POE.ino; then
        echo "✓ Function/keyword '$func' found"
    else
        echo "✗ Function/keyword '$func' missing"
    fi
done

echo
echo "Checking ESP32-POE specific features..."

# Check for ESP32-POE specific features
POE_FEATURES=(
    "ETH.begin"
    "WiFi.begin"
    "setupWebServer"
    "config.useEthernet"
    "loadConfig"
    "saveConfig"
)

for feature in "${POE_FEATURES[@]}"; do
    if grep -q "$feature" HomeAssistant-MQTT-POE.ino config.cpp webserver.cpp 2>/dev/null; then
        echo "✓ Feature '$feature' found"
    else
        echo "✗ Feature '$feature' missing"
    fi
done

echo
echo "Checking code structure..."

# Check for proper includes
if grep -q "#include.*dscKeybusInterface.h" HomeAssistant-MQTT-POE.ino; then
    echo "✓ DSC library included"
else
    echo "✗ DSC library include missing"
fi

if grep -q "#include.*PubSubClient.h" HomeAssistant-MQTT-POE.ino; then
    echo "✓ MQTT library included"
else
    echo "✗ MQTT library include missing"
fi

if grep -q "#include.*WebServer.h" HomeAssistant-MQTT-POE.ino; then
    echo "✓ WebServer library included"
else
    echo "✗ WebServer library include missing"
fi

echo
echo "Verification complete!"
echo
echo "Manual testing recommendations:"
echo "1. Compile the sketch in Arduino IDE with ESP32 board selected"
echo "2. Check for any compilation errors"
echo "3. Upload to ESP32-POE hardware for functional testing"
echo "4. Test web interface at device IP address"
echo "5. Verify MQTT topics with Home Assistant or MQTT client"
echo "6. Test zone status changes and alarm functions"