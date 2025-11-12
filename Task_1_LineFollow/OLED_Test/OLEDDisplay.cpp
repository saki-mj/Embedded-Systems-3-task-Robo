/*********************************************************************
 * OLED Display Library Implementation
 *********************************************************************/

#include "OLEDDisplay.h"

// Global OLED display object
OLEDDisplay oledDisplay;

// Constructor
OLEDDisplay::OLEDDisplay(int w, int h, int addr) 
    : display(w, h, &Wire, OLED_RESET) {
    width = w;
    height = h;
    address = addr;
    initialized = false;
}

// Initialize display
bool OLEDDisplay::begin() {
    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
        Serial.println("ERROR: OLED initialization failed");
        initialized = false;
        return false;
    }
    
    Serial.print("OLED initialized at address 0x");
    Serial.println(address, HEX);
    
    // Clear display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED Ready!");
    display.display();
    delay(1000);
    
    initialized = true;
    return true;
}

// Check if initialized
bool OLEDDisplay::isInitialized() {
    return initialized;
}

// Clear display
void OLEDDisplay::clear() {
    if (!initialized) return;
    display.clearDisplay();
    display.display();
}

// Show simple text (up to 4 lines)
void OLEDDisplay::showText(const String& line1, const String& line2, 
                           const String& line3, const String& line4) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println(line1);
    if (line2 != "") display.println(line2);
    if (line3 != "") display.println(line3);
    if (line4 != "") display.println(line4);
    
    display.display();
}

// Show robot status
void OLEDDisplay::showRobotStatus(const String& mode, int speed, const String& state) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Title
    display.setTextSize(1);
    display.println("=== ROBOT STATUS ===");
    display.println();
    
    // Mode
    display.print("Mode: ");
    display.println(mode);
    
    // Speed
    display.print("Speed: ");
    display.print(speed);
    display.println(" PWM");
    
    // State
    display.print("State: ");
    display.println(state);
    
    display.display();
}

// Show TOF sensor readings
void OLEDDisplay::showTOFReadings(uint16_t left, uint16_t front, uint16_t right) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== TOF SENSORS ===");
    display.println();
    
    display.print("Left:  ");
    display.print(left);
    display.println(" mm");
    
    display.print("Front: ");
    display.print(front);
    display.println(" mm");
    
    display.print("Right: ");
    display.print(right);
    display.println(" mm");
    
    display.display();
}

// Show IR sensor readings (8 sensors)
void OLEDDisplay::showIRReadings(int* irValues, int count) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== IR SENSORS ===");
    display.println();
    
    // Show as bar graph
    for (int i = 0; i < count && i < 8; i++) {
        display.print(i);
        display.print(":");
        
        // Scale value to fit display (assuming 0-4095 range)
        int bars = map(irValues[i], 0, 4095, 0, 10);
        for (int j = 0; j < bars; j++) {
            display.print("#");
        }
        display.println();
    }
    
    display.display();
}

// Show position (for maze navigation)
void OLEDDisplay::showPosition(int col, int row, int facing, int obstacles, int trackingMode) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Position
    display.print("Position: C");
    display.print(col);
    display.print(", R");
    display.println(row);
    
    // Direction
    const char* directions[] = {"Right", "Down", "Left", "Up"};
    display.print("Direction: ");
    display.println(directions[facing % 4]);
    
    // Obstacles
    display.print("Obstacles: ");
    display.println(obstacles);
    
    // Tracking mode
    const char* modes[] = {"Main", "L-Corner", "R-Corner"};
    display.print("Track: ");
    display.println(modes[trackingMode % 3]);
    
    display.display();
}

// Show custom formatted message
void OLEDDisplay::showMessage(const String& title, const String& message) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Title (larger text)
    display.setTextSize(1);
    display.println(title);
    display.println("----------------");
    
    // Message (normal text)
    display.setTextSize(1);
    display.println(message);
    
    display.display();
}

// Low-level access to display object
Adafruit_SSD1306& OLEDDisplay::getDisplay() {
    return display;
}

// Global functions for easy access

void initOLED() {
    oledDisplay.begin();
}

void displayText(const String& line1, const String& line2, 
                 const String& line3, const String& line4) {
    oledDisplay.showText(line1, line2, line3, line4);
}

void displayRobotStatus(const String& mode, int speed, const String& state) {
    oledDisplay.showRobotStatus(mode, speed, state);
}

void displayTOF(uint16_t left, uint16_t front, uint16_t right) {
    oledDisplay.showTOFReadings(left, front, right);
}

void displayMessage(const String& title, const String& message) {
    oledDisplay.showMessage(title, message);
}
