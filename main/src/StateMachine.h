/*********************************************************************
 * State Machine Library
 * Manages robot task states and transitions
 *********************************************************************/

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>

// Main robot states
enum RobotState {
  STATE_STANDBY,           // Initialization and calibration
  STATE_IDLE,              // Waiting for start command
  STATE_TASK1_PLANTATION,  // Plantation task
  STATE_TASK2_WALL_FOLLOW, // Wall following task
  STATE_TASK3_RAMP,        // Ramp task
  STATE_TASK4_BARCODE,     // Read barcode task
  STATE_TASK5_UNLOADING,   // Unloading balls task
  STATE_EMERGENCY_STOP     // Emergency stop state
};

// Operation mode
enum OperationMode {
  MODE_MANUAL,    // Manual task switching via commands
  MODE_AUTOMATIC  // Automatic task progression
};

class StateMachine {
  private:
    RobotState currentState;
    RobotState previousState;
    OperationMode currentMode;
    unsigned long stateStartTime;
    bool taskCompleted;

  public:
    // Constructor
    StateMachine();
    
    // Initialize state machine
    void begin();
    
    // Main state machine update (call in loop)
    void update();
    
    // State management
    void setState(RobotState newState);
    RobotState getState();
    RobotState getPreviousState();
    String getStateName(RobotState state);
    unsigned long getStateTime();
    
    // Mode management
    void setMode(OperationMode mode);
    OperationMode getMode();
    String getModeName();
    bool isManualMode();
    bool isAutomaticMode();
    
    // Task completion
    void markTaskCompleted();
    bool isTaskCompleted();
    void progressToNextTask();
    
    // Emergency stop
    void emergencyStop();
    void resumeFromStop();
    
    // State checks
    bool isStandby();
    bool isIdle();
    bool isRunningTask();
};

// Global state machine object
extern StateMachine stateMachine;

// Initialization function
void initStateMachine();

#endif
