#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_ADS1115 ads;

// Joystick button pins
const int leftButtonPin = D3;
const int rightButtonPin = D4;

// Calibration values
const int leftXmin = 18, leftXmid = 13551, leftXmax = 26475;
const int leftYmin = 16, leftYmid = 13192, leftYmax = 26474;
const int rightXmin = 7, rightXmid = 13145, rightXmax = 26484;
const int rightYmin = 7, rightYmid = 13354, rightYmax = 26483;

// Timer variables
unsigned long startTime = 0;
bool timerRunning = false;
String timerDisplay = "Ready";

void setup() {
  Serial.begin(115200);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  
  // Initialize ADS1115
  ads.begin();
  ads.setGain(GAIN_ONE);
  
  // Initialize buttons
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
  
  // Show initial message
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Joystick Calibrated");
  display.println("Testing...");
  display.display();
  delay(1000);
}

int calibratedX(int raw, int minVal, int midVal, int maxVal) {
  if (raw < midVal) {
    return map(raw, minVal, midVal, 0, 50);
  } else {
    return map(raw, midVal, maxVal, 50, 100);
  }
}

int calibratedY(int raw, int minVal, int midVal, int maxVal) {
  if (raw < midVal) {
    return map(raw, minVal, midVal, 100, 50);
  } else {
    return map(raw, midVal, maxVal, 50, 0);
  }
}

void updateTimer() {
  bool rightPressed = !digitalRead(rightButtonPin);
  bool leftPressed = !digitalRead(leftButtonPin);
  
  static bool lastRight = false;
  static bool lastLeft = false;
  
  // Start timer on right button press
  if (rightPressed && !lastRight && !timerRunning) {
    startTime = millis();
    timerRunning = true;
  }
  
  // Stop timer on left button press
  if (leftPressed && !lastLeft && timerRunning) {
    timerRunning = false;
    unsigned long elapsed = (millis() - startTime) / 1000;
    timerDisplay = String(elapsed / 60) + ":" + 
                 (elapsed % 60 < 10 ? "0" : "") + String(elapsed % 60);
  }
  
  // Update running timer
  if (timerRunning) {
    unsigned long elapsed = (millis() - startTime) / 1000;
    timerDisplay = String(elapsed / 60) + ":" + 
                  (elapsed % 60 < 10 ? "0" : "") + String(elapsed % 60);
  }
  
  lastRight = rightPressed;
  lastLeft = leftPressed;
}

void loop() {
  // Read raw values
  int16_t leftX = ads.readADC_SingleEnded(2);  // Roll
  int16_t leftY = ads.readADC_SingleEnded(1);  // Thrust
  int16_t rightX = ads.readADC_SingleEnded(0); // Yaw
  int16_t rightY = ads.readADC_SingleEnded(3); // Pitch
  
  // Read buttons
  bool leftButton = !digitalRead(leftButtonPin);
  bool rightButton = !digitalRead(rightButtonPin);

  // Calculate calibrated percentages
  int thrust = calibratedY(leftY, leftYmin, leftYmid, leftYmax);
  int yaw = calibratedX(rightX, rightXmin, rightXmid, rightXmax);
  int pitch = calibratedY(rightY, rightYmin, rightYmid, rightYmax);
  int roll = calibratedX(leftX, leftXmin, leftXmid, leftXmax);

  // Update timer
  updateTimer();

  // Update display
  display.clearDisplay();
  
  // Line 1: Thrust and Yaw
  display.setCursor(0,0);
  display.print("Thr:");
  display.print(thrust);
  display.print("%  Yaw:");
  display.print(yaw);
  display.print("%");
  
  // Line 2: Pitch and Roll
  display.setCursor(0,10);
  display.print("Pit:");
  display.print(pitch);
  display.print("%  Rol:");
  display.print(roll);
  display.print("%");
  
  // Line 3: Timer and buttons
  display.setCursor(0,20);
  if (timerRunning) {
    display.print("Running: ");
    display.print(timerDisplay);
  } else if (timerDisplay != "Ready") {
    display.print("Total Time: ");
    display.print(timerDisplay);
  } else {
    display.print("Press rjoy to start");
  }
  
  // Button states at end of line 3
  display.setCursor(0,30);
  display.print("L:");
  display.print(leftButton ? "1" : "0");
  display.print(" R:");
  display.print(rightButton ? "1" : "0");

  // Position indicators (kept but made smaller)
  // Left joystick (Thrust/Roll)
  display.drawCircle(32, 50, 10, WHITE);
  display.fillCircle(
    32 + map(yaw, 0, 100, -8, 8),
    50 + map(thrust, 0, 100, 8, -8),
    2, WHITE);
  
  // Right joystick (Yaw/Pitch)
  display.drawCircle(96, 50, 10, WHITE);
  display.fillCircle(
    96 + map(roll, 0, 100, -8, 8),
    50 + map(pitch, 0, 100, 8, -8),
    2, WHITE);

  display.display();
  delay(50);
}