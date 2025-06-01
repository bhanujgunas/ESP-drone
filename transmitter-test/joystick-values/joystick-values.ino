#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define OLED_RESET -1 // Reset pin

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_ADS1115 ads;

// Joystick button pins
const int leftButtonPin = D3;
const int rightButtonPin = D4;

// Calibration values - based on your measurements
const int leftXmin = 19;
const int leftXmid = 13549;
const int leftXmax = 26474;
const int leftYmin = 16;
const int leftYmid = 13192;
const int leftYmax = 26474;

const int rightXmin = 7;
const int rightXmid = 13147;
const int rightXmax = 26484;
const int rightYmin = 7;
const int rightYmid = 13355;
const int rightYmax = 26483;

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

void loop() {
  // Read raw values
  int16_t leftX = ads.readADC_SingleEnded(0);
  int16_t leftY = ads.readADC_SingleEnded(1);
  int16_t rightX = ads.readADC_SingleEnded(2);
  int16_t rightY = ads.readADC_SingleEnded(3);
  
  // Read buttons
  bool leftButton = !digitalRead(leftButtonPin);
  bool rightButton = !digitalRead(rightButtonPin);

  // Calculate calibrated percentages
  int leftXpercent = calibratedX(leftX, leftXmin, leftXmid, leftXmax);
  int leftYpercent = calibratedY(leftY, leftYmin, leftYmid, leftYmax);
  int rightXpercent = calibratedX(rightX, rightXmin, rightXmid, rightXmax);
  int rightYpercent = calibratedY(rightY, rightYmin, rightYmid, rightYmax);

  // Update display
  display.clearDisplay();
  
  // Left Joystick Display
  display.setCursor(0,0);
  display.print("Left:");
  display.setCursor(0,10);
  display.print("X");
  display.print(leftX);
  display.print("|");
  display.print(leftXpercent);
  display.print("%");
  
  display.setCursor(0,20);
  display.print("Y");
  display.print(leftY);
  display.print("|");
  display.print(leftYpercent);
  display.print("%");
  
  display.setCursor(0,30);
  display.print("BTN:");
  display.print(leftButton ? "PRESSED" : "OPEN");

  // Right Joystick Display
  display.setCursor(64,0);
  display.print("Right:");
  display.setCursor(64,10);
  display.print("X");
  display.print(rightX);
  display.print("|");
  display.print(rightXpercent);
  display.print("%");
  
  display.setCursor(64,20);
  display.print("Y");
  display.print(rightY);
  display.print("|");
  display.print(rightYpercent);
  display.print("%");
  
  display.setCursor(64,30);
  display.print("BTN:");
  display.print(rightButton ? "PRESSED" : "OPEN");

  // Position indicators
  // Left joystick
  display.drawCircle(32, 50, 15, WHITE);
  display.fillCircle(
    32 + map(leftXpercent, 0, 100, -12, 12),
    50 + map(leftYpercent, 0, 100, -12, 12),
    3, WHITE);
  
  // Right joystick
  display.drawCircle(96, 50, 15, WHITE);
  display.fillCircle(
    96 + map(rightXpercent, 0, 100, -12, 12),
    50 + map(rightYpercent, 0, 100, -12, 12),
    3, WHITE);

  display.display();
  delay(50);
}