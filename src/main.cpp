/*
 * Minimal WiFi AP Fallback Test for DSC-Alarm-Control-HA
 * 
 * This is a simplified test to demonstrate the fix for the infinite 
 * WiFi connection loop that prevents Access Point mode from activating
 * when WiFi credentials are empty or invalid.
 */

#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <dscKeybusInterface.h>

// Settings - intentionally empty to test AP fallback
const char* wifiSSID = "";
const char* wifiPassword = "";

// AP Configuration
DNSServer dnsServer;
WebServer server(80);
const byte DNS_PORT = 53;
bool accessPointMode = false;

// DSC Configuration
#define dscClockPin 18
#define dscReadPin  19
#define dscWritePin 21
dscKeybusInterface dsc(dscClockPin, dscReadPin, dscWritePin);

// Function prototypes
void setupWiFi();
void startAccessPointMode();
void handleRoot();
void handleNotFound();

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println();
  Serial.println("DSC WiFi AP Fallback Test Starting...");
  
  // Check if we have WiFi credentials
  if (strlen(wifiSSID) == 0) {
    Serial.println("No WiFi credentials configured, starting Access Point mode...");
    startAccessPointMode();
  } else {
    // Try to connect to WiFi with timeout
    setupWiFi();
    
    // Wait up to 30 seconds for connection
    int timeout = 30000; // 30 seconds
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
      Serial.print(".");
      delay(500);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("WiFi connected: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("Failed to connect to WiFi!");
      Serial.println("Starting Access Point mode for configuration...");
      startAccessPointMode();
    }
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  
  Serial.println("Web server started");
  if (accessPointMode) {
    Serial.println("Connect to the AP and navigate to http://" + WiFi.softAPIP().toString() + " to configure");
  }
  
  // Initialize DSC interface
  dsc.begin();
  Serial.println("DSC interface initialized");
  Serial.println("Setup complete!");
}

void loop() {
  // Handle DNS server for captive portal in AP mode
  if (accessPointMode) {
    dnsServer.processNextRequest();
  }
  
  // Handle web server
  server.handleClient();
  
  // Handle DSC interface
  if (dsc.loop()) {
    Serial.println("DSC data received");
  }
  
  delay(1);
}

void setupWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPassword);
  Serial.print("Connecting to WiFi");
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
  
  accessPointMode = true;
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>DSC Configuration</title></head><body>";
  html += "<h1>DSC Alarm Control Configuration</h1>";
  if (accessPointMode) {
    html += "<h2>Access Point Mode Active</h2>";
    html += "<p>The device could not connect to the configured WiFi network.</p>";
    html += "<p>Please configure your WiFi credentials:</p>";
    html += "<form action='/save' method='POST'>";
    html += "SSID: <input type='text' name='ssid'><br><br>";
    html += "Password: <input type='password' name='password'><br><br>";
    html += "<input type='submit' value='Save'>";
    html += "</form>";
  } else {
    html += "<h2>WiFi Connected</h2>";
    html += "<p>Device is connected to WiFi successfully.</p>";
    html += "<p>IP Address: " + WiFi.localIP().toString() + "</p>";
  }
  html += "<hr><p>DSC Keybus Interface Status: ";
  html += dsc.keybusConnected ? "Connected" : "Disconnected";
  html += "</p></body></html>";
  
  server.send(200, "text/html", html);
}

void handleNotFound() {
  // Redirect to root for captive portal functionality
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}