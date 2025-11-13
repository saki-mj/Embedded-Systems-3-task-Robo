# Task 4 Barcode - Configuration Guide

## Overview
Task 4 implements barcode reading with wall following navigation. All threshold values can be configured via serial commands with initial default values.

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

#### Speed Settings (Initial Values Provided)
```
T4BARSPEED <speed>  - Barcode reading speed (default: 50)
                      Speed while crossing barcode
                      
T4APPSPEED <speed>  - Approach speed (default: 70)
                      Speed during wall following and navigation
                      
T4TURNSPEED <speed> - Turn speed (default: 60)
                      Speed during turning maneuvers
```

#### Timing Thresholds (Initial Values Provided)
```
T4TURNDUR <ms>      - Turn duration for 90° (default: 1000ms)
                      Time to complete a 90-degree turn
                      
T4STRAIGHTDUR <ms>  - Straight movement duration (default: 1000ms)
                      Currently unused (legacy parameter)
                      
T4REVDUR <ms>       - Reverse movement duration (default: 500ms)
                      Time to move backward after second turn
```

#### Barcode Reading Threshold (Initial Value: 2000)
```
T4IRTHRESH <value>  - IR white threshold (default: 2000)
                      IR reading > threshold = white (bit 1)
                      IR reading ≤ threshold = black (bit 0)
                      *** IMPORTANT: Calibrate using IRCALIBRATE ***
```

## Task 4 State Machine

### States and Behavior

1. **T4_INIT**
   - Initialize all parameters
   - Prepare for barcode reading sequence

2. **T4_SEARCHING_WALL1**
   - **Wall Following Active**: Uses PD control to follow left wall
   - Target distance: `T4WFOLLOW` (default 150mm)
   - Speed: `T4APPSPEED` (default 70)
   - Exits when front TOF ≤ `T4WALLDIST` (default 200mm)

3. **T4_TURNING_RIGHT1**
   - Turn right 90 degrees
   - Duration: `T4TURNDUR` (default 1000ms)
   - Speed: `T4TURNSPEED` (default 60)

4. **T4_SEARCHING_WALL2**
   - **Wall Following Active**: Uses PD control to follow left wall
   - Target distance: `T4WFOLLOW` (default 150mm)
   - Speed: `T4APPSPEED` (default 70)
   - Exits when front TOF ≤ `T4WALLDIST` (default 200mm)

5. **T4_TURNING_LEFT2**
   - Turn left 90 degrees
   - Duration: `T4TURNDUR` (default 1000ms)
   - Speed: `T4TURNSPEED` (default 60)

6. **T4_MOVING_REVERSE**
   - Move backward
   - Duration: `T4REVDUR` (default 500ms)
   - Speed: `T4APPSPEED` (default 70)

7. **T4_ALIGNING**
   - Prepare for barcode reading
   - Reset barcode data structures

8. **T4_READING**
   - Move forward at constant speed
   - Speed: `T4BARSPEED` (default 50)
   - Read barcode using IR sensors 6, 7, 8, 9
   - Each sensor reads one bit:
     - **Sensor 6 → Bit 3 (MSB)**
     - **Sensor 7 → Bit 2**
     - **Sensor 8 → Bit 1**
     - **Sensor 9 → Bit 0 (LSB)**
   - IR value > threshold → white (1)
   - IR value ≤ threshold → black (0)
   - Collects ~20 samples and uses most frequent value

9. **T4_PROCESSING**
   - Analyzes all collected samples
   - Finds most frequently occurring barcode value
   - Converts to binary string
   - Displays results

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
- **Kp (Proportional)**: Higher = stronger correction, may oscillate (default: 1.0)
- **Kd (Derivative)**: Higher = damping, reduces oscillation (default: 0.5)
- **Target Distance**: Optimal at 150mm from wall (default)

### Example Tuning Sequence
```
WFKP 1.5      // Increase proportional gain
WFKD 0.8      // Increase derivative for stability
WFDIST 150    // Set target to 15cm
TASK4         // Start task
```

## Barcode Reading Algorithm

### New Detection Method (Using 4 Separate Sensors)

1. **Sensors Used**: IR sensors 6, 7, 8, 9 (array indices)
2. **Bit Assignment**:
   - Sensor 6 = Bit 3 (MSB - Most Significant Bit)
   - Sensor 7 = Bit 2
   - Sensor 8 = Bit 1
   - Sensor 9 = Bit 0 (LSB - Least Significant Bit)

3. **Threshold Logic**:
   - IR reading > `T4IRTHRESH` → White → Bit = 1
   - IR reading ≤ `T4IRTHRESH` → Black → Bit = 0

4. **Sampling Strategy**:
   - Reads every 100ms while moving across barcode
   - Collects minimum 20 samples
   - Each sample creates a 4-bit barcode value (0-15)
   - Counts occurrences of each barcode value

5. **Final Result**:
   - Barcode value that appears most frequently wins
   - Provides robustness against noise and reading errors

### Calibration Steps

1. **Determine IR Threshold**:
   ```
   IRCALIBRATE          // Calibrate IR sensors
   IRREAD               // Start continuous reading
   ```
   - Move sensors over white surface, note values for sensors 6,7,8,9
   - Move sensors over black surface, note values for sensors 6,7,8,9
   - Choose threshold midway between white and black values
   - Example: White=3500, Black=1000 → Threshold=2200

2. **Set Threshold**:
   ```
   T4IRTHRESH 2200      // Set based on your calibration
   ```

3. **Test Barcode Reading**:
   ```
   T4BARSPEED 40        // Slower speed for testing
   TASK4                // Start task
   ```

## Example Configuration Session

```
// Step 1: Calibrate IR sensors first
IRCALIBRATE

// Step 2: Check IR values on white/black surfaces
IRREAD
// Observe sensor 6,7,8,9 values specifically
// Press IRREAD again to stop

// Step 3: Set IR threshold (midpoint between white and black)
T4IRTHRESH 2200

// Step 4: Wall following tuning
WFKP 1.2
WFKD 0.6
WFDIST 150

// Step 5: Task 4 speeds
T4APPSPEED 70
T4TURNSPEED 55
T4BARSPEED 40

// Step 6: Timing adjustments
T4TURNDUR 1100
T4REVDUR 600

// Step 7: Start task
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
- **Calibrate IR sensors first**: `IRCALIBRATE`
- **Check sensor 6,7,8,9 specifically**: Use `IRREAD` and monitor these sensors
- **Adjust IR threshold**: `T4IRTHRESH <value>` - should be between white and black readings
- **Slow down reading speed**: `T4BARSPEED 30`
- **Ensure proper alignment**: Sensors 6,7,8,9 should be directly over barcode bars

### Inconsistent barcode readings
- Increase sample count in code if needed (currently 20 samples)
- Ensure barcode bars are wide enough for sensors
- Check that robot moves straight during reading
- Verify all 4 sensors (6,7,8,9) are working: `IRREAD`

### Turn overshoots/undershoots
- Adjust `T4TURNDUR` in increments of 100ms
- Reduce `T4TURNSPEED` for more accurate turns

### Doesn't detect walls
- Increase `T4WALLDIST` (try 250-300mm)
- Check TOF sensor operation: `TOFREAD`

## Default Values Summary

| Parameter | Default | Unit | Range | Notes |
|-----------|---------|------|-------|-------|
| Wall Detection Distance | 200 | mm | 100-500 | Front TOF threshold |
| Wall Follow Distance | 150 | mm | 100-300 | Left wall target |
| Barcode Speed | 50 | PWM | 20-100 | Slower = more accurate |
| Approach Speed | 70 | PWM | 30-150 | Wall following speed |
| Turn Speed | 60 | PWM | 20-100 | Turning maneuver |
| Turn Duration | 1000 | ms | 500-2000 | 90° turn time |
| Straight Duration | 1000 | ms | 500-2000 | Legacy parameter |
| Reverse Duration | 500 | ms | 200-1000 | After 2nd turn |
| IR White Threshold | 2000 | ADC | 0-4095 | **Calibrate with IRREAD** |
| Wall Follow Kp | 1.0 | - | 0.1-5.0 | Proportional gain |
| Wall Follow Kd | 0.5 | - | 0.0-2.0 | Derivative gain |

## Barcode Reading Examples

### Example Barcode: 0101 (Binary) = 5 (Decimal)

During reading, sensors 6,7,8,9 positioned over barcode bars:
- Sensor 6 (Bit 3): Black → 0
- Sensor 7 (Bit 2): White → 1
- Sensor 8 (Bit 1): Black → 0
- Sensor 9 (Bit 0): White → 1

Result: `0101` = 5

### Possible Barcode Values
- 0000 = 0
- 0001 = 1
- 0010 = 2
- ...
- 1111 = 15

## Notes

- All thresholds persist only during current session (not saved to EEPROM)
- Wall following uses left TOF sensor only
- **Barcode reading uses IR sensors 6, 7, 8, 9 independently**
- **IR threshold MUST be calibrated using IRCALIBRATE and IRREAD**
- Turn duration is time-based; consider adding gyro/encoder for accuracy
- **Barcode detection uses majority voting (most frequent reading wins)**
- Minimum 20 samples collected before processing barcode
- Each sample is taken every 100ms during barcode crossing
