# Complete ESPHome Setup Guide for DSC Alarm Integration

## Hardware Requirements

### **You Need Only 1 ESP Device** ✅

**One ESP32 or ESP8266 board is all that's required** to interface with your DSC alarm panel and provide complete Home Assistant integration via MQTT Discovery.

### Supported Hardware

| Board Type | Recommended | Notes |
|------------|-------------|-------|
| **ESP32 Dev Kit** | ⭐ **Best Choice** | More memory, faster, better WiFi |
| **NodeMCU v3 (ESP8266)** | ✅ **Good** | Popular, well-tested |
| **Wemos D1 Mini (ESP8266)** | ✅ **Compact** | Small form factor |
| **Arduino UNO + WiFi** | ❌ **Not Recommended** | Limited memory, complex setup |

### Additional Components Needed

#### For DSC PowerSeries (Default Configuration)
- **2x 33kΩ resistors** - For data lines
- **2x 10kΩ resistors** - Pull-down resistors  
- **1x 5V voltage regulator** (if using ESP8266 with 5V supply)
- **Jumper wires** - For connections
- **Breadboard or PCB** - For assembly

#### For DSC Classic Series (Additional Requirements)
- **1x 1kΩ resistor** - For PC-16 line
- **All PowerSeries components above**

## DSC Series Compatibility

### ✅ DSC PowerSeries (Default Configuration)
- PC1555MX, PC5015, PC1616, PC1832, PC1864
- PC1565, PC5010, PC5015, PC5020
- And other keybus-compatible PowerSeries panels

### ✅ DSC Classic Series (Requires Configuration Change)
- PC1500, PC1550, PC1832, PC1864, PC1616
- PC1500MX, PC1550MX
- And other keybus-compatible Classic panels

**Key Difference**: Classic series requires uncommenting build flags and additional PC-16 wiring.

## Complete Setup Process

### Step 1: Install ESPHome in Home Assistant

1. **Navigate to Add-ons**:
   - Go to **Settings** → **Add-ons**
   - Click **Add-on Store** (bottom right)

2. **Install ESPHome**:
   - Search for "ESPHome"
   - Click **ESPHome** by ESPHome
   - Click **INSTALL**
   - Wait for installation to complete

3. **Start ESPHome**:
   - Click **START**
   - Enable **"Start on boot"** and **"Show in sidebar"**
   - Click **OPEN WEB UI** or access via sidebar

### Step 2: Download and Prepare Configuration Files

1. **Download Repository Files**:
   ```bash
   # Download the repository
   git clone https://github.com/rejavarti/New-DSC-Alarm-Control-HA.git
   
   # Or download ZIP and extract
   ```

2. **Locate ESPHome Files**:
   - Navigate to `extras/ESPHome/` folder
   - You'll need:
     - `DscAlarm.yaml` - Main configuration file
     - `dscKeybusInterface/` folder - Contains `dscAlarm.h`

### Step 3: Create New ESPHome Device

1. **Open ESPHome Dashboard** (usually at `http://your-ha-ip:6052`)

2. **Create New Device**:
   - Click **"+ New Device"** (green button)
   - Click **"Continue"**
   - Enter device name: `dscalarm`
   - Select your device type:
     - **ESP32** (recommended)
     - **ESP8266** (NodeMCU, Wemos D1 Mini)

3. **Skip Initial Setup**:
   - Click **"Skip this step"** (we'll use our custom config)

### Step 4: Upload Custom Configuration

1. **Copy Configuration**:
   - Copy the entire contents of `extras/ESPHome/DscAlarm.yaml`
   - In ESPHome dashboard, click **"Edit"** on your new device
   - **Replace all content** with the copied configuration

2. **Copy Support Files**:
   - Create folder `dscKeybusInterface` in your ESPHome device folder
   - Copy `dscAlarm.h` from repository to this folder

3. **Configure for Your DSC Series**:

   **For DSC PowerSeries (Most Common)**:
   - No changes needed - default configuration works
   
   **For DSC Classic Series**:
   - Uncomment these lines in the `esphome:` section:
   ```yaml
   build_flags:
     - -DdscClassicSeries
   ```

### Step 5: Configure Secrets

1. **Create secrets.yaml** in ESPHome:
   - In ESPHome dashboard, click the folder icon (top right)
   - Create new file: `secrets.yaml`

2. **Add Your Configuration**:
   ```yaml
   # WiFi credentials
   wifi_ssid: "YourWiFiNetworkName"
   wifi_password: "YourWiFiPassword"
   
   # ESPHome API & OTA
   api_password: "your_secure_api_password"
   ota_password: "your_secure_ota_password"
   
   # DSC System Access Code
   access_code: "1234"  # Your DSC panel master code
   
   # MQTT Configuration (for discovery)
   mqtt_broker: "192.168.1.100"    # Your Home Assistant IP
   mqtt_username: "your_mqtt_user"  # MQTT username
   mqtt_password: "your_mqtt_pass"  # MQTT password
   ```

### Step 6: Hardware Wiring

#### DSC PowerSeries Wiring (Default)

```
DSC Panel Connections:
┌─────────────────┐
│ DSC Panel       │
│ AUX (+) Red     │──── 5V ──── Voltage Regulator ──── ESP32 VIN (or 3.3V)
│ AUX (-) Black   │──── GND ─── ESP32 GND
│ DATA (Yellow)   │──── 33kΩ resistor ──┬── ESP32 GPIO 18 (Clock)
│                 │                     └── 10kΩ resistor ──── GND
│ DATA (Green)    │──── 33kΩ resistor ──┬── ESP32 GPIO 19 (Data)
│                 │                     └── 10kΩ resistor ──── GND
└─────────────────┘
```

#### DSC Classic Series Wiring (Additional PC-16 Connection)

```
DSC Classic Panel Connections:
┌─────────────────┐
│ DSC Panel       │
│ AUX (+) Red     │──── 5V ──── Voltage Regulator ──── ESP32 VIN (or 3.3V)
│ AUX (-) Black   │──── GND ─── ESP32 GND
│ DATA (Yellow)   │──── 33kΩ resistor ──┬── ESP32 GPIO 18 (Clock)
│                 │                     └── 10kΩ resistor ──── GND
│ DATA (Green)    │──── 33kΩ resistor ──┬── ESP32 GPIO 19 (Data)
│                 │                     └── 10kΩ resistor ──── GND
│ PGM Output      │──── 1kΩ resistor ───┬── DSC AUX (+)
│                 │                     └── 33kΩ resistor ──┬── ESP32 GPIO 17 (PC-16)
│                 │                                        └── 10kΩ resistor ──── GND
└─────────────────┘
```

#### Pin Assignments (Default in Configuration)
- **GPIO 18**: DSC Clock line (Yellow wire)
- **GPIO 19**: DSC Data line (Green wire)  
- **GPIO 17**: PC-16 line (Classic series only)

### Step 7: Build and Flash Firmware

1. **Validate Configuration**:
   - Click **"Validate"** in ESPHome dashboard
   - Fix any configuration errors shown

2. **Build Firmware**:
   - Click **"Install"** 
   - Choose **"Manual download"** for first flash
   - Wait for compilation (may take 5-10 minutes)

3. **Flash Device**:
   - Connect ESP32/ESP8266 to computer via USB
   - Download the `.bin` file when build completes
   - Use ESPHome Web flasher or esptool to flash:
     - Visit https://web.esphome.io/
     - Click "Connect" and select your device
     - Click "Install" and select the downloaded `.bin` file

4. **Subsequent Updates**:
   - After first flash, you can update wirelessly via "Install" → "Wirelessly"

### Step 8: Verify Integration

1. **Check Device Status**:
   - In ESPHome dashboard, device should show "Online"
   - Check logs for any connection errors

2. **Verify Home Assistant Integration**:
   - Go to **Settings** → **Devices & Services**
   - Look for **"ESPHome"** integration
   - You should see "dscalarm" device listed

3. **Check Auto-Discovered Entities**:
   Click on the device to see automatically created entities:
   - **DSC Alarm Panel** (alarm_control_panel)
   - **Zone sensors** (binary_sensor) - doors, windows, motion
   - **System diagnostics** (sensor) - uptime, WiFi signal, free memory  
   - **Partition status** (text_sensor) - current system messages
   - **Emergency buttons** (button) - Fire, Aux, Panic alarms
   - **System health** (binary_sensor) - connection status, troubles

## Troubleshooting

### Device Won't Connect to WiFi
1. **Check credentials** in `secrets.yaml`
2. **Verify signal strength** - device needs good WiFi coverage
3. **Check ESPHome logs** for specific WiFi errors
4. **Try different WiFi channel** - 2.4GHz band only

### No Data from DSC Panel
1. **Verify wiring connections**:
   - Check resistor values (33kΩ and 10kΩ)
   - Ensure solid connections to DSC panel
   - Verify correct GPIO pins used
2. **For Classic series**:
   - Ensure build flags are uncommented
   - Check PC-16 wiring (1kΩ and 33kΩ resistors)
   - Verify PGM is programmed for PC-16 output

### MQTT Discovery Not Working
1. **Check MQTT broker settings** in `secrets.yaml`
2. **Verify MQTT integration** is working in Home Assistant
3. **Check device logs** for MQTT connection errors
4. **Restart Home Assistant** if needed

### Entities Not Appearing in Home Assistant
1. **Wait 2-3 minutes** for discovery process
2. **Check ESPHome device logs** for errors
3. **Verify MQTT discovery is enabled** (`discovery: true`)
4. **Restart ESPHome device** if needed

### Classic Series Specific Issues
1. **Keybus not detected**:
   - Verify build flags include `-DdscClassicSeries`
   - Check PC-16 wiring to GPIO 17
   - Ensure PGM output is configured in panel
2. **Intermittent connection**:
   - Classic series may have different timing requirements
   - Check all resistor values are correct
   - Verify panel model is actually Classic series

## Hardware Assembly Options

### Option 1: Breadboard (Testing/Development)
- **Pros**: Easy to modify, no soldering
- **Cons**: Less reliable for permanent installation
- **Best for**: Initial testing and development

### Option 2: PCB Layouts (Recommended for Production)
The repository includes professional PCB layouts:
- `extras/PCB Layouts/NodeMCU/` - For ESP8266 NodeMCU
- `extras/PCB Layouts/Arduino/` - For Arduino UNO shield
- `extras/PCB Layouts/Simple/` - Basic layout
- `extras/PCB Layouts/WemosD1Mini/` - For Wemos D1 Mini

### Option 3: Perfboard/Stripboard
- **Pros**: More reliable than breadboard, customizable
- **Cons**: Requires soldering skills
- **Best for**: Permanent installations without custom PCB

## Advanced Configuration

### Multiple Partitions
The configuration supports up to 2 partitions by default. For more partitions, modify the zone handling in the custom component section.

### Additional Zones
Currently configured for 11 zones. To add more zones, extend the switch statements in the `onZoneStatusChange` callback.

### Custom GPIO Pins
To use different GPIO pins, modify these lines in the configuration:
```yaml
# In the custom component section
DSCkeybus->begin(18, 19, 17);  // Clock, Data, PC-16 pins
```

## Performance and Memory Usage

### ESP32 (Recommended)
- **Memory**: ~50KB used, 250KB+ free
- **Performance**: Real-time processing, no delays
- **WiFi**: Excellent stability and range

### ESP8266 (Good Alternative)  
- **Memory**: ~30KB used, 50KB+ free
- **Performance**: Good, occasional brief delays
- **WiFi**: Good stability, shorter range than ESP32

## Security Considerations

1. **Change default passwords** in `secrets.yaml`
2. **Use WPA2/WPA3 WiFi encryption**
3. **Secure MQTT broker** with authentication
4. **Restrict network access** to device if needed
5. **Regular firmware updates** via OTA

## Integration Benefits

### ✅ What You Get with ESPHome:
- **Single Device Management** - All entities grouped under one device
- **Automatic MQTT Discovery** - No YAML configuration needed
- **Real-time Updates** - Instant status changes
- **OTA Updates** - Update firmware wirelessly
- **Rich Diagnostics** - System health, memory usage, WiFi signal
- **Home Assistant Integration** - Native support, no custom components
- **Reliable Operation** - Built-in reconnection and error handling

### 🔧 Available Controls:
- **Arm/Disarm** alarm system
- **Monitor all zones** (doors, windows, motion sensors)
- **Emergency alarms** (Fire, Aux, Panic buttons)
- **System diagnostics** (connection status, troubles, battery)
- **Partition management** (status messages, multiple partitions)

This comprehensive guide should provide everything needed to successfully set up ESPHome with your DSC alarm system using only **one ESP32/ESP8266 device**.