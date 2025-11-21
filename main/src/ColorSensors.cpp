/*********************************************************************
 * Color Sensors Library Implementation
 *********************************************************************/

#include "ColorSensors.h"

ColorSensors colorSensors;

ColorSensors::ColorSensors() {
  mux = new I2CMux();
  bottomSensor = nullptr;
  topSensor = nullptr;
  backSensor = nullptr;
  bottomInitialized = false;
  topInitialized = false;
  backInitialized = false;
  
  bottomContinuousReading = false;
  topContinuousReading = false;
  backContinuousReading = false;
  
  // Initialize data structures
  bottomData = {0, 0, 0, 0, 0, 0};
  topData = {0, 0, 0, 0, 0, 0};
  backData = {0, 0, 0, 0, 0, 0};
}

ColorSensors::~ColorSensors() {
  if (bottomSensor) delete bottomSensor;
  if (topSensor) delete topSensor;
  if (backSensor) delete backSensor;
  if (mux) delete mux;
}

bool ColorSensors::begin() {
  Serial.println("Initializing Color Sensors...");
  
  // Initialize I2C Multiplexer
  mux->begin();
  delay(100);  // Give mux time to stabilize
  
  // Disable all channels first
  mux->disableAll();
  delay(100);
  
  // Initialize bottom sensor (Channel 5)
  Serial.print("Selecting Channel "); Serial.print(BOTTOM_COLOR_CHANNEL); Serial.println(" for Bottom sensor...");
  mux->selectChannel(BOTTOM_COLOR_CHANNEL);
  delay(100);
  
  bottomSensor = new Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_600MS,
    TCS34725_GAIN_1X
  );
  
  if (bottomSensor->begin()) {
    bottomInitialized = true;
    Serial.println("  Bottom color sensor (Ch 5): OK");
  } else {
    Serial.println("  Bottom color sensor (Ch 5): FAILED - Retrying...");
    delay(100);
    if (bottomSensor->begin()) {
      bottomInitialized = true;
      Serial.println("  Bottom color sensor (Ch 5): OK on retry");
    } else {
      Serial.println("  Bottom color sensor (Ch 5): FAILED after retry");
      bottomInitialized = false;
    }
    Serial.println("  Bottom color sensor (Ch 5): FAILED");
    bottomInitialized = false;
  }
  
  mux->disableAll();
  delay(100);
  
  // Initialize top sensor (Channel 2)
  Serial.print("Selecting Channel "); Serial.print(TOP_COLOR_CHANNEL); Serial.println(" for Top sensor...");
  mux->selectChannel(TOP_COLOR_CHANNEL);
  delay(100);
  
  topSensor = new Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_600MS,
    TCS34725_GAIN_1X
  );
  
  if (topSensor->begin()) {
    topInitialized = true;
    Serial.println("  Top color sensor (Ch 2): OK - Ball Detection");
  } else {
    Serial.println("  Top color sensor (Ch 2): FAILED - Retrying...");
    delay(100);
    if (topSensor->begin()) {
      topInitialized = true;
      Serial.println("  Top color sensor (Ch 2): OK on retry");
    } else {
      Serial.println("  Top color sensor (Ch 2): FAILED after retry");
      topInitialized = false;
    }
  }
  
  mux->disableAll();
  delay(100);
  
  // Initialize back sensor (Channel 1)
  Serial.print("Selecting Channel "); Serial.print(BACK_COLOR_CHANNEL); Serial.println(" for Back sensor...");
  mux->selectChannel(BACK_COLOR_CHANNEL);
  delay(100);
  
  backSensor = new Adafruit_TCS34725(
    TCS34725_INTEGRATIONTIME_600MS,
    TCS34725_GAIN_1X
  );
  
  if (backSensor->begin()) {
    backInitialized = true;
    Serial.println("  Back color sensor (Ch 1): OK");
  } else {
    Serial.println("  Back color sensor (Ch 1): FAILED - Retrying...");
    delay(100);
    if (backSensor->begin()) {
      backInitialized = true;
      Serial.println("  Back color sensor (Ch 1): OK on retry");
    } else {
      Serial.println("  Back color sensor (Ch 1): FAILED after retry");
      backInitialized = false;
    }
  }
  
  mux->disableAll();
  
  if (bottomInitialized || topInitialized || backInitialized) {
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

bool ColorSensors::readBackSensor() {
  if (!backInitialized) return false;
  
  mux->selectChannel(BACK_COLOR_CHANNEL);
  delay(5);
  
  backSensor->getRawData(&backData.r, &backData.g, &backData.b, &backData.c);
  backData.colorTemp = backSensor->calculateColorTemperature(backData.r, backData.g, backData.b);
  backData.lux = backSensor->calculateLux(backData.r, backData.g, backData.b);
  
  mux->disableAll();
  return true;
}

void ColorSensors::readAll() {
  readBottomSensor();
  readTopSensor();
  readBackSensor();
}

ColorData ColorSensors::getBottomColorData() {
  return bottomData;
}

ColorData ColorSensors::getTopColorData() {
  return topData;
}

ColorData ColorSensors::getBackColorData() {
  return backData;
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

uint16_t ColorSensors::getBackRed() { return backData.r; }
uint16_t ColorSensors::getBackGreen() { return backData.g; }
uint16_t ColorSensors::getBackBlue() { return backData.b; }
uint16_t ColorSensors::getBackClear() { return backData.c; }
uint16_t ColorSensors::getBackLux() { return backData.lux; }

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
  // Bottom sensor only needs to detect: BLACK, GREEN, WHITE
  // Based on calibration data:
  // Black:  R~321,  G~276,  B~187,  C~749
  // Green:  R~1598, G~2125, B~1239, C~3960
  // White:  R~5272, G~4955, B~3331, C~13620
  
  uint16_t c = bottomData.c;
  uint16_t g = bottomData.g;
  uint16_t r = bottomData.r;
  
  // BLACK detection: C < 1500
  if (c < 1500) {
    return COLOR_BLACK;
  }
  
  // WHITE detection: C > 8000
  if (c > 8000) {
    return COLOR_WHITE;
  }
  
  // GREEN detection: C in range 2000-6000 AND G is dominant
  // Green should have G > R and G > B with G being significantly higher
  if (c >= 2000 && c <= 6000) {
    if (g > r * 1.2 && g > bottomData.b * 1.5) {
      return COLOR_GREEN;
    }
  }
  
  // Default to unknown if not clearly one of the three
  return COLOR_UNKNOWN;
}

DetectedColor ColorSensors::getTopColor() {
  // Special detection for top sensor (ball detection)
  // Only needs to differentiate YELLOW vs WHITE balls
  
  uint16_t r = topData.r;
  uint16_t g = topData.g;
  uint16_t b = topData.b;
  uint16_t c = topData.c;
  
  // Low brightness = no ball detected
  if (c < 1000) {
    return COLOR_UNKNOWN;
  }
  
  // Calculate R/G ratio
  // Yellow ball: R/G ratio is around 1.4-1.5 (R is significantly higher than G)
  // White ball: R/G ratio is around 0.9-1.1 (R and G are balanced, or G > R)
  
  if (g > 0) {  // Prevent division by zero
    float rgRatio = (float)r / (float)g;
    
    // Yellow detection: R is noticeably higher than G
    if (rgRatio > 1.25) {
      return COLOR_YELLOW;
    }
    // White detection: R and G are balanced or G is higher
    else if (rgRatio >= 0.85 && rgRatio <= 1.25) {
      return COLOR_WHITE;
    }
  }
  
  // Fallback to general classification
  return classifyColor(r, g, b, c);
}

DetectedColor ColorSensors::getBackColor() {
  // Back sensor only needs to detect: RED vs BLUE
  // Based on calibration data:
  // Red:  R~221, G~88,  B~69,  C~370,  Temp~3261K
  // Blue: R~95,  G~98,  B~100, C~251,  Temp~9965K
  
  uint16_t r = backData.r;
  uint16_t g = backData.g;
  uint16_t b = backData.b;
  uint16_t temp = backData.colorTemp;
  
  // Use color temperature as primary discriminator:
  // Red: ~3261K (warm)
  // Blue: ~9965K (cool)
  
  if (temp < 6000) {
    // Warm temperature = RED
    // Also verify R is dominant
    if (r > g * 1.5 && r > b * 2.0) {
      return COLOR_RED;
    }
  } else {
    // Cool temperature = BLUE
    // Also verify RGB values are balanced (blue surface reflects all colors)
    if (abs(r - g) < 20 && abs(g - b) < 20) {
      return COLOR_BLUE;
    }
  }
  
  // Fallback: use RGB ratios if temperature is inconclusive
  if (r > g && r > b && r > g * 1.5) {
    return COLOR_RED;
  } else if (abs(r - g) < 30 && abs(g - b) < 30) {
    return COLOR_BLUE;
  }
  
  return COLOR_UNKNOWN;
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

bool ColorSensors::isBackReady() {
  return backInitialized;
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
    Serial.println("Bottom Sensor (Ch 5):");
    Serial.print("  R: "); Serial.print(bottom.r);
    Serial.print("  G: "); Serial.print(bottom.g);
    Serial.print("  B: "); Serial.print(bottom.b);
    Serial.print("  C: "); Serial.println(bottom.c);
    Serial.print("  Color: "); Serial.println(colorSensors.getColorName(colorSensors.getBottomColor()));
    Serial.print("  Lux: "); Serial.print(bottom.lux);
    Serial.print("  Temp: "); Serial.print(bottom.colorTemp); Serial.println(" K");
  } else {
    Serial.println("Bottom Sensor (Ch 5): NOT INITIALIZED");
  }
  
  if (colorSensors.isTopReady()) {
    ColorData top = colorSensors.getTopColorData();
    Serial.println("Top Sensor (Ch 2 - Ball Detection):");
    Serial.print("  R: "); Serial.print(top.r);
    Serial.print("  G: "); Serial.print(top.g);
    Serial.print("  B: "); Serial.print(top.b);
    Serial.print("  C: "); Serial.println(top.c);
    Serial.print("  Color: "); Serial.println(colorSensors.getColorName(colorSensors.getTopColor()));
    Serial.print("  Lux: "); Serial.print(top.lux);
    Serial.print("  Temp: "); Serial.print(top.colorTemp); Serial.println(" K");
  } else {
    Serial.println("Top Sensor (Ch 2): NOT INITIALIZED");
  }
  
  if (colorSensors.isBackReady()) {
    ColorData back = colorSensors.getBackColorData();
    Serial.println("Back Sensor (Ch 1):");
    Serial.print("  R: "); Serial.print(back.r);
    Serial.print("  G: "); Serial.print(back.g);
    Serial.print("  B: "); Serial.print(back.b);
    Serial.print("  C: "); Serial.println(back.c);
    Serial.print("  Color: "); Serial.println(colorSensors.getColorName(colorSensors.getBackColor()));
    Serial.print("  Lux: "); Serial.print(back.lux);
    Serial.print("  Temp: "); Serial.print(back.colorTemp); Serial.println(" K");
  } else {
    Serial.println("Back Sensor (Ch 1): NOT INITIALIZED");
  }
  
  Serial.println("============================\n");
}

void ColorSensors::toggleBottomContinuousReading() {
  bottomContinuousReading = !bottomContinuousReading;
  if (bottomContinuousReading) {
    Serial.println("Bottom color sensor continuous reading: ON");
  } else {
    Serial.println("Bottom color sensor continuous reading: OFF");
  }
}

void ColorSensors::toggleTopContinuousReading() {
  topContinuousReading = !topContinuousReading;
  if (topContinuousReading) {
    Serial.println("Top color sensor continuous reading: ON");
  } else {
    Serial.println("Top color sensor continuous reading: OFF");
  }
}

void ColorSensors::toggleBackContinuousReading() {
  backContinuousReading = !backContinuousReading;
  if (backContinuousReading) {
    Serial.println("Back color sensor continuous reading: ON");
  } else {
    Serial.println("Back color sensor continuous reading: OFF");
  }
}

bool ColorSensors::isBottomContinuousReading() {
  return bottomContinuousReading;
}

bool ColorSensors::isTopContinuousReading() {
  return topContinuousReading;
}

bool ColorSensors::isBackContinuousReading() {
  return backContinuousReading;
}

void ColorSensors::stopAllContinuousReading() {
  bottomContinuousReading = false;
  topContinuousReading = false;
  backContinuousReading = false;
  Serial.println("All color sensor continuous reading stopped");
}

// Scan all I2C mux channels to find color sensors
void scanColorSensors() {
  Serial.println("\n=== Scanning for Color Sensors on All Channels ===");
  I2CMux* scanMux = new I2CMux();
  scanMux->begin();
  
  for (int channel = 0; channel < 8; channel++) {
    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(": ");
    
    scanMux->selectChannel(channel);
    delay(50);
    
    Adafruit_TCS34725* testSensor = new Adafruit_TCS34725(
      TCS34725_INTEGRATIONTIME_600MS,
      TCS34725_GAIN_1X
    );
    
    if (testSensor->begin()) {
      Serial.print("COLOR SENSOR FOUND! Testing...");
      uint16_t r, g, b, c;
      testSensor->getRawData(&r, &g, &b, &c);
      Serial.print(" R=");
      Serial.print(r);
      Serial.print(" G=");
      Serial.print(g);
      Serial.print(" B=");
      Serial.print(b);
      Serial.print(" C=");
      Serial.println(c);
    } else {
      Serial.println("No sensor detected");
    }
    
    delete testSensor;
    delay(50);
  }
  
  scanMux->disableAll();
  delete scanMux;
  Serial.println("=== Scan Complete ===");
  Serial.println("Update channel definitions in ColorSensors.h based on results above.\n");
}
