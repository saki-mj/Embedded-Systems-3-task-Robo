# Embedded Systems - Line Following Robot

This is a project we are going to make for the embedded systems

## Project Structure

```
Task_1_LineFollow/
├── Task_1_LineFollow.ino     # Main Arduino sketch
└── src/                       # Reusable library modules
    ├── Motors.h              # Motor driver header
    ├── Motors.cpp            # Motor driver implementation
    ├── IRReading.h           # IR sensor array header
    ├── IRReading.cpp         # IR sensor array implementation
    ├── LineFollow.h          # Line following controller header
    └── LineFollow.cpp        # Line following controller implementation
```

## Library Modules

### Motors (Motors.h / Motors.cpp)
**TB6612 Motor Driver Library for ESP32-S3**

Features:
- Dual motor control (Motor A/B)
- PWM speed control (10-bit resolution, 0-1023)
- 10 speed levels for easy control
- Individual motor and robot movement functions
- Serial command processing

Key Functions:
- `initMotors()` - Initialize motor driver
- `setMotorA(speed, forward)` / `setMotorB(speed, forward)` - Direct motor control
- `robotForward()`, `robotBackward()`, `robotTurnLeft()`, `robotTurnRight()` - Robot movements
- `setSpeedLevel(1-10)` - Set speed level
- `processCommand(String)` - Process serial commands

### IRReading (IRReading.h / IRReading.cpp)
**CD74HC4067 MUX IR Sensor Array Library for ESP32-S3**

Features:
- 16-channel IR sensor array using MUX
- Automatic calibration with 10-second procedure
- Binary threshold detection (black/white)
- Preset threshold values
- Continuous reading mode

Key Functions:
- `initIRSensors()` - Initialize MUX and sensors
- `readAllIRSensors()` - Read all 16 sensor values
- `readAllIRSensorsBinary()` - Read binary values (0/1)
- `calibrateIRSensors()` - 10-second calibration routine
- `toggleIRReading()` - Toggle continuous reading mode

### LineFollow (LineFollow.h / LineFollow.cpp)
**PD Controller Line Following Library for ESP32-S3**

Features:
- PD (Proportional-Derivative) controller
- Weighted line position calculation (-7 to +7)
- Cross detection (sensors 3-12)
- Line color inversion support (white on black / black on white)
- Tunable Kp and Kd parameters

Key Functions:
- `initLineFollow()` - Initialize line following system
- `executeLineFollow()` - Execute PD control (call in loop)
- `toggleLineFollow()` - Start/stop line following
- `setKp(value)` / `setKd(value)` - Tune PD parameters
- `toggleLineColor()` - Switch line color mode
- `resetCrossDetection()` - Reset cross detection flag

## Serial Commands

**Speed Control:**
- `SPEED1` to `SPEED10` - Set speed level

**Motor Control:**
- `LMF` / `LMB` - Left motor forward/backward
- `RMF` / `RMB` - Right motor forward/backward
- `RF` / `RB` - Robot forward/backward
- `RTL` / `RTR` - Robot turn left/right
- `STOP` - Stop all motors

**IR Sensors:**
- `IRCALIBRATE` - Calibrate sensors (10s)
- `IRREAD` - Toggle continuous raw IR reading
- `IRREADB` - Toggle continuous binary IR reading

**Line Following:**
- `LINEFOLLOW` - Toggle line following mode
- `INVERTLINE` - Toggle line color
- `SETP <value>` - Set Kp (e.g., `SETP 15.0`)
- `SETD <value>` - Set Kd (e.g., `SETD 5.0`)

**Help:**
- `HELP` or `?` - Show command list

## Hardware Configuration

### Motor Driver (TB6612)
- Motor A (Left): PWMA=4, AIN1=6, AIN2=5
- Motor B (Right): PWMB=17, BIN1=15, BIN2=16
- Standby: STBY=7

### IR Sensor Array (CD74HC4067 MUX)
- Signal Pin: SIG=1 (analog input)
- Select Pins: S0=40, S1=41, S2=42, S3=2
- 16 IR sensors on MUX channels 0-15

## Usage Example

```cpp
#include "src/Motors.h"
#include "src/IRReading.h"
#include "src/LineFollow.h"

void setup() {
  Serial.begin(115200);
  initMotors();
  initIRSensors();
  initLineFollow();
  setSpeedLevel(5);  // Mid-range speed
}

void loop() {
  // Process serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
  
  // Execute line following if active
  if (isLineFollowActive()) {
    executeLineFollow();
  }
  
  // Display IR readings if active
  else if (isIRReadingActive()) {
    if (irCalibrated) {
      printIRBinary();
    } else {
      printIRValues();
    }
  }
}
```

## Calibration Procedure

1. Upload the code to ESP32-S3
2. Open Serial Monitor (115200 baud)
3. Send command: `IRCALIBRATE`
4. Move robot over BLACK and WHITE surfaces for 10 seconds
5. Threshold values are automatically calculated
6. Ready for line following!

## Line Following Quick Start

1. Calibrate IR sensors (see above)
2. Set desired speed: `SPEED5`
3. Adjust PD parameters if needed: `SETP 15.0`, `SETD 5.0`
4. Start line following: `LINEFOLLOW`
5. Stop if needed: `LINEFOLLOW` (toggle off) or `STOP`

## Notes

- Default Kp = 15.0, Kd = 5.0
- Line center is between sensors 7 and 8
- Cross detection uses sensors 3-12
- Position range: -7 (far left) to +7 (far right)
- Speed range: PWM 50-1023 (levels 1-10)

