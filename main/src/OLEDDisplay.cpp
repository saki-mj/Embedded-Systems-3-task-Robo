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

// Show color sensor readings
void OLEDDisplay::showColor(const String& position, const String& colorName, uint16_t lux) {
    if (!initialized) return;
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.print("=== ");
    display.print(position);
    display.println(" ===");
    display.print("Color: ");
    display.println(colorName);
    display.print("Lux: ");
    display.println(lux);
    
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

// Show QR code for GitHub repository
void OLEDDisplay::showQRCode(const String& url) {
    if (!initialized) return;
    
    // Pre-generated QR code for: https://github.com/saki-mj/Embedded-Systems-3-task-Robo.git
    // Generated using online QR code generator, 29x29 modules (Version 3)
    // Each byte represents 8 pixels in a row (MSB first)
    const uint8_t qrCode[] PROGMEM = {
        0b11111110, 0b01101001, 0b10111111, 0b10000000,
        0b10000010, 0b01001111, 0b10100001, 0b10000000,
        0b10111010, 0b10110110, 0b10101101, 0b10000000,
        0b10111010, 0b11100101, 0b00101101, 0b10000000,
        0b10111010, 0b00011001, 0b10101101, 0b10000000,
        0b10000010, 0b11010111, 0b00100001, 0b10000000,
        0b11111110, 0b10101010, 0b10111111, 0b10000000,
        0b00000000, 0b10100001, 0b00000000, 0b00000000,
        0b11101011, 0b01111100, 0b01011010, 0b10000000,
        0b01101101, 0b11001010, 0b11110110, 0b00000000,
        0b10010111, 0b00110001, 0b10101001, 0b10000000,
        0b00110001, 0b10011101, 0b01011101, 0b10000000,
        0b11000010, 0b01010110, 0b00100010, 0b10000000,
        0b01101100, 0b10111010, 0b11011110, 0b00000000,
        0b10011111, 0b01100101, 0b10010001, 0b10000000,
        0b00011101, 0b11010011, 0b01101110, 0b00000000,
        0b11100010, 0b00001000, 0b10111001, 0b10000000,
        0b01010001, 0b10100110, 0b01000101, 0b10000000,
        0b10101110, 0b01011001, 0b00010010, 0b10000000,
        0b00111100, 0b10110111, 0b11001111, 0b00000000,
        0b11010011, 0b01101100, 0b10110000, 0b10000000,
        0b00000000, 0b11011010, 0b01101001, 0b10000000,
        0b11111110, 0b00100001, 0b10010101, 0b10000000,
        0b10000010, 0b10011110, 0b01111110, 0b00000000,
        0b10111010, 0b01100101, 0b00101001, 0b10000000,
        0b10111010, 0b11011011, 0b11010111, 0b00000000,
        0b10111010, 0b00000000, 0b10111000, 0b10000000,
        0b10000010, 0b10101101, 0b01100101, 0b10000000,
        0b11111110, 0b01010110, 0b00011010, 0b10000000,
    };
    
    display.clearDisplay();
    
    // Display title
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(40, 0);
    display.println("SCAN ME");
    
    // QR code dimensions
    const int qrSize = 29;  // 29x29 modules
    const int scale = 2;    // 2x2 pixels per module
    const int offsetX = (OLED_WIDTH - (qrSize * scale)) / 2;
    const int offsetY = 10;
    
    // Draw QR code
    for (int y = 0; y < qrSize; y++) {
        for (int x = 0; x < qrSize; x++) {
            // Calculate byte and bit position
            int byteIndex = y * 4 + (x / 8);  // 4 bytes per row (32 bits, using 29)
            int bitIndex = 7 - (x % 8);       // MSB first
            
            // Check if this module is black
            if (pgm_read_byte(&qrCode[byteIndex]) & (1 << bitIndex)) {
                // Draw scaled pixel
                display.fillRect(offsetX + x * scale, offsetY + y * scale, scale, scale, SSD1306_WHITE);
            }
        }
    }
    
    display.display();
}

// Initialization function
void initOLED() {
    oledDisplay.begin();
}
