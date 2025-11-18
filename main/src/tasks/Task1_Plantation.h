/*********************************************************************
 * Task 1: Plantation Task
 * Description: [Add your task description here]
 *********************************************************************/

#ifndef TASK1_PLANTATION_H
#define TASK1_PLANTATION_H

#include <Arduino.h>

// Task 1 Sub-states (customize as needed)
enum Task1SubState {
  T1_INIT,
  T1_SEARCHING,
  T1_FOLLOWING,
  T1_TURNING,
  T1_LINE_FOLLOWING,
  T1_COLLECTING,
  T1_PLANTING,
  T1_COMPLETED
};

class Task1Plantation {
  private:
    Task1SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t searchSpeed;
    uint16_t followSpeed;
    uint16_t turnSpeed;
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
    void setSearchSpeed(uint16_t speed);
    void setFollowSpeed(uint16_t speed);
    void setTurnSpeed(uint16_t speed);
    void setTurnDuration(unsigned long timeMs);
    void setSearchDuration(unsigned long timeMs);
    void setCollectDuration(unsigned long timeMs);
    void setBallDetectionThreshold(uint16_t threshold);
    
    // Configuration getters
    uint16_t getSearchSpeed();
    uint16_t getFollowSpeed();
    uint16_t getTurnSpeed();
    unsigned long getTurnDuration();
    unsigned long getSearchDuration();
    unsigned long getCollectDuration();
    uint16_t getBallDetectionThreshold();
};

// Global task object
extern Task1Plantation task1Plantation;

#endif
