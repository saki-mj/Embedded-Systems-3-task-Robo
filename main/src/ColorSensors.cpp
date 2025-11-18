/*********************************************************************
 * Color Sensors Library Implementation
 *********************************************************************/

#include "ColorSensors.h"

ColorSensors colorSensors;

ColorSensors::ColorSensors() {
  mux = new I2CMux();
  bottomSensor = nullptr;
  topSensor = nullptr;
  bottomInitialized = false;
  topInitialized = false;
  
  // Initialize data structures
  bottomData = {0, 0, 0, 0, 0, 0};
  topData = {0, 0, 0, 0, 0, 0};
}

ColorSensors::~ColorSensors() {
  if (bottomSensor) delete bottomSensor;
  if (topSensor) delete topSensor;
  if (mux) delete mux;
}

bool ColorSensors::begin() {
  Serial.println("Initializing Color Sensors...");
  
  // Initialize I2C Multiplexer
  mux->begin();
  
  // Initialize bottom sensor (Channel 5)
  mux->selectChannel(BOTTOM_COLOR_CHANNEL);
  delay(10);
  
  bottomSensor = new Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_600MS,
    TCS34725_GAIN_1X
  );
  
  if (bottomSensor->begin()) {
    bottomInitialized = true;
    Serial.println("  Bottom color sensor (Ch 5): OK");
  } else {
    Serial.println("  Bottom color sensor (Ch 5): FAILED");
    bottomInitialized = false;
  }
  
  delay(50);
  
  // Initialize top sensor (Channel 1)
  mux->selectChannel(TOP_COLOR_CHANNEL);
  delay(10);
  
  topSensor = new Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_600MS,
    TCS34725_GAIN_1X
  );
  
  if (topSensor->begin()) {
    topInitialized = true;
    Serial.println("  Top color sensor (Ch 1): OK");
  } else {
    Serial.println("  Top color sensor (Ch 1): FAILED");
    topInitialized = false;
  }
  
  mux->disableAll();
  
  if (bottomInitialized || topInitialized) {
    Serial.println("Color Sensors initialized!");
    return true;
  } else {
    Serial.println("Color Sensors initialization FAILED!");
    return false;
  }
}

bool ColorSensors::readBottomSensor() {
  if (!bottomInitialized) return false;
  
  mux->selectChannel(BOTTOM_COLOR_CHANNEL);
  delay(5);
  
  bottomSensor->getRawData(&bottomData.r, &bottomData.g, &bottomData.b, &bottomData.c);
  bottomData.colorTemp = bottomSensor->calculateColorTemperature(bottomData.r, bottomData.g, bottomData.b);
  bottomData.lux = bottomSensor->calculateLux(bottomData.r, bottomData.g, bottomData.b);
  
  mux->disableAll();
  return true;
}

bool ColorSensors::readTopSensor() {
  if (!topInitialized) return false;
  
  mux->selectChannel(TOP_COLOR_CHANNEL);
  delay(5);
  
  topSensor->getRawData(&topData.r, &topData.g, &topData.b, &topData.c);
  topData.colorTemp = topSensor->calculateColorTemperature(topData.r, topData.g, topData.b);
  topData.lux = topSensor->calculateLux(topData.r, topData.g, topData.b);
  
  mux->disableAll();
  return true;
}

void ColorSensors::readAll() {
  readBottomSensor();
  readTopSensor();
}

ColorData ColorSensors::getBottomColorData() {
  return bottomData;
}

ColorData ColorSensors::getTopColorData() {
  return topData;
}

uint16_t ColorSensors::getBottomRed() { return bottomData.r; }
uint16_t ColorSensors::getBottomGreen() { return bottomData.g; }
uint16_t ColorSensors::getBottomBlue() { return bottomData.b; }
uint16_t ColorSensors::getBottomClear() { return bottomData.c; }
uint16_t ColorSensors::getBottomLux() { return bottomData.lux; }

uint16_t ColorSensors::getTopRed() { return topData.r; }
uint16_t ColorSensors::getTopGreen() { return topData.g; }
uint16_t ColorSensors::getTopBlue() { return topData.b; }
uint16_t ColorSensors::getTopClear() { return topData.c; }
uint16_t ColorSensors::getTopLux() { return topData.lux; }

DetectedColor ColorSensors::classifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
  // Low brightness = black
  if (c < 100) {
    return COLOR_BLACK;
  }
  
  // Very high brightness with balanced RGB = white
  if (c > 3000 && abs(r - g) < 300 && abs(g - b) < 300 && abs(r - b) < 300) {
    return COLOR_WHITE;
  }
  
  // Find the dominant color
  uint16_t maxVal = max(r, max(g, b));
  uint16_t minVal = min(r, min(g, b));
  
  // Not enough color difference = white or gray
  if (maxVal - minVal < 200) {
    return c > 1500 ? COLOR_WHITE : COLOR_UNKNOWN;
  }
  
  // Check for specific colors
  if (r > g && r > b) {
    // Red is dominant
    if (g > b * 1.5) {
      return COLOR_YELLOW; // Red + Green = Yellow
    }
    return COLOR_RED;
  }
  else if (g > r && g > b) {
    // Green is dominant
    return COLOR_GREEN;
  }
  else if (b > r && b > g) {
    // Blue is dominant
    return COLOR_BLUE;
  }
  
  return COLOR_UNKNOWN;
}

DetectedColor ColorSensors::getBottomColor() {
  return classifyColor(bottomData.r, bottomData.g, bottomData.b, bottomData.c);
}

DetectedColor ColorSensors::getTopColor() {
  return classifyColor(topData.r, topData.g, topData.b, topData.c);
}

String ColorSensors::getColorName(DetectedColor color) {
  switch(color) {
    case COLOR_RED: return "RED";
    case COLOR_GREEN: return "GREEN";
    case COLOR_BLUE: return "BLUE";
    case COLOR_YELLOW: return "YELLOW";
    case COLOR_WHITE: return "WHITE";
    case COLOR_BLACK: return "BLACK";
    case COLOR_UNKNOWN: return "UNKNOWN";
    default: return "ERROR";
  }
}

bool ColorSensors::isBottomReady() {
  return bottomInitialized;
}

bool ColorSensors::isTopReady() {
  return topInitialized;
}

// Global initialization function
void initColorSensors() {
  colorSensors.begin();
}

void printColorValues() {
  colorSensors.readAll();
  
  Serial.println("\n=== Color Sensor Readings ===");
  
  if (colorSensors.isBottomReady()) {
    ColorData bottom = colorSensors.getBottomColorData();
    Serial.println("Bottom Sensor:");
    Serial.print("  R: "); Serial.print(bottom.r);
    Serial.print("  G: "); Serial.print(bottom.g);
    Serial.print("  B: "); Serial.print(bottom.b);
    Serial.print("  C: "); Serial.println(bottom.c);
    Serial.print("  Color: "); Serial.println(colorSensors.getColorName(colorSensors.getBottomColor()));
    Serial.print("  Lux: "); Serial.print(bottom.lux);
    Serial.print("  Temp: "); Serial.print(bottom.colorTemp); Serial.println(" K");
  }
  
  if (colorSensors.isTopReady()) {
    ColorData top = colorSensors.getTopColorData();
    Serial.println("Top Sensor:");
    Serial.print("  R: "); Serial.print(top.r);
    Serial.print("  G: "); Serial.print(top.g);
    Serial.print("  B: "); Serial.print(top.b);
    Serial.print("  C: "); Serial.println(top.c);
    Serial.print("  Color: "); Serial.println(colorSensors.getColorName(colorSensors.getTopColor()));
    Serial.print("  Lux: "); Serial.print(top.lux);
    Serial.print("  Temp: "); Serial.print(top.colorTemp); Serial.println(" K");
  }
  
  Serial.println("============================\n");
}
