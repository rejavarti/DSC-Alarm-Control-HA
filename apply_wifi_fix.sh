#!/bin/bash
# Script to apply WiFi timeout fix to core DSC examples

echo "Applying WiFi timeout fix to core examples..."

# List of critical examples to fix
examples=(
    "examples/esp32/HomeAssistant-MQTT/HomeAssistant-MQTT.ino"
    "examples/esp8266/HomeAssistant-MQTT/HomeAssistant-MQTT.ino"
    "examples/esp32/KeybusReaderIP/KeybusReaderIP.ino"
    "examples/esp8266/KeybusReaderIP/KeybusReaderIP.ino"
)

# Function to apply the WiFi timeout fix
apply_wifi_fix() {
    local file="$1"
    echo "Fixing: $file"
    
    # Check if file exists
    if [ ! -f "$file" ]; then
        echo "  ❌ File not found: $file"
        return 1
    fi
    
    # Check if fix is already applied
    if grep -q "maxAttempts.*60" "$file"; then
        echo "  ✅ Already fixed: $file"
        return 0
    fi
    
    # Create backup
    cp "$file" "$file.bak"
    
    # Apply the fix using sed (more reliable than manual editing for automation)
    sed -i '/WiFi\.begin(wifiSSID, wifiPassword);/,/Serial\.println(WiFi\.localIP());/ {
        /while (WiFi\.status() != WL_CONNECTED) {/,/}/ {
            /while (WiFi\.status() != WL_CONNECTED) {/ {
                i\  \
                i\  // Wait up to 30 seconds for connection instead of infinite loop\
                i\  int connectAttempts = 0;\
                i\  const int maxAttempts = 60; // 30 seconds with 500ms delay\
                i\  while (WiFi.status() != WL_CONNECTED && connectAttempts < maxAttempts) {\
                i\    Serial.print(".");\
                i\    delay(500);\
                i\    connectAttempts++;\
                i\  }\
                i\\
                i\  if (WiFi.status() == WL_CONNECTED) {\
                i\    Serial.print(F("connected: "));\
                i\    Serial.println(WiFi.localIP());\
                i\  } else {\
                i\    Serial.println("TIMEOUT: Failed to connect to WiFi after 30 seconds!");\
                i\    Serial.print("Configured SSID: '\''"); Serial.print(wifiSSID); Serial.println("'\''");\
                i\    Serial.println("Please check your WiFi credentials and network availability.");\
                i\    Serial.println("Device will halt - please reconfigure and restart.");\
                i\    while (1) {\
                i\      delay(1000);\
                i\    }\
                i\  }
                d
            }
            /Serial\.print(".");/ d
            /delay(500);/ d
            /}/ {
                /Serial\.print(F("connected: "));/,/Serial\.println(WiFi\.localIP());/ d
                d
            }
        }
    }' "$file"
    
    # Verify the fix was applied
    if grep -q "maxAttempts.*60" "$file"; then
        echo "  ✅ Successfully applied fix to: $file"
        rm "$file.bak"  # Remove backup if successful
    else
        echo "  ❌ Failed to apply fix to: $file"
        mv "$file.bak" "$file"  # Restore backup
        return 1
    fi
}

# Apply fix to each example
for example in "${examples[@]}"; do
    apply_wifi_fix "$example"
done

echo
echo "WiFi timeout fix application complete!"