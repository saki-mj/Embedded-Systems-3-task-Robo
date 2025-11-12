/*********************************************************************
 * State Machine Library Implementation
 *********************************************************************/

#include "StateMachine.h"
#include "OLEDDisplay.h"

// Global state machine object
StateMachine stateMachine;

// Constructor
StateMachine::StateMachine() {
  currentState = STATE_STANDBY;
  previousState = STATE_STANDBY;
  currentMode = MODE_MANUAL;
  stateStartTime = 0;
  taskCompleted = false;
}

// Initialize state machine
void StateMachine::begin() {
  currentState = STATE_STANDBY;
  previousState = STATE_STANDBY;
  currentMode = MODE_MANUAL;
  stateStartTime = millis();
  taskCompleted = false;
  
  Serial.println("State Machine initialized");
  Serial.print("Mode: ");
  Serial.println(getModeName());
}

// Main state machine update
void StateMachine::update() {
  // Automatic mode task progression
  if (currentMode == MODE_AUTOMATIC && taskCompleted) {
    progressToNextTask();
    taskCompleted = false;
  }
}

// Set state
void StateMachine::setState(RobotState newState) {
  if (currentState != newState) {
    previousState = currentState;
    currentState = newState;
    stateStartTime = millis();
    taskCompleted = false;
    
    Serial.print("State changed: ");
    Serial.print(getStateName(previousState));
    Serial.print(" -> ");
    Serial.println(getStateName(currentState));
    
    // Update OLED
    oledDisplay.show("State Change", getStateName(currentState));
    delay(1000);
  }
}

// Get current state
RobotState StateMachine::getState() {
  return currentState;
}

// Get previous state
RobotState StateMachine::getPreviousState() {
  return previousState;
}

// Get state name as string
String StateMachine::getStateName(RobotState state) {
  switch(state) {
    case STATE_STANDBY: return "STANDBY";
    case STATE_IDLE: return "IDLE";
    case STATE_TASK1_PLANTATION: return "TASK1_PLANTATION";
    case STATE_TASK2_WALL_FOLLOW: return "TASK2_WALL_FOLLOW";
    case STATE_TASK3_RAMP: return "TASK3_RAMP";
    case STATE_TASK4_BARCODE: return "TASK4_BARCODE";
    case STATE_TASK5_UNLOADING: return "TASK5_UNLOADING";
    case STATE_EMERGENCY_STOP: return "EMERGENCY_STOP";
    default: return "UNKNOWN";
  }
}

// Get time in current state (ms)
unsigned long StateMachine::getStateTime() {
  return millis() - stateStartTime;
}

// Set operation mode
void StateMachine::setMode(OperationMode mode) {
  if (currentMode != mode) {
    currentMode = mode;
    Serial.print("Mode changed to: ");
    Serial.println(getModeName());
    oledDisplay.show("Mode Change", getModeName());
    delay(1000);
  }
}

// Get current mode
OperationMode StateMachine::getMode() {
  return currentMode;
}

// Get mode name as string
String StateMachine::getModeName() {
  return (currentMode == MODE_MANUAL) ? "MANUAL" : "AUTOMATIC";
}

// Check if manual mode
bool StateMachine::isManualMode() {
  return currentMode == MODE_MANUAL;
}

// Check if automatic mode
bool StateMachine::isAutomaticMode() {
  return currentMode == MODE_AUTOMATIC;
}

// Mark current task as completed
void StateMachine::markTaskCompleted() {
  taskCompleted = true;
  Serial.print("Task completed: ");
  Serial.println(getStateName(currentState));
}

// Check if task is completed
bool StateMachine::isTaskCompleted() {
  return taskCompleted;
}

// Progress to next task (automatic mode)
void StateMachine::progressToNextTask() {
  Serial.println("Progressing to next task...");
  
  switch(currentState) {
    case STATE_IDLE:
      setState(STATE_TASK1_PLANTATION);
      break;
    case STATE_TASK1_PLANTATION:
      setState(STATE_TASK2_WALL_FOLLOW);
      break;
    case STATE_TASK2_WALL_FOLLOW:
      setState(STATE_TASK3_RAMP);
      break;
    case STATE_TASK3_RAMP:
      setState(STATE_TASK4_BARCODE);
      break;
    case STATE_TASK4_BARCODE:
      setState(STATE_TASK5_UNLOADING);
      break;
    case STATE_TASK5_UNLOADING:
      Serial.println("All tasks completed!");
      setState(STATE_IDLE);
      break;
    default:
      break;
  }
}

// Emergency stop
void StateMachine::emergencyStop() {
  Serial.println("!!! EMERGENCY STOP !!!");
  setState(STATE_EMERGENCY_STOP);
  oledDisplay.show("EMERGENCY", "STOP!");
}

// Resume from emergency stop
void StateMachine::resumeFromStop() {
  if (currentState == STATE_EMERGENCY_STOP) {
    Serial.println("Resuming from emergency stop");
    setState(previousState);
  }
}

// Check if in standby
bool StateMachine::isStandby() {
  return currentState == STATE_STANDBY;
}

// Check if idle
bool StateMachine::isIdle() {
  return currentState == STATE_IDLE;
}

// Check if running a task
bool StateMachine::isRunningTask() {
  return (currentState >= STATE_TASK1_PLANTATION && 
          currentState <= STATE_TASK5_UNLOADING);
}

// Initialization function
void initStateMachine() {
  stateMachine.begin();
}
