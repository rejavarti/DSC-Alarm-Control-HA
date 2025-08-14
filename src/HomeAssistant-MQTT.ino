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
 *    Partition 1 disarm: "1D" (uses configured access code)
 *    Partition 1 disarm with custom code: "1!7730" (uses specified access code)
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
#define dscClassicSeries

#include <WiFi.h>
#include <PubSubClient.h>
#include <dscKeybusInterface.h>
#include <esp_task_wdt.h>  // For watchdog timer management

// Configuration - CHANGE THESE VALUES FOR YOUR SETUP
// Security Note: For production use, consider using WiFiManager or SPIFFS/LittleFS to store credentials
const char* wifiSSID = "IoT_devices";
const char* wifiPassword = "943Nelson8034";
const char* accessCode = "7730";
const char* mqttServer = "192.168.222.41";
const int mqttPort = 1883;
const char* mqttUsername = "homeassistant";
const char* mqttPassword = "ofaibah2Luj3un7niezi3ooshieyaey1kahsooz3xeeJ9oobok8ishaesiephish";

// System Configuration
const unsigned long wifiReconnectInterval = 30000;     // WiFi reconnection attempt interval (30 seconds)
const unsigned long mqttReconnectInterval = 5000;      // MQTT reconnection attempt interval (5 seconds)
const unsigned long statusPublishInterval = 60000;     // Status publish interval (60 seconds)
const byte maxReconnectAttempts = 10;                   // Maximum reconnection attempts before restart
const bool enableDebugLogging = true;                  // Enable detailed debug logging


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

// Configures the Keybus interface with the specified pins - dscWritePin is optional, leaving it out disables the
// virtual keypad.
#define dscClockPin 18  // 4,13,16-39
#define dscReadPin  19  // 4,13,16-39
#define dscPC16Pin  17  // DSC Classic Series only, 4,13,16-39
#define dscWritePin 21  // 4,13,16-33

// Initialize components
#ifndef dscClassicSeries
dscKeybusInterface dsc(dscClockPin, dscReadPin, dscWritePin);
#else
dscClassicInterface dsc(dscClockPin, dscReadPin, dscPC16Pin, dscWritePin, accessCode);
#endif
WiFiClient ipClient;
PubSubClient mqtt(mqttServer, mqttPort, ipClient);
unsigned long mqttPreviousTime;

// System health and monitoring variables
unsigned long wifiReconnectAttempts = 0;
unsigned long mqttReconnectAttempts = 0;
unsigned long lastStatusPublish = 0;
unsigned long systemUptime = 0;
unsigned long lastWifiRecoveryAttempt = 0;
bool systemHealthy = true;
String lastError = "";

// Tracks the previous status for each partition to detect status changes
byte previousStatus[dscPartitions];

// Function prototypes for improved error handling
bool connectWiFiWithRetry();
bool connectMQTTWithRetry();
void publishSystemHealth();
void handleSystemError(const String& error);
void logMessage(const String& message, bool isError = false);


void setup() {
  Serial.begin(115200);
  delay(1000);
  yield();  // Feed watchdog early
  
  // Configure watchdog timer for 30 seconds to prevent timeout during setup
  esp_task_wdt_init(30, true);  // 30 second timeout, enable panic
  esp_task_wdt_add(NULL);       // Add current task to watchdog
  
  Serial.println();
  Serial.println(F("=== DSC Keybus Interface with Home Assistant Integration ==="));
  Serial.println(F("Version: 1.5 Enhanced - Improved Robustness"));
  
  systemUptime = millis();
  logMessage("System starting up...");

  // Initialize previous status tracking with bounds checking
  if (dscPartitions > 0 && dscPartitions <= 8) {  // Validate dscPartitions is reasonable
    for (byte i = 0; i < dscPartitions; i++) {
      previousStatus[i] = 0xFF;  // Initialize to invalid status to force initial publication
    }
    logMessage("Status tracking initialized for " + String(dscPartitions) + " partitions");
  } else {
    handleSystemError("Invalid dscPartitions value: " + String(dscPartitions));
    // Use safe default
    for (byte i = 0; i < 4; i++) {
      previousStatus[i] = 0xFF;
    }
    logMessage("Using safe default partition count of 4", true);
  }
  
  esp_task_wdt_reset();  // Reset watchdog after initialization

  // Connect to WiFi with retry logic
  if (!connectWiFiWithRetry()) {
    handleSystemError("Failed to connect to WiFi after maximum attempts");
    logMessage("Device will continue attempting connection in main loop", true);
    // Don't restart here - let the main loop handle reconnection attempts
  }
  
  esp_task_wdt_reset();  // Reset watchdog after WiFi attempt

  // Connect to MQTT with retry logic (only if WiFi is connected)
  mqtt.setCallback(mqttCallback);
  if (WiFi.isConnected() && connectMQTTWithRetry()) {
    mqttPreviousTime = millis();
    logMessage("MQTT connected successfully");
  } else {
    mqttPreviousTime = 0;
    if (WiFi.isConnected()) {
      logMessage("MQTT connection failed, will retry during operation", true);
    } else {
      logMessage("WiFi not connected, skipping MQTT setup", true);
    }
  }
  
  esp_task_wdt_reset();  // Reset watchdog before DSC initialization

  // Starts the Keybus interface and optionally specifies how to print data.
  logMessage("Initializing DSC Keybus Interface...");
  
  // CRITICAL: Enhanced LoadProhibited crash prevention for ESP32
  // The 0xcececece pattern indicates static variables accessed during app_main()
  // This enhanced approach provides multiple layers of protection
  
  #if defined(ESP32)
  // Check static variable initialization status  
  extern volatile bool dsc_static_variables_initialized;
  if (!dsc_static_variables_initialized) {
    handleSystemError("DSC static variables not initialized - potential LoadProhibited crash");
    ESP.restart();
  }
  
  // Add stabilization delay to prevent early ISR access to uninitialized timers
  delay(2000);  // 2 second stabilization delay
  esp_task_wdt_reset();  // Reset watchdog after stabilization delay
  
  logMessage("ESP32 LoadProhibited crash prevention: Static variables verified, system stabilized");
  #endif
  
  // Add safety check for DSC constants before initialization
  if (dscReadSize == 0 || dscBufferSize == 0 || dscPartitions == 0) {
    handleSystemError("Invalid DSC constants detected - potential memory corruption");
    ESP.restart();  // Force restart if constants are corrupted
  }
  
  // Check available heap memory before DSC initialization
  size_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < 30000) {  // Require at least 30KB free heap
    handleSystemError("Insufficient heap memory for DSC initialization: " + String(freeHeap) + " bytes");
    ESP.restart();
  }
  
  logMessage("Pre-initialization checks passed - Heap: " + String(freeHeap) + " bytes");
  
  dsc.begin();
  logMessage("DSC Keybus Interface is online");
  
  esp_task_wdt_reset();  // Reset watchdog after DSC initialization
  
  // Publish initial system health (only if MQTT is available)
  if (WiFi.isConnected() && mqtt.connected()) {
    publishSystemHealth();
  }
  
  logMessage("Setup completed successfully");
  
  // Keep watchdog configuration simple - just reset it for main loop operation
  esp_task_wdt_reset();  // Final watchdog reset before entering main loop
}


void loop() {
  // Reset watchdog at the start of each loop iteration
  esp_task_wdt_reset();
  
  // Check WiFi connection health
  if (!WiFi.isConnected()) {
    handleSystemError("WiFi connection lost");
    if (!connectWiFiWithRetry()) {
      delay(wifiReconnectInterval);
      esp_task_wdt_reset();  // Reset watchdog after delay
      // Continue loop instead of exiting - WiFi will retry on next iteration
    }
  }
  
  // Handle MQTT connection
  mqttHandle();

  // Main DSC processing
  dsc.loop();

  // Process DSC status changes
  if (dsc.statusChanged) {
    dsc.statusChanged = false;
    
    // Handle buffer overflow with enhanced error reporting
    if (dsc.bufferOverflow) {
      handleSystemError("Keybus buffer overflow - system may be too busy");
      dsc.bufferOverflow = false;
      
      // Publish buffer overflow diagnostic
      if (mqtt.connected()) {
        mqtt.publish("dsc/Diagnostics/BufferOverflow", "true", true);
      }
    }

    // Handle Keybus connection changes with enhanced logging
    if (dsc.keybusChanged) {
      dsc.keybusChanged = false;
      if (dsc.keybusConnected) {
        mqtt.publish(mqttStatusTopic, mqttBirthMessage, true);
        logMessage("DSC Keybus connected");
        systemHealthy = true;
      } else {
        mqtt.publish(mqttStatusTopic, mqttLwtMessage, true);
        logMessage("DSC Keybus disconnected", true);
        handleSystemError("DSC Keybus connection lost");
      }
    }

    // Handle access code prompts
    if (dsc.accessCodePrompt) {
      dsc.accessCodePrompt = false;
      dsc.write(accessCode);
      logMessage("Access code sent to panel");
    }

    // Handle trouble status changes with enhanced logging
    if (dsc.troubleChanged) {
      dsc.troubleChanged = false;
      if (dsc.trouble) {
        mqtt.publish(mqttTroubleTopic, "1", true);
        logMessage("System trouble detected", true);
      } else {
        mqtt.publish(mqttTroubleTopic, "0", true);
        logMessage("System trouble cleared");
      }
    }

    // Publishes status per partition
    for (byte partition = 0; partition < dscPartitions; partition++) {

      // Bounds check for partition index to prevent buffer overflow
      if (partition >= dscPartitions || partition >= 8) {
        logMessage("Invalid partition index detected: " + String(partition), true);
        break;  // Exit loop to prevent memory access violation
      }

      // Skips processing if the partition is disabled or in installer programming
      if (dsc.disabled[partition]) continue;

      // Publishes the partition status message
      publishMessage(mqttPartitionTopic, partition);

      // Check for status transitions that require partition state synchronization
      if (dsc.status[partition] != previousStatus[partition]) {
        // When transitioning from exit delay (0x08) to partition ready (0x01)
        // ensure the partition state topic is updated to "disarmed"
        if (previousStatus[partition] == 0x08 && dsc.status[partition] == 0x01 && 
            !dsc.armed[partition] && !dsc.exitDelay[partition]) {
          char publishTopic[strlen(mqttPartitionTopic) + 2];
          appendPartition(mqttPartitionTopic, partition, publishTopic);
          mqtt.publish(publishTopic, "disarmed", true);
        }
        previousStatus[partition] = dsc.status[partition];
      }

      // Publishes armed/disarmed status
      if (dsc.armedChanged[partition]) {
        char publishTopic[strlen(mqttPartitionTopic) + 2];
        appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        if (dsc.armed[partition]) {
          if (dsc.armedAway[partition] && dsc.noEntryDelay[partition]) mqtt.publish(publishTopic, "armed_night", true);
          else if (dsc.armedAway[partition]) mqtt.publish(publishTopic, "armed_away", true);
          else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition]) mqtt.publish(publishTopic, "armed_night", true);
          else if (dsc.armedStay[partition]) mqtt.publish(publishTopic, "armed_home", true);
        }
        else mqtt.publish(publishTopic, "disarmed", true);
      }

      // Publishes exit delay status
      if (dsc.exitDelayChanged[partition]) {
        dsc.exitDelayChanged[partition] = false;  // Resets the exit delay status flag
        char publishTopic[strlen(mqttPartitionTopic) + 2];
        appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        if (dsc.exitDelay[partition]) mqtt.publish(publishTopic, "pending", true);  // Publish as a retained message
        else if (!dsc.exitDelay[partition] && !dsc.armed[partition]) mqtt.publish(publishTopic, "disarmed", true);
        else if (!dsc.exitDelay[partition] && dsc.armed[partition]) {
          // Publish armed status when exiting delay and transitioning to armed state
          if (dsc.armedAway[partition] && dsc.noEntryDelay[partition]) mqtt.publish(publishTopic, "armed_night", true);
          else if (dsc.armedAway[partition]) mqtt.publish(publishTopic, "armed_away", true);
          else if (dsc.armedStay[partition] && dsc.noEntryDelay[partition]) mqtt.publish(publishTopic, "armed_night", true);
          else if (dsc.armedStay[partition]) mqtt.publish(publishTopic, "armed_home", true);
        }
      }

      // Publishes alarm status
      if (dsc.alarmChanged[partition]) {
        dsc.alarmChanged[partition] = false;  // Resets the partition alarm status flag
        char publishTopic[strlen(mqttPartitionTopic) + 2];
        appendPartition(mqttPartitionTopic, partition, publishTopic);  // Appends the mqttPartitionTopic with the partition number

        if (dsc.alarm[partition]) mqtt.publish(publishTopic, "triggered", true);  // Alarm tripped
        else if (!dsc.armedChanged[partition]) mqtt.publish(publishTopic, "disarmed", true);
      }
      if (dsc.armedChanged[partition]) dsc.armedChanged[partition] = false;  // Resets the partition armed status flag

      // Publishes fire alarm status
      if (dsc.fireChanged[partition]) {
        dsc.fireChanged[partition] = false;  // Resets the fire status flag
        char publishTopic[strlen(mqttFireTopic) + 2];
        appendPartition(mqttFireTopic, partition, publishTopic);  // Appends the mqttFireTopic with the partition number

        if (dsc.fire[partition]) mqtt.publish(publishTopic, "1");  // Fire alarm tripped
        else mqtt.publish(publishTopic, "0");                      // Fire alarm restored
      }
    }

    // Publishes zones 1-64 status in a separate topic per zone
    // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones:
    //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
    //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
    //   ...
    //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
    if (dsc.openZonesStatusChanged) {
      dsc.openZonesStatusChanged = false;                           // Resets the open zones status flag
      for (byte zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
        for (byte zoneBit = 0; zoneBit < 8; zoneBit++) {
          if (bitRead(dsc.openZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
            bitWrite(dsc.openZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag

            // Appends the mqttZoneTopic with the zone number
            char zonePublishTopic[strlen(mqttZoneTopic) + 3];
            char zone[3];
            strcpy(zonePublishTopic, mqttZoneTopic);
            itoa(zoneBit + 1 + (zoneGroup * 8), zone, 10);
            strcat(zonePublishTopic, zone);

            if (bitRead(dsc.openZones[zoneGroup], zoneBit)) {
              mqtt.publish(zonePublishTopic, "1", true);            // Zone open
            }
            else mqtt.publish(zonePublishTopic, "0", true);         // Zone closed
          }
        }
      }
    }

    // Publishes PGM outputs 1-14 status in a separate topic per zone
    // PGM status is stored in the pgmOutputs[] and pgmOutputsChanged[] arrays using 1 bit per PGM output:
    //   pgmOutputs[0] and pgmOutputsChanged[0]: Bit 0 = PGM 1 ... Bit 7 = PGM 8
    //   pgmOutputs[1] and pgmOutputsChanged[1]: Bit 0 = PGM 9 ... Bit 5 = PGM 14
    if (dsc.pgmOutputsStatusChanged) {
      dsc.pgmOutputsStatusChanged = false;  // Resets the PGM outputs status flag
      for (byte pgmGroup = 0; pgmGroup < 2; pgmGroup++) {
        for (byte pgmBit = 0; pgmBit < 8; pgmBit++) {
          if (bitRead(dsc.pgmOutputsChanged[pgmGroup], pgmBit)) {  // Checks an individual PGM output status flag
            bitWrite(dsc.pgmOutputsChanged[pgmGroup], pgmBit, 0);  // Resets the individual PGM output status flag

            // Appends the mqttPgmTopic with the PGM number
            char pgmPublishTopic[strlen(mqttPgmTopic) + 3];
            char pgm[3];
            strcpy(pgmPublishTopic, mqttPgmTopic);
            itoa(pgmBit + 1 + (pgmGroup * 8), pgm, 10);
            strcat(pgmPublishTopic, pgm);

            if (bitRead(dsc.pgmOutputs[pgmGroup], pgmBit)) {
              mqtt.publish(pgmPublishTopic, "1", true);           // PGM enabled
            }
            else mqtt.publish(pgmPublishTopic, "0", true);        // PGM disabled
          }
        }
      }
    }

    mqtt.subscribe(mqttSubscribeTopic);
  }
  
  // Periodic system health monitoring and publishing
  unsigned long currentTime = millis();
  if (currentTime - lastStatusPublish > statusPublishInterval) {
    lastStatusPublish = currentTime;
    publishSystemHealth();
    
    // Reset system health if no recent errors
    if (systemHealthy == false && (currentTime - systemUptime) > 300000) { // 5 minutes
      systemHealthy = true;
      lastError = "";
      logMessage("System health status reset");
    }
  }
  
  // End of main loop - this should never be reached in normal operation
  // If we get here, something went wrong, but the Arduino framework will
  // automatically call loop() again, so the system will continue running
}


// Handles messages received in the mqttSubscribeTopic
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Input validation
  if (length == 0 || length > 50) {  // Reasonable bounds checking
    logMessage("Invalid MQTT payload length: " + String(length), true);
    return;
  }
  
  // Null-terminate payload for safe string operations
  char safePayload[51];  // Max 50 chars + null terminator
  memcpy(safePayload, payload, min(length, 50U));
  safePayload[min(length, 50U)] = '\0';
  
  // Log command received
  logMessage("MQTT command received: " + String(safePayload));

  // Handles unused parameters
  (void)topic;

  byte partition = 0;
  byte payloadIndex = 0;
  bool disarmWithAccessCode = false;
  char extractedAccessCode[10] = "";  // Buffer for extracted access code

  // Checks if a partition number 1-8 has been sent and sets the second character as the payload
  if (payload[0] >= 0x31 && payload[0] <= 0x38) {
    partition = payload[0] - 49;
    payloadIndex = 1;
    
    // Additional validation for partition number with bounds checking
    if (partition >= dscPartitions || partition >= 8) {
      logMessage("Invalid partition number: " + String(partition + 1) + " (max: " + String(dscPartitions) + ")", true);
      return;
    }
    
    // Check for "!XXXX" format indicating disarm with specific access code
    if (length > 2 && payload[1] == '!') {
      disarmWithAccessCode = true;
      // Extract access code from payload starting at position 2
      byte codeLength = 0;
      for (byte i = 2; i < length && i < 11 && codeLength < 9; i++) {
        if (payload[i] >= '0' && payload[i] <= '9') {
          extractedAccessCode[codeLength++] = payload[i];
        }
      }
      extractedAccessCode[codeLength] = '\0';  // Null terminate
      
      // Validate extracted access code
      if (codeLength == 0) {
        logMessage("Invalid custom access code format", true);
        return;
      }
      
      // Enhanced debug output
      logMessage("Custom disarm code for partition " + String(partition + 1) + 
                " - Code length: " + String(codeLength));
    }
  }

  // Enhanced command processing with logging
  
  // Panic alarm
  if (payload[payloadIndex] == 'P') {
    logMessage("Panic alarm triggered");
    dsc.write('p');
    return;
  }

  // Additional command: Fire alarm
  if (payload[payloadIndex] == 'f' || payload[payloadIndex] == 'F') {
    logMessage("Fire alarm triggered");
    dsc.write('f');
    return;
  }

  // Additional command: Aux/Medical alarm  
  if (payload[payloadIndex] == 'a' || payload[payloadIndex] == 'A') {
    logMessage("Auxiliary alarm triggered");
    dsc.write('a');
    return;
  }

  // Validate system readiness for arm commands
  if (payload[payloadIndex] != 'D' && !disarmWithAccessCode && !dsc.ready[partition]) {
    logMessage("Partition " + String(partition + 1) + " not ready for arming", true);
    dsc.armedChanged[partition] = true;
    dsc.statusChanged = true;
    return;
  }

  // Arm stay
  if (payload[payloadIndex] == 'S' && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    logMessage("Arming partition " + String(partition + 1) + " in Stay mode");
    dsc.writePartition = partition + 1;
    dsc.write('s');
  }

  // Arm away
  else if (payload[payloadIndex] == 'A' && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    logMessage("Arming partition " + String(partition + 1) + " in Away mode");
    dsc.writePartition = partition + 1;
    dsc.write('w');
  }

  // Arm night
  else if (payload[payloadIndex] == 'N' && !dsc.armed[partition] && !dsc.exitDelay[partition]) {
    logMessage("Arming partition " + String(partition + 1) + " in Night mode");
    dsc.writePartition = partition + 1;
    dsc.write('n');
  }


  // Disarm - either standard "1D" format or custom access code "1!XXXX" format  
  else if ((payload[payloadIndex] == 'D' || disarmWithAccessCode) && 
           (dsc.armed[partition] || dsc.exitDelay[partition] || dsc.alarm[partition])) {
    
    String disarmType = disarmWithAccessCode ? "custom access code" : "default access code";
    logMessage("Disarming partition " + String(partition + 1) + " using " + disarmType);
    
    dsc.writePartition = partition + 1;
    if (disarmWithAccessCode) {
      dsc.write(extractedAccessCode);
    } else {
      dsc.write(accessCode);
    }
  }
  
  // Log unrecognized commands for debugging
  else if (payload[payloadIndex] != 'D' && payload[payloadIndex] != 'S' && 
           payload[payloadIndex] != 'A' && payload[payloadIndex] != 'N' &&
           payload[payloadIndex] != 'P' && payload[payloadIndex] != 'f' &&
           payload[payloadIndex] != 'a' && !disarmWithAccessCode) {
    logMessage("Unrecognized command: " + String((char)payload[payloadIndex]), true);
  }
}


void mqttHandle() {
  if (!mqtt.connected()) {
    unsigned long mqttCurrentTime = millis();
    if (mqttCurrentTime - mqttPreviousTime > mqttReconnectInterval) {
      mqttPreviousTime = mqttCurrentTime;
      if (connectMQTTWithRetry()) {
        if (dsc.keybusConnected) mqtt.publish(mqttStatusTopic, mqttBirthMessage, true);
        logMessage("MQTT reconnected successfully");
        mqttPreviousTime = 0;
      } else {
        logMessage("MQTT reconnection failed", true);
      }
    }
  } else {
    mqtt.loop();
  }
}


void appendPartition(const char* sourceTopic, byte sourceNumber, char* publishTopic) {
  char partitionNumber[2];
  strcpy(publishTopic, sourceTopic);
  itoa(sourceNumber + 1, partitionNumber, 10);
  strcat(publishTopic, partitionNumber);
}


// Publishes the partition status message
void publishMessage(const char* sourceTopic, byte partition) {
  char publishTopic[strlen(sourceTopic) + strlen(mqttPartitionMessageSuffix) + 2];
  char partitionNumber[2];

  // Appends the sourceTopic with the partition number and message topic
  itoa(partition + 1, partitionNumber, 10);
  strcpy(publishTopic, sourceTopic);
  strcat(publishTopic, partitionNumber);
  strcat(publishTopic, mqttPartitionMessageSuffix);

  // Publishes the current partition message
  switch (dsc.status[partition]) {
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


// Enhanced WiFi connection with retry logic and watchdog feeding
bool connectWiFiWithRetry() {
  logMessage("Connecting to WiFi: " + String(wifiSSID));
  
  // Ensure clean WiFi state before attempting connection
  WiFi.disconnect(true);
  delay(500);  // Reduced delay
  esp_task_wdt_reset();     // Reset watchdog
  
  // Configure WiFi for better connection stability
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  
  // Start connection attempt
  WiFi.begin(wifiSSID, wifiPassword);
  
  // Use reduced attempts during setup to prevent watchdog timeout
  byte setupMaxAttempts = (millis() < 30000) ? 5 : maxReconnectAttempts;
  
  wifiReconnectAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiReconnectAttempts < setupMaxAttempts) {
    // Use shorter delays and feed watchdog regularly
    for (int i = 0; i < 10; i++) {
      delay(100);
      esp_task_wdt_reset();  // Reset watchdog every 100ms
      if (WiFi.status() == WL_CONNECTED) break;
    }
    
    wifiReconnectAttempts++;
    if (enableDebugLogging) {
      Serial.print(".");
    }
    
    // Reset watchdog before potentially long operations
    esp_task_wdt_reset();
    
    // Try reconnecting every 3 attempts with clean state (reduced from 5)
    if (wifiReconnectAttempts % 3 == 0) {
      logMessage("WiFi retry attempt " + String(wifiReconnectAttempts) + "/" + String(setupMaxAttempts));
      WiFi.disconnect(true);
      delay(500);  // Reduced delay
      esp_task_wdt_reset();     // Reset watchdog
      WiFi.begin(wifiSSID, wifiPassword);
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    logMessage("WiFi connected - IP: " + WiFi.localIP().toString());
    wifiReconnectAttempts = 0;
    return true;
  } else {
    lastError = "WiFi connection failed after " + String(setupMaxAttempts) + " attempts";
    logMessage(lastError, true);
    return false;
  }
}


// Enhanced MQTT connection with retry logic and watchdog feeding
bool connectMQTTWithRetry() {
  if (!WiFi.isConnected()) {
    logMessage("WiFi not connected, cannot connect to MQTT", true);
    return false;
  }
  
  // Use reduced attempts during setup to prevent watchdog timeout
  byte setupMaxAttempts = (millis() < 30000) ? 3 : maxReconnectAttempts;
  
  mqttReconnectAttempts = 0;
  while (!mqtt.connected() && mqttReconnectAttempts < setupMaxAttempts) {
    logMessage("Attempting MQTT connection...");
    esp_task_wdt_reset();  // Reset watchdog
    
    String clientId = String(mqttClientName) + "-" + String(ESP.getEfuseMac(), HEX);
    
    if (mqtt.connect(clientId.c_str(), mqttUsername, mqttPassword, mqttStatusTopic, 0, true, mqttLwtMessage)) {
      logMessage("MQTT connected to: " + String(mqttServer));
      dsc.resetStatus();  // Reset status to get current state
      mqtt.subscribe(mqttSubscribeTopic);
      mqttReconnectAttempts = 0;
      return true;
    } else {
      mqttReconnectAttempts++;
      String error = "MQTT connection failed, rc=" + String(mqtt.state()) + 
                    " (attempt " + String(mqttReconnectAttempts) + "/" + String(setupMaxAttempts) + ")";
      logMessage(error, true);
      
      // Break down delay and reset watchdog
      for (int i = 0; i < 20; i++) {
        delay(100);
        esp_task_wdt_reset();  // Reset watchdog every 100ms
      }
    }
  }
  
  lastError = "MQTT connection failed after " + String(setupMaxAttempts) + " attempts";
  logMessage(lastError, true);
  return false;
}


// Publish system health and diagnostics
void publishSystemHealth() {
  if (!mqtt.connected()) return;
  
  unsigned long currentUptime = (millis() - systemUptime) / 1000; // Convert to seconds
  
  // Publish system diagnostic information
  mqtt.publish("dsc/Diagnostics/Uptime", String(currentUptime).c_str(), true);
  mqtt.publish("dsc/Diagnostics/FreeHeap", String(ESP.getFreeHeap()).c_str(), true);
  mqtt.publish("dsc/Diagnostics/WiFiRSSI", String(WiFi.RSSI()).c_str(), true);
  mqtt.publish("dsc/Diagnostics/WiFiReconnects", String(wifiReconnectAttempts).c_str(), true);
  mqtt.publish("dsc/Diagnostics/MQTTReconnects", String(mqttReconnectAttempts).c_str(), true);
  mqtt.publish("dsc/Diagnostics/SystemHealthy", systemHealthy ? "true" : "false", true);
  
  if (!lastError.isEmpty()) {
    mqtt.publish("dsc/Diagnostics/LastError", lastError.c_str(), true);
  }
  
  // DSC specific diagnostics
  mqtt.publish("dsc/Diagnostics/KeybusConnected", dsc.keybusConnected ? "true" : "false", true);
  mqtt.publish("dsc/Diagnostics/BufferOverflow", dsc.bufferOverflow ? "true" : "false", true);
  
  logMessage("System health published");
}


// Handle system errors with appropriate recovery actions
void handleSystemError(const String& error) {
  lastError = error;
  systemHealthy = false;
  logMessage("SYSTEM ERROR: " + error, true);
  
  // Publish error to MQTT if possible
  if (mqtt.connected()) {
    mqtt.publish("dsc/Diagnostics/LastError", error.c_str(), true);
    mqtt.publish("dsc/Diagnostics/SystemHealthy", "false", true);
  }
  
  // Attempt recovery based on error type with rate limiting
  if (error.indexOf("WiFi") >= 0) {
    unsigned long currentTime = millis();
    
    // Implement exponential backoff for WiFi recovery attempts
    unsigned long backoffDelay = min(30000UL, 5000UL * (wifiReconnectAttempts / 10 + 1));
    
    if (currentTime - lastWifiRecoveryAttempt > backoffDelay) {
      logMessage("Attempting WiFi recovery (attempt " + String(wifiReconnectAttempts + 1) + ")...");
      lastWifiRecoveryAttempt = currentTime;
      
      WiFi.disconnect(true);
      delay(5000);
      
      if (connectWiFiWithRetry()) {
        systemHealthy = true;
        lastError = "";
        logMessage("WiFi recovery successful");
      }
    }
  } else if (error.indexOf("MQTT") >= 0) {
    logMessage("Attempting MQTT recovery...");
    mqtt.disconnect();
    delay(2000);
    connectMQTTWithRetry();
  }
}


// Enhanced logging function with timestamps and error levels
void logMessage(const String& message, bool isError) {
  unsigned long timestamp = millis() / 1000;
  String logLevel = isError ? "[ERROR]" : "[INFO] ";
  String formattedMessage = "[" + String(timestamp) + "s] " + logLevel + " " + message;
  
  Serial.println(formattedMessage);
  
  // Also send to MQTT if connected and debug logging enabled
  if (mqtt.connected() && enableDebugLogging) {
    String topic = isError ? "dsc/Debug/Errors" : "dsc/Debug/Info";
    mqtt.publish(topic.c_str(), message.c_str(), false); // Don't retain debug messages
  }
}
