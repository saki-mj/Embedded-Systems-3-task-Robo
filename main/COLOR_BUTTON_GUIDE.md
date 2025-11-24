# Color Sensors & Push Button - Integration Guide

## Overview
Added TCS34725 RGB color sensors and 5-button analog switch to the robot system.

## Hardware Configuration

### Color Sensors (TCS34725)
- **Type**: RGB color sensors with Clear channel
- **I2C Address**: 0x29 (both sensors)
- **Connection**: Via PCA9548A I2C Multiplexer
  - **Bottom Sensor**: Channel 4
  - **Top Sensor**: Channel 2
- **Settings**:
  - Integration Time: 600ms
  - Gain: 1x

### Push Button Switch
- **Type**: 5-button analog voltage divider
- **GPIO**: Pin 19
- **ADC**: 12-bit resolution (0-4095)
- **Buttons & Ranges**:
  - UP: 0-50
  - LEFT: 1100-1200
  - MIDDLE: 1750-1850
  - RIGHT: 2600-2750 ⚡ **Task Switcher**
  - DOWN: 3750-4020 🛑 **Emergency Stop**

### Button Functions (Active)
- **DOWN**: Emergency stop - Stops all tasks and motors immediately
- **RIGHT**: Cycle through tasks - Switch between Task 1 → 2 → 3 → 4 → 5 → 1
- **UP, LEFT, MIDDLE**: Available for custom use in tasks

## Library Features

### ColorSensors Library

#### Initialization
```cpp
initColorSensors();  // Initializes both sensors through I2C mux
```

#### Reading Sensors
```cpp
colorSensors.readAll();                    // Read both sensors
colorSensors.readBottomSensor();           // Read bottom only
colorSensors.readTopSensor();              // Read top only
```

#### Getting Color Data
```cpp
// Get raw RGB values
uint16_t r = colorSensors.getBottomRed();
uint16_t g = colorSensors.getBottomGreen();
uint16_t b = colorSensors.getBottomBlue();
uint16_t lux = colorSensors.getBottomLux();

// Get detected color
DetectedColor color = colorSensors.getBottomColor();
String colorName = colorSensors.getColorName(color);

// Available colors: RED, GREEN, BLUE, YELLOW, WHITE, BLACK, UNKNOWN
```

#### Complete Example
```cpp
colorSensors.readBottomSensor();
DetectedColor bottomColor = colorSensors.getBottomColor();

if (bottomColor == COLOR_RED) {
  // Handle red color
  Serial.println("Red detected!");
}
```

### PushButton Library

#### Initialization
```cpp
initPushButton();  // Initializes ADC and button detection
```

#### Reading Button State
```cpp
pushButton.update();  // Call in loop() to update state

// Check current button
Button current = pushButton.getCurrentButton();

// Check if specific button is pressed
if (pushButton.isPressed(BTN_UP)) {
  // Handle UP button
}

// Check if button was just pressed (edge detection)
if (pushButton.wasPressed(BTN_MIDDLE)) {
  // Handle MIDDLE button press event
}
```

#### Available Buttons
- `BTN_NONE` - No button pressed
- `BTN_UP`
- `BTN_LEFT`
- `BTN_MIDDLE`
- `BTN_RIGHT`
- `BTN_DOWN`

#### Complete Example
```cpp
void loop() {
  pushButton.update();
  
  if (pushButton.wasPressed(BTN_UP)) {
    Serial.println("UP button pressed!");
    // Start a task or perform action
  }
  
  if (pushButton.isPressed(BTN_MIDDLE)) {
    // Continuously handle MIDDLE button being held
  }
}
```

## Serial Commands

### Color Sensor Commands
- `COLORREAD` - Read and display both sensors
- `COLORBOTTOM` - Read and display bottom sensor (Channel 4)
- `COLORTOP` - Read and display top sensor (Channel 2)

### Push Button Commands
- `BUTTONREAD` - Display current button state

## OLED Display Integration

New display function for color sensors:
```cpp
oledDisplay.showColor("Bottom", "RED", 150);  // position, color, lux
```

## Usage in Tasks

### Example: Color Detection in Task
```cpp
void Task1Plantation::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case T1_SEARCHING:
      // Read color sensor
      colorSensors.readBottomSensor();
      DetectedColor color = colorSensors.getBottomColor();
      
      if (color == COLOR_GREEN) {
        // Found plant area
        setSubState(T1_PLANTING);
      }
      break;
      
    case T1_PLANTING:
      // Plant logic here
      break;
  }
}
```

### Example: Button Control in Task
```cpp
void Task2WallFollow::execute() {
  if (!taskActive) return;
  
  // Check for user input (UP, LEFT, MIDDLE available for tasks)
  if (pushButton.wasPressed(BTN_UP)) {
    // Custom action - e.g., increase speed
    Serial.println("Speed increased!");
  }
  
  if (pushButton.wasPressed(BTN_MIDDLE)) {
    // Custom action - e.g., change mode
    Serial.println("Mode changed!");
  }
  
  // Normal task execution
  switch(currentSubState) {
    // ... task logic
  }
}
```

**Note**: DOWN and RIGHT buttons are handled globally in main.ino:
- DOWN → Emergency stop (stops all tasks immediately)
- RIGHT → Cycle through tasks (Task 1 → 2 → 3 → 4 → 5 → 1)

## Color Classification Algorithm

The library classifies colors based on RGB values:

1. **BLACK**: Low brightness (clear channel < 100)
2. **WHITE**: High brightness + balanced RGB
3. **RED**: Red dominant
4. **YELLOW**: Red + Green dominant
5. **GREEN**: Green dominant
6. **BLUE**: Blue dominant
7. **UNKNOWN**: Cannot determine

You can customize the classification by modifying `classifyColor()` in `ColorSensors.cpp`.

## Calibration Tips

### Color Sensors
- Test with different lighting conditions
- Adjust integration time if needed (in ColorSensors.cpp constructor)
- Adjust gain for very bright/dark environments
- Place sensors at consistent distance from objects

### Push Button
- Test actual analog values: use `BUTTONREAD` command
- If values differ from ranges, update constants in `PushButton.h`:
  ```cpp
  #define BUTTON_UP_MIN 0
  #define BUTTON_UP_MAX 50
  // etc.
  ```

## I2C Multiplexer Channel Map

| Channel | Device |
|---------|--------|
| 0 | Left TOF Sensor |
| 1 | Front TOF Sensor |
| 2 | **Top Color Sensor** |
| 3 | Right TOF Sensor |
| 4 | **Bottom Color Sensor** |
| 5-7 | Available |

## Notes

- Color sensors need ~600ms integration time - readings are not instant
- Push button uses debouncing (50ms) to prevent false triggers
- Both libraries are fully integrated with OLED and serial commands
- Color detection works best with consistent lighting
- Button analog values may vary slightly with temperature/voltage

---

**Ready to use!** Test with `COLORREAD` and `BUTTONREAD` commands.
