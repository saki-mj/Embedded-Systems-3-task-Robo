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
    if (!display.begin(SSD1306_SWITCHCAPVCC, address)) {
        Serial.println("ERROR: OLED initialization failed");
        initialized = false;
        return false;
    }
    
    Serial.println("OLED initialized");
    display.clearDisplay();
    display.display();
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
void OLEDDisplay::show(const String& l1, const String& l2, const String& l3, const String& l4) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println(l1);
    if (l2 != "") display.println(l2);
    if (l3 != "") display.println(l3);
    if (l4 != "") display.println(l4);
    
    display.display();
}

// Show robot status
void OLEDDisplay::showStatus(const String& mode, int speed) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== ROBOT ===");
    display.print("Mode: ");
    display.println(mode);
    display.print("Speed: ");
    display.println(speed);
    
    display.display();
}

// Show TOF sensor readings
void OLEDDisplay::showTOF(uint16_t left, uint16_t front, uint16_t right) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== TOF (mm) ===");
    display.print("L: ");
    display.println(left);
    display.print("F: ");
    display.println(front);
    display.print("R: ");
    display.println(right);
    
    display.display();
}

// Show IR sensor status
void OLEDDisplay::showIRStatus(bool calibrated, int activeCount) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== IR SENSORS ===");
    display.print("Cal: ");
    display.println(calibrated ? "Yes" : "No");
    display.print("Active: ");
    display.println(activeCount);
    
    display.display();
}

// Show line following state
void OLEDDisplay::showLineFollowing(bool active, float error) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.println("=== LINE FOLLOW ===");
    display.print("Status: ");
    display.println(active ? "ACTIVE" : "STOPPED");
    display.print("Error: ");
    display.println(error, 1);
    
    display.display();
}

// Initialization function
void initOLED() {
    oledDisplay.begin();
}
