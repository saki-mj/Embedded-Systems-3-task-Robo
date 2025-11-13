# Task 4 Barcode - Configuration Guide

## Overview
Task 4 implements barcode reading with wall following navigation. All threshold values can be configured via serial commands.

## Serial Commands for Task 4 Configuration

### Wall Following Parameters
```
WFKP <value>        - Set wall follow Kp gain (default: 1.0)
WFKD <value>        - Set wall follow Kd gain (default: 0.5)
WFDIST <mm>         - Set wall follow target distance (default: 150mm)
```

### Task 4 Specific Parameters

#### Distance Thresholds
```
T4WALLDIST <mm>     - Wall detection distance (default: 200mm)
                      Robot stops when front TOF reads this distance
                      
T4WFOLLOW <mm>      - Wall following target distance (default: 150mm)
                      Target distance from left wall during navigation
```

#### Speed Settings
```
T4BARSPEED <speed>  - Barcode reading speed (default: 60)
                      Speed while crossing barcode
                      
T4APPSPEED <speed>  - Approach speed (default: 60)
                      Speed during wall following and navigation
                      
T4TURNSPEED <speed> - Turn speed (default: 60)
                      Speed during turning maneuvers
```

#### Timing Thresholds
```
T4TURNDUR <ms>      - Turn duration for 90° (default: 1000ms)
                      Time to complete a 90-degree turn
                      
T4STRAIGHTDUR <ms>  - Straight movement duration (default: 1000ms)
                      Currently unused (legacy parameter)
                      
T4REVDUR <ms>       - Reverse movement duration (default: 500ms)
                      Time to move backward after second turn
```

#### Barcode Reading Thresholds
```
T4WHITE0 <ms>       - White bar threshold for bit 0 (default: 1000ms)
                      Duration of white bar indicates binary 0
                      
T4WHITE1 <ms>       - White bar threshold for bit 1 (default: 2000ms)
                      Duration of white bar indicates binary 1
```

## Task 4 State Machine

### States and Behavior

1. **T4_INIT**
   - Initialize all parameters
   - Prepare for barcode reading sequence

2. **T4_SEARCHING_WALL1**
   - **Wall Following Active**: Uses PD control to follow left wall
   - Target distance: `T4WFOLLOW` (default 15cm)
   - Speed: `T4APPSPEED`
   - Exits when front TOF ≤ `T4WALLDIST`

3. **T4_TURNING_RIGHT1**
   - Turn right 90 degrees
   - Duration: `T4TURNDUR`
   - Speed: `T4TURNSPEED`

4. **T4_SEARCHING_WALL2**
   - **Wall Following Active**: Uses PD control to follow left wall
   - Target distance: `T4WFOLLOW` (default 15cm)
   - Speed: `T4APPSPEED`
   - Exits when front TOF ≤ `T4WALLDIST`

5. **T4_TURNING_LEFT2**
   - Turn left 90 degrees
   - Duration: `T4TURNDUR`
   - Speed: `T4TURNSPEED`

6. **T4_MOVING_REVERSE**
   - Move backward
   - Duration: `T4REVDUR` (default 500ms)
   - Speed: `T4APPSPEED`

7. **T4_ALIGNING**
   - Prepare for barcode reading
   - Reset barcode data structures

8. **T4_READING**
   - Move forward at constant speed
   - Speed: `T4BARSPEED`
   - Read 4-bar barcode using IR sensors
   - White bar duration determines bit value:
     - ~1000ms (±300ms) → Bit 0
     - ~2000ms (±500ms) → Bit 1

9. **T4_PROCESSING**
   - Convert 4 bits to binary string
   - Calculate decimal value
   - Display results

10. **T4_COMPLETED**
    - Task finished
    - Barcode data available via `getBarcodeData()`

## Wall Following PD Control

### How It Works
- Reads left TOF sensor distance
- Calculates error: `error = current_distance - target_distance`
- Applies PD control: `correction = (Kp × error) + (Kd × derivative)`
- Adjusts left/right motor speeds to maintain target distance

### Tuning Guidelines
- **Kp (Proportional)**: Higher = stronger correction, may oscillate
- **Kd (Derivative)**: Higher = damping, reduces oscillation
- **Target Distance**: Optimal at 150mm (15cm) from wall

### Example Tuning Sequence
```
WFKP 1.5      // Increase proportional gain
WFKD 0.8      // Increase derivative for stability
WFDIST 150    // Set target to 15cm
TASK4         // Start task
```

## Barcode Reading Algorithm

### Detection Method
1. Uses middle 4 IR sensors (indices 2-5)
2. Considers "on white" when ≥2 sensors detect white
3. Measures duration of white bar
4. Classifies as bit 0 or 1 based on duration

### Timing Tolerance
- Bit 0: 700-1500ms (1000ms ±300-500ms)
- Bit 1: 1500-2500ms (2000ms ±500ms)

### Calibration Tips
1. Start with IRCALIBRATE to set thresholds
2. Test barcode reading speed: `T4BARSPEED 40`
3. Adjust white thresholds based on robot speed
4. Formula: `duration (ms) = bar_width (mm) / speed (mm/s)`

## Example Configuration Session

```
// Wall following tuning
WFKP 1.2
WFKD 0.6
WFDIST 150

// Task 4 speeds
T4APPSPEED 60
T4TURNSPEED 50
T4BARSPEED 40

// Timing adjustments
T4TURNDUR 1200
T4REVDUR 600

// Barcode thresholds
T4WHITE0 1200
T4WHITE1 2400

// Start task
TASK4
```

## Troubleshooting

### Robot doesn't follow wall straight
- Increase `WFKP` for stronger correction
- Adjust `WFDIST` (try 120-180mm range)

### Robot oscillates along wall
- Decrease `WFKP`
- Increase `WFKD` for damping

### Barcode reading errors
- Calibrate IR sensors: `IRCALIBRATE`
- Slow down barcode speed: `T4BARSPEED 30`
- Adjust white thresholds based on actual bar widths

### Turn overshoots/undershoots
- Adjust `T4TURNDUR` in increments of 100ms
- Reduce `T4TURNSPEED` for more accurate turns

### Doesn't detect walls
- Increase `T4WALLDIST` (try 250-300mm)
- Check TOF sensor operation: `TOFREAD`

## Default Values Summary

| Parameter | Default | Unit | Range |
|-----------|---------|------|-------|
| Wall Detection Distance | 200 | mm | 100-500 |
| Wall Follow Distance | 150 | mm | 100-300 |
| Barcode Speed | 60 | PWM | 20-150 |
| Approach Speed | 60 | PWM | 20-150 |
| Turn Speed | 60 | PWM | 20-100 |
| Turn Duration | 1000 | ms | 500-2000 |
| Reverse Duration | 500 | ms | 200-1000 |
| White Threshold 0 | 1000 | ms | 500-1500 |
| White Threshold 1 | 2000 | ms | 1500-3000 |
| Wall Follow Kp | 1.0 | - | 0.1-5.0 |
| Wall Follow Kd | 0.5 | - | 0.0-2.0 |

## Notes

- All thresholds persist only during current session (not saved to EEPROM)
- Wall following uses left TOF sensor only
- Barcode reading requires pre-calibrated IR sensors
- Turn duration is time-based; consider adding gyro/encoder for accuracy
