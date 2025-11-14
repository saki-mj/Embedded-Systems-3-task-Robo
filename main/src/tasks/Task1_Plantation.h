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
};

// Global task object
extern Task1Plantation task1Plantation;

#endif
