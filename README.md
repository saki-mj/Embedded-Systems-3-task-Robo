# Embedded Systems - Multi-Task Robot Control System

This is a complete robot control system with state machine architecture for autonomous task execution.

## Project Overview

This project implements a modular, task-based robot control system with:
- **5 Independent Tasks**: Plantation, Wall Following, Ramp Navigation, Barcode Reading, and Ball Unloading
- **State Machine Architecture**: Clean separation of tasks with sub-state management
- **Manual & Automatic Modes**: Full control or autonomous sequential execution
- **Multi-Sensor Integration**: TOF sensors, Color sensors, IR sensor array, OLED display, and Push button
- **Comprehensive Serial Interface**: Complete command-line control system

## Project Structure

```
main/
├── main.ino                  # Main Arduino sketch
├── STATE_MACHINE_GUIDE.md    # Complete architecture documentation
└── src/                      # Reusable library modules
    ├── Motors.h/cpp          # TB6612 motor driver
    ├── IRReading.h/cpp       # CD74HC4067 MUX IR sensor array
    ├── LineFollow.h/cpp      # PD controller for line following
    ├── TOFSensors.h/cpp      # VL53L0X TOF sensor manager
    ├── ColorSensors.h/cpp    # TCS34725 RGB color sensors (2x)
    ├── PushButton.h/cpp      # 5-button analog switch
    ├── I2CMux.h/cpp          # PCA9548A I2C multiplexer
    ├── OLEDDisplay.h/cpp     # SSD1306 OLED display wrapper
    ├── SerialCommands.h/cpp  # Command processing system
    ├── StateMachine.h/cpp    # State machine controller
    └── tasks/                # Individual task modules
        ├── Task1_Plantation.h/cpp
        ├── Task2_WallFollow.h/cpp
        ├── Task3_Ramp.h/cpp
        ├── Task4_Barcode.h/cpp
        └── Task5_Unloading.h/cpp
```

## System Architecture

### State Machine
- **8 States**: STANDBY, IDLE, TASK1-5, EMERGENCY_STOP
- **2 Modes**: Manual (command-driven) and Automatic (sequential)
- **Sub-States**: Each task has its own sub-state machine for detailed control
- **Safety**: Emergency stop capability with resume function

### Core Library Modules

#### Motors (Motors.h / Motors.cpp)
**TB6612 Motor Driver Library**
- Dual motor control with 10 speed levels (50-1023 PWM)
- Functions: `robotForward()`, `robotBackward()`, `turnLeft()`, `turnRight()`, `stopAllMotors()`

#### IRReading (IRReading.h / IRReading.cpp)
**CD74HC4067 MUX IR Sensor Array**
- 16-channel IR array with automatic calibration
- Binary threshold detection for line detection
- Functions: `calibrateIRSensors()`, `readAllIRSensors()`, `getLinePosition()`

#### LineFollow (LineFollow.h / LineFollow.cpp)
**PD Controller for Line Following**
- Proportional-Derivative control (tunable Kp, Kd)
- Weighted position calculation (-7 to +7)
- Functions: `executeLineFollow()`, `setKp()`, `setKd()`, `toggleLineColor()`

#### TOFSensors (TOFSensors.h / TOFSensors.cpp)
**VL53L0X Time-of-Flight Sensor Manager**
- 3 TOF sensors (Left, Front, Right) via I2C multiplexer
- Obstacle detection with configurable threshold
- Functions: `readAll()`, `getLeftDistance()`, `isObstacleFront()`

#### ColorSensors (ColorSensors.h / ColorSensors.cpp)
**TCS34725 RGB Color Sensor Manager**
- 2 color sensors (Bottom on Ch 4, Top on Ch 2) via I2C multiplexer
- RGB color detection with classification (RED, GREEN, BLUE, YELLOW, WHITE, BLACK)
- Lux and color temperature measurement
- Functions: `readAll()`, `getBottomColor()`, `getTopColor()`, `getColorName()`

#### PushButton (PushButton.h / PushButton.cpp)
**5-Button Analog Switch**
- Five buttons (UP, LEFT, MIDDLE, RIGHT, DOWN) on single GPIO 19
- Voltage divider detection with debouncing
- Edge detection for button press events
- Functions: `update()`, `isPressed()`, `wasPressed()`, `getCurrentButton()`

#### I2CMux (I2CMux.h / I2CMux.cpp)
**PCA9548A I2C Multiplexer Controller**
- 8-channel I2C multiplexer at address 0x70
- Enables multiple I2C devices with same address
- Functions: `selectChannel(0-7)`, `disableAll()`

#### OLEDDisplay (OLEDDisplay.h / OLEDDisplay.cpp)
**SSD1306 OLED Display Wrapper**
- 128x64 display at I2C address 0x3C
- Multiple display modes: status, TOF readings, color sensors, line following, IR status
- Functions: `show()`, `showStatus()`, `showTOF()`, `showColor()`, `showLineFollowing()`

#### SerialCommands (SerialCommands.h / SerialCommands.cpp)
**Centralized Command Processing**
- Complete command-line interface
- State machine integration
- Functions: `processSerialCommand()`, `printSerialCommands()`

#### StateMachine (StateMachine.h / StateMachine.cpp)
**Task State Management**
- State transitions and mode control
- Automatic task progression in AUTO mode
- Functions: `setState()`, `setMode()`, `emergencyStop()`, `progressToNextTask()`

### Task Modules

Each task is a self-contained module with:
- **Sub-states**: Detailed progress tracking (INIT, execution states, COMPLETED)
- **Independent execution**: `init()`, `execute()`, `updateDisplay()`
- **Status tracking**: `isActive()`, `isCompleted()`, `reset()`
- **OLED feedback**: Real-time sub-state display

#### Task 1: Plantation
Sub-states: INIT, SEARCHING, FOLLOWING, TURNING, LINE_FOLLOWING, PLANTING, COMPLETED

#### Task 2: Wall Following
Sub-states: INIT, FIND_WALL, ALIGN, FOLLOWING, CORNER_DETECTED, COMPLETED

#### Task 3: Ramp Navigation
Sub-states: INIT, APPROACH, CLIMBING, DESCENDING, COMPLETED

#### Task 4: Barcode Reading
Sub-states: INIT, SEARCHING, ALIGNING, READING, PROCESSING, COMPLETED

#### Task 5: Unloading Balls
Sub-states: INIT, NAVIGATE_TO_ZONE, ALIGN, UNLOADING, VERIFY, COMPLETED

## Serial Commands

### State Machine Control
- `START` - Enter IDLE state (ready to run tasks)
- `AUTO` - Enable automatic mode (tasks run sequentially)
- `MANUAL` - Enable manual mode (command-driven, default)
- `TASK1` - Start Task 1 (Plantation)
- `TASK2` - Start Task 2 (Wall Following)
- `TASK3` - Start Task 3 (Ramp)
- `TASK4` - Start Task 4 (Barcode Reading)
- `TASK5` - Start Task 5 (Unloading Balls)
- `EMERGENCY` - Emergency stop all tasks
- `RESUME` - Resume from emergency stop
- `STATUS` - Show current state and mode

### Speed Control
- `SPEED1` to `SPEED10` - Set speed level (50-1023 PWM)

### Motor Control
- `LMF` / `LMB` - Left motor forward/backward
- `RMF` / `RMB` - Right motor forward/backward
- `RF` / `RB` - Robot forward/backward
- `RTL` / `RTR` - Robot turn left/right
- `STOP` - Stop all motors

### IR Sensors
- `IRCALIBRATE` - Calibrate sensors (10 seconds)
- `IRREAD` - Toggle continuous raw IR reading
- `IRREADB` - Toggle continuous binary IR reading

### Line Following
- `LINEFOLLOW` - Toggle line following mode
- `INVERTLINE` - Toggle line color (white/black)
- `SETP <value>` - Set proportional gain (e.g., `SETP 15.0`)
- `SETD <value>` - Set derivative gain (e.g., `SETD 5.0`)

### TOF Sensors
- `TOFREAD` - Read and display TOF distances
- `TOFTHRESHOLD <mm>` - Set obstacle threshold (e.g., `TOFTHRESHOLD 200`)

### Color Sensors
- `COLORREAD` - Read both color sensors (bottom and top)
- `COLORBOTTOM` - Read bottom color sensor (Channel 4)
- `COLORTOP` - Read top color sensor (Channel 2)

### Push Button
- `BUTTONREAD` - Read current button state

### OLED Display
- `OLEDCLEAR` - Clear OLED display

### Help
- `HELP` or `?` - Show complete command list

## Hardware Configuration

### ESP32-S3 Microcontroller
- I2C Bus: SDA=8, SCL=9
- Serial: 115200 baud

### Motor Driver (TB6612)
- Motor A (Left): PWMA=4, AIN1=6, AIN2=5
- Motor B (Right): PWMB=17, BIN1=15, BIN2=16
- Standby: STBY=7

### IR Sensor Array (CD74HC4067 MUX)
- Signal Pin: SIG=1 (analog input)
- Select Pins: S0=40, S1=41, S2=42, S3=2
- 16 IR sensors on MUX channels 0-15

### I2C Devices (Shared Bus)
- **PCA9548A I2C Multiplexer**: Address 0x70
- **SSD1306 OLED Display**: Address 0x3C, 128x64 pixels
- **VL53L0X TOF Sensors** (3x): Address 0x29 each
  - Left TOF: Mux Channel 0
  - Front TOF: Mux Channel 1
  - Right TOF: Mux Channel 3
- **TCS34725 Color Sensors** (2x): Address 0x29 each
  - Bottom Color Sensor: Mux Channel 4
  - Top Color Sensor: Mux Channel 2

### Push Button Switch
- **5-Button Analog Switch**: GPIO 19 (12-bit ADC)
  - UP: 0-50 (available for custom use)
  - LEFT: 1100-1200 (available for custom use)
  - MIDDLE: 1750-1850 (available for custom use)
  - RIGHT: 2600-2750 ⚡ **Cycle through tasks**
  - DOWN: 3750-4020 🛑 **Emergency stop**

## Quick Start Guide

### 1. Manual Task Testing
```
1. Upload code to ESP32-S3
2. Open Serial Monitor (115200 baud)
3. Type: START
4. Type: TASK1 (or TASK2, TASK3, etc.)
5. Watch OLED for sub-state updates
6. Press RIGHT button to switch tasks
7. Press DOWN button for emergency stop
```

### 2. Automatic Mode (Competition Run)
```
1. Upload code to ESP32-S3
2. Type: AUTO
3. Type: START
4. Type: TASK1
5. Robot automatically progresses through all tasks!
```

### 3. Basic Testing
```
1. Test motors: RF, RB, RTL, RTR, STOP
2. Calibrate IR: IRCALIBRATE (move over black/white for 10s)
3. Test TOF: TOFREAD
4. Test colors: COLORREAD, COLORBOTTOM, COLORTOP
5. Test button: BUTTONREAD (press buttons to see detection)
6. Test line follow: LINEFOLLOW
7. Check status: STATUS
```

## Implementing Custom Task Logic

Each task file (`src/tasks/TaskX_Name.cpp`) has this structure:

```cpp
void TaskX::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case TX_INIT:
      // Your initialization code
      setSubState(TX_NEXT_STATE);
      break;
      
    case TX_YOUR_STATE:
      // Your task logic here
      // Available functions:
      // - tofSensors.getLeftDistance()
      // - colorSensors.getBottomColor()
      // - pushButton.isPressed(BTN_UP)
      // - readIRSensor(0-7)
      // - robotForward(), turnLeft(), etc.
      // - executeLineFollow()
      setSubState(TX_NEXT_STATE);
      break;
      
    case TX_COMPLETED:
      taskActive = false;
      break;
  }
}
```

## Features

✅ **Modular Architecture** - Each task is independent and testable  
✅ **State Machine** - Clean state management with auto-progression  
✅ **Multi-Sensor Integration** - TOF, Color, IR, OLED, and Push button working together  
✅ **Flexible Control** - Manual testing or automatic competition runs  
✅ **Real-time Feedback** - OLED shows current task and sub-state  
✅ **Safety Features** - Emergency stop with resume capability + physical button  
✅ **Comprehensive Commands** - Complete serial interface  
✅ **Reusable Libraries** - Portable sensor and motor code  
✅ **Color Detection** - RGB sensors for object identification  
✅ **User Input** - 5-button switch (RIGHT=task switch, DOWN=emergency)  

## Documentation

- **STATE_MACHINE_GUIDE.md** - Complete architecture documentation
- **Task Templates** - All 5 tasks ready for implementation
- **Serial Commands** - Type `HELP` for full command list

## Development Notes

- IntelliSense errors for Arduino.h are normal - code compiles fine
- Each task template has placeholder logic - customize as needed
- State machine handles automatic progression in AUTO mode
- OLED updates show sub-states for debugging
- All existing functionality preserved and integrated

## Competition Workflow

1. **Development**: Implement each task in `src/tasks/`
2. **Testing**: Use `MANUAL` mode to test tasks individually
3. **Integration**: Test `AUTO` mode for full run
4. **Tuning**: Adjust PD parameters, speed, thresholds
5. **Competition**: Set to `AUTO`, run `START` → `TASK1`

---

**Project Status**: ✅ Complete architecture - Ready for task implementation!  
**Last Updated**: November 12, 2025

