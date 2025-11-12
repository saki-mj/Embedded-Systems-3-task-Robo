# Task 1 - Line Following Robot with TOF Sensors

## Project Structure

### Source Files in `src/` folder:
- **Motors.h/cpp** - TB6612 motor driver control
- **IRReading.h/cpp** - IR sensor array reading and calibration
- **LineFollow.h/cpp** - PD controller for line following
- **I2CMux.h/cpp** - PCA9548A I2C multiplexer control
- **TOFSensors.h/cpp** - VL53L0X Time-of-Flight distance sensors

## Hardware Configuration

### ESP32-S3 Pin Assignments

#### Motor Driver (TB6612)
- Motor A (Left): PWM=4, AIN1=6, AIN2=5
- Motor B (Right): PWM=17, BIN1=15, BIN2=16
- Standby: 7

#### I2C Bus (for TOF Sensors)
- SDA: Pin 8
- SCL: Pin 9

#### PCA9548A I2C Multiplexer
- Address: 0x70
- Channel 0: Left TOF sensor
- Channel 1: Front TOF sensor
- Channel 3: Right TOF sensor

#### IR Sensor Array
- (Check IRReading.h for pin assignments)

## TOF Sensor Library Features

### Initialization
```cpp
initTOFSensors();  // Initialize all three TOF sensors
```

### Reading Sensors
```cpp
readTOFSensors();           // Read all three sensors
uint16_t left = tofSensors.getLeftDistance();
uint16_t front = tofSensors.getFrontDistance();
uint16_t right = tofSensors.getRightDistance();
```

### Obstacle Detection
```cpp
tofSensors.setObstacleThreshold(200);  // Set threshold in mm
bool obstacleLeft = tofSensors.isObstacleLeft();
bool obstacleFront = tofSensors.isObstacleFront();
bool obstacleRight = tofSensors.isObstacleRight();
```

### Display Values
```cpp
printTOFValues();  // Print all distances to Serial
```

## Serial Commands

### Speed Control
- `SPEED1` to `SPEED10` - Set motor speed level

### Motor Control
- `LMF` / `LMB` - Left motor forward/backward
- `RMF` / `RMB` - Right motor forward/backward
- `RF` / `RB` - Robot forward/backward
- `RTL` / `RTR` - Robot turn left/right
- `STOP` - Stop all motors

### IR Sensors
- `IRCALIBRATE` - Calibrate IR sensors (10 second process)
- `IRREAD` - Toggle continuous IR reading mode

### Line Following
- `LINEFOLLOW` - Toggle autonomous line following
- `INVERTLINE` - Toggle between black/white line
- `SETP <value>` - Set proportional gain (e.g., `SETP 15.0`)
- `SETD <value>` - Set derivative gain (e.g., `SETD 5.0`)

### TOF Distance Sensors
- `TOFREAD` - Read and display all TOF distances
- `TOFTHRESHOLD <mm>` - Set obstacle detection threshold (e.g., `TOFTHRESHOLD 200`)

### Help
- `HELP` or `?` - Display all available commands

## I2CMux Library Usage

The I2CMux library provides a simple interface for the PCA9548A multiplexer:

```cpp
#include "I2CMux.h"

I2CMux mux;  // Create multiplexer object (default: 0x70, SDA=8, SCL=9)

void setup() {
  mux.begin();                    // Initialize
  mux.selectChannel(0);           // Select channel 0
  // ... communicate with device on channel 0 ...
  mux.selectChannel(1);           // Switch to channel 1
  // ... communicate with device on channel 1 ...
  mux.disableAll();               // Disable all channels
}
```

### Key Features
- Automatic I2C initialization
- Channel selection (0-7)
- Connection verification
- Current channel tracking

## Dependencies

Required Arduino libraries:
- Wire (built-in)
- Adafruit_VL53L0X

## Getting Started

1. Install required libraries in Arduino IDE
2. Upload `Task_1_LineFollow.ino` to ESP32-S3
3. Open Serial Monitor (115200 baud)
4. Type `HELP` to see all commands
5. Calibrate IR sensors: `IRCALIBRATE`
6. Test TOF sensors: `TOFREAD`
7. Start line following: `LINEFOLLOW`

## Example Usage

```cpp
void loop() {
  // Read TOF sensors periodically
  readTOFSensors();
  
  // Check for obstacles
  if (tofSensors.isObstacleFront()) {
    stopAllMotors();
    Serial.println("Obstacle detected!");
  }
  
  // Line following mode
  if (isLineFollowActive()) {
    executeLineFollow();
  }
}
```

## Notes

- TOF sensors have a maximum reliable range of ~2000mm
- Default obstacle threshold is 200mm
- All three TOF sensors are read sequentially through the multiplexer
- I2C bus runs at 100kHz by default
