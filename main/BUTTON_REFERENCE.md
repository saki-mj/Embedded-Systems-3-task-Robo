# Push Button Quick Reference

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

## Button Functions

### 🛑 DOWN Button - EMERGENCY STOP
**Action**: Immediate emergency stop
**What it does**:
- Stops all active tasks
- Stops all motors
- Sets state to EMERGENCY_STOP
- Shows "EMERGENCY STOP" on OLED

**Use when**: 
- Robot needs to stop immediately
- Something goes wrong
- Safety intervention needed

**Recovery**: 
- Serial: Type `RESUME` to return to IDLE
- Or start a new task with RIGHT button

---

### ⚡ RIGHT Button - TASK SWITCHER
**Action**: Cycle through tasks
**What it does**:
- Stops current task
- Starts next task in sequence
- Updates OLED with new task
- Task cycle: 1 → 2 → 3 → 4 → 5 → 1

**Use when**:
- Want to skip to next task
- Testing individual tasks
- Manual task control

**Note**: Cannot switch during EMERGENCY_STOP or STANDBY

---

### 📝 UP, LEFT, MIDDLE Buttons
**Action**: Available for custom task logic
**What it does**:
- Currently no global action
- Available for use within task code
- Can be programmed for task-specific functions

**Example uses**:
- UP: Increase speed/sensitivity
- LEFT: Change mode within task
- MIDDLE: Pause/resume within task

## Example: Using Buttons in Tasks

```cpp
void Task1Plantation::execute() {
  if (!taskActive) return;
  
  // Custom button handling inside tasks
  if (pushButton.wasPressed(BTN_UP)) {
    // Increase planting speed
    plantingSpeed += 10;
    Serial.println("Planting speed increased!");
  }
  
  if (pushButton.wasPressed(BTN_MIDDLE)) {
    // Toggle precision mode
    precisionMode = !precisionMode;
    Serial.println("Precision mode toggled!");
  }
  
  // Your task logic continues...
}
```

## Button Detection Values

| Button | ADC Range | Typical Value |
|--------|-----------|---------------|
| UP     | 0-50      | ~25           |
| LEFT   | 1100-1200 | ~1150         |
| MIDDLE | 1750-1850 | ~1800         |
| RIGHT  | 2600-2750 | ~2675         |
| DOWN   | 3750-4020 | ~3885         |

## Troubleshooting

### Button Not Responding
1. Check GPIO 19 connection
2. Test with `BUTTONREAD` command
3. Verify analog values match ranges
4. Check for debounce delay (50ms)

### Wrong Button Detected
1. Use `BUTTONREAD` to check actual values
2. Update ranges in `PushButton.h` if needed
3. Check for voltage supply stability

### Emergency Stop Not Working
1. DOWN button has highest priority
2. Check button detection in serial monitor
3. Verify physical button connection

## Integration with State Machine

```
Button Press Flow:

DOWN pressed
    ↓
Emergency Stop
    ↓
Stop all tasks + motors
    ↓
Set EMERGENCY_STOP state
    ↓
Display message
    ↓
Wait for RESUME


RIGHT pressed
    ↓
Check state (not EMERGENCY/STANDBY)
    ↓
Stop current task
    ↓
Start next task
    ↓
Update state machine
    ↓
Display new task
```

## Safety Notes

⚠️ **Important**:
- DOWN button works at ANY time (highest priority)
- RIGHT button disabled during EMERGENCY_STOP
- Buttons use debouncing (50ms) to prevent accidental presses
- Physical button provides backup to serial commands

## Testing Checklist

- [ ] Test DOWN button - should trigger emergency stop
- [ ] Test RIGHT button - should cycle through tasks
- [ ] Verify OLED shows correct messages
- [ ] Check serial monitor for button press logs
- [ ] Test RESUME after emergency stop
- [ ] Verify UP/LEFT/MIDDLE available for tasks

---

**Quick Commands**:
- Serial: `BUTTONREAD` - Check button state
- Serial: `RESUME` - Resume after emergency stop
- Serial: `STATUS` - Check current state

**Last Updated**: November 12, 2025
