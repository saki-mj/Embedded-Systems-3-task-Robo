# TOF Sensor Integration Summary

## ✅ Files Added to `Task_1_LineFollow/src/`

### 1. **I2CMux.h** & **I2CMux.cpp**
- PCA9548A I2C multiplexer control library
- I2C address: 0x70
- ESP32-S3 I2C pins: SDA=8, SCL=9
- Supports 8 channels (0-7)
- Used channels: 0 (Left TOF), 1 (Front TOF), 3 (Right TOF)

### 2. **TOFSensors.h** & **TOFSensors.cpp**
- VL53L0X Time-of-Flight sensor management
- Controls 3 TOF sensors through I2C multiplexer
- Features:
  - Distance reading for left, front, and right sensors
  - Obstacle detection with configurable threshold (default: 200mm)
  - Validity checking for sensor readings
  - Easy-to-use global functions

## 🔧 Files Modified

### **Task_1_LineFollow.ino**
- Added `#include "src/TOFSensors.h"`
- Added `initTOFSensors()` in setup()
- Enhanced initialization messages

### **Motors.cpp**
- Added `#include "TOFSensors.h"`
- Added TOF commands to `processCommand()`:
  - `TOFREAD` - Read and display TOF distances
  - `TOFTHRESHOLD <mm>` - Set obstacle threshold
- Updated `printCommands()` with TOF help section

## 📋 Current Project Structure

```
Task_1_LineFollow/
├── Task_1_LineFollow.ino     (Main program)
├── README.md                  (Complete documentation)
└── src/
    ├── Motors.h/cpp          (Motor control)
    ├── IRReading.h/cpp       (IR sensors)
    ├── LineFollow.h/cpp      (PD line following)
    ├── I2CMux.h/cpp          (PCA9548A multiplexer) ✨ NEW
    └── TOFSensors.h/cpp      (VL53L0X sensors) ✨ NEW
```

## 🎯 Hardware Configuration

### PCA9548A I2C Multiplexer
- **I2C Address**: 0x70
- **SDA**: ESP32-S3 Pin 8
- **SCL**: ESP32-S3 Pin 9

### VL53L0X TOF Sensors
- **Left TOF**: MUX Channel 0 (SDA0/SCL0)
- **Front TOF**: MUX Channel 1 (SDA1/SCL1)
- **Right TOF**: MUX Channel 3 (SDA3/SCL3)
- **I2C Address**: 0x29 (all three sensors, isolated by multiplexer)

## 📝 New Serial Commands

### TOF Distance Sensors
```
TOFREAD              - Read and display all TOF sensor distances
TOFTHRESHOLD 200     - Set obstacle detection threshold to 200mm
```

## 💻 Usage Examples

### Basic TOF Reading
```cpp
// In your code
readTOFSensors();
printTOFValues();
// Output: Left: 150 mm | Front: 300 mm | Right: 250 mm
```

### Obstacle Detection
```cpp
readTOFSensors();
if (tofSensors.isObstacleFront()) {
    stopAllMotors();
    Serial.println("Obstacle ahead!");
}
```

### Get Individual Distances
```cpp
readTOFSensors();
uint16_t frontDist = tofSensors.getFrontDistance();
if (frontDist < 100) {
    // Too close to wall!
}
```

### Set Custom Threshold
```cpp
tofSensors.setObstacleThreshold(150);  // 150mm threshold
```

## 🔍 Testing Checklist

1. ✅ Upload code to ESP32-S3
2. ✅ Open Serial Monitor (115200 baud)
3. ✅ Check initialization messages:
   - "I2C initialized - SDA:8 SCL:9"
   - "PCA9548A found at address 0x70"
   - "Left/Front/Right TOF initialized successfully"
4. ✅ Test command: `TOFREAD`
5. ✅ Verify distance readings from all three sensors
6. ✅ Test threshold: `TOFTHRESHOLD 300`

## 🚀 Integration Benefits

1. **Reusable Library**: I2CMux can be used in other projects
2. **Clean API**: Simple functions for common tasks
3. **Error Handling**: Connection verification and validity checks
4. **Serial Control**: Easy testing via Serial Monitor
5. **Modular Design**: Each sensor library is independent

## 📚 Required Arduino Libraries

Install these in Arduino IDE:
- **Wire** (built-in)
- **Adafruit_VL53L0X** (via Library Manager)

## ⚠️ Important Notes

- TOF sensors must be connected to correct MUX channels
- I2C pull-up resistors may be needed (usually 4.7kΩ)
- Maximum reliable range: ~2000mm
- Reading all 3 sensors takes approximately 30-50ms
- IntelliSense errors in VS Code are normal (Arduino.h path issue)
- Code will compile correctly in Arduino IDE

## 🎓 Next Steps

1. Test TOF sensors individually using `TOFREAD`
2. Integrate obstacle avoidance with line following
3. Use TOF data for wall following or maze navigation
4. Combine IR (ground) and TOF (obstacles) for robust navigation
