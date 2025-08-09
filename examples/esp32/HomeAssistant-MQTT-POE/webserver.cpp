#include "webserver.h"
#include "config.h"
#include <WiFi.h>
#include <ETH.h>
#include <time.h>

// External references to global objects
extern DSCConfig config;
extern bool ethernetConnected;
extern bool networkConnected;

// Web server instance
WebServer server(80);
DebugInfo debugInfo;

// Setup web server routes
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/config/save", HTTP_POST, handleConfigSave);
  server.on("/debug", handleDebug);
  server.on("/restart", handleRestart);
  server.on("/scan", handleWiFiScan);
  server.on("/mqtt-test", handleMQTTTest);
  
  server.begin();
  Serial.println("Web server started on port 80");
}

// Handle web server requests
void handleWebServer() {
  server.handleClient();
}

// Root page handler
void handleRoot() {
  String html = getHTMLHeader("DSC Alarm Interface");
  html += "<div class='container'>";
  html += "<h1>DSC Keybus Interface - ESP32 POE</h1>";
  html += "<div class='menu'>";
  html += "<a href='/config' class='button'>Configuration</a>";
  html += "<a href='/debug' class='button'>Debug Status</a>";
  html += "<a href='/restart' class='button danger'>Restart System</a>";
  html += "</div>";
  
  html += "<h2>System Information</h2>";
  html += "<table>";
  html += "<tr><td>Module Type:</td><td>" + String(getModuleName(config.moduleType)) + "</td></tr>";
  html += "<tr><td>DSC Clock Pin:</td><td>GPIO " + String(config.dscClockPin) + "</td></tr>";
  html += "<tr><td>DSC Read Pin:</td><td>GPIO " + String(config.dscReadPin) + "</td></tr>";
  html += "<tr><td>DSC Write Pin:</td><td>GPIO " + String(config.dscWritePin) + "</td></tr>";
  html += "<tr><td>Network Mode:</td><td>" + String(config.useEthernet ? "Ethernet" : "WiFi") + "</td></tr>";
  html += "<tr><td>IP Address:</td><td>" + (config.useEthernet ? (ethernetConnected ? ETH.localIP().toString() : "Not connected") : WiFi.localIP().toString()) + "</td></tr>";
  html += "<tr><td>MQTT Server:</td><td>" + String(config.mqttServer) + ":" + String(config.mqttPort) + "</td></tr>";
  html += "<tr><td>MQTT Status:</td><td>" + String(debugInfo.mqttConnected ? "Connected" : "Disconnected") + "</td></tr>";
  html += "<tr><td>Uptime:</td><td>" + String(millis() / 1000) + " seconds</td></tr>";
  html += "</table>";
  
  html += "</div>";
  html += getHTMLFooter();
  
  server.send(200, "text/html", html);
}

// Configuration page handler
void handleConfig() {
  server.send(200, "text/html", getConfigPage());
}

// Configuration save handler
void handleConfigSave() {
  // Module and pin configuration
  if (server.hasArg("module_type")) {
    int moduleType = server.arg("module_type").toInt();
    if (moduleType >= MODULE_ESP32_GENERIC && moduleType <= MODULE_CUSTOM) {
      config.moduleType = (ModuleType)moduleType;
      
      // Set default pins if not custom module
      if (config.moduleType != MODULE_CUSTOM) {
        setDefaultPinsForModule(config.moduleType);
      }
    }
  }
  
  // Custom pin configuration (only applied if custom module or explicitly set)
  if (server.hasArg("dsc_clock_pin")) {
    config.dscClockPin = server.arg("dsc_clock_pin").toInt();
  }
  if (server.hasArg("dsc_read_pin")) {
    config.dscReadPin = server.arg("dsc_read_pin").toInt();
  }
  if (server.hasArg("dsc_pc16_pin")) {
    config.dscPC16Pin = server.arg("dsc_pc16_pin").toInt();
  }
  if (server.hasArg("dsc_write_pin")) {
    config.dscWritePin = server.arg("dsc_write_pin").toInt();
  }
  
  // Network configuration
  if (server.hasArg("network_mode")) {
    config.useEthernet = (server.arg("network_mode") == "ethernet");
  }
  
  if (server.hasArg("ip_mode")) {
    config.useDHCP = (server.arg("ip_mode") == "dhcp");
  }
  
  // WiFi settings
  if (server.hasArg("wifi_ssid")) {
    strncpy(config.wifiSSID, server.arg("wifi_ssid").c_str(), sizeof(config.wifiSSID) - 1);
  }
  if (server.hasArg("wifi_password")) {
    strncpy(config.wifiPassword, server.arg("wifi_password").c_str(), sizeof(config.wifiPassword) - 1);
  }
  
  // Static IP settings
  if (server.hasArg("static_ip")) {
    strncpy(config.staticIP, server.arg("static_ip").c_str(), sizeof(config.staticIP) - 1);
  }
  if (server.hasArg("static_gateway")) {
    strncpy(config.staticGateway, server.arg("static_gateway").c_str(), sizeof(config.staticGateway) - 1);
  }
  if (server.hasArg("static_subnet")) {
    strncpy(config.staticSubnet, server.arg("static_subnet").c_str(), sizeof(config.staticSubnet) - 1);
  }
  if (server.hasArg("static_dns")) {
    strncpy(config.staticDNS, server.arg("static_dns").c_str(), sizeof(config.staticDNS) - 1);
  }
  
  // MQTT settings
  if (server.hasArg("mqtt_server")) {
    strncpy(config.mqttServer, server.arg("mqtt_server").c_str(), sizeof(config.mqttServer) - 1);
  }
  if (server.hasArg("mqtt_port")) {
    config.mqttPort = server.arg("mqtt_port").toInt();
  }
  if (server.hasArg("mqtt_username")) {
    strncpy(config.mqttUsername, server.arg("mqtt_username").c_str(), sizeof(config.mqttUsername) - 1);
  }
  if (server.hasArg("mqtt_password")) {
    strncpy(config.mqttPassword, server.arg("mqtt_password").c_str(), sizeof(config.mqttPassword) - 1);
  }
  
  // Access code
  if (server.hasArg("access_code")) {
    strncpy(config.accessCode, server.arg("access_code").c_str(), sizeof(config.accessCode) - 1);
  }
  
  // Zone configuration
  for (int i = 0; i < 8; i++) {
    String zoneName = "zone" + String(i + 1) + "_name";
    String zoneClass = "zone" + String(i + 1) + "_class";
    String zoneEnabled = "zone" + String(i + 1) + "_enabled";
    
    if (server.hasArg(zoneName)) {
      strncpy(config.zones[i].name, server.arg(zoneName).c_str(), sizeof(config.zones[i].name) - 1);
    }
    if (server.hasArg(zoneClass)) {
      strncpy(config.zones[i].deviceClass, server.arg(zoneClass).c_str(), sizeof(config.zones[i].deviceClass) - 1);
    }
    config.zones[i].enabled = server.hasArg(zoneEnabled);
  }
  
  // Save configuration
  saveConfig();
  
  // Redirect back to config page with success message
  String html = getHTMLHeader("Configuration Saved");
  html += "<div class='container'>";
  html += "<h1>Configuration Saved</h1>";
  html += "<p>Configuration has been saved successfully. The system will need to be restarted for network changes to take effect.</p>";
  html += "<a href='/config' class='button'>Back to Configuration</a>";
  html += "<a href='/restart' class='button danger'>Restart System</a>";
  html += "</div>";
  html += getHTMLFooter();
  
  server.send(200, "text/html", html);
}

// Debug page handler
void handleDebug() {
  server.send(200, "text/html", getDebugPage());
}

// Restart handler
void handleRestart() {
  String html = getHTMLHeader("System Restart");
  html += "<div class='container'>";
  html += "<h1>System Restart</h1>";
  html += "<p>The system is restarting...</p>";
  html += "<p>Please wait a moment and refresh the page.</p>";
  html += "</div>";
  html += getHTMLFooter();
  
  server.send(200, "text/html", html);
  delay(1000);
  ESP.restart();
}

// WiFi scan handler
void handleWiFiScan() {
  String json = "[";
  int n = WiFi.scanNetworks();
  
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false");
    json += "}";
  }
  json += "]";
  
  server.send(200, "application/json", json);
}

// MQTT test handler
void handleMQTTTest() {
  // This would implement MQTT connection testing
  // For now, return current status
  String json = "{";
  json += "\"connected\":" + String(debugInfo.mqttConnected ? "true" : "false") + ",";
  json += "\"last_connect\":" + String(debugInfo.mqttLastConnectTime) + ",";
  json += "\"reconnect_count\":" + String(debugInfo.mqttReconnectCount);
  json += "}";
  
  server.send(200, "application/json", json);
}

// Utility functions
void addAlarmEvent(const char* description, uint8_t partition) {
  strncpy(debugInfo.recentAlarms[debugInfo.alarmIndex].description, description, sizeof(debugInfo.recentAlarms[0].description) - 1);
  strncpy(debugInfo.recentAlarms[debugInfo.alarmIndex].timestamp, getTimeString().c_str(), sizeof(debugInfo.recentAlarms[0].timestamp) - 1);
  debugInfo.recentAlarms[debugInfo.alarmIndex].partition = partition;
  debugInfo.alarmIndex = (debugInfo.alarmIndex + 1) % 5;
}

void addAccessCodeEvent(const char* code, bool valid) {
  strncpy(debugInfo.recentCodes[debugInfo.codeIndex].code, getMaskedCode(code).c_str(), sizeof(debugInfo.recentCodes[0].code) - 1);
  strncpy(debugInfo.recentCodes[debugInfo.codeIndex].timestamp, getTimeString().c_str(), sizeof(debugInfo.recentCodes[0].timestamp) - 1);
  debugInfo.recentCodes[debugInfo.codeIndex].valid = valid;
  debugInfo.codeIndex = (debugInfo.codeIndex + 1) % 5;
}

void updateZoneStatus(int zone, bool open, bool motion, bool glassBreak, bool fire, bool pgmOutput) {
  if (zone >= 0 && zone < 8) {
    debugInfo.zones[zone].open = open;
    debugInfo.zones[zone].motion = motion;
    debugInfo.zones[zone].glassBreak = glassBreak;
    debugInfo.zones[zone].fire = fire;
    debugInfo.zones[zone].pgmOutput = pgmOutput;
    debugInfo.zones[zone].lastChanged = millis();
  }
}

String getTimeString() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
  }
  return String(millis() / 1000) + "s";
}

String getMaskedCode(const char* code) {
  int len = strlen(code);
  if (len == 0) return "****";
  return String(code[0]) + "***";
}

// HTML Templates
String getHTMLHeader(const char* title) {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>" + String(title) + "</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: #f5f5f5; }";
  html += ".container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }";
  html += "h1, h2 { color: #333; }";
  html += "table { width: 100%; border-collapse: collapse; margin: 10px 0; }";
  html += "th, td { padding: 8px; text-align: left; border-bottom: 1px solid #ddd; }";
  html += "th { background-color: #f8f9fa; }";
  html += ".button { display: inline-block; padding: 10px 20px; margin: 5px; text-decoration: none; background: #007bff; color: white; border-radius: 4px; border: none; cursor: pointer; }";
  html += ".button:hover { background: #0056b3; }";
  html += ".button.danger { background: #dc3545; }";
  html += ".button.danger:hover { background: #c82333; }";
  html += ".menu { margin: 20px 0; }";
  html += "input, select { padding: 8px; margin: 4px; border: 1px solid #ddd; border-radius: 4px; width: 200px; }";
  html += "input[type=checkbox] { width: auto; }";
  html += ".form-group { margin: 15px 0; }";
  html += "label { display: inline-block; width: 200px; font-weight: bold; }";
  html += ".status-ok { color: #28a745; }";
  html += ".status-error { color: #dc3545; }";
  html += ".zone-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 15px; }";
  html += ".zone-card { border: 1px solid #ddd; padding: 15px; border-radius: 8px; background: #f8f9fa; }";
  html += "</style></head><body>";
  return html;
}

String getHTMLFooter() {
  return "</body></html>";
}

String getConfigPage() {
  String html = getHTMLHeader("Configuration");
  html += "<div class='container'>";
  html += "<h1>DSC Interface Configuration</h1>";
  html += "<form action='/config/save' method='post'>";
  
  // Module and Pin Configuration
  html += "<h2>Hardware Configuration</h2>";
  html += "<div class='form-group'>";
  html += "<label>Module Type:</label>";
  html += "<select name='module_type' onchange='updatePinDefaults()'>";
  html += "<option value='" + String(MODULE_ESP32_GENERIC) + "'" + String(config.moduleType == MODULE_ESP32_GENERIC ? " selected" : "") + ">ESP32 Generic</option>";
  html += "<option value='" + String(MODULE_ESP32_POE) + "'" + String(config.moduleType == MODULE_ESP32_POE ? " selected" : "") + ">ESP32-POE (Olimex)</option>";
  html += "<option value='" + String(MODULE_ESP32_S2) + "'" + String(config.moduleType == MODULE_ESP32_S2 ? " selected" : "") + ">ESP32-S2</option>";
  html += "<option value='" + String(MODULE_ESP32_C3) + "'" + String(config.moduleType == MODULE_ESP32_C3 ? " selected" : "") + ">ESP32-C3</option>";
  html += "<option value='" + String(MODULE_CUSTOM) + "'" + String(config.moduleType == MODULE_CUSTOM ? " selected" : "") + ">Custom</option>";
  html += "</select>";
  html += "</div>";
  
  html += "<h3>DSC Keybus Pin Configuration</h3>";
  html += "<div class='form-group'>";
  html += "<label>Clock Pin (Yellow):</label>";
  html += "<input type='number' name='dsc_clock_pin' value='" + String(config.dscClockPin) + "' min='0' max='39'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>Data Pin (Green):</label>";
  html += "<input type='number' name='dsc_read_pin' value='" + String(config.dscReadPin) + "' min='0' max='39'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>PC16 Pin (Classic only):</label>";
  html += "<input type='number' name='dsc_pc16_pin' value='" + String(config.dscPC16Pin) + "' min='0' max='39'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>Write Pin (Keypad out):</label>";
  html += "<input type='number' name='dsc_write_pin' value='" + String(config.dscWritePin) + "' min='0' max='39'>";
  html += "</div>";
  
  // Network Configuration
  html += "<h2>Network Configuration</h2>";
  html += "<div class='form-group'>";
  html += "<label>Network Mode:</label>";
  html += "<select name='network_mode'>";
  html += "<option value='ethernet'" + String(config.useEthernet ? " selected" : "") + ">Ethernet (POE)</option>";
  html += "<option value='wifi'" + String(!config.useEthernet ? " selected" : "") + ">WiFi</option>";
  html += "</select>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>IP Configuration:</label>";
  html += "<select name='ip_mode'>";
  html += "<option value='dhcp'" + String(config.useDHCP ? " selected" : "") + ">DHCP (Automatic)</option>";
  html += "<option value='static'" + String(!config.useDHCP ? " selected" : "") + ">Static IP</option>";
  html += "</select>";
  html += "</div>";
  
  // WiFi Settings
  html += "<h3>WiFi Settings</h3>";
  html += "<div class='form-group'>";
  html += "<label>WiFi SSID:</label>";
  html += "<input type='text' name='wifi_ssid' value='" + String(config.wifiSSID) + "'>";
  html += "<button type='button' onclick='scanWiFi()' class='button'>Scan Networks</button>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>WiFi Password:</label>";
  html += "<input type='password' name='wifi_password' value='" + String(config.wifiPassword) + "'>";
  html += "</div>";
  
  // Static IP Settings
  html += "<h3>Static IP Settings</h3>";
  html += "<div class='form-group'>";
  html += "<label>IP Address:</label>";
  html += "<input type='text' name='static_ip' value='" + String(config.staticIP) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>Gateway:</label>";
  html += "<input type='text' name='static_gateway' value='" + String(config.staticGateway) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>Subnet Mask:</label>";
  html += "<input type='text' name='static_subnet' value='" + String(config.staticSubnet) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>DNS Server:</label>";
  html += "<input type='text' name='static_dns' value='" + String(config.staticDNS) + "'>";
  html += "</div>";
  
  // MQTT Configuration
  html += "<h2>MQTT Configuration</h2>";
  html += "<div class='form-group'>";
  html += "<label>MQTT Server:</label>";
  html += "<input type='text' name='mqtt_server' value='" + String(config.mqttServer) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>MQTT Port:</label>";
  html += "<input type='number' name='mqtt_port' value='" + String(config.mqttPort) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>MQTT Username:</label>";
  html += "<input type='text' name='mqtt_username' value='" + String(config.mqttUsername) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<label>MQTT Password:</label>";
  html += "<input type='password' name='mqtt_password' value='" + String(config.mqttPassword) + "'>";
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<button type='button' onclick='testMQTT()' class='button'>Test MQTT Connection</button>";
  html += "</div>";
  
  // DSC Configuration
  html += "<h2>DSC System Configuration</h2>";
  html += "<div class='form-group'>";
  html += "<label>Access Code:</label>";
  html += "<input type='password' name='access_code' value='" + String(config.accessCode) + "' maxlength='6'>";
  html += "</div>";
  
  // Zone Configuration
  html += "<h2>Zone Configuration</h2>";
  html += "<div class='zone-grid'>";
  for (int i = 0; i < 8; i++) {
    html += "<div class='zone-card'>";
    html += "<h4>Zone " + String(i + 1) + "</h4>";
    html += "<div class='form-group'>";
    html += "<label>Enabled:</label>";
    html += "<input type='checkbox' name='zone" + String(i + 1) + "_enabled'" + String(config.zones[i].enabled ? " checked" : "") + ">";
    html += "</div>";
    html += "<div class='form-group'>";
    html += "<label>Name:</label>";
    html += "<input type='text' name='zone" + String(i + 1) + "_name' value='" + String(config.zones[i].name) + "'>";
    html += "</div>";
    html += "<div class='form-group'>";
    html += "<label>Device Class:</label>";
    html += "<select name='zone" + String(i + 1) + "_class'>";
    html += "<option value='door'" + String(strcmp(config.zones[i].deviceClass, "door") == 0 ? " selected" : "") + ">Door</option>";
    html += "<option value='window'" + String(strcmp(config.zones[i].deviceClass, "window") == 0 ? " selected" : "") + ">Window</option>";
    html += "<option value='motion'" + String(strcmp(config.zones[i].deviceClass, "motion") == 0 ? " selected" : "") + ">Motion</option>";
    html += "<option value='smoke'" + String(strcmp(config.zones[i].deviceClass, "smoke") == 0 ? " selected" : "") + ">Smoke</option>";
    html += "<option value='glass'" + String(strcmp(config.zones[i].deviceClass, "glass") == 0 ? " selected" : "") + ">Glass Break</option>";
    html += "<option value='opening'" + String(strcmp(config.zones[i].deviceClass, "opening") == 0 ? " selected" : "") + ">Opening</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
  }
  html += "</div>";
  
  html += "<div class='form-group'>";
  html += "<button type='submit' class='button'>Save Configuration</button>";
  html += "<a href='/' class='button'>Cancel</a>";
  html += "</div>";
  
  html += "</form>";
  
  // JavaScript for additional functionality
  html += "<script>";
  
  // Pin defaults for different modules
  html += "const pinDefaults = {";
  html += "  " + String(MODULE_ESP32_GENERIC) + ": {clock: 4, read: 16, pc16: 17, write: 21},";
  html += "  " + String(MODULE_ESP32_POE) + ": {clock: 13, read: 16, pc16: 32, write: 33},";
  html += "  " + String(MODULE_ESP32_S2) + ": {clock: 1, read: 3, pc16: 5, write: 7},";
  html += "  " + String(MODULE_ESP32_C3) + ": {clock: 0, read: 1, pc16: 2, write: 3}";
  html += "};";
  
  html += "function updatePinDefaults() {";
  html += "  const moduleSelect = document.querySelector('select[name=module_type]');";
  html += "  const moduleType = moduleSelect.value;";
  html += "  if (moduleType != " + String(MODULE_CUSTOM) + " && pinDefaults[moduleType]) {";
  html += "    const pins = pinDefaults[moduleType];";
  html += "    document.querySelector('input[name=dsc_clock_pin]').value = pins.clock;";
  html += "    document.querySelector('input[name=dsc_read_pin]').value = pins.read;";
  html += "    document.querySelector('input[name=dsc_pc16_pin]').value = pins.pc16;";
  html += "    document.querySelector('input[name=dsc_write_pin]').value = pins.write;";
  html += "  }";
  html += "}";
  
  html += "function scanWiFi() {";
  html += "  fetch('/scan').then(response => response.json()).then(data => {";
  html += "    let ssidSelect = document.querySelector('input[name=wifi_ssid]');";
  html += "    let html = '';";
  html += "    data.forEach(network => {";
  html += "      html += '<option value=\"' + network.ssid + '\">' + network.ssid + ' (' + network.rssi + 'dBm)' + (network.secure ? ' 🔒' : '') + '</option>';";
  html += "    });";
  html += "    if (data.length > 0) {";
  html += "      ssidSelect.outerHTML = '<select name=\"wifi_ssid\" onchange=\"document.querySelector(\\'input[name=wifi_ssid]\\').value=this.value\">' + html + '</select>';";
  html += "    }";
  html += "  });";
  html += "}";
  html += "function testMQTT() {";
  html += "  fetch('/mqtt-test').then(response => response.json()).then(data => {";
  html += "    alert('MQTT Status: ' + (data.connected ? 'Connected' : 'Disconnected'));";
  html += "  });";
  html += "}";
  html += "</script>";
  
  html += "</div>";
  html += getHTMLFooter();
  
  return html;
}

String getDebugPage() {
  String html = getHTMLHeader("Debug Status");
  html += "<div class='container'>";
  html += "<h1>Debug Status</h1>";
  
  html += "<h2>System Status</h2>";
  html += "<table>";
  html += "<tr><td>System Uptime</td><td>" + String(millis() / 1000) + " seconds</td></tr>";
  html += "<tr><td>Free Heap</td><td>" + String(ESP.getFreeHeap()) + " bytes</td></tr>";
  html += "<tr><td>Network Mode</td><td>" + String(config.useEthernet ? "Ethernet" : "WiFi") + "</td></tr>";
  html += "<tr><td>IP Address</td><td>" + (config.useEthernet ? (ethernetConnected ? ETH.localIP().toString() : "Not connected") : WiFi.localIP().toString()) + "</td></tr>";
  html += "<tr><td>MAC Address</td><td>" + (config.useEthernet ? ETH.macAddress() : WiFi.macAddress()) + "</td></tr>";
  html += "</table>";
  
  html += "<h2>MQTT Status</h2>";
  html += "<table>";
  html += "<tr><td>Connection Status</td><td class='" + String(debugInfo.mqttConnected ? "status-ok" : "status-error") + "'>" + String(debugInfo.mqttConnected ? "Connected" : "Disconnected") + "</td></tr>";
  html += "<tr><td>Last Connect Time</td><td>" + String(debugInfo.mqttLastConnectTime) + "</td></tr>";
  html += "<tr><td>Reconnect Count</td><td>" + String(debugInfo.mqttReconnectCount) + "</td></tr>";
  html += "<tr><td>Server</td><td>" + String(config.mqttServer) + ":" + String(config.mqttPort) + "</td></tr>";
  html += "</table>";
  
  html += "<h2>Alarm System Status</h2>";
  html += "<table>";
  html += "<tr><td>Keybus Connection</td><td class='" + String(debugInfo.alarmSystemConnected ? "status-ok" : "status-error") + "'>" + String(debugInfo.alarmSystemConnected ? "Connected" : "Disconnected") + "</td></tr>";
  html += "</table>";
  
  html += "<h2>Zone Status</h2>";
  html += "<div class='zone-grid'>";
  for (int i = 0; i < 8; i++) {
    html += "<div class='zone-card'>";
    html += "<h4>" + String(config.zones[i].name) + " (Zone " + String(i + 1) + ")</h4>";
    html += "<table>";
    html += "<tr><td>Open/Closed</td><td class='" + String(debugInfo.zones[i].open ? "status-error" : "status-ok") + "'>" + String(debugInfo.zones[i].open ? "Open" : "Closed") + "</td></tr>";
    html += "<tr><td>Motion</td><td class='" + String(debugInfo.zones[i].motion ? "status-error" : "status-ok") + "'>" + String(debugInfo.zones[i].motion ? "Motion" : "No Motion") + "</td></tr>";
    html += "<tr><td>Glass Break</td><td class='" + String(debugInfo.zones[i].glassBreak ? "status-error" : "status-ok") + "'>" + String(debugInfo.zones[i].glassBreak ? "Broken" : "OK") + "</td></tr>";
    html += "<tr><td>Fire</td><td class='" + String(debugInfo.zones[i].fire ? "status-error" : "status-ok") + "'>" + String(debugInfo.zones[i].fire ? "Fire" : "No Fire") + "</td></tr>";
    html += "<tr><td>PGM Output</td><td class='" + String(debugInfo.zones[i].pgmOutput ? "status-ok" : "status-error") + "'>" + String(debugInfo.zones[i].pgmOutput ? "Active" : "Inactive") + "</td></tr>";
    html += "<tr><td>Last Changed</td><td>" + String(debugInfo.zones[i].lastChanged > 0 ? String(debugInfo.zones[i].lastChanged / 1000) + "s ago" : "Never") + "</td></tr>";
    html += "</table>";
    html += "</div>";
  }
  html += "</div>";
  
  html += "<h2>Recent Alarm Events</h2>";
  html += "<table>";
  html += "<tr><th>Timestamp</th><th>Description</th><th>Partition</th></tr>";
  for (int i = 0; i < 5; i++) {
    int idx = (debugInfo.alarmIndex - 1 - i + 5) % 5;
    if (strlen(debugInfo.recentAlarms[idx].description) > 0) {
      html += "<tr>";
      html += "<td>" + String(debugInfo.recentAlarms[idx].timestamp) + "</td>";
      html += "<td>" + String(debugInfo.recentAlarms[idx].description) + "</td>";
      html += "<td>" + String(debugInfo.recentAlarms[idx].partition) + "</td>";
      html += "</tr>";
    }
  }
  html += "</table>";
  
  html += "<h2>Recent Access Code Entries</h2>";
  html += "<table>";
  html += "<tr><th>Timestamp</th><th>Code (Masked)</th><th>Valid</th></tr>";
  for (int i = 0; i < 5; i++) {
    int idx = (debugInfo.codeIndex - 1 - i + 5) % 5;
    if (strlen(debugInfo.recentCodes[idx].code) > 0) {
      html += "<tr>";
      html += "<td>" + String(debugInfo.recentCodes[idx].timestamp) + "</td>";
      html += "<td>" + String(debugInfo.recentCodes[idx].code) + "</td>";
      html += "<td class='" + String(debugInfo.recentCodes[idx].valid ? "status-ok" : "status-error") + "'>" + String(debugInfo.recentCodes[idx].valid ? "Valid" : "Invalid") + "</td>";
      html += "</tr>";
    }
  }
  html += "</table>";
  
  html += "<div class='menu'>";
  html += "<a href='/' class='button'>Home</a>";
  html += "<a href='javascript:location.reload()' class='button'>Refresh</a>";
  html += "</div>";
  
  html += "</div>";
  html += getHTMLFooter();
  
  return html;
}