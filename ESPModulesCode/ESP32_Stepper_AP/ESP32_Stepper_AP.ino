#include <Arduino.h>
#include <Stepper.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

// Change this to fit the number of steps per revolution
const int stepsPerRevolution = 512;
// Adjustable range of 28BYJ-48 stepper is 0~17 rpm
const int rolePerMinute = 17; 

// Pins must be in order 1st, 3rd, 2nd, 4th 
// Change these values to suit pin setup
Stepper myStepper(stepsPerRevolution, 1, 4, 12, 27);

// Initialize webserver on port 8081
WebServer server(8081);

void setup() {
  myStepper.setSpeed(rolePerMinute);

  Serial.begin(115200);
  Serial.println();

  // Set station mode
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  // Manually connect everytime board is rebooted
  // wm.resetSettings();

  // Wifi manager connection portal SSID "Pet Feeder Motor"
  bool res;
  res = wm.autoConnect("Pet Feeder Motor");
  if(!res) {
    Serial.println("Failed to connect to ESP32 MOTOR");
  }
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // Print out information once connected to WiFi
  Serial.println();
  Serial.print("Server IP address: ");
  Serial.println(WiFi.localIP());
 
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Calls functions set with server.on()
  server.handleClient();
}

void handleRoot() {
  // Check for POST request, get message, and print it
  if (server.method() == HTTP_POST) { 
    String message = server.arg("message");
    Serial.print("Received message: ");
    Serial.println(message);

    // Rotate motor when python sends 'FEED'
    if (message.equals("FEED")) {
      Serial.println("Rotating stepper motor");
      myStepper.step(stepsPerRevolution);
      server.send(200, "text/plain", "Feeding");
      delay(1000);
    } else {
      Serial.println("Invalid message received");
    }

    server.send(200, "text/plain", "Message received. ESP32MTR Connected");
    
    // Send Hello message to show connection
    } else {
    server.send(200, "text/plain", "Hello from ESP32MTR!");
  }
}

