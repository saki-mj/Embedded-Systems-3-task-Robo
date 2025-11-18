# Serial Output Guidelines

## Design Principle: Eliminate Repetitive Serial Spam

This document establishes rules for serial output to ensure readable, meaningful debug information without repetitive spam.

---

## **CRITICAL RULES**

### ✅ DO Print:
1. **State transitions** - When entering a new state/sub-state (ONCE per transition)
2. **Time-based actions** - Format: `"Action description for X ms"` (print ONCE at start)
3. **Condition-based actions** - Format: `"Action until condition met"` (print ONCE at start)
4. **Significant events** - Sensor detections, completed operations, errors
5. **Manual command feedback** - User needs confirmation of manual serial commands
6. **Configuration changes** - When settings are updated via serial commands

### ❌ DO NOT Print:
1. **Inside loops that run every cycle** - This causes spam
2. **Low-level motor control calls** - `robotForward()`, `stopAllMotors()`, etc.
3. **Repeated sensor readings** - Unless specifically debugging that sensor
4. **Every iteration of PID/control loops** - Wall following, line following execution
5. **Same message on every loop iteration** - Use `static bool printedOnce` pattern

---

## **Implementation Pattern for Tasks**

### Time-Based Actions (Correct Pattern):
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
    
    // Execute action
    setCurrentSpeed(turnSpeed);
    robotTurnRight();
    
    if (millis() - startTime >= turnDuration) {
      stopAllMotors();
      printedOnce = false;  // Reset for next time
      setSubState(NEXT_STATE);
    }
  }
  break;
```

### Condition-Based Actions (Correct Pattern):
```cpp
case STATE_APPROACH_WALL:
  readTOFSensors();
  
  if (!wallFollow.isActive()) {
    Serial.print("Following wall until front TOF reads ");
    Serial.print(wallDetectionDistance);
    Serial.println(" mm");
    wallFollow.start();
  }
  
  wallFollow.executeWallFollow(tofSensors.getLeftDistance());
  
  if (tofSensors.getFrontDistance() <= wallDetectionDistance) {
    Serial.print("Wall detected at ");
    Serial.print(tofSensors.getFrontDistance());
    Serial.println(" mm");
    wallFollow.stop();
    setSubState(NEXT_STATE);
  }
  break;
```

---

## **Modified Files & Rationale**

### 1. **Motors.cpp**
- ❌ **Removed** all `Serial.println()` from:
  - `robotForward()` - Called in loops
  - `robotBackward()` - Called in loops
  - `robotTurnLeft()` - Called in loops
  - `robotTurnRight()` - Called in loops
  - `stopAllMotors()` - Called at every state transition
  - Individual motor functions
  
- ✅ **Why**: These are LOW-LEVEL functions called hundreds of times. Tasks print context.
- ✅ **Guidance comment** added at top of file

### 2. **WallFollow.cpp**
- ❌ **Removed** `Serial.println()` from:
  - `start()` - Called whenever wall following begins
  - `stop()` - Called at state transitions
  
- ✅ **Modified** setters to only print when value CHANGES:
  - `setKp()`, `setKd()`, `setTargetDistance()`, `setBaseSpeed()`
  - Prevents spam when tasks set same value repeatedly
  
- ✅ **Why**: Control library shouldn't spam. Tasks print meaningful context.
- ✅ **Guidance comment** added at top of file

### 3. **SerialCommands.cpp**
- ✅ **Added back** serial output for MANUAL commands:
  - `RF`, `RB`, `RTL`, `RTR` - User needs feedback
  - `STOP` - Prints "All Motors Stopped" and "Wall Following stopped"
  - Individual motor commands
  
- ✅ **Why**: Manual commands MUST give user feedback

### 4. **All Task Files (Task1-5)**
- ✅ **Added guidance comments** at top of each file
- ✅ **Task4** already uses `static bool printedOnce` pattern correctly

---

## **Expected Clean Output Example**

### Task 4 Barcode Reading (After Fixes):
```
Task 4: Initializing barcode reading
Task 4 sub-state: SEARCH_WALL1
Following left wall until front TOF reads 200 mm
Wall 1 detected at 195 mm
Task 4 sub-state: TURN_RIGHT1
Turning right 90° for 1000 ms
Task 4 sub-state: SEARCH_WALL2
Following left wall until front TOF reads 200 mm
Wall 2 detected at 143 mm
Task 4 sub-state: TURN_LEFT2
Turning left 90° for 1000 ms
Task 4 sub-state: MOVE_REVERSE
Moving reverse for 500 ms
Task 4 sub-state: ALIGNING
Task 4: Preparing to read barcode
Task 4 sub-state: READING
Reading barcode while moving forward...
Barcode reading: 1010 (10) - IR[6-9]: 2150, 1800, 2200, 1750
Barcode reading: 1010 (10) - IR[6-9]: 2140, 1810, 2190, 1760
...
Barcode reading complete!
Task 4 sub-state: PROCESSING
Task 4: Processing barcode data
Barcode 10 (1010): 18 samples
Most Frequent Barcode: 10 (appeared 18 times)
Binary: 1010
Decimal: 10
Task 4 sub-state: COMPLETED
Task 4: COMPLETED
Barcode Result: 1010
```

**No more spam!** ✨

---

## **Future Development Rules**

When adding new features:

1. **Never print in functions called repeatedly** (loops, control functions)
2. **Use `static bool printedOnce`** pattern for actions that repeat in a state
3. **Print state transitions** from `setSubState()` or equivalent
4. **Print "Action + duration/condition"** when starting time/condition-based actions
5. **Low-level libraries stay silent** - tasks provide context
6. **Manual commands always respond** - user needs feedback

---

## **Quick Checklist Before Committing Code**

- [ ] No serial prints inside `execute()` loop without `printedOnce` guard?
- [ ] No serial prints in motor control functions?
- [ ] No serial prints in sensor reading functions (unless debugging)?
- [ ] State changes print clear, meaningful messages?
- [ ] Time-based actions print duration ONCE?
- [ ] Condition-based actions print condition ONCE?
- [ ] Manual serial commands have feedback?

---

**Last Updated**: November 17, 2025  
**Status**: ✅ All tasks updated with guidelines
