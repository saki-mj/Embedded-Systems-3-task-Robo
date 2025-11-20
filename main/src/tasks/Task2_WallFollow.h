/*********************************************************************
 * Task 2: Wall Following
 * Description: Right wall following until corner, then left wall following
 *********************************************************************/

#ifndef TASK2_WALLFOLLOW_H
#define TASK2_WALLFOLLOW_H

#include <Arduino.h>

// Task 2 Sub-states
enum Task2SubState {
  T2_INIT,
  T2_RIGHT_WALL_FOLLOW,      // Following right wall
  T2_RIGHT_CORNER_DELAY,     // Delay after right wall ends
  T2_RIGHT_TURN,             // Turn right 90 degrees
  T2_FORWARD_TO_FRONT_WALL,  // Go forward until front wall detected
  T2_FRONT_TURN,             // Turn right 90 degrees at front wall
  T2_LEFT_WALL_FOLLOW,       // Following left wall
  T2_LEFT_CORNER_DELAY,      // Delay after left wall ends
  T2_LEFT_TURN,              // Turn left 90 degrees
  T2_FINAL_FORWARD,          // Go forward after left turn
  T2_COMPLETED
};

class Task2WallFollow {
  private:
    Task2SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    bool stateMessagePrinted;  // Track if state message was printed
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t wallNearThreshold;    // Distance to detect wall nearby (70mm default)
    uint16_t wallFarThreshold;     // Distance to detect wall ended (100mm default)
    unsigned long cornerDelay;      // Delay after corner detected (1500ms default)
    unsigned long turnDuration;     // Duration for 90° turn (1000ms default)
    
    // PID-like control variables for smooth wall following
    int lastError;
    uint16_t targetWallDistance;   // Target distance to maintain from wall

  public:
    Task2WallFollow();
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task2SubState newSubState);
    Task2SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    
    // Configuration setters
    void setWallNearThreshold(uint16_t distance);
    void setWallFarThreshold(uint16_t distance);
    void setCornerDelay(unsigned long timeMs);
    void setTurnDuration(unsigned long timeMs);
    void setTargetWallDistance(uint16_t distance);
    
    // Configuration getters
    uint16_t getWallNearThreshold();
    uint16_t getWallFarThreshold();
    unsigned long getCornerDelay();
    unsigned long getTurnDuration();
    uint16_t getTargetWallDistance();
};

extern Task2WallFollow task2WallFollow;

#endif