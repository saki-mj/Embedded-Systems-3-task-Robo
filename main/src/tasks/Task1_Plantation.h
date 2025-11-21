/*********************************************************************
 * Task 1: Plantation Task
 * 4 vertical lines, 3 intersections per line, snake pattern.
 *********************************************************************/

#ifndef TASK1_PLANTATION_H
#define TASK1_PLANTATION_H

#include <Arduino.h>

// Task 1 Sub-states
enum Task1SubState {
  T1_INIT,              // Reset and start
  T1_SEARCHING,         // Move forward until first main line is found
  T1_BACKUP_AFTER_TURN, // Backup on a vertical line after a 90° turn
  T1_FOLLOWING,         // (legacy name, not used directly in execute)
  T1_TURNING,           // Timed 90° / 180° turns
  T1_LINE_FOLLOWING,    // Follow current vertical line (down and up)
  T1_MOVE_TO_NEXT_LINE, // Move horizontally on top corridor to next line
  T1_EXIT_FORWARD,      // Final 3000 ms forward after last line
  T1_COLLECTING,        // (Reserved / unused here)
  T1_PLANTING,          // (Reserved / unused here)
  T1_COMPLETED          // Task finished
};

class Task1Plantation {
  private:
    Task1SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    
    // Configuration parameters (can be changed via serial commands)
    unsigned long turnDuration;
    unsigned long searchDuration;
    unsigned long collectDuration;
    uint16_t ballDetectionThreshold;  // TOF or color sensor threshold

  public:
    // Constructor
    Task1Plantation();
    
    // Initialize task
    void init();
    
    // Execute task (call in loop)
    void execute();
    
    // Update OLED display
    void updateDisplay();
    
    // Sub-state management
    void setSubState(Task1SubState newSubState);
    Task1SubState getSubState();
    String getSubStateName();
    
    // Task control
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    
    // Reset task
    void reset();
    
    // Configuration setters
    void setTurnDuration(unsigned long timeMs);
    void setSearchDuration(unsigned long timeMs);
    void setCollectDuration(unsigned long timeMs);
    void setBallDetectionThreshold(uint16_t threshold);
    
    // Configuration getters
    unsigned long getTurnDuration();
    unsigned long getSearchDuration();
    unsigned long getCollectDuration();
    uint16_t getBallDetectionThreshold();
};

// Global task object
extern Task1Plantation task1Plantation;

// Tunable Task1 parameters (modifiable via Serial commands)
extern unsigned long T1_TURN_90_TIME_MS;
extern unsigned long T1_TURN_180_TIME_MS;
extern unsigned long T1_BACKUP_TIME_MS;
extern unsigned long T1_EXIT_FORWARD_TIME_MS;
extern int T1_INTERSECTION_WHITE_MIN;      // how many white sensors = "intersection"

// NEW: Task1 default speed level (1–12), used by SerialCommands.cpp
extern uint8_t T1_SPEED_LEVEL;   // Task1 default speed level (1–12)

// Timing setters (for Serial tuning) – free functions
void T1_setTurn90Time(unsigned long ms);
void T1_setTurn180Time(unsigned long ms);
void T1_setBackupTime(unsigned long ms);
void T1_setExitForwardTime(unsigned long ms);
void T1_setIntersectionWhiteMin(int min);

#endif


