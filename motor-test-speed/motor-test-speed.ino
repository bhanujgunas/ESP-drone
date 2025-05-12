#include <ESP32Servo.h>

Servo mot1;
Servo mot2;
Servo mot3;
Servo mot4;
const int mot1_pin = 13;
const int mot2_pin = 12;
const int mot3_pin = 14;
const int mot4_pin = 27;

// Motor control variables
int motorSpeed = 0;  // 0-100% speed
bool serialControl = false;  // Flag for serial control mode

void setup() {
  Serial.begin(115200);
  
  // Initialize motors
  mot1.attach(mot1_pin, 0, 65536);
  mot2.attach(mot2_pin, 0, 65536);
  mot3.attach(mot3_pin, 0, 65536);
  mot4.attach(mot4_pin, 0, 65536);
  
  // Stop all motors initially
  setAllMotors(0);
  
  Serial.println("Motor Control Ready");
  Serial.println("Enter speed (0-100) in Serial Monitor");
  Serial.println("or 'r' to switch to receiver control");
}

void loop() {
  // Check for serial input
  if (Serial.available()) {
    handleSerialInput();
  }

  if (serialControl) {
    // Set all motors to the same speed (0-100%)
    setAllMotors(motorSpeed);
  }
  
  // Small delay to prevent flooding the serial port
  delay(20);
}

void handleSerialInput() {
  String input = Serial.readStringUntil('\n');
  input.trim();
  
  if (input.equalsIgnoreCase("r")) {
    serialControl = false;
    Serial.println("Switched to receiver control mode");
  } 
  else {
    int speed = input.toInt();
    if (speed >= 0 && speed <= 100) {
      motorSpeed = speed;
      serialControl = true;
      Serial.print("Set all motors to: ");
      Serial.print(motorSpeed);
      Serial.println("%");
    } else {
      Serial.println("Invalid input! Enter 0-100 or 'r'");
    }
  }
}

void setAllMotors(int speedPercent) {
  // Convert 0-100% to 1000-2000µs PWM signal
  int pwmValue = map(speedPercent, 0, 100, 0, 2000);
  
  // Write to all motors
  mot1.writeMicroseconds(pwmValue);
  mot2.writeMicroseconds(pwmValue);
  mot3.writeMicroseconds(pwmValue);
  mot4.writeMicroseconds(pwmValue);
}