/*********************************************************************
 * Task 3: Ramp Navigation Implementation
 *********************************************************************/

#include "Task3_Ramp.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

Task3Ramp task3Ramp;

Task3Ramp::Task3Ramp() {
  currentSubState = T3_INIT;
  subStateStartTime = 0;
  taskActive = false;
}

void Task3Ramp::init() {
  Serial.println("=== Task 3: Ramp - Initializing ===");
  currentSubState = T3_INIT;
  subStateStartTime = millis();
  taskActive = false;
  oledDisplay.show("Task 3", "Ramp", "Initialized");
  delay(1000);
}

void Task3Ramp::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case T3_INIT:
      Serial.println("Task 3: INIT state");
      // Add your code here
      break;
      
    case T3_APPROACH:
      Serial.println("Task 3: APPROACH state");
      // Add your code here
      break;
      
    case T3_CLIMBING:
      Serial.println("Task 3: CLIMBING state");
      // Add your code here
      break;
      
    case T3_DESCENDING:
      Serial.println("Task 3: DESCENDING state");
      // Add your code here
      break;
      
    case T3_COMPLETED:
      Serial.println("Task 3: COMPLETED");
      taskActive = false;
      break;
  }
}

void Task3Ramp::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 3: Ramp",
    "State: " + getSubStateName(),
    "Time: " + String((millis() - subStateStartTime) / 1000) + "s"
  );
}

void Task3Ramp::setSubState(Task3SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 3 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task3SubState Task3Ramp::getSubState() {
  return currentSubState;
}

String Task3Ramp::getSubStateName() {
  switch(currentSubState) {
    case T3_INIT: return "INIT";
    case T3_APPROACH: return "APPROACH";
    case T3_CLIMBING: return "CLIMBING";
    case T3_DESCENDING: return "DESCENDING";
    case T3_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task3Ramp::start() {
  Serial.println("Starting Task 3: Ramp");
  taskActive = true;
  setSubState(T3_INIT);
}

void Task3Ramp::stop() {
  Serial.println("Stopping Task 3: Ramp");
  taskActive = false;
  stopAllMotors();
}

bool Task3Ramp::isActive() {
  return taskActive;
}

bool Task3Ramp::isCompleted() {
  return (currentSubState == T3_COMPLETED);
}

void Task3Ramp::reset() {
  currentSubState = T3_INIT;
  subStateStartTime = millis();
  taskActive = false;
}
