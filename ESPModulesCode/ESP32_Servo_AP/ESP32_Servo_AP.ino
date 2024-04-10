#include <Arduino.h>
#include <ESP32Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiManager.h>

// Set servo coms to pin 13 on esp32
const int servoPin = 13;
// variable to store the servo position
int pos = 0;
// Create Servo object
Servo myservo;

// Initialize webserver on port 8081
WebServer server(8081);

void setup() {
  // Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);
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

// void moveServo(int repetitions) {
//   for (int i = 0; i < repetitions; i++) {
//     // Move the servo to the maximum position (180 degrees)
//     myservo.writeMicroseconds(2500);
//     delay(500);
//     // Move the servo to the minimum position (0 degrees)
//     myservo.writeMicroseconds(500);
//     delay(500);
//   }

void moveServo(int repetitions) {
  for (int i = 0; i < repetitions; i++) {
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(10);             // waits 15ms for the servo to reach the position
  }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);    // tell servo to go to position in variable 'pos'
      delay(10);             // waits 15ms for the servo to reach the position
    }
  }
}


void handleRoot() {
  // Check for POST request, get message, and print it
  if (server.method() == HTTP_POST) { 
    String message = server.arg("message");
    Serial.print("Received message: ");
    Serial.println(message);

    // Rotate motor when python sends 'FEED'
    if (message.equals("SMALL")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "Feeding small portion");
      moveServo(1);
      delay(1000);

    } else if (message.equals("MED")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "Feeding medium portion");
      moveServo(2);
      delay(1000);

    } else if (message.equals("LARGE")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "Feeding large portion");
      moveServo(3);
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

