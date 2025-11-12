# Task 4 Updates - Dynamic Help and Separate Turn Durations

## Changes Made

### 1. Separate Turn Durations ✅
Instead of one `turnDuration` for all turns, now there are:
- **`turnRightDuration`** - For right turn (T4_TURNING_RIGHT1)
- **`turnLeftDuration`** - For left turn (T4_TURNING_LEFT2)

### 2. New Serial Commands ✅
```
T4TURNRDUR <ms>   - Set RIGHT turn duration (default: 1000ms)
T4TURNLDUR <ms>   - Set LEFT turn duration (default: 1000ms)
```

Old command `T4TURNDUR` is replaced by the two above.

### 3. Dynamic Help Menu ✅
The HELP command now shows **CURRENT values** instead of hardcoded defaults.

**Example Output:**
```
Task 4 Barcode Configuration:
  T4WALLDIST <mm> - Wall detection distance (200)
  T4WFOLLOW <mm> - Wall following target distance (150)
  T4BARSPEED <speed> - Barcode reading speed (50)
  T4APPSPEED <speed> - Approach speed (70)
  T4TURNSPEED <speed> - Turn speed (60)
  T4TURNRDUR <ms> - Turn RIGHT duration for 90° (1000)
  T4TURNLDUR <ms> - Turn LEFT duration for 90° (1000)
  T4STRAIGHTDUR <ms> - Straight movement duration (1000)
  T4REVDUR <ms> - Reverse movement duration (500)
  T4IRTHRESH <value> - IR threshold: >value=white(1) (2000)
```

**After changing a value:**
```
T4TURNRDUR 1200
T4IRTHRESH 2500
HELP
```

**New output shows updated values:**
```
Task 4 Barcode Configuration:
  ...
  T4TURNRDUR <ms> - Turn RIGHT duration for 90° (1200)  ← Updated!
  ...
  T4IRTHRESH <value> - IR threshold: >value=white(1) (2500)  ← Updated!
```

## Implementation Details

### Added Getter Functions
All configuration parameters now have getter functions:
- `getWallDetectionDistance()`
- `getWallFollowDistance()`
- `getBarcodeSpeed()`
- `getApproachSpeed()`
- `getTurnSpeed()`
- `getTurnRightDuration()`
- `getTurnLeftDuration()`
- `getStraightDuration()`
- `getReverseDuration()`
- `getIRWhiteThreshold()`

### Smart Turn Duration Selection
The `isTurnComplete()` function now checks the current sub-state:
```cpp
if (currentSubState == T4_TURNING_RIGHT1) {
  requiredDuration = turnRightDuration;
} else if (currentSubState == T4_TURNING_LEFT2) {
  requiredDuration = turnLeftDuration;
}
```

## Usage Example

```bash
# Check current values
HELP

# Adjust right turn (if robot overshoots)
T4TURNRDUR 900

# Adjust left turn (if robot undershoots)
T4TURNLDUR 1100

# Adjust IR threshold after calibration
IRCALIBRATE
IRREAD
# (observe sensor 6,7,8,9 values)
T4IRTHRESH 2300

# Verify new values
HELP
# Will show: T4TURNRDUR (900), T4TURNLDUR (1100), T4IRTHRESH (2300)

# Start task with new settings
TASK4
```

## Benefits

1. **Real-time Feedback**: Always see current configuration
2. **Easy Tuning**: Adjust values and immediately see them in HELP
3. **Independent Turn Control**: Fine-tune left and right turns separately
4. **Better Debugging**: Know exactly what values are being used

## Default Values

| Parameter | Default | Unit |
|-----------|---------|------|
| Wall Detection Distance | 200 | mm |
| Wall Follow Distance | 150 | mm |
| Barcode Speed | 50 | PWM |
| Approach Speed | 70 | PWM |
| Turn Speed | 60 | PWM |
| Turn RIGHT Duration | 1000 | ms |
| Turn LEFT Duration | 1000 | ms |
| Straight Duration | 1000 | ms |
| Reverse Duration | 500 | ms |
| IR White Threshold | 2000 | ADC |
