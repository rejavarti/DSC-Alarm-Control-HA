/*
 *  HomeAssistant-MQTT 1.5 (esp32)
 *
 *  Processes the security system status and allows for control using Home Assistant via MQTT.
 *
 *  Home Assistant: https://www.home-assistant.io
 *  Mosquitto MQTT broker: https://mosquitto.org
 *
 *  Usage:
 *    1. Set the WiFi SSID and password in the sketch.
 *    2. Set the security system access code to permit disarming through Home Assistant.
 *    3. Set the MQTT server address in the sketch.
 *    4. Copy the example configuration to Home Assistant's configuration.yaml and customize.
 *    5. Upload the sketch.
 *    6. Restart Home Assistant.
 *
 *  Example Home Assistant configuration.yaml for 2 partitions, 3 zones:

# https://www.home-assistant.io/components/mqtt/
mqtt:
  broker: URL or IP address
  client_id: homeAssistant

# https://www.home-assistant.io/components/alarm_control_panel.mqtt/
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
  - platform: mqtt
    name: "Security Partition 2"
    state_topic: "dsc/Get/Partition2"
    availability_topic: "dsc/Status"
    command_topic: "dsc/Set"
    payload_disarm: "2D"
    payload_arm_home: "2S"
    payload_arm_away: "2A"
    payload_arm_night: "2N"

# The sensor component displays the partition status message - edit the Home
# view ("Configure UI"), click "+", select the "Sensor" card, select "Entity",
# and select the security system partition.
# https://www.home-assistant.io/components/sensor.mqtt/
sensor:
  - platform: mqtt
    name: "Security Partition 1"
    state_topic: "dsc/Get/Partition1/Message"
    availability_topic: "dsc/Status"
    icon: "mdi:shield"
  - platform: mqtt
    name: "Security Partition 2"
    state_topic: "dsc/Get/Partition2/Message"
    availability_topic: "dsc/Status"
    icon: "mdi:shield"

# https://www.home-assistant.io/components/binary_sensor.mqtt/
binary_sensor:
  - platform: mqtt
    name: "Security Trouble"
    state_topic: "dsc/Get/Trouble"
    device_class: "problem"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "Smoke Alarm 1"
    state_topic: "dsc/Get/Fire1"
    device_class: "smoke"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "Smoke Alarm 2"
    state_topic: "dsc/Get/Fire2"
    device_class: "smoke"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "Zone 1"
    state_topic: "dsc/Get/Zone1"
    device_class: "door"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "Zone 2"
    state_topic: "dsc/Get/Zone2"
    device_class: "window"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "Zone 3"
    state_topic: "dsc/Get/Zone3"
    device_class: "motion"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "PGM 1"
    state_topic: "dsc/Get/PGM1"
    payload_on: "1"
    payload_off: "0"
  - platform: mqtt
    name: "PGM 8"
    state_topic: "dsc/Get/PGM8"
    payload_on: "1"
    payload_off: "0"

 *  Example button card configuration to add a panic button: https://www.home-assistant.io/lovelace/button/

type: entity-button
name: Panic alarm
tap_action:
  action: call-service
  service: mqtt.publish
  service_data:
    payload: P
    topic: dsc/Set
hold_action:
  action: none
show_icon: true
show_name: true
entity: alarm_control_panel.security_partition_1

 *  The commands to set the alarm state are setup in Home Assistant with the partition number (1-8) as a
 *  prefix to the command, except to trigger the panic alarm:
 *    Partition 1 disarm: "1D"
 *    Partition 2 arm stay: "2S"
 *    Partition 2 arm away: "2A"
 *    Partition 1 arm night: "1N"
 *    Panic alarm: "P"
 *
 *  The interface listens for commands in the configured mqttSubscribeTopic, and publishes partition status in a
 *  separate topic per partition with the configured mqttPartitionTopic appended with the partition number:
 *    Disarmed: "disarmed"
 *    Arm stay: "armed_home"
 *    Arm away: "armed_away"
 *    Arm night: "armed_night"
 *    Exit delay in progress: "pending"
 *    Alarm tripped: "triggered"
 *
 *  The trouble state is published as an integer in the configured mqttTroubleTopic:
 *    Trouble: "1"
 *    Trouble restored: "0"
 *
 *  Zone states are published as an integer in a separate topic per zone with the configured mqttZoneTopic appended
 *  with the zone number:
 *    Open: "1"
 *    Closed: "0"
 *
 *  Fire states are published as an integer in a separate topic per partition with the configured mqttFireTopic
 *  appended with the partition number:
 *    Fire alarm: "1"
 *    Fire alarm restored: "0"
 *
 *  PGM outputs states are published as an integer in a separate topic per PGM with the configured mqttPgmTopic
 *  appended with the PGM output number:
 *    Open: "1"
 *    Closed: "0"
 *
 *  Release notes:
 *    1.5 - Added DSC Classic series support
 *    1.4 - Added PGM outputs 1-14 status
 *    1.0 - Initial release
 *
 *  Wiring:
 *      DSC Aux(+) --- 5v voltage regulator --- esp32 development board 5v pin
 *
 *      DSC Aux(-) --- esp32 Ground
 *
 *                                         +--- dscClockPin  // Default: 18
 *      DSC Yellow --- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *                                         +--- dscReadPin   // Default: 19
 *      DSC Green ---- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *      Classic series only, PGM configured for PC-16 output:
 *      DSC PGM ---+-- 1k ohm resistor --- DSC Aux(+)
 *                 |
 *                 |                       +--- dscPC16Pin   // Default: 17
 *                 +-- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *      Virtual keypad (optional):
 *      DSC Green ---- NPN collector --\
 *                                      |-- NPN base --- 1k ohm resistor --- dscWritePin  // Default: 21
 *            Ground --- NPN emitter --/
 *
 *  Virtual keypad uses an NPN transistor to pull the data line low - most small signal NPN transistors should
 *  be suitable, for example:
 *   -- 2N3904
 *   -- BC547, BC548, BC549
 *
 *  Issues and (especially) pull requests are welcome:
 *  https://github.com/taligentx/dscKeybusInterface
 *
 *  This example code is in the public domain.
 */

// DSC Classic series: uncomment for PC1500/PC1550 support (requires PC16-OUT configuration per README.md)
//#define dscClassicSeries

#include <WiFi.h>
#include <ETH.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <dscKeybusInterface.h>

// Settings - Default values, will be overridden by stored configuration
String wifiSSID = "";
String wifiPassword = "";
String accessCode = "";      // An access code is required to disarm/night arm and may be required to arm or enable command outputs based on panel configuration.
String mqttServer = "";      // MQTT server domain name or IP address
int    mqttPort = 1883;      // MQTT server port
String mqttUsername = "";    // Optional, leave blank if not required
String mqttPassword = "";    // Optional, leave blank if not required

// Network configuration
String networkType = "wifi";   // "wifi" or "ethernet"
String ipType = "dhcp";        // "dhcp" or "static"
String staticIP = "";
String staticGateway = "";
String staticSubnet = "";
String staticDNS = "";

// Pin configuration - Default values, will be overridden by stored configuration
int clockPin = 18;  // dscClockPin
int readPin = 19;   // dscReadPin  
int writePin = 21;  // dscWritePin
int pc16Pin = 17;   // dscPC16Pin

// MQTT topics - match to Home Assistant's configuration.yaml
const char* mqttClientName = "dscKeybusInterface";
const char* mqttPartitionTopic = "dsc/Get/Partition";  // Sends armed and alarm status per partition: dsc/Get/Partition1 ... dsc/Get/Partition8
const char* mqttPartitionMessageSuffix = "/Message";   // Sends partition status messages: dsc/Get/Partition1/Message ... dsc/Get/Partition8/Message
const char* mqttZoneTopic = "dsc/Get/Zone";            // Sends zone status per zone: dsc/Get/Zone1 ... dsc/Get/Zone64
const char* mqttFireTopic = "dsc/Get/Fire";            // Sends fire status per partition: dsc/Get/Fire1 ... dsc/Get/Fire8
const char* mqttPgmTopic = "dsc/Get/PGM";              // Sends PGM status per PGM: dsc/Get/PGM1 ... dsc/Get/PGM14
const char* mqttTroubleTopic = "dsc/Get/Trouble";      // Sends trouble status
const char* mqttStatusTopic = "dsc/Status";            // Sends online/offline status
const char* mqttBirthMessage = "online";
const char* mqttLwtMessage = "offline";
const char* mqttSubscribeTopic = "dsc/Set";            // Receives messages to write to the panel

// Pin assignments will be configured from stored settings
// Configures the Keybus interface with the specified pins - dscWritePin is optional, leaving it out disables the
// virtual keypad.
// Note: Actual pin assignments are loaded from configuration and set during setup()

// Initialize components (will be reconfigured in setup() based on stored settings)
// DSC interface will be initialized in setup() with configured pins
#ifndef dscClassicSeries
dscKeybusInterface* dsc = nullptr;
#else
dscClassicInterface* dsc = nullptr;
#endif
WiFiClient ipClient;
PubSubClient* mqtt = nullptr;
unsigned long mqttPreviousTime;

// WiFi Manager variables
WebServer configServer(80);
Preferences preferences;
bool configMode = false;

// Forward declarations
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttHandle();
bool mqttConnect();
void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic);
void publishMessage(const char* sourceTopic, byte partition);
void loadFullConfiguration();
void saveFullConfiguration();
void startConfigMode();
bool connectToNetwork();
bool connectToWiFi(String ssid, String password);
bool connectToEthernet();
void configureStaticIP();


// Configuration management functions
void loadFullConfiguration() {
  preferences.begin("config", true);
  
  // Load WiFi settings  
  wifiSSID = preferences.getString("ssid", "");
  wifiPassword = preferences.getString("password", "");
  
  // Load Network configuration
  networkType = preferences.getString("networkType", "wifi");
  ipType = preferences.getString("ipType", "dhcp");
  staticIP = preferences.getString("staticIP", "");
  staticGateway = preferences.getString("staticGW", "");
  staticSubnet = preferences.getString("staticSN", "");
  staticDNS = preferences.getString("staticDNS", "");
  
  // Load MQTT settings
  mqttServer = preferences.getString("mqttServer", "");
  mqttPort = preferences.getInt("mqttPort", 1883);
  mqttUsername = preferences.getString("mqttUser", "");
  mqttPassword = preferences.getString("mqttPass", "");
  
  // Load access code
  accessCode = preferences.getString("accessCode", "");
  
  // Load pin assignments
  clockPin = preferences.getInt("clockPin", 18);
  readPin = preferences.getInt("readPin", 19);
  writePin = preferences.getInt("writePin", 21);
  pc16Pin = preferences.getInt("pc16Pin", 17);
  
  preferences.end();
  
  Serial.println("Configuration loaded from storage");
}

void saveFullConfiguration() {
  preferences.begin("config", false);
  
  // Save WiFi settings
  preferences.putString("ssid", wifiSSID);
  preferences.putString("password", wifiPassword);
  
  // Save Network configuration
  preferences.putString("networkType", networkType);
  preferences.putString("ipType", ipType);
  preferences.putString("staticIP", staticIP);
  preferences.putString("staticGW", staticGateway);
  preferences.putString("staticSN", staticSubnet);
  preferences.putString("staticDNS", staticDNS);
  
  // Save MQTT settings  
  preferences.putString("mqttServer", mqttServer);
  preferences.putInt("mqttPort", mqttPort);
  preferences.putString("mqttUser", mqttUsername);
  preferences.putString("mqttPass", mqttPassword);
  
  // Save access code
  preferences.putString("accessCode", accessCode);
  
  // Save pin assignments
  preferences.putInt("clockPin", clockPin);
  preferences.putInt("readPin", readPin);
  preferences.putInt("writePin", writePin);
  preferences.putInt("pc16Pin", pc16Pin);
  
  preferences.end();
  
  Serial.println("Configuration saved to storage");
}

bool hasStoredConfiguration() {
  preferences.begin("config", true);
  bool hasConfig = preferences.getString("ssid", "").length() > 0;
  preferences.end();
  return hasConfig;
}

// WiFi Manager functions
void startConfigMode() {
  configMode = true;
  Serial.println("Starting comprehensive configuration mode...");
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("DSC-Config", "12345678");
  
  Serial.println("Access Point started");
  Serial.println("Connect to: DSC-Config (password: 12345678)");
  Serial.print("Configuration portal: http://");
  Serial.println(WiFi.softAPIP());
  
  configServer.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>DSC Configuration Portal</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;max-width:800px;margin:20px auto;padding:20px;background:#f5f5f5}";
    html += ".config-section{background:white;padding:20px;margin:20px 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}";
    html += "h1{color:#333;text-align:center;margin-bottom:30px}";
    html += "h2{color:#555;border-bottom:2px solid #4CAF50;padding-bottom:10px}";
    html += "label{display:block;margin-top:15px;font-weight:bold;color:#333}";
    html += "input[type=text],input[type=password],input[type=number]{width:100%;padding:12px;margin:8px 0;border:2px solid #ddd;border-radius:4px;box-sizing:border-box}";
    html += "input:focus{border-color:#4CAF50;outline:none}";
    html += "input[type=submit]{background:#4CAF50;color:white;padding:15px 30px;border:none;border-radius:4px;cursor:pointer;width:100%;font-size:16px;margin-top:20px}";
    html += "input[type=submit]:hover{background:#45a049}";
    html += ".info{background:#e7f3ff;border:1px solid #b3d9ff;padding:15px;border-radius:4px;margin:20px 0}";
    html += ".current-value{color:#666;font-size:0.9em;margin-top:5px}";
    html += "select{width:100%;padding:12px;margin:8px 0;border:2px solid #ddd;border-radius:4px;box-sizing:border-box}";
    html += "select:focus{border-color:#4CAF50;outline:none}";
    html += ".static-fields{margin-top:10px;padding:10px;background:#f8f8f8;border-radius:4px}";
    html += ".hidden{display:none}";
    html += "</style></head><body>";
    
    html += "<h1>DSC Alarm System Configuration</h1>";
    html += "<div class='info'><strong>Note:</strong> This portal configures your DSC alarm system interface. ";
    html += "Fill in all required fields. Empty fields will keep existing values.</div>";
    
    html += "<form method='POST' action='/save'>";
    
    // Network Configuration Section
    html += "<div class='config-section'>";
    html += "<h2>Network Configuration</h2>";
    
    // Network Type Selection
    html += "<label for='networkType'>Network Type: *</label>";
    html += "<select id='networkType' name='networkType' onchange='toggleNetworkSettings()' required>";
    html += "<option value='wifi'";
    if (networkType == "wifi") html += " selected";
    html += ">WiFi</option>";
    html += "<option value='ethernet'";
    if (networkType == "ethernet") html += " selected";
    html += ">Ethernet</option>";
    html += "</select>";
    html += "<div class='current-value'>Current: " + networkType + "</div>";
    
    // IP Configuration Type
    html += "<label for='ipType'>IP Configuration: *</label>";
    html += "<select id='ipType' name='ipType' onchange='toggleStaticFields()' required>";
    html += "<option value='dhcp'";
    if (ipType == "dhcp") html += " selected";
    html += ">DHCP (Automatic)</option>";
    html += "<option value='static'";
    if (ipType == "static") html += " selected";
    html += ">Static IP</option>";
    html += "</select>";
    html += "<div class='current-value'>Current: " + ipType + "</div>";
    
    // WiFi Settings (shown only when WiFi is selected)
    html += "<div id='wifiSettings' class='";
    if (networkType != "wifi") html += "hidden";
    html += "'>";
    html += "<label for='ssid'>WiFi Network Name (SSID): *</label>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='Enter WiFi SSID' value='" + wifiSSID + "'>";
    html += "<div class='current-value'>Current: " + (wifiSSID.length() > 0 ? wifiSSID : "Not configured") + "</div>";
    html += "<label for='password'>WiFi Password: *</label>";
    html += "<input type='password' id='password' name='password' placeholder='Enter WiFi Password' value='" + wifiPassword + "'>";
    html += "</div>";
    
    // Static IP Settings (shown only when Static IP is selected)
    html += "<div id='staticSettings' class='static-fields ";
    if (ipType != "static") html += "hidden";
    html += "'>";
    html += "<label for='staticIP'>Static IP Address:</label>";
    html += "<input type='text' id='staticIP' name='staticIP' placeholder='192.168.1.100' value='" + staticIP + "'>";
    html += "<label for='staticGateway'>Gateway:</label>";
    html += "<input type='text' id='staticGateway' name='staticGateway' placeholder='192.168.1.1' value='" + staticGateway + "'>";
    html += "<label for='staticSubnet'>Subnet Mask:</label>";
    html += "<input type='text' id='staticSubnet' name='staticSubnet' placeholder='255.255.255.0' value='" + staticSubnet + "'>";
    html += "<label for='staticDNS'>DNS Server:</label>";
    html += "<input type='text' id='staticDNS' name='staticDNS' placeholder='8.8.8.8' value='" + staticDNS + "'>";
    html += "</div>";
    html += "</div>";
    
    // MQTT Configuration Section  
    html += "<div class='config-section'>";
    html += "<h2>MQTT Broker Settings</h2>";
    html += "<label for='mqttServer'>MQTT Server/IP: *</label>";
    html += "<input type='text' id='mqttServer' name='mqttServer' placeholder='mqtt.example.com or 192.168.1.100' value='" + mqttServer + "' required>";
    html += "<div class='current-value'>Current: " + (mqttServer.length() > 0 ? mqttServer : "Not configured") + "</div>";
    html += "<label for='mqttPort'>MQTT Port:</label>";
    html += "<input type='number' id='mqttPort' name='mqttPort' placeholder='1883' value='" + String(mqttPort) + "' min='1' max='65535'>";
    html += "<label for='mqttUsername'>MQTT Username (optional):</label>";
    html += "<input type='text' id='mqttUsername' name='mqttUsername' placeholder='Leave blank if not required' value='" + mqttUsername + "'>";
    html += "<label for='mqttPassword'>MQTT Password (optional):</label>";
    html += "<input type='password' id='mqttPassword' name='mqttPassword' placeholder='Leave blank if not required' value='" + mqttPassword + "'>";
    html += "</div>";
    
    // DSC System Configuration
    html += "<div class='config-section'>";
    html += "<h2>DSC System Settings</h2>";
    html += "<label for='accessCode'>Access Code: *</label>";
    html += "<input type='text' id='accessCode' name='accessCode' placeholder='Enter DSC access code' value='" + accessCode + "' required>";
    html += "<div class='current-value'>Required for disarming and some arm operations</div>";
    html += "</div>";
    
    // Pin Configuration Section
    html += "<div class='config-section'>";
    html += "<h2>ESP32 Pin Assignments</h2>";
    html += "<label for='clockPin'>Clock Pin (DSC Yellow wire):</label>";
    html += "<input type='number' id='clockPin' name='clockPin' placeholder='18' value='" + String(clockPin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(clockPin) + " (Default: 18)</div>";
    html += "<label for='readPin'>Data Read Pin (DSC Green wire):</label>";
    html += "<input type='number' id='readPin' name='readPin' placeholder='19' value='" + String(readPin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(readPin) + " (Default: 19)</div>";
    html += "<label for='writePin'>Data Write Pin (Virtual Keypad):</label>";
    html += "<input type='number' id='writePin' name='writePin' placeholder='21' value='" + String(writePin) + "' min='4' max='33'>";
    html += "<div class='current-value'>Current: " + String(writePin) + " (Default: 21, set to 0 to disable virtual keypad)</div>";
    html += "<label for='pc16Pin'>PC16 Pin (Classic Series only):</label>";
    html += "<input type='number' id='pc16Pin' name='pc16Pin' placeholder='17' value='" + String(pc16Pin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(pc16Pin) + " (Default: 17, only used for DSC Classic series)</div>";
    html += "</div>";
    
    html += "<input type='submit' value='Save Configuration and Restart'>";
    html += "</form>";
    
    html += "<div class='info'><strong>After saving:</strong> The device will restart and attempt to connect with the new settings. ";
    html += "If connection fails, this configuration portal will restart automatically.</div>";
    
    // Add JavaScript for dynamic form behavior
    html += "<script>";
    html += "function toggleNetworkSettings() {";
    html += "  var networkType = document.getElementById('networkType').value;";
    html += "  var wifiSettings = document.getElementById('wifiSettings');";
    html += "  if (networkType === 'wifi') {";
    html += "    wifiSettings.classList.remove('hidden');";
    html += "    document.getElementById('ssid').required = true;";
    html += "    document.getElementById('password').required = true;";
    html += "  } else {";
    html += "    wifiSettings.classList.add('hidden');";
    html += "    document.getElementById('ssid').required = false;";
    html += "    document.getElementById('password').required = false;";
    html += "  }";
    html += "}";
    html += "function toggleStaticFields() {";
    html += "  var ipType = document.getElementById('ipType').value;";
    html += "  var staticSettings = document.getElementById('staticSettings');";
    html += "  if (ipType === 'static') {";
    html += "    staticSettings.classList.remove('hidden');";
    html += "  } else {";
    html += "    staticSettings.classList.add('hidden');";
    html += "  }";
    html += "}";
    html += "// Initialize form state on load";
    html += "document.addEventListener('DOMContentLoaded', function() {";
    html += "  toggleNetworkSettings();";
    html += "  toggleStaticFields();";
    html += "});";
    html += "</script>";
    
    html += "</body></html>";
    configServer.send(200, "text/html", html);
  });
  
  configServer.on("/save", HTTP_POST, []() {
    // Get all parameters from the form
    String newNetworkType = configServer.arg("networkType");
    String newIpType = configServer.arg("ipType");
    String newSSID = configServer.arg("ssid");
    String newPassword = configServer.arg("password");
    String newStaticIP = configServer.arg("staticIP");
    String newStaticGateway = configServer.arg("staticGateway");
    String newStaticSubnet = configServer.arg("staticSubnet");
    String newStaticDNS = configServer.arg("staticDNS");
    String newMqttServer = configServer.arg("mqttServer");
    String newMqttPort = configServer.arg("mqttPort");
    String newMqttUsername = configServer.arg("mqttUsername");
    String newMqttPassword = configServer.arg("mqttPassword");
    String newAccessCode = configServer.arg("accessCode");
    String newClockPin = configServer.arg("clockPin");
    String newReadPin = configServer.arg("readPin");
    String newWritePin = configServer.arg("writePin");
    String newPc16Pin = configServer.arg("pc16Pin");
    
    // Validate required fields
    bool validConfig = true;
    String errorMsg = "";
    
    if (newMqttServer.length() == 0) {
      validConfig = false;
      errorMsg += "MQTT Server is required. ";
    }
    if (newAccessCode.length() == 0) {
      validConfig = false;
      errorMsg += "Access Code is required. ";
    }
    if (newNetworkType == "wifi" && newSSID.length() == 0) {
      validConfig = false;
      errorMsg += "WiFi SSID is required when using WiFi. ";
    }
    
    if (!validConfig) {
      String html = "<!DOCTYPE html><html><head><title>Configuration Error</title>";
      html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;text-align:center}</style></head><body>";
      html += "<h2>Configuration Error</h2>";
      html += "<p>" + errorMsg + "</p>";
      html += "<p><a href='/'>Go back and try again</a></p>";
      html += "</body></html>";
      configServer.send(400, "text/html", html);
      return;
    }
    
    // Update configuration variables
    networkType = newNetworkType;
    ipType = newIpType;
    wifiSSID = newSSID;
    wifiPassword = newPassword;
    staticIP = newStaticIP;
    staticGateway = newStaticGateway;
    staticSubnet = newStaticSubnet;
    staticDNS = newStaticDNS;
    mqttServer = newMqttServer;
    if (newMqttPort.length() > 0) mqttPort = newMqttPort.toInt();
    mqttUsername = newMqttUsername;
    mqttPassword = newMqttPassword;
    accessCode = newAccessCode;
    if (newClockPin.length() > 0) clockPin = newClockPin.toInt();
    if (newReadPin.length() > 0) readPin = newReadPin.toInt();  
    if (newWritePin.length() > 0) writePin = newWritePin.toInt();
    if (newPc16Pin.length() > 0) pc16Pin = newPc16Pin.toInt();
    
    // Save all configuration
    saveFullConfiguration();
    
    String html = "<!DOCTYPE html><html><head><title>DSC Configuration Saved</title>";
    html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;text-align:center}</style></head><body>";
    html += "<h2>Configuration Saved Successfully!</h2>";
    html += "<p>All settings have been saved. The device will now restart and attempt to connect.</p>";
    html += "<p><strong>WiFi:</strong> " + wifiSSID + "</p>";
    html += "<p><strong>MQTT Server:</strong> " + mqttServer + ":" + String(mqttPort) + "</p>";
    html += "<p><strong>DSC Pins:</strong> Clock=" + String(clockPin) + ", Read=" + String(readPin) + ", Write=" + String(writePin) + "</p>";
    html += "<p>If connection fails, the configuration portal will restart automatically.</p>";
    html += "</body></html>";
    configServer.send(200, "text/html", html);
    
    delay(2000);
    ESP.restart();
  });
  
  configServer.begin();
}

bool loadWiFiCredentials() {
  // This function is kept for compatibility but now uses the comprehensive config system
  loadFullConfiguration();
  return (wifiSSID.length() > 0);
}

bool connectToWiFi(String ssid, String password) {
  WiFi.mode(WIFI_STA);
  
  // Configure static IP if needed
  if (ipType == "static") {
    configureStaticIP();
  }
  
  WiFi.begin(ssid.c_str(), password.c_str());
  
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  const int maxAttempts = 60; // 30 seconds with 500ms delay
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(" connected! IP: ");
    Serial.println(WiFi.localIP());
    return true;
  } else {
    Serial.println(" failed!");
    return false;
  }
}

// Network connection functions
void configureStaticIP() {
  if (ipType == "static" && staticIP.length() > 0) {
    IPAddress ip, gateway, subnet, dns;
    
    if (ip.fromString(staticIP) && 
        gateway.fromString(staticGateway.length() > 0 ? staticGateway : "192.168.1.1") &&
        subnet.fromString(staticSubnet.length() > 0 ? staticSubnet : "255.255.255.0")) {
      
      IPAddress dns1 = dns.fromString(staticDNS.length() > 0 ? staticDNS : "8.8.8.8") ? dns : IPAddress(8, 8, 8, 8);
      IPAddress dns2(8, 8, 4, 4); // Google secondary DNS
      
      if (networkType == "wifi") {
        WiFi.config(ip, gateway, subnet, dns1, dns2);
      } else if (networkType == "ethernet") {
        ETH.config(ip, gateway, subnet, dns1, dns2);
      }
      
      Serial.println("Static IP configuration applied:");
      Serial.println("  IP: " + staticIP);
      Serial.println("  Gateway: " + staticGateway);
      Serial.println("  Subnet: " + staticSubnet);
      Serial.println("  DNS: " + staticDNS);
    }
  }
}

bool connectToEthernet() {
  Serial.print("Connecting to Ethernet");
  
  // Configure static IP if needed
  if (ipType == "static") {
    configureStaticIP();
  }
  
  ETH.begin();
  
  int attempts = 0;
  const int maxAttempts = 60; // 30 seconds with 500ms delay
  
  while (!ETH.linkUp() && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (ETH.linkUp()) {
    Serial.print(" connected! IP: ");
    Serial.println(ETH.localIP());
    return true;
  } else {
    Serial.println(" failed!");
    return false;
  }
}

bool connectToNetwork() {
  if (networkType == "ethernet") {
    Serial.println("Connecting via Ethernet...");
    return connectToEthernet();
  } else {
    Serial.println("Connecting via WiFi...");
    if (wifiSSID.length() > 0) {
      return connectToWiFi(wifiSSID, wifiPassword);
    } else {
      Serial.println("WiFi SSID not configured");
      return false;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println();

  Serial.println("DSC Keybus Interface - Comprehensive Configuration System");
  
  // Load all configuration from storage
  loadFullConfiguration();
  
  // Try to connect to network with stored configuration
  bool networkConnected = false;
  
  // Check if we have network configuration
  if (networkType == "ethernet" || (networkType == "wifi" && wifiSSID.length() > 0)) {
    Serial.println("Trying stored network credentials...");
    networkConnected = connectToNetwork();
  }
  
  // If still no connection, start configuration mode
  if (!networkConnected) {
    Serial.println("Network connection failed. Starting configuration portal...");
    startConfigMode();
    return; // Exit setup, loop() will handle config mode
  }

  // Initialize DSC interface with configured pins
  Serial.println("Initializing DSC interface with configured pins...");
  Serial.println("Pin configuration: Clock=" + String(clockPin) + ", Read=" + String(readPin) + 
                 ", Write=" + String(writePin) + ", PC16=" + String(pc16Pin));
  
  #ifndef dscClassicSeries
  if (writePin > 0) {
    dsc = new dscKeybusInterface(clockPin, readPin, writePin);
  } else {
    dsc = new dscKeybusInterface(clockPin, readPin);
  }
  #else
  if (writePin > 0) {
    dsc = new dscClassicInterface(clockPin, readPin, pc16Pin, writePin, accessCode.c_str());
  } else {
    dsc = new dscClassicInterface(clockPin, readPin, pc16Pin, 255, accessCode.c_str());
  }
  #endif

  // Initialize MQTT with configured settings
  Serial.println("Initializing MQTT with server: " + mqttServer + ":" + String(mqttPort));
  mqtt = new PubSubClient(ipClient);
  mqtt->setServer(mqttServer.c_str(), mqttPort);
  mqtt->setCallback(mqttCallback);
  
  if (mqttConnect()) mqttPreviousTime = millis();
  else mqttPreviousTime = 0;

  // Start the DSC Keybus interface
  dsc->begin();
  Serial.println(F("DSC Keybus Interface is online with full configuration."));
  
  // Add configuration endpoint for normal operation (shows same form but without AP mode)
  configServer.on("/config", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>DSC Configuration Portal</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;max-width:800px;margin:20px auto;padding:20px;background:#f5f5f5}";
    html += ".config-section{background:white;padding:20px;margin:20px 0;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}";
    html += "h1{color:#333;text-align:center;margin-bottom:30px}";
    html += "h2{color:#555;border-bottom:2px solid #4CAF50;padding-bottom:10px}";
    html += "label{display:block;margin-top:15px;font-weight:bold;color:#333}";
    html += "input[type=text],input[type=password],input[type=number]{width:100%;padding:12px;margin:8px 0;border:2px solid #ddd;border-radius:4px;box-sizing:border-box}";
    html += "input:focus{border-color:#4CAF50;outline:none}";
    html += "input[type=submit]{background:#4CAF50;color:white;padding:15px 30px;border:none;border-radius:4px;cursor:pointer;width:100%;font-size:16px;margin-top:20px}";
    html += "input[type=submit]:hover{background:#45a049}";
    html += ".info{background:#e7f3ff;border:1px solid #b3d9ff;padding:15px;border-radius:4px;margin:20px 0}";
    html += ".current-value{color:#666;font-size:0.9em;margin-top:5px}";
    html += ".status-info{background:#f0f0f0;padding:15px;border-radius:4px;margin:20px 0}";
    html += "select{width:100%;padding:12px;margin:8px 0;border:2px solid #ddd;border-radius:4px;box-sizing:border-box}";
    html += "select:focus{border-color:#4CAF50;outline:none}";
    html += ".static-fields{margin-top:10px;padding:10px;background:#f8f8f8;border-radius:4px}";
    html += ".hidden{display:none}";
    html += "</style></head><body>";
    
    html += "<h1>DSC Alarm System Configuration</h1>";
    
    html += "<div class='status-info'>";
    html += "<strong>Current Status:</strong><br>";
    if (networkType == "ethernet" && ETH.linkUp()) {
      html += "Network: Ethernet connected (" + ETH.localIP().toString() + ")<br>";
    } else if (networkType == "wifi" && WiFi.status() == WL_CONNECTED) {
      html += "Network: WiFi connected to " + WiFi.SSID() + " (" + WiFi.localIP().toString() + ")<br>";
    } else {
      html += "Network: Disconnected<br>";
    }
    html += "IP Configuration: " + ipType + "<br>";
    html += "MQTT: " + (mqtt != nullptr && mqtt->connected() ? "Connected to " + mqttServer : "Disconnected") + "<br>";
    html += "DSC Interface: " + String(dsc != nullptr && dsc->keybusConnected ? "Connected" : "Disconnected");
    html += "</div>";
    
    html += "<div class='info'><strong>Note:</strong> This portal allows you to reconfigure your DSC system. ";
    html += "Changes will take effect after restart. Empty fields will keep existing values.</div>";
    
    html += "<form method='POST' action='/save-config'>";
    
    // Network Configuration Section
    html += "<div class='config-section'>";
    html += "<h2>Network Configuration</h2>";
    
    // Network Type Selection
    html += "<label for='networkType'>Network Type:</label>";
    html += "<select id='networkType' name='networkType' onchange='toggleNetworkSettings()'>";
    html += "<option value='wifi'";
    if (networkType == "wifi") html += " selected";
    html += ">WiFi</option>";
    html += "<option value='ethernet'";
    if (networkType == "ethernet") html += " selected";
    html += ">Ethernet</option>";
    html += "</select>";
    html += "<div class='current-value'>Current: " + networkType + "</div>";
    
    // IP Configuration Type
    html += "<label for='ipType'>IP Configuration:</label>";
    html += "<select id='ipType' name='ipType' onchange='toggleStaticFields()'>";
    html += "<option value='dhcp'";
    if (ipType == "dhcp") html += " selected";
    html += ">DHCP (Automatic)</option>";
    html += "<option value='static'";
    if (ipType == "static") html += " selected";
    html += ">Static IP</option>";
    html += "</select>";
    html += "<div class='current-value'>Current: " + ipType + "</div>";
    
    // WiFi Settings (shown only when WiFi is selected)
    html += "<div id='wifiSettings' class='";
    if (networkType != "wifi") html += "hidden";
    html += "'>";
    html += "<label for='ssid'>WiFi Network Name (SSID):</label>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='Enter WiFi SSID' value='" + wifiSSID + "'>";
    html += "<div class='current-value'>Current: " + wifiSSID + "</div>";
    html += "<label for='password'>WiFi Password:</label>";
    html += "<input type='password' id='password' name='password' placeholder='Enter WiFi Password'>";
    html += "<div class='current-value'>Leave blank to keep current password</div>";
    html += "</div>";
    
    // Static IP Settings (shown only when Static IP is selected)
    html += "<div id='staticSettings' class='static-fields ";
    if (ipType != "static") html += "hidden";
    html += "'>";
    html += "<label for='staticIP'>Static IP Address:</label>";
    html += "<input type='text' id='staticIP' name='staticIP' placeholder='192.168.1.100' value='" + staticIP + "'>";
    html += "<label for='staticGateway'>Gateway:</label>";
    html += "<input type='text' id='staticGateway' name='staticGateway' placeholder='192.168.1.1' value='" + staticGateway + "'>";
    html += "<label for='staticSubnet'>Subnet Mask:</label>";
    html += "<input type='text' id='staticSubnet' name='staticSubnet' placeholder='255.255.255.0' value='" + staticSubnet + "'>";
    html += "<label for='staticDNS'>DNS Server:</label>";
    html += "<input type='text' id='staticDNS' name='staticDNS' placeholder='8.8.8.8' value='" + staticDNS + "'>";
    html += "</div>";
    html += "</div>";
    
    // MQTT Configuration Section  
    html += "<div class='config-section'>";
    html += "<h2>MQTT Broker Settings</h2>";
    html += "<label for='mqttServer'>MQTT Server/IP:</label>";
    html += "<input type='text' id='mqttServer' name='mqttServer' placeholder='mqtt.example.com or 192.168.1.100' value='" + mqttServer + "'>";
    html += "<div class='current-value'>Current: " + mqttServer + "</div>";
    html += "<label for='mqttPort'>MQTT Port:</label>";
    html += "<input type='number' id='mqttPort' name='mqttPort' placeholder='1883' value='" + String(mqttPort) + "' min='1' max='65535'>";
    html += "<label for='mqttUsername'>MQTT Username (optional):</label>";
    html += "<input type='text' id='mqttUsername' name='mqttUsername' placeholder='Leave blank if not required' value='" + mqttUsername + "'>";
    html += "<label for='mqttPassword'>MQTT Password (optional):</label>";
    html += "<input type='password' id='mqttPassword' name='mqttPassword' placeholder='Leave blank if not required'>";
    html += "<div class='current-value'>Leave blank to keep current password</div>";
    html += "</div>";
    
    // DSC System Configuration
    html += "<div class='config-section'>";
    html += "<h2>DSC System Settings</h2>";
    html += "<label for='accessCode'>Access Code:</label>";
    html += "<input type='text' id='accessCode' name='accessCode' placeholder='Enter DSC access code' value='" + accessCode + "'>";
    html += "<div class='current-value'>Required for disarming and some arm operations</div>";
    html += "</div>";
    
    // Pin Configuration Section
    html += "<div class='config-section'>";
    html += "<h2>ESP32 Pin Assignments</h2>";
    html += "<label for='clockPin'>Clock Pin (DSC Yellow wire):</label>";
    html += "<input type='number' id='clockPin' name='clockPin' value='" + String(clockPin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(clockPin) + "</div>";
    html += "<label for='readPin'>Data Read Pin (DSC Green wire):</label>";
    html += "<input type='number' id='readPin' name='readPin' value='" + String(readPin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(readPin) + "</div>";
    html += "<label for='writePin'>Data Write Pin (Virtual Keypad):</label>";
    html += "<input type='number' id='writePin' name='writePin' value='" + String(writePin) + "' min='0' max='33'>";
    html += "<div class='current-value'>Current: " + String(writePin) + " (set to 0 to disable virtual keypad)</div>";
    html += "<label for='pc16Pin'>PC16 Pin (Classic Series only):</label>";
    html += "<input type='number' id='pc16Pin' name='pc16Pin' value='" + String(pc16Pin) + "' min='4' max='39'>";
    html += "<div class='current-value'>Current: " + String(pc16Pin) + "</div>";
    html += "</div>";
    
    html += "<input type='submit' value='Save Configuration and Restart'>";
    html += "</form>";
    
    html += "<div class='info'><strong>After saving:</strong> The device will restart and apply the new configuration.</div>";
    
    // Add JavaScript for dynamic form behavior
    html += "<script>";
    html += "function toggleNetworkSettings() {";
    html += "  var networkType = document.getElementById('networkType').value;";
    html += "  var wifiSettings = document.getElementById('wifiSettings');";
    html += "  if (networkType === 'wifi') {";
    html += "    wifiSettings.classList.remove('hidden');";
    html += "    document.getElementById('ssid').required = true;";
    html += "    document.getElementById('password').required = false;";
    html += "  } else {";
    html += "    wifiSettings.classList.add('hidden');";
    html += "    document.getElementById('ssid').required = false;";
    html += "    document.getElementById('password').required = false;";
    html += "  }";
    html += "}";
    html += "function toggleStaticFields() {";
    html += "  var ipType = document.getElementById('ipType').value;";
    html += "  var staticSettings = document.getElementById('staticSettings');";
    html += "  if (ipType === 'static') {";
    html += "    staticSettings.classList.remove('hidden');";
    html += "  } else {";
    html += "    staticSettings.classList.add('hidden');";
    html += "  }";
    html += "}";
    html += "// Initialize form state on load";
    html += "document.addEventListener('DOMContentLoaded', function() {";
    html += "  toggleNetworkSettings();";
    html += "  toggleStaticFields();";
    html += "});";
    html += "</script>";
    
    html += "</body></html>";
    configServer.send(200, "text/html", html);
  });
  
  // Add separate save handler for normal mode
  configServer.on("/save-config", HTTP_POST, []() {
    // Get all parameters from the form (same logic as AP mode)
    String newNetworkType = configServer.arg("networkType");
    String newIpType = configServer.arg("ipType");
    String newSSID = configServer.arg("ssid");
    String newPassword = configServer.arg("password");
    String newStaticIP = configServer.arg("staticIP");
    String newStaticGateway = configServer.arg("staticGateway");
    String newStaticSubnet = configServer.arg("staticSubnet");
    String newStaticDNS = configServer.arg("staticDNS");
    String newMqttServer = configServer.arg("mqttServer");
    String newMqttPort = configServer.arg("mqttPort");
    String newMqttUsername = configServer.arg("mqttUsername");
    String newMqttPassword = configServer.arg("mqttPassword");
    String newAccessCode = configServer.arg("accessCode");
    String newClockPin = configServer.arg("clockPin");
    String newReadPin = configServer.arg("readPin");
    String newWritePin = configServer.arg("writePin");
    String newPc16Pin = configServer.arg("pc16Pin");
    
    // Update configuration variables (only if values are provided)
    if (newNetworkType.length() > 0) networkType = newNetworkType;
    if (newIpType.length() > 0) ipType = newIpType;
    if (newSSID.length() > 0) wifiSSID = newSSID;
    if (newPassword.length() > 0) wifiPassword = newPassword;
    if (newStaticIP.length() > 0 || configServer.hasArg("staticIP")) staticIP = newStaticIP;
    if (newStaticGateway.length() > 0 || configServer.hasArg("staticGateway")) staticGateway = newStaticGateway;
    if (newStaticSubnet.length() > 0 || configServer.hasArg("staticSubnet")) staticSubnet = newStaticSubnet;
    if (newStaticDNS.length() > 0 || configServer.hasArg("staticDNS")) staticDNS = newStaticDNS;
    if (newMqttServer.length() > 0) mqttServer = newMqttServer;
    if (newMqttPort.length() > 0) mqttPort = newMqttPort.toInt();
    if (newMqttUsername.length() > 0 || configServer.hasArg("mqttUsername")) mqttUsername = newMqttUsername;
    if (newMqttPassword.length() > 0) mqttPassword = newMqttPassword;
    if (newAccessCode.length() > 0) accessCode = newAccessCode;
    if (newClockPin.length() > 0) clockPin = newClockPin.toInt();
    if (newReadPin.length() > 0) readPin = newReadPin.toInt();
    if (newWritePin.length() > 0) writePin = newWritePin.toInt();
    if (newPc16Pin.length() > 0) pc16Pin = newPc16Pin.toInt();
    
    // Save configuration
    saveFullConfiguration();
    
    String html = "<!DOCTYPE html><html><head><title>DSC Configuration Updated</title>";
    html += "<style>body{font-family:Arial,sans-serif;max-width:600px;margin:50px auto;padding:20px;text-align:center}</style></head><body>";
    html += "<h2>Configuration Updated Successfully!</h2>";
    html += "<p>Settings have been saved. The device will restart in 5 seconds.</p>";
    html += "<p>Please reconnect after restart if WiFi settings were changed.</p>";
    html += "</body></html>";
    configServer.send(200, "text/html", html);
    
    delay(5000);
    ESP.restart();
  });
  
  configServer.begin();
  String currentIP = (networkType == "ethernet" && ETH.linkUp()) ? ETH.localIP().toString() : WiFi.localIP().toString();
  Serial.println("Configuration endpoint available at: http://" + currentIP + "/config");
}


void loop() {
  // If in configuration mode, handle the web server
  if (configMode) {
    configServer.handleClient();
    return;
  }

  // Handle normal web server for config endpoint
  configServer.handleClient();

  mqttHandle();

  if (dsc != nullptr) {
    dsc->loop();

    if (dsc->statusChanged) {      // Checks if the security system status has changed
      dsc->statusChanged = false;  // Reset the status tracking flag

      // If the Keybus data buffer is exceeded, the sketch is too busy to process all Keybus commands.  Call
      // loop() more often, or increase dscBufferSize in the library: src/dscKeybus.h or src/dscClassic.h
      if (dsc->bufferOverflow) {
        Serial.println(F("Keybus buffer overflow"));
        dsc->bufferOverflow = false;
      }

      // Checks if the interface is connected to the Keybus
      if (dsc->keybusChanged) {
        dsc->keybusChanged = false;  // Resets the Keybus data status flag
        if (dsc->keybusConnected) mqtt->publish(mqttStatusTopic, mqttBirthMessage, true);
        else mqtt->publish(mqttStatusTopic, mqttLwtMessage, true);
      }

      // Sends the access code when needed by the panel for arming or command outputs
      if (dsc->accessCodePrompt) {
        dsc->accessCodePrompt = false;
        dsc->write(accessCode.c_str());
      }

      if (dsc->troubleChanged) {
        dsc->troubleChanged = false;  // Resets the trouble status flag
        if (dsc->trouble) mqtt->publish(mqttTroubleTopic, "1", true);
        else mqtt->publish(mqttTroubleTopic, "0", true);
      }

      // Publishes status per partition
      for (byte partition = 0; partition < dscPartitions; partition++) {

        // Skips processing if the partition is disabled or in installer programming
        if (dsc->disabled[partition]) continue;

        // Publishes the partition status message
        publishMessage(mqttPartitionTopic, partition);

        // Publishes armed/disarmed status
        if (dsc->armedChanged[partition]) {
          char publishTopic[strlen(mqttPartitionTopic) + 2];
          appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

          if (dsc->armed[partition]) {
            if (dsc->armedAway[partition] && dsc->noEntryDelay[partition]) mqtt->publish(publishTopic, "armed_night", true);
            else if (dsc->armedAway[partition]) mqtt->publish(publishTopic, "armed_away", true);
            else if (dsc->armedStay[partition] && dsc->noEntryDelay[partition]) mqtt->publish(publishTopic, "armed_night", true);
            else if (dsc->armedStay[partition]) mqtt->publish(publishTopic, "armed_home", true);
          }
          else mqtt->publish(publishTopic, "disarmed", true);
        }

        // Publishes exit delay status
        if (dsc->exitDelayChanged[partition]) {
          dsc->exitDelayChanged[partition] = false;  // Resets the exit delay status flag
          char publishTopic[strlen(mqttPartitionTopic) + 2];
          appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

          if (dsc->exitDelay[partition]) mqtt->publish(publishTopic, "pending", true);  // Publish as a retained message
          else if (!dsc->exitDelay[partition] && !dsc->armed[partition]) mqtt->publish(publishTopic, "disarmed", true);
          // Note: When exit delay ends and system is armed, the armedChanged[partition] flag handles publishing the armed state
        }

        // Publishes alarm status
        if (dsc->alarmChanged[partition]) {
          dsc->alarmChanged[partition] = false;  // Resets the partition alarm status flag
          char publishTopic[strlen(mqttPartitionTopic) + 2];
          appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

          if (dsc->alarm[partition]) mqtt->publish(publishTopic, "triggered", true);  // Alarm tripped
          else if (!dsc->armedChanged[partition]) mqtt->publish(publishTopic, "disarmed", true);
        }
        if (dsc->armedChanged[partition]) dsc->armedChanged[partition] = false;  // Resets the partition armed status flag

        // Publishes fire alarm status
        if (dsc->fireChanged[partition]) {
          dsc->fireChanged[partition] = false;  // Resets the fire status flag
          char publishTopic[strlen(mqttFireTopic) + 2];
          appendPartition(mqttFireTopic, partition, publishTopic);  // Appends the mqttFireTopic with the partition number

          if (dsc->fire[partition]) mqtt->publish(publishTopic, "1");  // Fire alarm tripped
          else mqtt->publish(publishTopic, "0");                      // Fire alarm restored
        }
      }

      // Publishes zones 1-64 status in a separate topic per zone
      // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones:
      //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
      //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
      //   ...
      //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
      if (dsc->openZonesStatusChanged) {
        dsc->openZonesStatusChanged = false;                           // Resets the open zones status flag
        for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
          for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
            if (bitRead(dsc->openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
              bitWrite(dsc->openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag

              // Appends the mqttZoneTopic with the zone number
              char zonePublishTopic[strlen(mqttZoneTopic) + 3];
              char zone[3];
              strcpy(zonePublishTopic, mqttZoneTopic);
              itoa(zoneBit + 1 + (zoneGroup * 8), zone, 10);
              strcat(zonePublishTopic, zone);

              if (bitRead(dsc->openZones[zoneGroup], zoneBit)) {
                mqtt->publish(zonePublishTopic, "1", true);            // Zone open
              }
              else mqtt->publish(zonePublishTopic, "0", true);         // Zone closed
            }
          }
        }
      }

      // Publishes PGM outputs 1-14 status in a separate topic per zone
      // PGM status is stored in the pgmOutputs[] and pgmOutputsChanged[] arrays using 1 bit per PGM output:
      //   pgmOutputs[0] and pgmOutputsChanged[0]: Bit 0 = PGM 1 ... Bit 7 = PGM 8
      //   pgmOutputs[1] and pgmOutputsChanged[1]: Bit 0 = PGM 9 ... Bit 5 = PGM 14
      if (dsc->pgmOutputsStatusChanged) {
        dsc->pgmOutputsStatusChanged = false;  // Resets the PGM outputs status flag
        for (byte pgmGroup = 0; pgmGroup < 2; pgmGroup++) {
          for (byte pgmBit = 0; pgmBit < 8; pgmBit++) {
            if (bitRead(dsc->pgmOutputsChanged[pgmGroup], pgmBit)) {  // Checks an individual PGM output status flag
              bitWrite(dsc->pgmOutputsChanged[pgmGroup], pgmBit, 0);  // Resets the individual PGM output status flag

              // Appends the mqttPgmTopic with the PGM number
              char pgmPublishTopic[strlen(mqttPgmTopic) + 3];
              char pgm[3];
              strcpy(pgmPublishTopic, mqttPgmTopic);
              itoa(pgmBit + 1 + (pgmGroup * 8), pgm, 10);
              strcat(pgmPublishTopic, pgm);

              if (bitRead(dsc->pgmOutputs[pgmGroup], pgmBit)) {
                mqtt->publish(pgmPublishTopic, "1", true);           // PGM enabled
              }
              else mqtt->publish(pgmPublishTopic, "0", true);        // PGM disabled
            }
          }
        }
      }

      mqtt->subscribe(mqttSubscribeTopic);
    }
  }
}


// Handles messages received in the mqttSubscribeTopic
void mqttCallback(char* topic, byte* payload, unsigned int length) {

  // Handles unused parameters
  (void)topic;
  (void)length;

  // Check if DSC is initialized
  if (dsc == nullptr) return;

  byte partition = 0;
  byte payloadIndex = 0;

  // Checks if a partition number 1-8 has been sent and sets the second character as the payload
  if (payload[0] >= 0x31 && payload[0] <= 0x38) {
    partition = payload[0] - 49;
    payloadIndex = 1;
  }

  // Panic alarm
  if (payload[payloadIndex] == 'P') {
    dsc->write('p');
  }

  // Resets status if attempting to change the armed mode while armed or not ready
  if (payload[payloadIndex] != 'D' && !dsc->ready[partition]) {
    dsc->armedChanged[partition] = true;
    dsc->statusChanged = true;
    return;
  }

  // Arm stay
  if (payload[payloadIndex] == 'S' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;         // Sets writes to the partition number
    dsc->write('s');                             // Virtual keypad arm stay
  }

  // Arm away
  else if (payload[payloadIndex] == 'A' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;         // Sets writes to the partition number
    dsc->write('w');                             // Virtual keypad arm away
  }

  // Arm night
  else if (payload[payloadIndex] == 'N' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;         // Sets writes to the partition number
    dsc->write('n');                             // Virtual keypad arm away
  }

  // Disarm
  else if (payload[payloadIndex] == 'D' && (dsc->armed[partition] || dsc->exitDelay[partition] || dsc->alarm[partition])) {
    dsc->writePartition = partition + 1;         // Sets writes to the partition number
    dsc->write(accessCode.c_str());
  }
}


void mqttHandle() {
  if (mqtt == nullptr || !mqtt->connected()) {
    if (mqtt != nullptr) {
      unsigned long mqttCurrentTime = millis();
      if (mqttCurrentTime - mqttPreviousTime > 5000) {
        mqttPreviousTime = mqttCurrentTime;
        if (mqttConnect()) {
          if (dsc != nullptr && dsc->keybusConnected) mqtt->publish(mqttStatusTopic, mqttBirthMessage, true);
          Serial.println(F("MQTT disconnected, successfully reconnected."));
          mqttPreviousTime = 0;
        }
        else Serial.println(F("MQTT disconnected, failed to reconnect."));
      }
    }
  }
  else mqtt->loop();
}


bool mqttConnect() {
  if (mqtt == nullptr) return false;
  
  Serial.print(F("MQTT...."));
  if (mqtt->connect(mqttClientName, mqttUsername.c_str(), mqttPassword.c_str(), mqttStatusTopic, 0, true, mqttLwtMessage)) {
    Serial.print(F("connected: "));
    Serial.println(mqttServer);
    if (dsc != nullptr) dsc->resetStatus();  // Resets the state of all status components as changed to get the current status
  }
  else {
    Serial.print(F("connection error: "));
    Serial.println(mqttServer);
  }
  return mqtt->connected();
}


void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic) {
  char partitionNumber[2];
  strcpy(publishTopic, sourceTopic);
  itoa(sourceNumber + 1, partitionNumber, 10);
  strcat(publishTopic, partitionNumber);
}


// Publishes the partition status message
void publishMessage(const char* sourceTopic, byte partition) {
  if (dsc == nullptr || mqtt == nullptr) return;
  
  char publishTopic[strlen(sourceTopic) + strlen(mqttPartitionMessageSuffix) + 2];
  char partitionNumber[2];

  // Appends the sourceTopic with the partition number and message topic
  itoa(partition + 1, partitionNumber, 10);
  strcpy(publishTopic, sourceTopic);
  strcat(publishTopic, partitionNumber);
  strcat(publishTopic, mqttPartitionMessageSuffix);

  // Publishes the current partition message
  switch (dsc->status[partition]) {
    case 0x01: mqtt->publish(publishTopic, "Partition ready", true); break;
    case 0x02: mqtt->publish(publishTopic, "Stay zones open", true); break;
    case 0x03: mqtt->publish(publishTopic, "Zones open", true); break;
    case 0x04: mqtt->publish(publishTopic, "Armed: Stay", true); break;
    case 0x05: mqtt->publish(publishTopic, "Armed: Away", true); break;
    case 0x06: mqtt->publish(publishTopic, "Armed: Stay with no entry delay", true); break;
    case 0x07: mqtt->publish(publishTopic, "Failed to arm", true); break;
    case 0x08: mqtt->publish(publishTopic, "Exit delay in progress", true); break;
    case 0x09: mqtt->publish(publishTopic, "Arming with no entry delay", true); break;
    case 0x0B: mqtt->publish(publishTopic, "Quick exit in progress", true); break;
    case 0x0C: mqtt->publish(publishTopic, "Entry delay in progress", true); break;
    case 0x0D: mqtt->publish(publishTopic, "Entry delay after alarm", true); break;
    case 0x0E: mqtt->publish(publishTopic, "Function not available"); break;
    case 0x10: mqtt->publish(publishTopic, "Keypad lockout", true); break;
    case 0x11: mqtt->publish(publishTopic, "Partition in alarm", true); break;
    case 0x12: mqtt->publish(publishTopic, "Battery check in progress"); break;
    case 0x14: mqtt->publish(publishTopic, "Auto-arm in progress", true); break;
    case 0x15: mqtt->publish(publishTopic, "Arming with bypassed zones", true); break;
    case 0x16: mqtt->publish(publishTopic, "Armed: Away with no entry delay", true); break;
    case 0x17: mqtt->publish(publishTopic, "Power saving: Keypad blanked", true); break;
    case 0x19: mqtt->publish(publishTopic, "Disarmed: Alarm memory"); break;
    case 0x22: mqtt->publish(publishTopic, "Disarmed: Recent closing", true); break;
    case 0x2F: mqtt->publish(publishTopic, "Keypad LCD test"); break;
    case 0x33: mqtt->publish(publishTopic, "Command output in progress", true); break;
    case 0x3D: mqtt->publish(publishTopic, "Disarmed: Alarm memory", true); break;
    case 0x3E: mqtt->publish(publishTopic, "Partition disarmed", true); break;
    case 0x40: mqtt->publish(publishTopic, "Keypad blanked", true); break;
    case 0x8A: mqtt->publish(publishTopic, "Activate stay/away zones", true); break;
    case 0x8B: mqtt->publish(publishTopic, "Quick exit", true); break;
    case 0x8E: mqtt->publish(publishTopic, "Function not available", true); break;
    case 0x8F: mqtt->publish(publishTopic, "Invalid access code", true); break;
    case 0x9E: mqtt->publish(publishTopic, "Enter * function key", true); break;
    case 0x9F: mqtt->publish(publishTopic, "Enter access code", true); break;
    case 0xA0: mqtt->publish(publishTopic, "*1: Zone bypass", true); break;
    case 0xA1: mqtt->publish(publishTopic, "*2: Trouble menu", true); break;
    case 0xA2: mqtt->publish(publishTopic, "*3: Alarm memory", true); break;
    case 0xA3: mqtt->publish(publishTopic, "*4: Door chime enabled", true); break;
    case 0xA4: mqtt->publish(publishTopic, "*4: Door chime disabled", true); break;
    case 0xA5: mqtt->publish(publishTopic, "Enter master code", true); break;
    case 0xA6: mqtt->publish(publishTopic, "*5: Access codes", true); break;
    case 0xA7: mqtt->publish(publishTopic, "*5: Enter new 4-digit code", true); break;
    case 0xA9: mqtt->publish(publishTopic, "*6: User functions", true); break;
    case 0xAA: mqtt->publish(publishTopic, "*6: Time and date", true); break;
    case 0xAB: mqtt->publish(publishTopic, "*6: Auto-arm time", true); break;
    case 0xAC: mqtt->publish(publishTopic, "*6: Auto-arm enabled", true); break;
    case 0xAD: mqtt->publish(publishTopic, "*6: Auto-arm disabled", true); break;
    case 0xAF: mqtt->publish(publishTopic, "*6: System test", true); break;
    case 0xB0: mqtt->publish(publishTopic, "*6: Enable DLS", true); break;
    case 0xB2: mqtt->publish(publishTopic, "*7: Command output", true); break;
    case 0xB3: mqtt->publish(publishTopic, "*7: Command output", true); break;
    case 0xB7: mqtt->publish(publishTopic, "Enter installer code", true); break;
    case 0xB8: mqtt->publish(publishTopic, "Enter * function key while armed", true); break;
    case 0xB9: mqtt->publish(publishTopic, "*2: Zone tamper menu", true); break;
    case 0xBA: mqtt->publish(publishTopic, "*2: Zones with low batteries", true); break;
    case 0xBC: mqtt->publish(publishTopic, "*5: Enter new 6-digit code"); break;
    case 0xBF: mqtt->publish(publishTopic, "*6: Auto-arm select day"); break;
    case 0xC6: mqtt->publish(publishTopic, "*2: Zone fault menu", true); break;
    case 0xC8: mqtt->publish(publishTopic, "*2: Service required menu", true); break;
    case 0xCD: mqtt->publish(publishTopic, "Downloading in progress"); break;
    case 0xCE: mqtt->publish(publishTopic, "Active camera monitor selection"); break;
    case 0xD0: mqtt->publish(publishTopic, "*2: Keypads with low batteries", true); break;
    case 0xD1: mqtt->publish(publishTopic, "*2: Keyfobs with low batteries", true); break;
    case 0xD4: mqtt->publish(publishTopic, "*2: Sensors with RF delinquency", true); break;
    case 0xE4: mqtt->publish(publishTopic, "*8: Installer programming, 3 digits", true); break;
    case 0xE5: mqtt->publish(publishTopic, "Keypad slot assignment", true); break;
    case 0xE6: mqtt->publish(publishTopic, "Input: 2 digits", true); break;
    case 0xE7: mqtt->publish(publishTopic, "Input: 3 digits", true); break;
    case 0xE8: mqtt->publish(publishTopic, "Input: 4 digits", true); break;
    case 0xE9: mqtt->publish(publishTopic, "Input: 5 digits", true); break;
    case 0xEA: mqtt->publish(publishTopic, "Input HEX: 2 digits", true); break;
    case 0xEB: mqtt->publish(publishTopic, "Input HEX: 4 digits", true); break;
    case 0xEC: mqtt->publish(publishTopic, "Input HEX: 6 digits", true); break;
    case 0xED: mqtt->publish(publishTopic, "Input HEX: 32 digits", true); break;
    case 0xEE: mqtt->publish(publishTopic, "Input: 1 option per zone", true); break;
    case 0xEF: mqtt->publish(publishTopic, "Module supervision field", true); break;
    case 0xF0: mqtt->publish(publishTopic, "Function key 1", true); break;
    case 0xF1: mqtt->publish(publishTopic, "Function key 2", true); break;
    case 0xF2: mqtt->publish(publishTopic, "Function key 3", true); break;
    case 0xF3: mqtt->publish(publishTopic, "Function key 4", true); break;
    case 0xF4: mqtt->publish(publishTopic, "Function key 5", true); break;
    case 0xF5: mqtt->publish(publishTopic, "Wireless module placement test", true); break;
    case 0xF6: mqtt->publish(publishTopic, "Activate device for test"); break;
    case 0xF7: mqtt->publish(publishTopic, "*8: Installer programming, 2 digits", true); break;
    case 0xF8: mqtt->publish(publishTopic, "Keypad programming", true); break;
    case 0xFA: mqtt->publish(publishTopic, "Input: 6 digits"); break;
    default: return;
  }
}
