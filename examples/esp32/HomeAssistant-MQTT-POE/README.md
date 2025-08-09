# DSC Keybus Interface - ESP32 POE with Web Configuration

This example provides a comprehensive DSC alarm system interface for the ESP32 with Power over Ethernet (POE) support, specifically designed for the **Olimex ESP32-POE** board. It includes a web-based configuration interface, debug monitoring, and maintains full compatibility with Home Assistant via MQTT.

## Features

- **Dual Network Support**: Both Ethernet (POE) and WiFi connectivity
- **Web Configuration Interface**: Easy setup via web browser
- **Persistent Configuration**: Settings saved to flash memory  
- **Debug Monitoring**: Real-time system status and event logging
- **Zone Configuration**: Custom zone names and device classes
- **MQTT Testing**: Built-in connectivity testing
- **Home Assistant Compatible**: Preserves all original MQTT functionality
- **Comprehensive Logging**: Last 5 alarm events and access code attempts

## Hardware Requirements

### Primary Target: Olimex ESP32-POE
- Built-in Ethernet with POE support
- ESP32 microcontroller
- Integrated voltage regulation
- Compact design suitable for installation

### Alternative: Standard ESP32 with Ethernet Module
- ESP32 development board
- W5500 or LAN8720 Ethernet module
- External power supply

## Installation

1. **Hardware Connection**:
   ```
   DSC Aux(+) --- 5V voltage regulator --- ESP32-POE 5V pin
   DSC Aux(-) --- ESP32-POE Ground
   
   DSC Yellow --- 33kΩ resistor --- GPIO 18 (dscClockPin)
                                |-- 10kΩ resistor --- Ground
   
   DSC Green ---- 33kΩ resistor --- GPIO 19 (dscReadPin)  
                                |-- 10kΩ resistor --- Ground
   
   Optional Virtual Keypad:
   DSC Green ---- NPN collector --\
                                   |-- NPN base --- 1kΩ resistor --- GPIO 21 (dscWritePin)
           Ground --- NPN emitter --/
   ```

2. **Software Upload**:
   - Install required libraries: `WiFi`, `ETH`, `PubSubClient`, `dscKeybusInterface`, `WebServer`, `Preferences`
   - Upload the sketch to your ESP32-POE
   - Connect via Ethernet cable or configure WiFi

3. **Initial Configuration**:
   - Connect to the device's IP address via web browser
   - Access the configuration page
   - Set up network and MQTT settings
   - Configure zones and system settings

## Web Interface

### Main Dashboard (`/`)
- System information overview  
- Network status
- Quick access to configuration and debug pages
- System restart option

### Configuration Page (`/config`)
- **Network Settings**: Choose between Ethernet/WiFi, DHCP/Static IP
- **WiFi Configuration**: SSID selection with network scanning
- **MQTT Settings**: Server, credentials, connection testing
- **DSC System**: Access code configuration
- **Zone Configuration**: Names, device classes, enable/disable per zone

### Debug Interface (`/debug`)
- **System Status**: Uptime, memory usage, connectivity status
- **MQTT Status**: Connection state, reconnection count
- **Zone Status**: Real-time zone states (open/closed, motion, fire, etc.)
- **Event Logging**: Last 5 alarm events and access code attempts
- **Alarm System**: Keybus connection status

## Configuration Options

### Network Configuration
- **Network Mode**: Ethernet (POE) or WiFi
- **IP Mode**: DHCP (automatic) or Static IP
- **WiFi**: SSID, password, network scanning
- **Static IP**: IP address, gateway, subnet mask, DNS

### MQTT Configuration  
- **Server**: IP address or hostname
- **Port**: Default 1883
- **Credentials**: Username and password (optional)
- **Topics**: Default Home Assistant compatible topics
- **Testing**: Built-in connection verification

### Zone Configuration (8 zones)
- **Enable/Disable**: Individual zone activation
- **Custom Names**: User-friendly zone identification
- **Device Classes**: door, window, motion, smoke, glass, opening
- **Status Monitoring**: Real-time zone state tracking

## Home Assistant Integration

This interface maintains full compatibility with the original DSC-HomeAssistant-MQTT functionality. Use the same configuration.yaml entries:

```yaml
# MQTT Broker Configuration
mqtt:
  broker: [ESP32-POE-IP-ADDRESS]
  client_id: homeAssistant

# Alarm Control Panel
alarm_control_panel:
  - platform: mqtt
    name: "Security Partition 1"
    state_topic: "dsc/Get/Partition1"
    availability_topic: "dsc/Status"  
    command_topic: "dsc/Set"
    payload_disarm: "1D"
    payload_arm_home: "1S"
    payload_arm_away: "1A"
    payload_arm_night: "1N"

# Zone Sensors (example)
binary_sensor:
  - platform: mqtt
    name: "Zone 1"
    state_topic: "dsc/Get/Zone1"
    device_class: "door"
    payload_on: "1"
    payload_off: "0"
```

## MQTT Topics

All original MQTT topics are preserved:

- **Status**: `dsc/Status` (online/offline)
- **Partitions**: `dsc/Get/Partition1-8` (armed/disarmed states)
- **Zones**: `dsc/Get/Zone1-64` (open/closed states)
- **Fire**: `dsc/Get/Fire1-8` (fire alarm states)
- **PGM**: `dsc/Get/PGM1-14` (PGM output states)  
- **Trouble**: `dsc/Get/Trouble` (system trouble)
- **Commands**: `dsc/Set` (control commands)

## API Endpoints

- `GET /` - Main dashboard
- `GET /config` - Configuration interface
- `POST /config/save` - Save configuration
- `GET /debug` - Debug information
- `GET /restart` - System restart
- `GET /scan` - WiFi network scan (JSON)
- `GET /mqtt-test` - MQTT connection test (JSON)

## Default Pins (ESP32-POE)

- **GPIO 18**: DSC Clock (Yellow wire)
- **GPIO 19**: DSC Data (Green wire)  
- **GPIO 21**: DSC Write (Virtual keypad, optional)
- **Ethernet**: Built-in POE connector

## Troubleshooting

1. **Network Connection Issues**:
   - Check Ethernet cable and POE injector
   - Verify WiFi credentials in configuration
   - Try switching between Ethernet and WiFi modes

2. **MQTT Connection Problems**:
   - Use the built-in MQTT test function
   - Verify server address and credentials
   - Check firewall settings on MQTT broker

3. **DSC Keybus Issues**:
   - Verify wiring connections and resistor values
   - Check debug page for keybus connection status
   - Ensure proper grounding between DSC and ESP32

4. **Web Interface Access**:
   - Check device IP address in router/DHCP server
   - Ensure firewall allows HTTP traffic on port 80
   - Try factory reset if configuration is corrupted

## Factory Reset

To reset to factory defaults:
1. Access `/restart` endpoint
2. Or power cycle the device multiple times during startup
3. Configuration will reset to defaults, accessible via DHCP-assigned IP

## Advanced Features

### Event Logging
- Automatic logging of alarm events with timestamps
- Access code attempt tracking (masked for security)
- Persistent storage of recent events

### Zone Monitoring
- Real-time zone status updates
- Multiple sensor types per zone (motion, glass break, fire)
- Historical change tracking

### Network Redundancy  
- Automatic fallback from Ethernet to WiFi
- Connection status monitoring and reporting
- Graceful handling of network interruptions

## License

This code is in the public domain and builds upon the excellent work of the dscKeybusInterface library.