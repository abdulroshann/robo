#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

// Replace with your network credentials
const char* ssid = "OnePlus 13R ";
const char* password = "Zam@1256";

// Stepper 1 pins (X-axis)
#define STEP_PIN1 14
#define DIR_PIN1 12

// Stepper 2 pins (Y-axis)
#define STEP_PIN2 26
#define DIR_PIN2 25

#define stepsPerRevolution 6400 // Full rotation steps

void connectToWiFi() {
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  
  // Wait for connection with timeout
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(500);
  }
  
  // Check connection status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

void controlSteppers(char command) {
  Serial.print("Executing command: ");
  Serial.println(command);
  
  // Reset initial state
  digitalWrite(STEP_PIN1, LOW);
  digitalWrite(STEP_PIN2, LOW);
  
  switch (command) {
    case '1': // Both motors clockwise, full rotation
      digitalWrite(DIR_PIN1, HIGH);
      digitalWrite(DIR_PIN2, HIGH);
      // Synchronized stepping for full rotation
      for(int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(STEP_PIN1, HIGH);
        digitalWrite(STEP_PIN2, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN1, LOW);
        digitalWrite(STEP_PIN2, LOW);
        delayMicroseconds(100);
      }
      break;
    
    case '2': // Both motors counterclockwise, full rotation
      digitalWrite(DIR_PIN1, LOW);
      digitalWrite(DIR_PIN2, LOW);
      // Synchronized stepping for full rotation
      for(int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(STEP_PIN1, HIGH);
        digitalWrite(STEP_PIN2, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN1, LOW);
        digitalWrite(STEP_PIN2, LOW);
        delayMicroseconds(100);
      }
      break;
    
    case '3': // Motor 1 clockwise, Motor 2 counterclockwise, full rotation
      digitalWrite(DIR_PIN1, HIGH); // Clockwise
      digitalWrite(DIR_PIN2, LOW); // Counterclockwise
      // Synchronized stepping for full rotation
      for(int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(STEP_PIN1, HIGH);
        digitalWrite(STEP_PIN2, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN1, LOW);
        digitalWrite(STEP_PIN2, LOW);
        delayMicroseconds(100);
      }
      break;
    
    case '4': // Motor 1 counterclockwise, Motor 2 clockwise, full rotation
      digitalWrite(DIR_PIN1, LOW); // Counterclockwise
      digitalWrite(DIR_PIN2, HIGH); // Clockwise
      // Synchronized stepping for full rotation
      for(int i = 0; i < stepsPerRevolution; i++) {
        digitalWrite(STEP_PIN1, HIGH);
        digitalWrite(STEP_PIN2, HIGH);
        delayMicroseconds(100);
        digitalWrite(STEP_PIN1, LOW);
        digitalWrite(STEP_PIN2, LOW);
        delayMicroseconds(100);
      }
      break;
    
    default:
      Serial.println("Unknown command received");
      return;
  }

  // Ensure final state has both step pins low
  digitalWrite(STEP_PIN1, LOW);
  digitalWrite(STEP_PIN2, LOW);
}

// ESP-NOW data receive callback
void onReceiveData(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  Serial.println("Data received");
  if (len > 0) {
    char receivedData[len + 1];
    memcpy(receivedData, incomingData, len);
    receivedData[len] = '\0';
    Serial.print("Received command: ");
    Serial.println(receivedData);
    controlSteppers(receivedData[0]);
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Connect to WiFi first
  connectToWiFi();
  
  // Configure stepper motor pins
  pinMode(STEP_PIN1, OUTPUT);
  pinMode(DIR_PIN1, OUTPUT);
  pinMode(STEP_PIN2, OUTPUT);
  pinMode(DIR_PIN2, OUTPUT);
  
  // Set initial directions
  digitalWrite(DIR_PIN1, HIGH);
  digitalWrite(DIR_PIN2, HIGH);

  // Initialize WiFi for ESP-NOW
  WiFi.mode(WIFI_STA);
  
  // Print MAC address for reference
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(onReceiveData);
  
  Serial.println("Receiver initialized successfully");
  Serial.println("Ready to receive stepper commands...");
}

void loop() {
  // Check WiFi connection periodically
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Reconnecting...");
    connectToWiFi();
  }
  
  // Nothing else needed here as ESP-NOW uses callbacks
}
