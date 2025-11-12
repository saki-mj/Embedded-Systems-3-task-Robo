/*********************************************************************
 * Color Sensors Library - TCS34725 RGB Color Sensors
 * 
 * Two TCS34725 sensors connected via PCA9548A I2C Multiplexer:
 * - Bottom sensor on Channel 4
 * - Top sensor on Channel 2
 * 
 * I2C Address: 0x29 (both sensors, isolated by mux)
 *********************************************************************/

#ifndef COLORSENSORS_H
#define COLORSENSORS_H

#include <Arduino.h>
#include <Adafruit_TCS34725.h>
#include "I2CMux.h"

// Color sensor channels on I2C multiplexer
#define BOTTOM_COLOR_CHANNEL 4
#define TOP_COLOR_CHANNEL 2

// Color sensor I2C address (default for TCS34725)
#define TCS34725_ADDRESS 0x29

// Color structure
struct ColorData {
  uint16_t r;
  uint16_t g;
  uint16_t b;
  uint16_t c;        // Clear/brightness
  uint16_t colorTemp; // Color temperature in Kelvin
  uint16_t lux;      // Brightness in lux
};

// Color detection results
enum DetectedColor {
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_WHITE,
  COLOR_BLACK,
  COLOR_UNKNOWN
};

class ColorSensors {
  private:
    Adafruit_TCS34725* bottomSensor;
    Adafruit_TCS34725* topSensor;
    
    ColorData bottomData;
    ColorData topData;
    
    bool bottomInitialized;
    bool topInitialized;
    
    // Helper function to detect color from RGB values
    DetectedColor classifyColor(uint16_t r, uint16_t g, uint16_t b, uint16_t c);
    
  public:
    ColorSensors();
    ~ColorSensors();
    
    // Initialize color sensors through I2C mux
    bool begin();
    
    // Read from specific sensor
    bool readBottomSensor();
    bool readTopSensor();
    
    // Read both sensors
    void readAll();
    
    // Get raw color data
    ColorData getBottomColorData();
    ColorData getTopColorData();
    
    // Get specific color values
    uint16_t getBottomRed();
    uint16_t getBottomGreen();
    uint16_t getBottomBlue();
    uint16_t getBottomClear();
    uint16_t getBottomLux();
    
    uint16_t getTopRed();
    uint16_t getTopGreen();
    uint16_t getTopBlue();
    uint16_t getTopClear();
    uint16_t getTopLux();
    
    // Detect dominant color
    DetectedColor getBottomColor();
    DetectedColor getTopColor();
    
    // Get color name as string
    String getColorName(DetectedColor color);
    
    // Check sensor status
    bool isBottomReady();
    bool isTopReady();
};

// Global instance
extern ColorSensors colorSensors;

// Initialization function
void initColorSensors();
void printColorValues();

#endif
