/*
 *  HomeAssistant-MQTT-POE 1.0 (esp32 with POE support)
 *
 *  Processes the security system status and allows for control using Home Assistant via MQTT.
 *  Features web-based configuration interface and comprehensive debugging.
 *  Supports both Ethernet (with POE) and WiFi connectivity.
 *
 *  Home Assistant: https://www.home-assistant.io
 *  Mosquitto MQTT broker: https://mosquitto.org
 *  Olimex ESP32-POE: https://www.olimex.com/Products/IoT/ESP32/ESP32-POE/
 *
 *  Usage:
 *    1. Connect to the ESP32-POE via Ethernet or WiFi
 *    2. Access the web interface at the device's IP address
 *    3. Configure network settings, MQTT server, and alarm system settings
 *    4. Set up Home Assistant configuration using the same MQTT topics
 *    5. Monitor and debug via the web interface
 *
 *  Web Interface Features:
 *    - Network configuration (Ethernet/WiFi, DHCP/Static)
 *    - MQTT server configuration and testing
 *    - Zone configuration and naming
 *    - Debug interface with system status and recent events
 *    - Live zone status monitoring
 *
 *  Hardware Support:
 *    - Olimex ESP32-POE (primary target)
 *    - Standard ESP32 with optional Ethernet module
 *    - WiFi fallback when Ethernet is unavailable
 *
 *  Release notes:
 *    1.0 - Initial ESP32-POE release
 *        - Web-based configuration interface
 *        - Ethernet and WiFi support
 *        - Configuration persistence in flash
 *        - Debug monitoring interface
 *        - Zone configuration and status
 *        - MQTT connection testing
 *        - HomeAssistant compatibility preserved
 *
 *  Wiring (ESP32-POE):
 *      DSC Aux(+) --- 5v voltage regulator --- ESP32-POE 5v pin
 *      DSC Aux(-) --- ESP32-POE Ground
 *
 *                                         +--- dscClockPin  // Default: GPIO 4 (changed from 18 to avoid Ethernet conflict)
 *      DSC Yellow --- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *                                         +--- dscReadPin   // Default: GPIO 16 (changed from 19)
 *      DSC Green ---- 33k ohm resistor ---|
 *                                         +--- 10k ohm resistor --- Ground
 *
 *      Virtual keypad (optional):
 *      DSC Green ---- NPN collector --\
 *                                      |-- NPN base --- 1k ohm resistor --- dscWritePin  // Default: GPIO 21
 *            Ground --- NPN emitter --/
 *
 *      Ethernet: Built-in ESP32-POE connector (LAN8720 PHY uses GPIO 18,23,12,0)
 *      WiFi: Built-in ESP32 WiFi
 *
 *  This example code is in the public domain.
 */

// DSC Classic series: uncomment for PC1500/PC1550 support (requires PC16-OUT configuration per README.md)
//#define dscClassicSeries

#include <WiFi.h>
#include <ETH.h>
#include <PubSubClient.h>
#include <dscKeybusInterface.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <time.h>
#include "config.h"
#include "webserver.h"

// Pin definitions - now configured via web interface instead of hardcoded
// Default values are set in config.h for ESP32-POE: 13, 16, 32, 33
// These can be changed via the web configuration interface

// Network clients
WiFiClient wifiClient;
WiFiClient ethClient;
PubSubClient mqtt;

// DNS server for captive portal in AP mode
DNSServer dnsServer;
const byte DNS_PORT = 53;

// System variables
unsigned long mqttPreviousTime = 0;
unsigned long lastWebUpdate = 0;
bool networkConnected = false;
bool ethernetConnected = false;
bool accessPointMode = false;

// DSC Keybus Interface - will be initialized with configurable pins in setup()
dscKeybusInterface* dsc;

// Function prototypes
void setupNetwork();
void setupEthernet();
void setupWiFi();
void startAccessPointMode();
void handleNetworkEvents();
void mqttHandle();
bool mqttConnect();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void processDSCStatus();
void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic);
void publishMessage(const char* sourceTopic, byte partition);
void updateDebugInfo();

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("DSC Keybus Interface - ESP32 POE Starting...");
  
  // Load configuration from flash
  loadConfig();
  
  // Initialize DSC Keybus Interface with configurable pins
  Serial.printf("Initializing DSC interface with pins: Clock=%d, Read=%d, PC16=%d, Write=%d\n", 
                config.dscClockPin, config.dscReadPin, config.dscPC16Pin, config.dscWritePin);
                
  #ifndef dscClassicSeries
    dsc = new dscKeybusInterface(config.dscClockPin, config.dscReadPin, config.dscWritePin);
  #else
    dsc = new dscClassicInterface(config.dscClockPin, config.dscReadPin, config.dscPC16Pin, config.dscWritePin, config.accessCode);
  #endif
  
  // Check if we have network configuration - if not, start AP mode
  bool hasNetworkConfig = (config.useEthernet || (strlen(config.wifiSSID) > 0));
  
  if (!hasNetworkConfig) {
    Serial.println("No network configuration found, starting Access Point mode...");
    startAccessPointMode();
  } else {
    // Setup network connectivity
    setupNetwork();
    
    // Wait for network connection
    int timeout = 30000; // 30 seconds
    unsigned long startTime = millis();
    while (!networkConnected && (millis() - startTime) < timeout) {
      handleNetworkEvents();
      delay(100);
    }
    
    if (!networkConnected) {
      Serial.println("Failed to connect to configured network!");
      Serial.println("Starting Access Point mode for reconfiguration...");
      startAccessPointMode();
    }
  }
  
  // Setup web server and other services if connected (either normal network or AP mode)
  if (networkConnected) {
    if (!accessPointMode) {
      Serial.print("Network connected - IP: ");
      if (config.useEthernet && ethernetConnected) {
        Serial.println(ETH.localIP());
      } else {
        Serial.println(WiFi.localIP());
      }
      
      // Setup time (for timestamps)
      configTime(0, 0, "pool.ntp.org");
      
      // Setup MQTT
      if (strlen(config.mqttServer) > 0) {
        if (config.useEthernet && ethernetConnected) {
          mqtt.setClient(ethClient);
        } else {
          mqtt.setClient(wifiClient);
        }
        mqtt.setServer(config.mqttServer, config.mqttPort);
        mqtt.setCallback(mqttCallback);
        
        if (mqttConnect()) {
          mqttPreviousTime = millis();
          debugInfo.mqttConnected = true;
          debugInfo.mqttLastConnectTime = millis();
          
          // Reset DSC status to get current state if DSC is initialized
          if (dsc && dsc->keybusConnected) {
            dsc->resetStatus();
          }
        }
      }
    }
    
    // Setup web server (for both normal mode and AP mode)
    setupWebServer();
  }
  
  // Initialize DSC Keybus Interface
  dsc->begin();
  Serial.println("DSC Keybus Interface initialized");
  
  // Initialize debug info
  debugInfo.systemUptime = millis();
  
  Serial.println("System startup complete");
}

void loop() {
  // Handle DNS server for captive portal in AP mode
  if (accessPointMode) {
    dnsServer.processNextRequest();
  }
  
  // Handle network connectivity
  handleNetworkEvents();
  
  // Handle web server requests
  handleWebServer();
  
  // Handle MQTT
  if (networkConnected && !accessPointMode) {
    mqttHandle();
  }
  
  // Process DSC Keybus
  if (dsc) {
    dsc->loop();
    processDSCStatus();
  }
  
  // Update debug information periodically
  if (millis() - lastWebUpdate > 1000) {
    updateDebugInfo();
    lastWebUpdate = millis();
  }
}

void setupNetwork() {
  if (config.useEthernet) {
    Serial.println("Setting up Ethernet...");
    setupEthernet();
    delay(2000); // Give Ethernet time to initialize
    
    // If Ethernet fails, fall back to WiFi if configured
    if (!ethernetConnected && strlen(config.wifiSSID) > 0) {
      Serial.println("Ethernet failed, trying WiFi...");
      setupWiFi();
    }
  } else {
    Serial.println("Setting up WiFi...");
    setupWiFi();
  }
}

void setupEthernet() {
  // Initialize Ethernet for ESP32-POE (Olimex board) with LAN8720 PHY
  // Parameters: PHY_ADDR, PHY_POWER, PHY_MDC, PHY_MDIO, PHY_TYPE, CLK_MODE
  // ESP32-POE uses external crystal oscillator, so using GPIO0_IN mode
  ETH.begin(0, 12, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN);
  
  Serial.println("Ethernet PHY initialized (LAN8720)");
  
  if (!config.useDHCP) {
    IPAddress ip, gateway, subnet, dns;
    ip.fromString(config.staticIP);
    gateway.fromString(config.staticGateway);
    subnet.fromString(config.staticSubnet);
    dns.fromString(config.staticDNS);
    
    ETH.config(ip, gateway, subnet, dns);
    Serial.println("Static IP configuration applied");
  }
  
  // Wait for connection with more detailed status reporting
  Serial.print("Waiting for Ethernet link");
  int attempts = 0;
  while (!ethernetConnected && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (ETH.linkUp()) {
      ethernetConnected = true;
      networkConnected = true;
      Serial.println();
      Serial.print("Ethernet connected! IP: ");
      Serial.println(ETH.localIP());
      Serial.print("MAC: ");
      Serial.println(ETH.macAddress());
      break;
    }
  }
  
  if (!ethernetConnected) {
    Serial.println();
    Serial.println("Ethernet connection failed - no link detected");
    Serial.println("Check cable, POE power, and ensure no GPIO conflicts");
  }
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  
  if (!config.useDHCP) {
    IPAddress ip, gateway, subnet, dns;
    ip.fromString(config.staticIP);
    gateway.fromString(config.staticGateway);
    subnet.fromString(config.staticSubnet);
    dns.fromString(config.staticDNS);
    
    WiFi.config(ip, gateway, subnet, dns);
  }
  
  WiFi.begin(config.wifiSSID, config.wifiPassword);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    networkConnected = true;
    Serial.println();
    Serial.println("WiFi connected!");
  }
}

void startAccessPointMode() {
  // Create AP with default name using MAC address
  String apName = "DSC-Config-" + WiFi.macAddress().substring(9);
  apName.replace(":", "");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName.c_str(), "configure123");
  
  // Start DNS server for captive portal
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  Serial.println("Access Point started");
  Serial.println("SSID: " + apName);
  Serial.println("Password: configure123");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  networkConnected = true; // Consider AP mode as connected
  accessPointMode = true;
  
  // Setup web server for configuration
  setupWebServer();
  
  Serial.println("Connect to the AP and navigate to http://" + WiFi.softAPIP().toString() + " to configure");
  Serial.println("Or navigate to any URL to be redirected to configuration page (captive portal)");
}

void handleNetworkEvents() {
  // Check network connectivity and handle reconnections
  if (config.useEthernet) {
    if (ETH.linkUp() && !ethernetConnected) {
      ethernetConnected = true;
      networkConnected = true;
      Serial.println("Ethernet reconnected");
    } else if (!ETH.linkUp() && ethernetConnected) {
      ethernetConnected = false;
      networkConnected = false;
      Serial.println("Ethernet disconnected");
    }
  } else {
    if (WiFi.status() == WL_CONNECTED && !networkConnected) {
      networkConnected = true;
      Serial.println("WiFi reconnected");
    } else if (WiFi.status() != WL_CONNECTED && networkConnected) {
      networkConnected = false;
      Serial.println("WiFi disconnected");
    }
  }
}

void mqttHandle() {
  if (!mqtt.connected()) {
    debugInfo.mqttConnected = false;
    unsigned long mqttCurrentTime = millis();
    if (mqttCurrentTime - mqttPreviousTime > 5000) {
      mqttPreviousTime = mqttCurrentTime;
      if (mqttConnect()) {
        debugInfo.mqttConnected = true;
        debugInfo.mqttLastConnectTime = millis();
        debugInfo.mqttReconnectCount++;
        
        if (dsc->keybusConnected) {
          mqtt.publish(config.mqttStatusTopic, "online", true);
        }
        Serial.println("MQTT reconnected");
        mqttPreviousTime = 0;
      } else {
        Serial.println("MQTT connection failed");
      }
    }
  } else {
    mqtt.loop();
  }
}

bool mqttConnect() {
  if (strlen(config.mqttServer) == 0) return false;
  
  Serial.print("Connecting to MQTT...");
  bool connected = mqtt.connect(
    config.mqttClientName, 
    config.mqttUsername, 
    config.mqttPassword, 
    config.mqttStatusTopic, 
    0, 
    true, 
    "offline"
  );
  
  if (connected) {
    Serial.println(" connected");
    if (dsc) dsc->resetStatus(); // Get current status
    mqtt.subscribe(config.mqttSubscribeTopic);
  } else {
    Serial.println(" failed");
  }
  
  return connected;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Handle unused parameters
  (void)topic;
  (void)length;

  if (!dsc) return; // Safety check

  byte partition = 0;
  byte payloadIndex = 0;

  // Check if a partition number 1-8 has been sent
  if (payload[0] >= 0x31 && payload[0] <= 0x38) {
    partition = payload[0] - 49;
    payloadIndex = 1;
  }

  // Log access code attempts
  if (payloadIndex == 1 && payload[1] == 'D') {
    addAccessCodeEvent(config.accessCode, true); // Assume valid if we're processing disarm
  }

  // Panic alarm
  if (payload[payloadIndex] == 'P') {
    dsc->write('p');
    addAlarmEvent("Panic alarm triggered", partition + 1);
    return;
  }

  // Check if system is ready for arming commands
  if (payload[payloadIndex] != 'D' && !dsc->ready[partition]) {
    dsc->armedChanged[partition] = true;
    dsc->statusChanged = true;
    return;
  }

  // Arm stay
  if (payload[payloadIndex] == 'S' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;
    dsc->write('s');
    addAlarmEvent("Armed stay", partition + 1);
  }
  // Arm away
  else if (payload[payloadIndex] == 'A' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;
    dsc->write('w');
    addAlarmEvent("Armed away", partition + 1);
  }
  // Arm night
  else if (payload[payloadIndex] == 'N' && !dsc->armed[partition] && !dsc->exitDelay[partition]) {
    dsc->writePartition = partition + 1;
    dsc->write('n');
    addAlarmEvent("Armed night", partition + 1);
  }
  // Disarm
  else if (payload[payloadIndex] == 'D' && (dsc->armed[partition] || dsc->exitDelay[partition] || dsc->alarm[partition])) {
    dsc->writePartition = partition + 1;
    dsc->write(config.accessCode);
    addAlarmEvent("Disarmed", partition + 1);
  }
}

void processDSCStatus() {
  if (!dsc) return; // Safety check
  
  if (dsc->statusChanged) {
    dsc->statusChanged = false;

    // Handle buffer overflow
    if (dsc->bufferOverflow) {
      Serial.println("Keybus buffer overflow");
      dsc->bufferOverflow = false;
    }

    // Handle keybus connection changes
    if (dsc->keybusChanged) {
      dsc->keybusChanged = false;
      debugInfo.alarmSystemConnected = dsc->keybusConnected;
      
      if (mqtt.connected()) {
        if (dsc->keybusConnected) {
          mqtt.publish(config.mqttStatusTopic, "online", true);
        } else {
          mqtt.publish(config.mqttStatusTopic, "offline", true);
        }
      }
    }

    // Send access code when prompted
    if (dsc->accessCodePrompt) {
      dsc->accessCodePrompt = false;
      dsc->write(config.accessCode);
    }

    // Handle trouble status
    if (dsc->troubleChanged) {
      dsc->troubleChanged = false;
      if (mqtt.connected()) {
        mqtt.publish(config.mqttTroubleTopic, dsc->trouble ? "1" : "0", true);
      }
      if (dsc->trouble) {
        addAlarmEvent("System trouble detected");
      }
    }

    // Process each partition
    for (byte partition = 0; partition < dscPartitions; partition++) {
      if (dsc->disabled[partition]) continue;

      // Publish partition status message
      if (mqtt.connected()) {
        publishMessage(config.mqttPartitionTopic, partition);
      }

      // Handle armed status changes
      if (dsc->armedChanged[partition]) {
        char publishTopic[strlen(config.mqttPartitionTopic) + 2];
        appendPartition(config.mqttPartitionTopic, partition, publishTopic);

        if (mqtt.connected()) {
          if (dsc->armed[partition]) {
            if (dsc->armedAway[partition] && dsc->noEntryDelay[partition]) {
              mqtt.publish(publishTopic, "armed_night", true);
            } else if (dsc->armedAway[partition]) {
              mqtt.publish(publishTopic, "armed_away", true);
            } else if (dsc->armedStay[partition] && dsc->noEntryDelay[partition]) {
              mqtt.publish(publishTopic, "armed_night", true);
            } else if (dsc->armedStay[partition]) {
              mqtt.publish(publishTopic, "armed_home", true);
            }
          } else {
            mqtt.publish(publishTopic, "disarmed", true);
          }
        }
        dsc->armedChanged[partition] = false;
      }

      // Handle exit delay
      if (dsc->exitDelayChanged[partition]) {
        dsc->exitDelayChanged[partition] = false;
        char publishTopic[strlen(config.mqttPartitionTopic) + 2];
        appendPartition(config.mqttPartitionTopic, partition, publishTopic);

        if (mqtt.connected()) {
          if (dsc->exitDelay[partition]) {
            mqtt.publish(publishTopic, "pending", true);
          } else if (!dsc->exitDelay[partition] && !dsc->armed[partition]) {
            mqtt.publish(publishTopic, "disarmed", true);
          }
        }
      }

      // Handle alarm status
      if (dsc->alarmChanged[partition]) {
        dsc->alarmChanged[partition] = false;
        char publishTopic[strlen(config.mqttPartitionTopic) + 2];
        appendPartition(config.mqttPartitionTopic, partition, publishTopic);

        if (dsc->alarm[partition]) {
          if (mqtt.connected()) {
            mqtt.publish(publishTopic, "triggered", true);
          }
          addAlarmEvent("Alarm triggered", partition + 1);
        } else if (!dsc->armedChanged[partition] && mqtt.connected()) {
          mqtt.publish(publishTopic, "disarmed", true);
        }
      }

      // Handle fire alarm status
      if (dsc->fireChanged[partition]) {
        dsc->fireChanged[partition] = false;
        char publishTopic[strlen(config.mqttFireTopic) + 2];
        appendPartition(config.mqttFireTopic, partition, publishTopic);

        if (mqtt.connected()) {
          if (dsc->fire[partition]) {
            mqtt.publish(publishTopic, "1");
            addAlarmEvent("Fire alarm", partition + 1);
          } else {
            mqtt.publish(publishTopic, "0");
          }
        }
      }
    }

    // Handle zone status changes
    if (dsc->openZonesStatusChanged) {
      dsc->openZonesStatusChanged = false;
      
      for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
        for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
          if (bitRead(dsc->openZonesChanged[zoneGroup], zoneBit)) {
            bitWrite(dsc->openZonesChanged[zoneGroup], zoneBit, 0);

            int zoneNumber = zoneBit + (zoneGroup * 8);
            bool zoneOpen = bitRead(dsc->openZones[zoneGroup], zoneBit);
            
            // Update debug info
            if (zoneNumber < 8) {
              updateZoneStatus(zoneNumber, zoneOpen);
            }

            // Publish to MQTT
            if (mqtt.connected()) {
              char zonePublishTopic[strlen(config.mqttZoneTopic) + 3];
              char zone[3];
              strcpy(zonePublishTopic, config.mqttZoneTopic);
              itoa(zoneNumber + 1, zone, 10);
              strcat(zonePublishTopic, zone);

              mqtt.publish(zonePublishTopic, zoneOpen ? "1" : "0", true);
            }
          }
        }
      }
    }

    // Handle PGM output status changes
    if (dsc->pgmOutputsStatusChanged) {
      dsc->pgmOutputsStatusChanged = false;
      
      for (byte pgmGroup = 0; pgmGroup < 2; pgmGroup++) {
        for (byte pgmBit = 0; pgmBit < 8; pgmBit++) {
          if (bitRead(dsc->pgmOutputsChanged[pgmGroup], pgmBit)) {
            bitWrite(dsc->pgmOutputsChanged[pgmGroup], pgmBit, 0);

            int pgmNumber = pgmBit + (pgmGroup * 8);
            bool pgmActive = bitRead(dsc->pgmOutputs[pgmGroup], pgmBit);
            
            // Update debug info for first 8 PGM outputs (matching zones)
            if (pgmNumber < 8) {
              debugInfo.zones[pgmNumber].pgmOutput = pgmActive;
            }

            // Publish to MQTT
            if (mqtt.connected()) {
              char pgmPublishTopic[strlen(config.mqttPgmTopic) + 3];
              char pgm[3];
              strcpy(pgmPublishTopic, config.mqttPgmTopic);
              itoa(pgmNumber + 1, pgm, 10);
              strcat(pgmPublishTopic, pgm);

              mqtt.publish(pgmPublishTopic, pgmActive ? "1" : "0", true);
            }
          }
        }
      }
    }

    // Subscribe to MQTT commands
    if (mqtt.connected()) {
      mqtt.subscribe(config.mqttSubscribeTopic);
    }
  }
}

void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic) {
  char partitionNumber[2];
  strcpy(publishTopic, sourceTopic);
  itoa(sourceNumber + 1, partitionNumber, 10);
  strcat(publishTopic, partitionNumber);
}

void publishMessage(const char* sourceTopic, byte partition) {
  char publishTopic[strlen(sourceTopic) + 10]; // Extra space for "/Message" suffix
  char partitionNumber[2];

  // Appends the sourceTopic with partition number and "/Message" suffix for compatibility
  itoa(partition + 1, partitionNumber, 10);
  strcpy(publishTopic, sourceTopic);
  strcat(publishTopic, partitionNumber);
  strcat(publishTopic, "/Message");  // This creates topics like "dsc/Get/Partition1/Message"

  // Publish current partition message based on status (compatible with original implementation)
  switch (dsc->status[partition]) {
    case 0x01: mqtt.publish(publishTopic, "Partition ready", true); break;
    case 0x02: mqtt.publish(publishTopic, "Stay zones open", true); break;
    case 0x03: mqtt.publish(publishTopic, "Zones open", true); break;
    case 0x04: mqtt.publish(publishTopic, "Armed: Stay", true); break;
    case 0x05: mqtt.publish(publishTopic, "Armed: Away", true); break;
    case 0x06: mqtt.publish(publishTopic, "Armed: Stay with no entry delay", true); break;
    case 0x07: mqtt.publish(publishTopic, "Failed to arm", true); break;
    case 0x08: mqtt.publish(publishTopic, "Exit delay in progress", true); break;
    case 0x09: mqtt.publish(publishTopic, "Arming with no entry delay", true); break;
    case 0x0B: mqtt.publish(publishTopic, "Quick exit in progress", true); break;
    case 0x0C: mqtt.publish(publishTopic, "Entry delay in progress", true); break;
    case 0x0D: mqtt.publish(publishTopic, "Entry delay after alarm", true); break;
    case 0x0E: mqtt.publish(publishTopic, "Function not available"); break;
    case 0x10: mqtt.publish(publishTopic, "Keypad lockout", true); break;
    case 0x11: mqtt.publish(publishTopic, "Partition in alarm", true); break;
    case 0x12: mqtt.publish(publishTopic, "Battery check in progress"); break;
    case 0x14: mqtt.publish(publishTopic, "Auto-arm in progress", true); break;
    case 0x15: mqtt.publish(publishTopic, "Arming with bypassed zones", true); break;
    case 0x16: mqtt.publish(publishTopic, "Armed: Away with no entry delay", true); break;
    case 0x17: mqtt.publish(publishTopic, "Power saving: Keypad blanked", true); break;
    case 0x19: mqtt.publish(publishTopic, "Disarmed: Alarm memory"); break;
    case 0x22: mqtt.publish(publishTopic, "Disarmed: Recent closing", true); break;
    case 0x2F: mqtt.publish(publishTopic, "Keypad LCD test"); break;
    case 0x33: mqtt.publish(publishTopic, "Command output in progress", true); break;
    case 0x3D: mqtt.publish(publishTopic, "Disarmed: Alarm memory", true); break;
    case 0x3E: mqtt.publish(publishTopic, "Partition disarmed", true); break;
    case 0x40: mqtt.publish(publishTopic, "Keypad blanked", true); break;
    case 0x8A: mqtt.publish(publishTopic, "Activate stay/away zones", true); break;
    case 0x8B: mqtt.publish(publishTopic, "Quick exit", true); break;
    case 0x8E: mqtt.publish(publishTopic, "Function not available", true); break;
    case 0x8F: mqtt.publish(publishTopic, "Invalid access code", true); break;
    case 0x9E: mqtt.publish(publishTopic, "Enter * function key", true); break;
    case 0x9F: mqtt.publish(publishTopic, "Enter access code", true); break;
    case 0xA0: mqtt.publish(publishTopic, "*1: Zone bypass", true); break;
    case 0xA1: mqtt.publish(publishTopic, "*2: Trouble menu", true); break;
    case 0xA2: mqtt.publish(publishTopic, "*3: Alarm memory", true); break;
    case 0xA3: mqtt.publish(publishTopic, "*4: Door chime enabled", true); break;
    case 0xA4: mqtt.publish(publishTopic, "*4: Door chime disabled", true); break;
    case 0xA5: mqtt.publish(publishTopic, "Enter master code", true); break;
    case 0xA6: mqtt.publish(publishTopic, "*5: Access codes", true); break;
    case 0xA7: mqtt.publish(publishTopic, "*5: Enter new 4-digit code", true); break;
    case 0xA9: mqtt.publish(publishTopic, "*6: User functions", true); break;
    case 0xAA: mqtt.publish(publishTopic, "*6: Time and date", true); break;
    case 0xAB: mqtt.publish(publishTopic, "*6: Auto-arm time", true); break;
    case 0xAC: mqtt.publish(publishTopic, "*6: Auto-arm enabled", true); break;
    case 0xAD: mqtt.publish(publishTopic, "*6: Auto-arm disabled", true); break;
    case 0xAF: mqtt.publish(publishTopic, "*6: System test", true); break;
    case 0xB0: mqtt.publish(publishTopic, "*6: Enable DLS", true); break;
    case 0xB2: mqtt.publish(publishTopic, "*7: Command output", true); break;
    case 0xB3: mqtt.publish(publishTopic, "*7: Command output", true); break;
    case 0xB7: mqtt.publish(publishTopic, "Enter installer code", true); break;
    case 0xB8: mqtt.publish(publishTopic, "Enter * function key while armed", true); break;
    case 0xB9: mqtt.publish(publishTopic, "*2: Zone tamper menu", true); break;
    case 0xBA: mqtt.publish(publishTopic, "*2: Zones with low batteries", true); break;
    case 0xBC: mqtt.publish(publishTopic, "*5: Enter new 6-digit code"); break;
    case 0xBF: mqtt.publish(publishTopic, "*6: Auto-arm select day"); break;
    case 0xC6: mqtt.publish(publishTopic, "*2: Zone fault menu", true); break;
    case 0xC8: mqtt.publish(publishTopic, "*2: Service required menu", true); break;
    case 0xCD: mqtt.publish(publishTopic, "Downloading in progress"); break;
    case 0xCE: mqtt.publish(publishTopic, "Active camera monitor selection"); break;
    case 0xD0: mqtt.publish(publishTopic, "*2: Keypads with low batteries", true); break;
    case 0xD1: mqtt.publish(publishTopic, "*2: Keyfobs with low batteries", true); break;
    case 0xD4: mqtt.publish(publishTopic, "*2: Sensors with RF delinquency", true); break;
    case 0xE4: mqtt.publish(publishTopic, "*8: Installer programming, 3 digits", true); break;
    case 0xE5: mqtt.publish(publishTopic, "Keypad slot assignment", true); break;
    case 0xE6: mqtt.publish(publishTopic, "Input: 2 digits", true); break;
    case 0xE7: mqtt.publish(publishTopic, "Input: 3 digits", true); break;
    case 0xE8: mqtt.publish(publishTopic, "Input: 4 digits", true); break;
    case 0xE9: mqtt.publish(publishTopic, "Input: 5 digits", true); break;
    case 0xEA: mqtt.publish(publishTopic, "Input HEX: 2 digits", true); break;
    case 0xEB: mqtt.publish(publishTopic, "Input HEX: 4 digits", true); break;
    case 0xEC: mqtt.publish(publishTopic, "Input HEX: 6 digits", true); break;
    case 0xED: mqtt.publish(publishTopic, "Input HEX: 32 digits", true); break;
    case 0xEE: mqtt.publish(publishTopic, "Input: 1 option per zone", true); break;
    case 0xEF: mqtt.publish(publishTopic, "Module supervision field", true); break;
    case 0xF0: mqtt.publish(publishTopic, "Function key 1", true); break;
    case 0xF1: mqtt.publish(publishTopic, "Function key 2", true); break;
    case 0xF2: mqtt.publish(publishTopic, "Function key 3", true); break;
    case 0xF3: mqtt.publish(publishTopic, "Function key 4", true); break;
    case 0xF4: mqtt.publish(publishTopic, "Function key 5", true); break;
    case 0xF5: mqtt.publish(publishTopic, "Wireless module placement test", true); break;
    case 0xF6: mqtt.publish(publishTopic, "Activate device for test"); break;
    case 0xF7: mqtt.publish(publishTopic, "*8: Installer programming, 2 digits", true); break;
    case 0xF8: mqtt.publish(publishTopic, "Keypad programming", true); break;
    case 0xFA: mqtt.publish(publishTopic, "Input: 6 digits"); break;
    default: return;
  }
}

void updateDebugInfo() {
  debugInfo.systemUptime = millis();
  debugInfo.mqttConnected = mqtt.connected();
  debugInfo.alarmSystemConnected = dsc ? dsc->keybusConnected : false;
}