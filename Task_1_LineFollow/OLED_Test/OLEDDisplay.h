/*********************************************************************
 * OLED Display Library
 * SSD1306 128x64 OLED display wrapper for ESP32
 * 
 * Features:
 * - Simple text display (up to 4 lines)
 * - Robot status display
 * - TOF sensor display
 * - IR sensor display
 * - Custom message display
 *********************************************************************/

#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Configuration
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)

class OLEDDisplay {
  private:
    Adafruit_SSD1306 display;
    int width;
    int height;
    int address;
    bool initialized;

  public:
    // Constructor
    OLEDDisplay(int w = OLED_WIDTH, int h = OLED_HEIGHT, int addr = OLED_ADDRESS);
    
    // Initialize display
    bool begin();
    
    // Check if initialized
    bool isInitialized();
    
    // Clear display
    void clear();
    
    // Show simple text (up to 4 lines)
    void showText(const String& line1, const String& line2 = "", 
                  const String& line3 = "", const String& line4 = "");
    
    // Show robot status
    void showRobotStatus(const String& mode, int speed, const String& state);
    
    // Show TOF sensor readings
    void showTOFReadings(uint16_t left, uint16_t front, uint16_t right);
    
    // Show IR sensor readings (8 sensors)
    void showIRReadings(int* irValues, int count);
    
    // Show position (for maze navigation)
    void showPosition(int col, int row, int facing, int obstacles, int trackingMode);
    
    // Show custom formatted message
    void showMessage(const String& title, const String& message);
    
    // Low-level access to display object
    Adafruit_SSD1306& getDisplay();
};

// Global OLED display object
extern OLEDDisplay oledDisplay;

// Global functions for easy access
void initOLED();
void displayText(const String& line1, const String& line2 = "", 
                 const String& line3 = "", const String& line4 = "");
void displayRobotStatus(const String& mode, int speed, const String& state);
void displayTOF(uint16_t left, uint16_t front, uint16_t right);
void displayMessage(const String& title, const String& message);

#endif // OLEDDISPLAY_H
