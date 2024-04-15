#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
#include <ESP32Servo.h>
#include <WiFiManager.h>

// Code for camera functionality with resolution options from:
// https://www.electroniclinic.com/esp32-cam-with-python-opencv-yolo-v3-for-object-detection-and-identification/

// ESP32 cam flash led = pin 4
const int ledPin = 4;

// Initialize webserver on port 8080
WebServer server(8080);

// Set servo coms to pin 13 on esp32
const int servoPin = 14;
// variable to store the servo position
int pos = 0;
// Create Servo object
Servo myservo;
 
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}
 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}
 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
 
void  setup(){
  Serial.begin(115200);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
 
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

  ESP32PWM::allocateTimer(5);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 500, 2500);

  // Set station mode
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  // Manually configure WiFi everytime board reboots
  // wm.resetSettings();

  // Wifi manager connection portal SSID "Pet Feeder Cam"
  bool res;
  res = wm.autoConnect("PetFeederCam");
  if(!res) {
    Serial.println("Failed to connect to Pet Cam");
  }
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // Print out information once connected to WiFi
  Serial.print("Server IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");
 
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-hi.jpg", handleJpgHi);
  server.on("/cam-mid.jpg", handleJpgMid);
 
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");

  // Initialise with flash off
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}
 
void loop()
{
  // Call functions set with server.on()
  server.handleClient();
}

void moveServo(int repetitions) {
  for (int i = 0; i < repetitions; i++) {
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);    // tell servo to go to position in variable 'pos'
    delay(5);             // waits 15ms for the servo to reach the position
  }
    delay(500);
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      myservo.write(pos);    // tell servo to go to position in variable 'pos'
      delay(5);             // waits 15ms for the servo to reach the position
    }
    delay(500);
  }
}

void handleRoot() {
  // Check for POST request, get message, and print it
  if (server.method() == HTTP_POST) {
    String message = server.arg("message");
    Serial.print("Received message: ");
    Serial.println(message);

    // Turn on flash
    if (message.equals("FLASHON")) {
      Serial.println("Turning on flash");
      digitalWrite(ledPin, HIGH);
      server.send(200, "text/plain", "Flash ON");

    // Turn off flash
    } else if (message.equals("FLASHOFF")) {
      digitalWrite(ledPin, LOW);
      Serial.println("Turning off flash");
      server.send(200, "text/plain", "Flash OFF");

        // Rotate motor when python sends 'FEED'
    } else if (message.equals("SMALL")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "ESP32: Feeding small portion");
      moveServo(1);
      delay(1000);

    } else if (message.equals("MED")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "ESP32: Feeding medium portion");
      moveServo(2);
      delay(1000);

    } else if (message.equals("LARGE")) {
      Serial.println("Rotating servo");
      server.send(200, "text/plain", "ESP32: Feeding large portion");
      moveServo(3);
      delay(1000);

    } else {
      Serial.println("Invalid message received");
    }

    server.send(200, "text/plain", "Message received. ESP32 CAM Connected");
  
  // Send Hello message to show connection
  } else {
    server.send(200, "text/plain", "Hello from ESP32 CAM!");
  }
}
