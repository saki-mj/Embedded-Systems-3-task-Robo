# State Machine Architecture Guide

## Overview
Your robot now has a complete state machine architecture with 5 separate task modules. Each task can run independently and has its own sub-states for tracking progress.

## State Machine States

### 1. STANDBY
- **Purpose**: Initial calibration and setup
- **Entry**: Robot starts in this state
- **Actions**: IR calibration, sensor checks
- **Next**: IDLE

### 2. IDLE
- **Purpose**: Ready to start tasks
- **Entry**: After STANDBY or via START command
- **Actions**: Waiting for task commands
- **Next**: Any TASK or EMERGENCY_STOP

### 3. TASK1_PLANTATION
- **Purpose**: First competition task
- **Sub-states**: INIT, SEARCHING, FOLLOWING, TURNING, LINE_FOLLOWING, PLANTING, COMPLETED
- **Next (Auto)**: TASK2_WALL_FOLLOW
- **Next (Manual)**: Stay in IDLE

### 4. TASK2_WALL_FOLLOW
- **Purpose**: Wall following navigation
- **Sub-states**: INIT, FIND_WALL, ALIGN, FOLLOWING, CORNER_DETECTED, COMPLETED
- **Next (Auto)**: TASK3_RAMP
- **Next (Manual)**: Stay in IDLE

### 5. TASK3_RAMP
- **Purpose**: Navigate ramp obstacle
- **Sub-states**: INIT, APPROACH, CLIMBING, DESCENDING, COMPLETED
- **Next (Auto)**: TASK4_BARCODE
- **Next (Manual)**: Stay in IDLE

### 6. TASK4_BARCODE
- **Purpose**: Read barcode pattern
- **Sub-states**: INIT, SEARCHING, ALIGNING, READING, PROCESSING, COMPLETED
- **Next (Auto)**: TASK5_UNLOADING
- **Next (Manual)**: Stay in IDLE

### 7. TASK5_UNLOADING
- **Purpose**: Unload balls at target zone
- **Sub-states**: INIT, NAVIGATE_TO_ZONE, ALIGN, UNLOADING, VERIFY, COMPLETED
- **Next (Auto)**: IDLE (all tasks complete!)
- **Next (Manual)**: Stay in IDLE

### 8. EMERGENCY_STOP
- **Purpose**: Safety stop
- **Entry**: EMERGENCY command
- **Actions**: Stop all motors, halt all tasks
- **Next**: Can RESUME to IDLE

## Operation Modes

### Manual Mode (Default)
- Tasks must be started via serial commands (TASK1, TASK2, etc.)
- Task completion does NOT automatically start next task
- User has full control over task execution
- Good for testing individual tasks

### Automatic Mode
- Start with AUTO command
- Tasks run sequentially: TASK1 → TASK2 → TASK3 → TASK4 → TASK5 → IDLE
- Task completion automatically triggers next task
- Good for full competition run

## Serial Commands

### State Machine Control
```
START       - Move to IDLE state (ready to run)
AUTO        - Enable automatic mode
MANUAL      - Enable manual mode (default)
STATUS      - Show current state and mode
EMERGENCY   - Emergency stop all tasks
RESUME      - Resume from emergency stop
```

### Task Control
```
TASK1       - Start Task 1 (Plantation)
TASK2       - Start Task 2 (Wall Following)
TASK3       - Start Task 3 (Ramp)
TASK4       - Start Task 4 (Barcode Reading)
TASK5       - Start Task 5 (Unloading Balls)
```

### Existing Commands (Still Available)
All your previous commands still work:
- Speed: SPEED1 to SPEED10
- Motors: LMF, LMB, RMF, RMB, RF, RB, RTL, RTR, STOP
- IR: IRCALIBRATE, IRREAD, IRREADB
- Line Follow: LINEFOLLOW, INVERTLINE, SETP, SETD
- TOF: TOFREAD, TOFTHRESHOLD
- OLED: OLEDCLEAR
- Help: HELP or ?

## File Structure

```
Task_1_LineFollow/
├── Task_1_LineFollow.ino       # Main file (updated)
└── src/
    ├── Motors.h/cpp
    ├── IRReading.h/cpp
    ├── LineFollow.h/cpp
    ├── TOFSensors.h/cpp
    ├── OLEDDisplay.h/cpp
    ├── I2CMux.h/cpp
    ├── SerialCommands.h/cpp    # Updated with state machine commands
    ├── StateMachine.h/cpp      # State machine logic
    └── tasks/
        ├── Task1_Plantation.h/cpp
        ├── Task2_WallFollow.h/cpp
        ├── Task3_Ramp.h/cpp
        ├── Task4_Barcode.h/cpp
        └── Task5_Unloading.h/cpp
```

## How to Use

### 1. Testing Individual Tasks (Manual Mode)
```
1. Upload code to ESP32
2. Open serial monitor (115200 baud)
3. Type: START
4. Type: TASK1 (or TASK2, TASK3, etc.)
5. Watch OLED display for sub-state updates
6. Use EMERGENCY to stop if needed
```

### 2. Full Competition Run (Automatic Mode)
```
1. Upload code to ESP32
2. Type: AUTO
3. Type: START
4. Type: TASK1
5. Robot will automatically progress through all 5 tasks!
```

### 3. Implementing Task Logic

Each task file has this structure:

```cpp
void TaskX::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case TX_INIT:
      // Your initialization code here
      // When ready: setSubState(TX_NEXT_STATE);
      break;
      
    case TX_SOME_STATE:
      // Your task logic here
      // Use: tofSensors.getLeftDistance()
      //      readIRSensor()
      //      robotForward(), turnLeft(), etc.
      // When done: setSubState(TX_NEXT_STATE);
      break;
      
    case TX_COMPLETED:
      // Cleanup code
      taskActive = false;
      break;
  }
}
```

**Available Sensors/Functions:**
- TOF Sensors: `tofSensors.getLeftDistance()`, `getFrontDistance()`, `getRightDistance()`
- IR Sensors: `readIRSensor(0-7)`, `getLinePosition()`
- Motors: `robotForward()`, `robotBackward()`, `turnLeft()`, `turnRight()`, `stopAllMotors()`
- Line Follow: `executeLineFollow()`
- Display: `oledDisplay.show()`, `showTOF()`, etc.

## Implementation Priority

1. ✅ **All task templates created** - Ready to code!
2. ✅ **State machine integrated** - Working!
3. ✅ **Serial commands updated** - Full control!

### Next Steps (For You):
1. **Implement Task 1 logic** in `Task1_Plantation.cpp`
2. **Test Task 1** using `TASK1` command
3. **Repeat for Tasks 2-5**
4. **Test automatic mode** with `AUTO` command

## Tips

- **Sub-states show on OLED**: You can see exactly where each task is
- **Serial logging**: Each state change prints to serial monitor
- **Emergency stop**: Always available with EMERGENCY command
- **Modular design**: Each task is independent, easy to test/debug
- **Reusable sensors**: All tasks share the same TOF, IR, motor libraries

## Example Workflow

```
Serial Monitor:
> START
State: IDLE

> TASK1
Starting Task 1: Plantation
Task 1 sub-state: INIT
Task 1 sub-state: SEARCHING
Task 1 sub-state: FOLLOWING
... (your logic here)
Task 1 sub-state: COMPLETED

> TASK2
Starting Task 2: Wall Following
...
```

## Notes

- The IntelliSense errors about Arduino.h are normal - code will compile fine
- Each task template has placeholder sub-states - customize as needed
- The state machine automatically handles task progression in AUTO mode
- Display updates show current sub-state for debugging
- All existing functionality (line follow, IR, TOF) still works!

---

**Ready to code your tasks!** Start with Task 1 and build from there. The architecture is complete and modular! 🚀
