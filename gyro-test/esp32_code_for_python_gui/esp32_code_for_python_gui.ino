#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// Sensor data with calibration offsets
float pitch = 0, roll = 0, yaw = 0;
float pitch_offset = 0, roll_offset = 0, yaw_offset = 0;
bool calibrating = false;
unsigned long calibration_start = 0;
const unsigned long calibration_duration = 5000; // 5 seconds calibration

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port
  
  Wire.begin();
  mpu.initialize();
  
  if (mpu.testConnection()) {
    Serial.println("MPU6050 OK");
  } else {
    Serial.println("MPU6050 FAIL");
  }
  
  // Initial calibration
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
  Serial.println("READY");
}

void loop() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();
  
  // Handle calibration
  if (calibrating) {
    if (currentTime - calibration_start >= calibration_duration) {
      calibrating = false;
      Serial.println("CALIBRATION_COMPLETE");
    }
    return; // Skip normal processing during calibration
  }
  
  // Normal operation at ~20Hz
  if (currentTime - lastTime >= 50) {
    lastTime = currentTime;
    
    // Get sensor data
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    // Convert to readable values
    float accX = ax / 16384.0;
    float accY = ay / 16384.0;
    float accZ = az / 16384.0;
    
    // Calculate angles
    float accPitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180/PI;
    float accRoll = atan2(accY, accZ) * 180/PI;
    
    // Gyro integration
    float gyroY = gy / 131.0;
    float gyroZ = gz / 131.0;
    
    // Time difference in seconds
    float dt = 0.05;
    
    // Complementary filter
    pitch = 0.98 * (pitch + gyroY * dt) + 0.02 * accPitch - pitch_offset;
    roll = 0.98 * (roll + (gx / 131.0) * dt) + 0.02 * accRoll - roll_offset;
    yaw += gyroZ * dt - yaw_offset;
    
    // Send data
    Serial.print("PITCH:");
    Serial.print(pitch, 1);
    Serial.print(",ROLL:");
    Serial.print(roll, 1);
    Serial.print(",YAW:");
    Serial.println(yaw, 1);
    
    // Check for calibration command
    if (Serial.available()) {
      char cmd = Serial.read();
      if (cmd == 'c') {
        startCalibration();
      }
    }
  }
}

void startCalibration() {
  calibrating = true;
  calibration_start = millis();
  pitch_offset = 0;
  roll_offset = 0;
  yaw_offset = 0;
  
  // Collect samples for averaging
  int samples = 0;
  float pitch_sum = 0, roll_sum = 0, yaw_sum = 0;
  
  while (millis() - calibration_start < calibration_duration) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    float accX = ax / 16384.0;
    float accY = ay / 16384.0;
    float accZ = az / 16384.0;
    
    pitch_sum += atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180/PI;
    roll_sum += atan2(accY, accZ) * 180/PI;
    yaw_sum += gz / 131.0 * 0.05;
    
    samples++;
  }
  
  // Calculate offsets
  pitch_offset = pitch_sum / samples;
  roll_offset = roll_sum / samples;
  yaw_offset = yaw_sum / samples;
}