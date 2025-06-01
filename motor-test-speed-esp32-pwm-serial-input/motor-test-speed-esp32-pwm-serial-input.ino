#include <Arduino.h>

// Using safe GPIO pins
const int motorPins[4] = {13, 12, 14, 27}; 
const int pwmChannels[4] = {0, 1, 2, 3};

void setup() {
  Serial.begin(115200);
  
  // Initialize all motors
  for(int i=0; i<4; i++){
    pinMode(motorPins[i], OUTPUT);
    ledcAttach(motorPins[i] ,22000,10);
    ledcWrite(motorPins[i], 0);  // 50% duty

  }
  
  Serial.println("\nMotor Test Ready");
  Serial.println("Send '+' to increase speed, '-' to decrease");
}

void loop() {
  if(Serial.available()){
    char cmd = Serial.read();
    
    if(cmd == '+'){
      for(int i=0; i<4; i++){
        int current = ledcRead(motorPins[i]);
        ledcWrite(motorPins[i], min(current+5, 255));
      }
    }
    else if(cmd == '-'){
      for(int i=0; i<4; i++){
        int current = ledcRead(motorPins[i]);
        ledcWrite(motorPins[i], max(current-5, 0));
      }
    }
    
    // Print current speed
    Serial.print("Current duty: ");
    Serial.println(ledcRead(motorPins[0])); // All same speed
  }
}