#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>

MPU6050 mpu;

// WiFi credentials
const char* ssid = ".";
const char* password = "4pnumtvzvd";

WebServer server(80);

// MPU6050 data
float pitch = 0;
float roll = 0;
float yaw = 0;

void setup() {
  Serial.begin(115200);
  
  // Initialize MPU6050
  Wire.begin();
  mpu.initialize();
  
  // Verify connection
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Read MPU6050 data
  readMPU();
  
  // Small delay to prevent overwhelming the server
  delay(50);
}

void readMPU() {
  int16_t ax, ay, az, gx, gy, gz;
  
  // Read raw accel/gyro measurements from device
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Convert to readable values (adjust these scaling factors as needed)
  float accelX = ax / 16384.0;
  float accelY = ay / 16384.0;
  float accelZ = az / 16384.0;
  
  // Calculate pitch and roll (in radians)
  pitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ));
  roll = atan2(accelY, accelZ);
  
  // Convert to degrees
  pitch = pitch * 180.0 / PI;
  roll = roll * 180.0 / PI;
  
  // For yaw, we'd need a magnetometer, but we'll approximate with gyro
  // Note: This will drift over time without a magnetometer
  static float previousTime = 0;
  float currentTime = millis() / 1000.0;
  float elapsedTime = currentTime - previousTime;
  previousTime = currentTime;
  
  float gyroY = gy / 131.0; // Convert to deg/s
  yaw += gyroY * elapsedTime;
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>ESP32 Plane Simulator</title>";
  html += "<style>body { font-family: Arial, sans-serif; text-align: center; }";
  html += "#plane { width: 200px; transition: transform 0.1s; }";
  html += ".container { margin-top: 50px; }";
  html += ".data { margin-top: 20px; font-size: 18px; }</style></head>";
  html += "<body><div class='container'>";
  html += "<h1>ESP32 Plane Simulator</h1>";
  html += "<img id='plane' src='https://cdn-icons-png.flaticon.com/512/47/47015.png' alt='Plane'>";
  html += "<div class='data'>";
  html += "<p>Pitch: <span id='pitch'>0</span>°</p>";
  html += "<p>Roll: <span id='roll'>0</span>°</p>";
  html += "<p>Yaw: <span id='yaw'>0</span>°</p>";
  html += "</div></div>";
  html += "<script>";
  html += "function updateData() {";
  html += "fetch('/data').then(response => response.json())";
  html += ".then(data => {";
  html += "document.getElementById('pitch').textContent = data.pitch.toFixed(1);";
  html += "document.getElementById('roll').textContent = data.roll.toFixed(1);";
  html += "document.getElementById('yaw').textContent = data.yaw.toFixed(1);";
  html += "const plane = document.getElementById('plane');";
  html += "plane.style.transform = `rotateX(${data.pitch}deg) rotateY(${data.yaw}deg) rotateZ(${data.roll}deg)`;";
  html += "setTimeout(updateData, 50);";
  html += "}).catch(error => setTimeout(updateData, 1));";
  html += "}";
  html += "updateData();";
  html += "</script></body></html>";
  
  server.send(200, "text/html", html);
}

void handleData() {
  String data = "{";
  data += "\"pitch\":" + String(pitch, 2) + ",";
  data += "\"roll\":" + String(roll, 2) + ",";
  data += "\"yaw\":" + String(yaw, 2);
  data += "}";
  
  server.send(200, "application/json", data);
}