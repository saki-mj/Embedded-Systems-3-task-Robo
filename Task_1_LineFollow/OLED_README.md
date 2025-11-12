# OLED Display Library Integration Guide

## ✅ Files Added

### Library Files (in `src/` folder):
1. **OLEDDisplay.h** - Header file with class definition
2. **OLEDDisplay.cpp** - Implementation file

### Test Files:
1. **OLED_Test.ino** - Standalone test program

## 📋 Features

### Display Functions:
- **Simple Text Display** - Show up to 4 lines of text
- **Robot Status** - Display mode, speed, and state
- **TOF Sensors** - Show left, front, and right distances
- **IR Sensors** - Bar graph display of 8 sensors
- **Position Display** - For maze navigation
- **Custom Messages** - Title and message format

## 🔌 Hardware Connection

### SSD1306 OLED (128x64) I2C Connection:
- **VCC** → 3.3V
- **GND** → GND
- **SDA** → ESP32-S3 Pin 8 (shared with TOF sensors)
- **SCL** → ESP32-S3 Pin 9 (shared with TOF sensors)
- **I2C Address**: 0x3C (default)

**Note**: The OLED uses the same I2C bus as the TOF sensors and PCA9548A multiplexer.

## 💻 Usage Examples

### Basic Initialization
```cpp
#include "src/OLEDDisplay.h"

void setup() {
    Serial.begin(115200);
    
    // Initialize OLED
    initOLED();  // or oledDisplay.begin();
    
    // Show welcome message
    displayMessage("Robot Ready!", "System OK");
}
```

### Simple Text Display
```cpp
// Method 1: Using global function
displayText("Line 1", "Line 2", "Line 3", "Line 4");

// Method 2: Using object directly
oledDisplay.showText("Hello", "World", "from", "ESP32");
```

### Robot Status Display
```cpp
// Show current robot state
displayRobotStatus("Line Follow", 512, "Active");

// Example with variables
String mode = "Manual";
int speed = getCurrentSpeed();
String state = "Running";
oledDisplay.showRobotStatus(mode, speed, state);
```

### TOF Sensor Display
```cpp
// Read and display TOF sensors
readTOFSensors();
uint16_t left = tofSensors.getLeftDistance();
uint16_t front = tofSensors.getFrontDistance();
uint16_t right = tofSensors.getRightDistance();

displayTOF(left, front, right);
// or
oledDisplay.showTOFReadings(left, front, right);
```

### IR Sensor Display
```cpp
// Display IR sensor values as bar graph
extern int irValues[8];
readIRSensors();
oledDisplay.showIRReadings(irValues, 8);
```

### Custom Message
```cpp
displayMessage("Warning!", "Obstacle detected");

// or
oledDisplay.showMessage("Status", "Line following active");
```

### Clear Display
```cpp
oledDisplay.clear();
```

## 🎮 Integration with Main Project

### Add to Task_1_LineFollow.ino:

```cpp
#include "src/Motors.h"
#include "src/IRReading.h"
#include "src/LineFollow.h"
#include "src/TOFSensors.h"
#include "src/OLEDDisplay.h"  // Add this line

void setup() {
    Serial.begin(115200);
    
    // Initialize OLED first
    initOLED();
    displayMessage("Init", "Starting...");
    
    // Initialize other systems
    initMotors();
    initIRSensors();
    initTOFSensors();
    initLineFollow();
    
    // Show ready status
    displayRobotStatus("Ready", getCurrentSpeed(), "Idle");
}

void loop() {
    // Update display periodically
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 500) {
        lastDisplayUpdate = millis();
        
        if (isLineFollowActive()) {
            displayRobotStatus("Line Follow", getCurrentSpeed(), "Active");
        } else {
            // Show TOF readings
            readTOFSensors();
            displayTOF(
                tofSensors.getLeftDistance(),
                tofSensors.getFrontDistance(),
                tofSensors.getRightDistance()
            );
        }
    }
    
    // Rest of your code...
}
```

### Add Commands to Motors.cpp:

```cpp
// In processCommand() function, add:
else if (command == "OLEDTEST") {
    displayText("Test Line 1", "Test Line 2", "Test Line 3", "Test Line 4");
}
else if (command == "OLEDSTATUS") {
    displayRobotStatus("Manual", getCurrentSpeed(), "Testing");
}
else if (command == "OLEDTOF") {
    readTOFSensors();
    displayTOF(
        tofSensors.getLeftDistance(),
        tofSensors.getFrontDistance(),
        tofSensors.getRightDistance()
    );
}
else if (command == "OLEDCLEAR") {
    oledDisplay.clear();
}

// In printCommands() function, add:
Serial.println("OLED Display:");
Serial.println("  OLEDTEST - Show test text");
Serial.println("  OLEDSTATUS - Show robot status");
Serial.println("  OLEDTOF - Show TOF readings");
Serial.println("  OLEDCLEAR - Clear display");
```

## 🧪 Testing with OLED_Test.ino

The `OLED_Test.ino` file is a standalone test program.

### How to Test:
1. Open `OLED_Test.ino` in Arduino IDE
2. Upload to ESP32-S3
3. Open Serial Monitor (115200 baud)
4. Type commands to test different display modes

### Available Test Commands:
- `TEXT` - Show simple text demo
- `STATUS` - Show robot status demo
- `TOF` - Show TOF sensor demo (simulated data)
- `IR` - Show IR sensor demo (simulated data)
- `POS` - Show position demo
- `MSG` - Show custom message demo
- `CLEAR` - Clear display
- `CYCLE` - Auto-cycle through all demos (3 second intervals)
- `STOP` - Stop auto-cycle
- `HELP` - Show all commands

## 📚 API Reference

### Class: OLEDDisplay

#### Constructor
```cpp
OLEDDisplay(int width = 128, int height = 64, int address = 0x3C)
```

#### Methods
```cpp
bool begin()                          // Initialize display
bool isInitialized()                  // Check if initialized
void clear()                          // Clear display
void showText(...)                    // Show up to 4 lines
void showRobotStatus(...)             // Show robot status
void showTOFReadings(...)             // Show TOF distances
void showIRReadings(...)              // Show IR sensor graph
void showPosition(...)                // Show position info
void showMessage(...)                 // Show title & message
Adafruit_SSD1306& getDisplay()       // Get raw display object
```

### Global Functions
```cpp
void initOLED()
void displayText(String l1, String l2="", String l3="", String l4="")
void displayRobotStatus(String mode, int speed, String state)
void displayTOF(uint16_t left, uint16_t front, uint16_t right)
void displayMessage(String title, String message)
```

### Global Object
```cpp
extern OLEDDisplay oledDisplay;  // Global instance
```

## 🔧 Dependencies

Required Arduino Libraries:
- **Wire** (built-in)
- **Adafruit GFX Library** (install via Library Manager)
- **Adafruit SSD1306** (install via Library Manager)

## ⚠️ Important Notes

1. **I2C Bus Sharing**: OLED shares I2C bus with TOF sensors and PCA9548A
2. **Address Conflict**: Ensure OLED is at 0x3C (different from PCA9548A at 0x70 and VL53L0X at 0x29)
3. **Display Updates**: Don't update too frequently (max ~20 Hz) to avoid I2C bus congestion
4. **Memory**: OLED library uses ~1KB RAM for display buffer
5. **Text Size**: Default font is 5x7 pixels, allowing 21 chars x 8 lines at size 1

## 🎯 Example Project Structure

```
Task_1_LineFollow/
├── Task_1_LineFollow.ino     (Main program with OLED)
├── OLED_Test.ino             (Standalone OLED test)
└── src/
    ├── Motors.h/cpp
    ├── IRReading.h/cpp
    ├── LineFollow.h/cpp
    ├── I2CMux.h/cpp
    ├── TOFSensors.h/cpp
    └── OLEDDisplay.h/cpp     ← NEW
```

## 🚀 Quick Start

1. Install required libraries in Arduino IDE
2. Upload `OLED_Test.ino` to test OLED functionality
3. Integrate into main project using examples above
4. Use `displayRobotStatus()` for real-time feedback
5. Use `displayTOF()` during autonomous navigation

Enjoy your new OLED display! 🎉
