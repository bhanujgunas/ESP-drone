#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Motor control pins
#define MOTOR1_PIN D5  // GPIO13
#define MOTOR2_PIN D6  // GPIO12
#define MOTOR3_PIN D7  // GPIO14
#define MOTOR4_PIN D8  // GPIO15

// WiFi credentials
const char* ssid = "wifiname";
const char* password = "password";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// PWM properties
const int freq = 50000;     // 1kHz frequency
const int resolution = 10;  // 10-bit resolution
const int maxDuty = (int)(pow(2, resolution) - 1);

// Store current motor values
int motorValues[4] = {0, 0, 0, 0};

// HTML and CSS for web page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP8266 Motor Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 2.3rem; color: #0A1128;}
    .slider-container {margin: 20px auto; max-width: 400px;}
    .slider {width: 100%; height: 25px; margin: 10px 0;}
    .motor-label {font-size: 1.2rem; margin-bottom: 5px;}
    .master-container {background-color: #f0f0f0; padding: 15px; border-radius: 10px; margin-bottom: 20px;}
  </style>
</head>
<body>
  <h2>ESP8266 Motor Control</h2>
  
  <div class="master-container">
    <div class="motor-label">Master Control: <span id="masterValue">0</span>%</div>
    <input type="range" min="0" max="100" value="0" class="slider" id="masterSlider" oninput="updateMaster(this.value)">
  </div>
  
  <div class="slider-container">
    <div class="motor-label">Motor 1 Speed: <span id="motor1Value">0</span>%</div>
    <input type="range" min="0" max="100" value="0" class="slider" id="motor1Slider" oninput="updateMotor('motor1', this.value)">
  </div>
  
  <div class="slider-container">
    <div class="motor-label">Motor 2 Speed: <span id="motor2Value">0</span>%</div>
    <input type="range" min="0" max="100" value="0" class="slider" id="motor2Slider" oninput="updateMotor('motor2', this.value)">
  </div>
  
  <div class="slider-container">
    <div class="motor-label">Motor 3 Speed: <span id="motor3Value">0</span>%</div>
    <input type="range" min="0" max="100" value="0" class="slider" id="motor3Slider" oninput="updateMotor('motor3', this.value)">
  </div>
  
  <div class="slider-container">
    <div class="motor-label">Motor 4 Speed: <span id="motor4Value">0</span>%</div>
    <input type="range" min="0" max="100" value="0" class="slider" id="motor4Slider" oninput="updateMotor('motor4', this.value)">
  </div>

  <script>
    function updateMotor(motor, value) {
      document.getElementById(motor + "Value").innerHTML = value;
      document.getElementById(motor + "Slider").value = value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/" + motor + "?value=" + value, true);
      xhr.send();
      
      // Update master slider to match if all motors have same value
      updateMasterIfAllSame();
    }
    
    function updateMaster(value) {
      document.getElementById("masterValue").innerHTML = value;
      document.getElementById("masterSlider").value = value;
      
      // Set all motor sliders to the same value
      document.getElementById("motor1Value").innerHTML = value;
      document.getElementById("motor1Slider").value = value;
      document.getElementById("motor2Value").innerHTML = value;
      document.getElementById("motor2Slider").value = value;
      document.getElementById("motor3Value").innerHTML = value;
      document.getElementById("motor3Slider").value = value;
      document.getElementById("motor4Value").innerHTML = value;
      document.getElementById("motor4Slider").value = value;
      
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/master?value=" + value, true);
      xhr.send();
    }
    
    function updateMasterIfAllSame() {
      var m1 = parseInt(document.getElementById("motor1Slider").value);
      var m2 = parseInt(document.getElementById("motor2Slider").value);
      var m3 = parseInt(document.getElementById("motor3Slider").value);
      var m4 = parseInt(document.getElementById("motor4Slider").value);
      
      if(m1 === m2 && m2 === m3 && m3 === m4) {
        document.getElementById("masterValue").innerHTML = m1;
        document.getElementById("masterSlider").value = m1;
      }
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  
  // Initialize motor control pins
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  pinMode(MOTOR4_PIN, OUTPUT);
  
  // Setup PWM
  analogWriteFreq(freq);
  analogWriteRange(maxDuty);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Routes to handle motor control
  server.on("/motor1", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      motorValues[0] = request->getParam("value")->value().toInt();
      updateMotor(0, motorValues[0]);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/motor2", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      motorValues[1] = request->getParam("value")->value().toInt();
      updateMotor(1, motorValues[1]);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/motor3", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      motorValues[2] = request->getParam("value")->value().toInt();
      updateMotor(2, motorValues[2]);
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/motor4", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      motorValues[3] = request->getParam("value")->value().toInt();
      updateMotor(3, motorValues[3]);
    }
    request->send(200, "text/plain", "OK");
  });

  // Route for master control
  server.on("/master", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      int masterValue = request->getParam("value")->value().toInt();
      
      // Set all motors to the same value
      for (int i = 0; i < 4; i++) {
        motorValues[i] = masterValue;
        updateMotor(i, masterValue);
      }
    }
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void updateMotor(int motorIndex, int value) {
  int duty = (value * maxDuty) / 100;
  switch(motorIndex) {
    case 0: analogWrite(MOTOR1_PIN, duty); break;
    case 1: analogWrite(MOTOR2_PIN, duty); break;
    case 2: analogWrite(MOTOR3_PIN, duty); break;
    case 3: analogWrite(MOTOR4_PIN, duty); break;
  }
  Serial.print("Motor ");
  Serial.print(motorIndex + 1);
  Serial.print(" set to: ");
  Serial.print(value);
  Serial.println("%");
}

void loop() {
  // Nothing to do here
}