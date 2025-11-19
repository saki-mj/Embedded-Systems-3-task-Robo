# Embedded Robot System - Complete Documentation

**Project**: Multi-Task Autonomous Robot Control System  
**Platform**: ESP32-S3  
**Last Updated**: November 19, 2025  
**Status**: Ready for deployment

---

## Table of Contents

1. [System Overview](#system-overview)
2. [Hardware Configuration](#hardware-configuration)
3. [Software Architecture](#software-architecture)
4. [Serial Commands](#serial-commands)
5. [Physical Button Controls](#physical-button-controls)
6. [Task Descriptions](#task-descriptions)
7. [Configuration & Tuning](#configuration--tuning)
8. [Quick Start Guide](#quick-start-guide)
9. [Development Guidelines](#development-guidelines)
10. [Troubleshooting](#troubleshooting)

---

## System Overview

This is a **competition-ready autonomous robot** with a sophisticated state machine architecture managing 5 independent tasks. The robot integrates multiple sensors for navigation, object detection, and color recognition.

### Key Features

✅ **Modular Architecture** - Each task is independent and testable  
✅ **State Machine** - Clean state management with sub-state tracking  
✅ **Multi-Sensor Integration** - TOF, Color, IR, OLED, Push buttons  
✅ **Flexible Control** - Manual testing or automatic competition runs  
✅ **Real-time Feedback** - OLED shows current task and sub-state  
✅ **Safety Features** - Emergency stop with resume capability  
✅ **Comprehensive Commands** - Complete serial interface  
✅ **Physical Controls** - 5-button interface for on-the-fly control

---

## Hardware Configuration

### ESP32-S3 Microcontroller
- **I2C Bus**: SDA=8, SCL=9
- **Serial**: 115200 baud
- **ADC**: 12-bit resolution (0-4095)

### Motor Driver (TB6612)
- **Motor A (Left)**: PWMA=4, AIN1=6, AIN2=5
- **Motor B (Right)**: PWMB=17, BIN1=15, BIN2=16
- **Standby**: STBY=7
- **Speed Levels**: 12 levels (40-1023 PWM)

### IR Sensor Array (CD74HC4067 MUX)
- **Signal Pin**: SIG=1 (analog input)
- **Select Pins**: S0=40, S1=41, S2=42, S3=2
- **Sensors**: 16 IR sensors on MUX channels 0-15
- **Function**: Line detection, barcode reading

### I2C Devices via PCA9548A Multiplexer

**I2C Multiplexer (PCA9548A)**
- **Address**: 0x70
- **Frequency**: 100kHz standard mode

**Channel Map:**
```
Channel 0: Left TOF Sensor (VL53L0X)
Channel 1: Back Color Sensor (TCS34725)
Channel 2: Top Color Sensor (TCS34725) - Ball Detection
Channel 3: Right TOF Sensor (VL53L0X)
Channel 4: Front TOF Sensor (VL53L0X)
Channel 5: Bottom Color Sensor (TCS34725)
Channels 6-7: Available for expansion
```

**OLED Display (SSD1306)**
- **Address**: 0x3C
- **Size**: 128x64 pixels
- **Function**: Real-time status display

**TOF Sensors (VL53L0X)** - 3x sensors
- **Address**: 0x29 (each, isolated by mux)
- **Range**: Up to 2000mm
- **Function**: Distance measurement, obstacle detection

**Color Sensors (TCS34725)** - 3x sensors
- **Address**: 0x29 (each, isolated by mux)
- **Integration Time**: 600ms
- **Gain**: 1x
- **Function**: RGB color detection, ball sorting

### Push Button Switch
- **GPIO**: Pin 19 (analog input)
- **Type**: 5-button voltage divider
- **Debounce**: 50ms

**Button Analog Values:**
```
UP:     0-50      (typical: ~25)
LEFT:   1100-1200 (typical: ~1150)
MIDDLE: 1750-1850 (typical: ~1800)
RIGHT:  2600-2750 (typical: ~2675)
DOWN:   3750-4020 (typical: ~3885)
```

### Servo Motors (Ball Collection)
- **Arm Servo**: GPIO 35
- **Gripper Servo**: GPIO 36
- **Sorting Servo**: GPIO 37

---

## Software Architecture

### State Machine (8 States)

```
┌─────────────────────────────────────────────────────┐
│                  State Machine                       │
├─────────────────────────────────────────────────────┤
│                                                      │
│  STANDBY ──► IDLE ──► TASK1_PLANTATION              │
│                │            │                        │
│                │            ▼                        │
│                │       TASK2_WALL_FOLLOW             │
│                │            │                        │
│                │            ▼                        │
│                │       TASK3_RAMP                    │
│                │            │                        │
│                │            ▼                        │
│                │       TASK4_BARCODE                 │
│                │            │                        │
│                │            ▼                        │
│                │       TASK5_UNLOADING               │
│                │            │                        │
│                │            └──► IDLE (completed)    │
│                │                                     │
│                └──► EMERGENCY_STOP ◄────────────────┤
│                                                      │
└─────────────────────────────────────────────────────┘
```

**State Descriptions:**

1. **STANDBY** - Initialization and calibration phase
2. **IDLE** - Ready state, waiting for task commands
3. **TASK1_PLANTATION** - Navigate plantation field with line following
4. **TASK2_WALL_FOLLOW** - Wall following navigation task
5. **TASK3_RAMP** - Ramp navigation challenge
6. **TASK4_BARCODE** - Read 4-bit barcode using IR sensors
7. **TASK5_UNLOADING** - Ball sorting and unloading
8. **EMERGENCY_STOP** - Safety stop state (can resume)

### Operation Modes

**MANUAL Mode (Default)**
- Tasks must be started via serial commands
- Task completion does NOT automatically start next task
- User has full control over task execution
- Best for: Testing individual tasks

**AUTOMATIC Mode**
- Tasks run sequentially: TASK1 → TASK2 → TASK3 → TASK4 → TASK5 → IDLE
- Task completion automatically triggers next task
- Best for: Full competition runs

### Library Modules

**Core Libraries:**
- `Motors.h/cpp` - TB6612 motor driver control
- `IRReading.h/cpp` - IR sensor array with calibration
- `LineFollow.h/cpp` - PD controller for line following
- `WallFollow.h/cpp` - PD controller for wall following
- `TOFSensors.h/cpp` - VL53L0X distance sensor manager
- `ColorSensors.h/cpp` - TCS34725 RGB sensor manager
- `PushButton.h/cpp` - 5-button analog switch handler
- `I2CMux.h/cpp` - PCA9548A multiplexer controller
- `OLEDDisplay.h/cpp` - SSD1306 display wrapper
- `SerialCommands.h/cpp` - Command processing system
- `StateMachine.h/cpp` - State machine controller

**Task Modules:**
- `Task1_Plantation.h/cpp` - Plantation navigation
- `Task2_WallFollow.h/cpp` - Wall following task
- `Task3_Ramp.h/cpp` - Ramp navigation
- `Task4_Barcode.h/cpp` - Barcode reading
- `Task5_Unloading.h/cpp` - Ball unloading
- `BallCollector.h/cpp` - Ball collection mechanism

---

## Serial Commands

### State Machine Control
```
START       - Enter IDLE state (ready to run tasks)
AUTO        - Enable automatic mode (sequential task execution)
MANUAL      - Enable manual mode (command-driven, default)
STATUS      - Show current state and mode
EMERGENCY   - Emergency stop all tasks and motors
RESUME      - Resume from emergency stop to IDLE
```

### Task Control
```
TASK1       - Start Task 1: Plantation
TASK2       - Start Task 2: Wall Following
TASK3       - Start Task 3: Ramp
TASK4       - Start Task 4: Barcode Reading
TASK5       - Start Task 5: Unloading Balls
```

### Speed Control
```
SPEED1 to SPEED12  - Set speed level (40-1023 PWM)
                     Level 1: 40 PWM (slowest)
                     Level 6: 200 PWM (medium)
                     Level 12: 1023 PWM (max)
```

### Motor Control
```
LMF         - Left motor forward
LMB         - Left motor backward
RMF         - Right motor forward
RMB         - Right motor backward
RF          - Robot forward (both motors)
RB          - Robot backward (both motors)
RTL         - Robot turn left
RTR         - Robot turn right
STOP        - Stop all motors and tasks
```

### IR Sensors
```
IRCALIBRATE - Calibrate IR sensors (10 second process)
              Move robot over black and white surfaces
IRREAD      - Toggle continuous raw IR reading mode
IRREADB     - Toggle continuous binary IR reading mode
```

### Line Following
```
LINEFOLLOW  - Toggle autonomous line following mode
INVERTLINE  - Toggle line color (white on black / black on white)
SETP <val>  - Set proportional gain (Kp) for PD controller
              Example: SETP 15.0
SETD <val>  - Set derivative gain (Kd) for PD controller
              Example: SETD 5.0
```

### Wall Following
```
WFKP <val>    - Set wall follow proportional gain (default: 1.0)
WFKD <val>    - Set wall follow derivative gain (default: 0.5)
WFDIST <mm>   - Set wall follow target distance (default: 150mm)
```

### TOF Distance Sensors
```
TOFREAD         - Toggle continuous TOF reading mode
TOFTHRESHOLD <mm> - Set obstacle detection threshold
                    Example: TOFTHRESHOLD 200
```

### Color Sensors
```
COLORREAD   - Read all three color sensors (bottom, top, back)
COLORBOTTOM - Read bottom color sensor (Channel 5)
COLORTOP    - Read top color sensor (Channel 2)
COLORBACK   - Read back color sensor (Channel 1)
```

### Task 4 Configuration
```
T4WALLDIST <mm>   - Wall detection distance (default: 200mm)
T4WFOLLOW <mm>    - Wall following target distance (default: 150mm)
T4BARSPEED <spd>  - Barcode reading speed (default: 50)
T4APPSPEED <spd>  - Approach speed (default: 70)
T4TURNSPEED <spd> - Turn speed (default: 60)
T4TURNRDUR <ms>   - Right turn duration (default: 1000ms)
T4TURNLDUR <ms>   - Left turn duration (default: 1000ms)
T4STRAIGHTDUR <ms>- Straight movement duration (default: 1000ms)
T4REVDUR <ms>     - Reverse movement duration (default: 500ms)
T4IRTHRESH <val>  - IR white threshold (default: 2000)
                    IR > threshold = white (1), ≤ = black (0)
```

### Push Button & Display
```
BUTTONREAD  - Display current button state
OLEDCLEAR   - Clear OLED display
```

### Help
```
HELP or ?   - Display all available commands with current values
```

---

## Physical Button Controls

```
┌─────────────────────────────────┐
│     5-Button Control Panel      │
│         (GPIO 19)                │
├─────────────────────────────────┤
│                                  │
│          [  UP  ]                │
│            ▲                     │
│            │                     │
│   [ LEFT ] ─┼─ [ RIGHT ]        │
│            │                     │
│            ▼                     │
│        [ MIDDLE ]                │
│                                  │
│          [ DOWN ]                │
│                                  │
└─────────────────────────────────┘
```

### Button Functions

**🛑 DOWN Button - EMERGENCY STOP**
- **Action**: Immediate emergency stop (highest priority)
- **What it does**:
  - Stops all motors immediately
  - Stops all active tasks
  - Stops line/wall following
  - Sets state to EMERGENCY_STOP
  - Shows "EMERGENCY STOP" on OLED
- **Use when**: Robot needs to stop immediately for safety
- **Recovery**: Type `RESUME` or start new task with RIGHT button

**⚡ RIGHT Button - TASK SWITCHER**
- **Action**: Cycle through tasks
- **What it does**:
  - Safely stops current task
  - Stops motors and active controllers
  - Starts next task in sequence
  - Updates OLED with new task
- **Task cycle**: 1 → 2 → 3 → 4 → 5 → 1
- **Note**: Cannot switch during EMERGENCY_STOP or STANDBY

**📝 UP, LEFT, MIDDLE Buttons**
- **Action**: Available for custom task logic
- **Use**: Can be programmed within tasks for task-specific functions
- **Examples**:
  - UP: Increase speed/sensitivity
  - LEFT: Change mode within task
  - MIDDLE: Pause/resume within task

### Using Buttons in Task Code

```cpp
void Task1Plantation::execute() {
  if (!taskActive) return;
  
  // Custom button handling
  if (pushButton.wasPressed(BTN_UP)) {
    plantingSpeed += 10;
    Serial.println("Speed increased!");
  }
  
  if (pushButton.wasPressed(BTN_MIDDLE)) {
    precisionMode = !precisionMode;
    Serial.println("Mode toggled!");
  }
  
  // Task logic continues...
}
```

---

## Task Descriptions

### Task 1: Plantation
**Purpose**: Navigate plantation field using line following

**Sub-states:**
```
INIT → SEARCHING → FOLLOWING → TURNING → 
LINE_FOLLOWING → MOVE_TO_NEXT_LINE → 
EXIT_FORWARD → COMPLETED
```

**Key Features:**
- Follows 4 vertical lines with 3 intersections each
- Reads bottom color sensor at each intersection
- Executes snake pattern navigation
- Uses IR sensors for line detection

**Tunable Parameters:**
```
T1_TURN_90_TIME_MS      - 90° turn duration (default: 2000ms)
T1_TURN_180_TIME_MS     - 180° turn duration (default: 3500ms)
T1_BACKUP_TIME_MS       - Backup time (default: 1500ms)
T1_EXIT_FORWARD_TIME_MS - Final exit time (default: 3000ms)
T1_SPEED_LEVEL          - Speed level 1-12 (default: 6)
T1_INTERSECTION_WHITE_MIN - White sensors for intersection (default: 7)
```

### Task 2: Wall Following
**Purpose**: Navigate using wall following

**Sub-states:**
```
INIT → FIND_WALL → ALIGN → FOLLOWING → 
CORNER_DETECTED → COMPLETED
```

**Key Features:**
- PD control for smooth wall following
- Left TOF sensor feedback
- Corner detection capability

### Task 3: Ramp Navigation
**Purpose**: Navigate ramp obstacle

**Sub-states:**
```
INIT → APPROACH → CLIMBING → DESCENDING → COMPLETED
```

**Key Features:**
- Ramp detection using TOF sensors
- Speed adjustment for climbing
- Descent detection and control

### Task 4: Barcode Reading
**Purpose**: Read 4-bit barcode using IR sensors

**Sub-states:**
```
INIT → SEARCHING_WALL1 → TURNING_RIGHT1 → 
SEARCHING_WALL2 → TURNING_LEFT2 → MOVING_REVERSE → 
ALIGNING → READING → PROCESSING → COMPLETED
```

**Key Features:**
- Wall following navigation to barcode
- Direct IR sensor reading (sensors 6, 7, 8, 9)
- Majority voting algorithm (20+ samples)
- Binary to decimal conversion

**Barcode Reading:**
```
Sensor 6 → Bit 3 (MSB)
Sensor 7 → Bit 2
Sensor 8 → Bit 1
Sensor 9 → Bit 0 (LSB)

IR value > T4IRTHRESH → White (1)
IR value ≤ T4IRTHRESH → Black (0)

Example: 1010 (binary) = 10 (decimal)
```

### Task 5: Unloading Balls
**Purpose**: Sort and unload balls by color

**Sub-states:**
```
INIT → NAVIGATE_TO_ZONE → ALIGN → 
UNLOADING → VERIFY → COMPLETED
```

**Key Features:**
- Top color sensor for ball detection
- Servo-based sorting mechanism
- Position verification

---

## Configuration & Tuning

### IR Sensor Calibration
```bash
# 1. Start calibration (10 seconds)
IRCALIBRATE

# 2. During calibration:
#    - Move robot over BLACK surfaces (low values)
#    - Move robot over WHITE surfaces (high values)

# 3. System calculates thresholds automatically
#    Threshold = (min + max) / 2 for each sensor

# 4. Test detection
IRREADB
# Should show 0 on black, 1 on white
```

### Line Following Tuning
```bash
# Default values: Kp=15.0, Kd=5.0

# Increase Kp for stronger correction (may oscillate)
SETP 20.0

# Increase Kd to reduce oscillation
SETD 8.0

# For black line on white surface
INVERTLINE

# Test
LINEFOLLOW
```

### Wall Following Tuning
```bash
# Default: Kp=1.0, Kd=0.5, Distance=150mm

# Increase Kp for stronger correction
WFKP 1.5

# Increase Kd for smoother response
WFKD 0.8

# Adjust target distance from wall
WFDIST 120

# Test with Task 2 or Task 4
TASK2
```

### Task 4 Barcode Calibration
```bash
# 1. Calibrate IR sensors first
IRCALIBRATE

# 2. Check sensors 6, 7, 8, 9 specifically
IRREAD
# Observe values:
#   White surface: ~3500
#   Black surface: ~1000

# 3. Set threshold (midpoint between white/black)
T4IRTHRESH 2200

# 4. Adjust speeds if needed
T4BARSPEED 40      # Slower = more accurate
T4APPSPEED 60      # Navigation speed
T4TURNSPEED 50     # Turn speed

# 5. Fine-tune turn durations
T4TURNRDUR 1100    # Right turn time
T4TURNLDUR 950     # Left turn time

# 6. Test
TASK4
```

### Speed Level Mapping
```
Level 1:  40 PWM   - Very slow, precise
Level 2:  45 PWM
Level 3:  50 PWM
Level 4:  75 PWM
Level 5:  100 PWM
Level 6:  200 PWM  - Medium (default)
Level 7:  337 PWM
Level 8:  474 PWM
Level 9:  611 PWM
Level 10: 748 PWM
Level 11: 885 PWM
Level 12: 1023 PWM - Maximum speed
```

---

## Quick Start Guide

### First Time Setup

1. **Upload Code**
```bash
# Open main.ino in Arduino IDE
# Select Board: ESP32S3 Dev Module
# Upload to robot
```

2. **Open Serial Monitor**
```bash
# Set baud rate: 115200
# Wait for initialization messages
```

3. **Calibrate IR Sensors**
```bash
IRCALIBRATE
# Move robot over black and white surfaces for 10 seconds
```

4. **Test Basic Functions**
```bash
# Test motors
SPEED3
RF
STOP

# Test TOF sensors
TOFREAD

# Test color sensors
COLORREAD

# Test buttons
BUTTONREAD
# Press each button and verify detection
```

### Manual Task Testing

```bash
# Enter IDLE state
START

# Set speed
SPEED6

# Test individual task
TASK1

# Emergency stop if needed
EMERGENCY

# Resume
RESUME

# Test next task
TASK2
```

### Automatic Competition Run

```bash
# 1. Set to automatic mode
AUTO

# 2. Enter IDLE state
START

# 3. Start first task
TASK1

# Robot automatically progresses:
# TASK1 → TASK2 → TASK3 → TASK4 → TASK5 → IDLE (complete!)
```

### Testing Line Following

```bash
# 1. Calibrate IR sensors
IRCALIBRATE

# 2. Check binary detection
IRREADB
# Should show: 0 on black, 1 on white

# 3. Set PD parameters
SETP 15.0
SETD 5.0

# 4. Start line following
LINEFOLLOW

# 5. Stop
LINEFOLLOW
```

---

## Development Guidelines

### Serial Output Rules

**✅ DO Print:**
- State transitions (once per transition)
- Time-based actions: "Action for X ms" (once at start)
- Condition-based actions: "Action until condition" (once at start)
- Significant events (sensor detections, errors)
- Manual command feedback
- Configuration changes

**❌ DO NOT Print:**
- Inside loops that run every cycle
- Low-level motor control calls
- Repeated sensor readings (unless debugging)
- Every PID control iteration
- Same message repeatedly

**Pattern for Time-Based Actions:**
```cpp
case STATE_TURNING:
  {
    static bool printedOnce = false;
    if (!printedOnce) {
      Serial.print("Turning right 90° for ");
      Serial.print(turnDuration);
      Serial.println(" ms");
      printedOnce = true;
    }
    
    robotTurnRight();
    
    if (millis() - startTime >= turnDuration) {
      stopAllMotors();
      printedOnce = false;  // Reset
      setSubState(NEXT_STATE);
    }
  }
  break;
```

### Adding New Tasks

1. **Create Task Files**
```cpp
// TaskX_Name.h
#ifndef TASKX_NAME_H
#define TASKX_NAME_H

enum TaskXSubState {
  TX_INIT,
  TX_DOING_SOMETHING,
  TX_COMPLETED
};

class TaskXName {
  private:
    TaskXSubState currentSubState;
    bool taskActive;
    
  public:
    TaskXName();
    void init();
    void execute();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
};

extern TaskXName taskXName;

#endif
```

2. **Implement execute()**
```cpp
void TaskXName::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case TX_INIT:
      // Initialize
      setSubState(TX_DOING_SOMETHING);
      break;
      
    case TX_DOING_SOMETHING:
      // Use available functions:
      // - tofSensors.getLeftDistance()
      // - colorSensors.getBottomColor()
      // - robotForward(), turnLeft(), etc.
      // - executeLineFollow()
      
      if (/* condition met */) {
        setSubState(TX_COMPLETED);
      }
      break;
      
    case TX_COMPLETED:
      taskActive = false;
      stopAllMotors();
      break;
  }
}
```

3. **Add to State Machine**
- Update `StateMachine.h` with new state
- Add to `progressToNextTask()` logic
- Add serial command in `SerialCommands.cpp`

### Sensor Integration Examples

**TOF Sensors:**
```cpp
readTOFSensors();
if (tofSensors.isObstacleFront()) {
  stopAllMotors();
  Serial.println("Obstacle detected!");
}
```

**Color Sensors:**
```cpp
colorSensors.readBottomSensor();
DetectedColor color = colorSensors.getBottomColor();
if (color == COLOR_RED) {
  // Handle red color
}
```

**IR Sensors:**
```cpp
readAllIRSensorsBinary();
int whiteCount = 0;
for (int i = 0; i < 16; i++) {
  if (irBinary[i] == 1) whiteCount++;
}
if (whiteCount >= 7) {
  // Intersection detected
}
```

**Line Following:**
```cpp
if (isLineFollowActive()) {
  executeLineFollow();  // PD control handles steering
}
```

---

## Troubleshooting

### Motors Not Moving

**Check:**
1. STBY pin HIGH (initialized in `initMotors()`)
2. Speed level set: `SPEED6`
3. Try manual control: `RF`, `RB`, `RTL`, `RTR`
4. Check power supply to motor driver
5. Verify motor wiring (Left: PWMA=4, Right: PWMB=17)

### IR Sensors Not Detecting Line

**Solutions:**
1. **Calibrate first**: `IRCALIBRATE` (most common issue)
2. Check sensor wiring (SIG=1, S0-S3 = 40,41,42,2)
3. Test raw values: `IRREAD`
4. Verify MUX is working
5. Check lighting conditions (consistent lighting needed)
6. Adjust sensor height above surface

### TOF Sensors Reading 2000mm

**Causes:**
1. No object in range (>2000mm)
2. Sensor not initialized properly
3. I2C mux channel conflict
4. Check initialization messages in serial monitor

**Solutions:**
1. Check wiring to I2C bus (SDA=8, SCL=9)
2. Verify I2C mux address (0x70)
3. Test each sensor: `TOFREAD`
4. Check channel assignments (0, 3, 4)

### Color Sensors Not Working

**Check:**
1. Initialization status in serial monitor
2. I2C mux channel (1, 2, 5)
3. Sensor address (0x29)
4. Integration time (600ms - readings not instant)
5. Proper lighting (consistent ambient light)

**Test:**
```bash
COLORREAD
# Should show RGB values and detected color
```

### Line Following Oscillates

**Solutions:**
1. **Reduce Kp**: `SETP 10.0` (less aggressive correction)
2. **Increase Kd**: `SETD 8.0` (more damping)
3. **Lower speed**: `SPEED4`
4. Check sensor calibration: `IRCALIBRATE`

### Wall Following Not Smooth

**Solutions:**
1. **Adjust Kp**: `WFKP 1.2` (try 0.8-1.5 range)
2. **Adjust Kd**: `WFKD 0.6` (try 0.3-1.0 range)
3. **Change target distance**: `WFDIST 120` (try 100-180mm)
4. Verify left TOF sensor working: `TOFREAD`

### Barcode Reading Errors

**Solutions:**
1. **Calibrate IR first**: `IRCALIBRATE` (critical!)
2. **Check sensors 6,7,8,9**: Use `IRREAD`, observe these specifically
3. **Adjust threshold**: `T4IRTHRESH 2300` (midpoint of white/black)
4. **Slow down**: `T4BARSPEED 30`
5. Ensure barcode bars wide enough for sensors
6. Check robot moves straight during reading

### Button Not Responding

**Check:**
1. GPIO 19 connection
2. Test detection: `BUTTONREAD`
3. Verify analog values match expected ranges
4. Check debounce (50ms between presses)
5. Power supply stable

### Emergency Stop Not Working

**Check:**
1. DOWN button highest priority (always works)
2. Serial: `EMERGENCY` command
3. Check button detection in serial monitor
4. Physical button connection to GPIO 19

### Compilation Errors

**Common Issues:**
1. Missing libraries: Install `Adafruit_VL53L0X`, `Adafruit_TCS34725`, `Adafruit_SSD1306`
2. Arduino.h errors: Normal IntelliSense issue, code compiles fine
3. Redefinition errors: Check for duplicate #include guards

### Robot Behavior Unexpected

**Debug Steps:**
1. Check current state: `STATUS`
2. Check current mode: `HELP` (shows MANUAL or AUTO)
3. Emergency stop and reset: `EMERGENCY` → `RESUME`
4. Check speed level: Should show in OLED
5. Review serial output for state transitions
6. Test sensors individually before running tasks

---

## Additional Resources

### File Structure
```
main/
├── main.ino                  # Main Arduino sketch
└── src/                      # Library modules
    ├── Motors.h/cpp
    ├── IRReading.h/cpp
    ├── LineFollow.h/cpp
    ├── WallFollow.h/cpp
    ├── TOFSensors.h/cpp
    ├── ColorSensors.h/cpp
    ├── PushButton.h/cpp
    ├── I2CMux.h/cpp
    ├── OLEDDisplay.h/cpp
    ├── SerialCommands.h/cpp
    ├── StateMachine.h/cpp
    └── tasks/
        ├── Task1_Plantation.h/cpp
        ├── Task2_WallFollow.h/cpp
        ├── Task3_Ramp.h/cpp
        ├── Task4_Barcode.h/cpp
        ├── Task5_Unloading.h/cpp
        └── BallCollector.h/cpp
```

### Dependencies
- Arduino IDE (1.8.x or 2.x)
- ESP32 Board Support (esp32 by Espressif)
- Adafruit_VL53L0X library
- Adafruit_TCS34725 library
- Adafruit_SSD1306 library
- Adafruit_GFX library
- ESP32Servo library
- Wire library (built-in)

### Pin Summary
```
Motors:
  Left:  PWM=4, IN1=6, IN2=5
  Right: PWM=17, IN1=15, IN2=16
  STBY:  7

IR Array:
  SIG: 1, S0: 40, S1: 41, S2: 42, S3: 2

I2C Bus:
  SDA: 8, SCL: 9

Buttons:
  Analog: 19

Servos:
  Arm: 35, Gripper: 36, Sorting: 37
```

---

## Contact & Support

**Repository**: https://github.com/saki-mj/Embedded-Systems-3-task-Robo.git  
**Branch**: develop  
**Last Updated**: November 19, 2025

For issues or questions, refer to the serial output and this documentation. The robot provides extensive real-time feedback through both serial monitor and OLED display.

---

**System Status**: ✅ Ready for deployment  
**All conflicts resolved**: ✅  
**Documentation complete**: ✅

