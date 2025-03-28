#include <WiFi.h>
#include <WebServer.h>
#include <esp_now.h>

// Replace with your network credentials
const char* ssid = "OnePlus 13R ";
const char* password = "Zam@1256";

// Replace with your receiver MAC address
uint8_t receiverAddress[] = { 0xD0, 0xEF, 0x76, 0x33, 0x90, 0x70 };

// Create WebServer object on port 80
WebServer server(80);

// Callback function for when data is sent
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// HTML web page
const char WEBPAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Stepper Motor Control</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        .button {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 10px;
            cursor: pointer;
        }
    </style>
</head>
<body>
    <h1>Stepper Motor Control</h1>
    <div>
        <button class="button" onclick="sendCommand('1')">Both CW</button>
        <button class="button" onclick="sendCommand('2')">Both CCW</button>
        <button class="button" onclick="sendCommand('3')">Motor 1 CW, Motor 2 CCW</button>
        <button class="button" onclick="sendCommand('4')">Motor 1 CCW, Motor 2 CW</button>
    </div>
    <p id="status"></p>
    <script>
        function sendCommand(cmd) {
            fetch('/command?value=' + cmd)
                .then(response => response.text())
                .then(data => {
                    document.getElementById('status').innerText = data;
                });
        }
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }
  
  // Register send callback
  esp_now_register_send_cb(onSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Web server routes
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", WEBPAGE);
  });

  server.on("/command", HTTP_GET, []() {
    if (server.hasArg("value")) {
      String command = server.arg("value");
      
      // Send message via ESP-NOW
      esp_err_t result = esp_now_send(receiverAddress, (uint8_t *)command.c_str(), command.length());
      
      if (result == ESP_OK) {
        server.send(200, "text/plain", "Command sent successfully");
      } else {
        server.send(500, "text/plain", "Error sending command");
      }
    } else {
      server.send(400, "text/plain", "No command received");
    }
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client connections
  server.handleClient();
}
