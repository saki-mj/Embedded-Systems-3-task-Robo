# Task 4 Barcode - Quick Reference

## Updated Changes Summary

### 1. Barcode Detection Method Changed
**OLD**: Duration-based detection (white bar timing)
**NEW**: Direct sensor reading with threshold

### 2. Single IR Threshold
- **Command**: `T4IRTHRESH <value>`
- **Default**: 2000
- **Logic**: 
  - IR > threshold → White (1)
  - IR ≤ threshold → Black (0)

### 3. Sensors Used: 6, 7, 8, 9
```
Sensor 6 → Bit 3 (MSB)
Sensor 7 → Bit 2
Sensor 8 → Bit 1  
Sensor 9 → Bit 0 (LSB)
```

### 4. Majority Voting
- Collects 20+ samples
- Uses most frequently occurring barcode value
- Robust against noise

### 5. Initial Values Added

| Parameter | Command | Default Value |
|-----------|---------|---------------|
| Wall Detection Distance | T4WALLDIST | 200 mm |
| Wall Follow Distance | T4WFOLLOW | 150 mm |
| Barcode Speed | T4BARSPEED | 50 |
| Approach Speed | T4APPSPEED | 70 |
| Turn Speed | T4TURNSPEED | 60 |
| Turn Duration | T4TURNDUR | 1000 ms |
| Straight Duration | T4STRAIGHTDUR | 1000 ms |
| Reverse Duration | T4REVDUR | 500 ms |
| IR White Threshold | T4IRTHRESH | 2000 |
| Wall Follow Kp | WFKP | 1.0 |
| Wall Follow Kd | WFKD | 0.5 |
| Wall Follow Distance | WFDIST | 150 mm |

## Quick Calibration Steps

```bash
# 1. Calibrate IR sensors
IRCALIBRATE

# 2. Check sensors 6,7,8,9 values
IRREAD
# Place over white: note values (e.g., 3500)
# Place over black: note values (e.g., 1000)
# Stop: IRREAD

# 3. Set threshold (midpoint)
T4IRTHRESH 2200

# 4. Test
TASK4
```

## Serial Commands Quick Reference

### Wall Following
```
WFKP 1.2         # Proportional gain
WFKD 0.6         # Derivative gain  
WFDIST 150       # Target distance (mm)
```

### Task 4 Configuration
```
T4WALLDIST 200   # Wall detection (mm)
T4WFOLLOW 150    # Wall follow target (mm)
T4BARSPEED 50    # Barcode read speed
T4APPSPEED 70    # Approach speed
T4TURNSPEED 60   # Turn speed
T4TURNDUR 1000   # 90° turn time (ms)
T4REVDUR 500     # Reverse time (ms)
T4IRTHRESH 2000  # IR threshold
```

## Barcode Examples

| Barcode (Binary) | Decimal | Sensor 6 | Sensor 7 | Sensor 8 | Sensor 9 |
|------------------|---------|----------|----------|----------|----------|
| 0000 | 0 | Black | Black | Black | Black |
| 0101 | 5 | Black | White | Black | White |
| 1010 | 10 | White | Black | White | Black |
| 1111 | 15 | White | White | White | White |

## Files Modified

1. `src/tasks/Task4_Barcode.h` - Updated threshold structure
2. `src/tasks/Task4_Barcode.cpp` - New barcode reading logic
3. `src/SerialCommands.cpp` - Updated commands and help
4. `src/WallFollow.h/cpp` - New wall following library
5. `src/Motors.h/cpp` - Added individual motor speed control
6. `main.ino` - Added WallFollow initialization
7. `TASK4_CONFIGURATION.md` - Complete documentation
