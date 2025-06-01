#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

MPU6050 mpu;

// Sensor variables
int16_t ax, ay, az, gx, gy, gz;

// Timing
uint32_t prev_time = 0;
float dt;

// Orientation
float pitch = 0.0, roll = 0.0, yaw = 0.0;
const float comp_alpha = 0.98; // Complementary filter coefficient

// Drift correction
float yaw_drift_rate = 0.0;
float yaw_drift_alpha = 0.0001;
const float yaw_zero_threshold = 0.1;
bool drift_calibrated = false;

// Offsets
float gyro_x_offset = 0, gyro_y_offset = 0, gyro_z_offset = 0;
float accel_x_offset = 0, accel_y_offset = 0, accel_z_offset = 0;

// Sample rate control
const uint8_t DLPF_BANDWIDTH = MPU6050_DLPF_BW_42; // Digital Low Pass Filter setting
const uint8_t SAMPLE_RATE_DIVIDER = 0; // 0 gives maximum sample rate
const float EXPECTED_SAMPLE_RATE = 1000.0; // Hz (when using DLPF_BW_42)

void setup() {
  Wire.begin();
  Wire.setClock(400000); // I2C fast mode (400kHz)
  Serial.begin(115200); // High baud rate for faster serial output
  
  mpu.initialize();
  
  // Configure MPU6050 for higher sample rates
  mpu.setDLPFMode(DLPF_BANDWIDTH);
  mpu.setRate(SAMPLE_RATE_DIVIDER);
  mpu.setClockSource(MPU6050_CLOCK_PLL_XGYRO); // Best clock source
  
  Serial.println(mpu.testConnection() ? "MPU6050 connected" : "MPU6050 failed");
  
  calibrateSensors();
  calibrateDrift();
}

void loop() {
  uint32_t current_time = micros();
  dt = (current_time - prev_time) / 1000000.0;
  prev_time = current_time;
  
  // Read all sensors in one I2C transaction
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  
  // Convert units (optimized to avoid duplicate calculations)
  const float gyroX = (gx - gyro_x_offset) * (1.0f/131.0f);
  const float gyroY = (gy - gyro_y_offset) * (1.0f/131.0f);
  const float gyroZ = (gz - gyro_z_offset) * (1.0f/131.0f);
  
  const float accelX = (ax - accel_x_offset) * (1.0f/16384.0f);
  const float accelY = (ay - accel_y_offset) * (1.0f/16384.0f);
  const float accelZ = (az - accel_z_offset) * (1.0f/16384.0f);
  
  // Calculate angles from accelerometer (optimized)
  const float accel_sqrt = sqrt(accelX*accelX + accelZ*accelZ);
  const float accelPitch = atan2(accelY, accel_sqrt) * RAD_TO_DEG;
  const float accelRoll = atan2(-accelX, sqrt(accelY*accelY + accelZ*accelZ)) * RAD_TO_DEG;

  // Complementary filter
  pitch = comp_alpha*(pitch + gyroY*dt) + (1-comp_alpha)*accelPitch;
  roll = comp_alpha*(roll + gyroX*dt) + (1-comp_alpha)*accelRoll;

  // Drift correction
  if(drift_calibrated) {
    if(abs(gyroX)<yaw_zero_threshold && abs(gyroY)<yaw_zero_threshold) {
      yaw_drift_rate = yaw_drift_rate*(1-yaw_drift_alpha) + gyroZ*yaw_drift_alpha;
    }
    yaw -= (gyroZ - yaw_drift_rate)*dt;
  } else {
    yaw -= gyroZ*dt;
  }
  
  // Optional: Print at a controlled rate (e.g., 100Hz)
  static uint32_t last_print = 0;
  if(micros() - last_print > 10000) { // 10ms = 100Hz
    last_print = micros();
    Serial.print(pitch); Serial.print(",");
    Serial.print(roll); Serial.print(",");
    Serial.println(yaw);
  }
}


void calibrateSensors() {
  Serial.println("Calibrating sensors - keep still...");
  long gxSum=0, gySum=0, gzSum=0, axSum=0, aySum=0, azSum=0;
  const int samples = 20000;
  
  for(int i=0; i<samples; i++) {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    gxSum += gx; gySum += gy; gzSum += gz;
    axSum += ax; aySum += ay; azSum += az;
    delay(5);
  }
  
  gyro_x_offset = gxSum/samples;
  gyro_y_offset = gySum/samples;
  gyro_z_offset = gzSum/samples;
  
  accel_x_offset = axSum/samples;
  accel_y_offset = aySum/samples;
  accel_z_offset = (azSum/samples) - 16384; // Remove 1g
  
  Serial.println("Sensor calibration complete");
}

void calibrateDrift() {
  Serial.println("Calibrating yaw drift - keep sensor stationary for 10 seconds...");
  
  const unsigned long calibration_time = 20000; // 10 seconds
  unsigned long start_time = millis();
  unsigned long last_print = start_time;
  
  float drift_samples = 0;
  float drift_sum = 0;
  float max_drift = 0;
  
  while(millis() - start_time < calibration_time) {
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float gyroZ = (gz - gyro_z_offset)/131.0;
    
    // Only collect samples when truly stationary
    if(abs((ax-accel_x_offset)/16384.0) < 0.05 && 
       abs((ay-accel_y_offset)/16384.0) < 0.05 &&
       abs(gyroZ) < yaw_zero_threshold) {
      drift_sum += abs(gyroZ);
      if(abs(gyroZ) > max_drift) max_drift = abs(gyroZ);
      drift_samples++;
    }
    
    // Print progress every second
    if(millis() - last_print >= 1000) {
      last_print = millis();
      float progress = (millis()-start_time)/100;
      Serial.print("Calibrating... ");
      Serial.print(progress);
      Serial.println("%");
    }
    
    delay(10);
  }
  
  if(drift_samples > 0) {
    float avg_drift = drift_sum/drift_samples;
    
    // Auto-set drift alpha based on observed drift
    // More drift -> more aggressive correction (higher alpha)
    yaw_drift_alpha = constrain(map(avg_drift, 0, 0.5, 0.00005, 0.001), 0.00005, 0.001);
    
    Serial.print("Avg drift: "); Serial.print(avg_drift); Serial.print("°/s");
    Serial.print(" Max drift: "); Serial.print(max_drift); Serial.print("°/s");
    Serial.print(" Auto-set alpha: "); Serial.println(yaw_drift_alpha);
  } else {
    Serial.println("No valid drift samples collected!");
    yaw_drift_alpha = 0.0001; // Default fallback
  }
  
  drift_calibrated = true;
  Serial.println("Drift calibration complete");
}