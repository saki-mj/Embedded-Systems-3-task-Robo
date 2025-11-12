/*********************************************************************
 * OLED Display Library
 * SSD1306 128x64 OLED display wrapper for ESP32
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
#define OLED_RESET -1

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
    void show(const String& l1, const String& l2 = "", const String& l3 = "", const String& l4 = "");
    
    // Show robot status
    void showStatus(const String& mode, int speed);
    
    // Show TOF sensor readings
    void showTOF(uint16_t left, uint16_t front, uint16_t right);
    
    // Show IR sensor status
    void showIRStatus(bool calibrated, int activeCount);
    
    // Show line following state
    void showLineFollowing(bool active, float error);
};

// Global OLED display object
extern OLEDDisplay oledDisplay;

// Initialization function
void initOLED();

#endif
