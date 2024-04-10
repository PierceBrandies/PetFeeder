#include <Arduino.h>
#include <Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>


const int servoPin = 13;

Servo myservo;

// Initialize webserver on port 8081
WebServer server(8081);

void setup() {
  myservo.attach(servoPin, 500, 2500);

  Serial.begin(115200);
  Serial.println();

  // Set station mode
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  // Manually connect everytime board is rebooted
  // wm.resetSettings();

  // Wifi manager connection portal SSID "Pet Feeder Servo"
  bool res;
  res = wm.autoConnect("Pet Feeder Servo");
  if(!res) {
    Serial.println("Failed to connect to ESP32 Servo");
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
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "Feeding");

      // Move the servo to the maximum position (180 degrees)
      myservo.writeMicroseconds(2500);
      delay(1000);
  
      // Move the servo to the minimum position (0 degrees)
      myservo.writeMicroseconds(500);
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

