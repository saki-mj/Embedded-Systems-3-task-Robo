/*********************************************************************
 * Task 2: Wall Following Implementation
 *********************************************************************/

#include "Task2_WallFollow.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

Task2WallFollow task2WallFollow;

Task2WallFollow::Task2WallFollow() {
  currentSubState = T2_INIT;
  subStateStartTime = 0;
  taskActive = false;
}

void Task2WallFollow::init() {
  Serial.println("=== Task 2: Wall Following - Initializing ===");
  currentSubState = T2_INIT;
  subStateStartTime = millis();
  taskActive = false;
  oledDisplay.show("Task 2", "Wall Follow", "Initialized");
  delay(1000);
}

void Task2WallFollow::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case T2_INIT:
      Serial.println("Task 2: INIT state");
      // Add your code here
      break;
      
    case T2_FIND_WALL:
      Serial.println("Task 2: FIND_WALL state");
      // Add your code here
      break;
      
    case T2_ALIGN:
      Serial.println("Task 2: ALIGN state");
      // Add your code here
      break;
      
    case T2_FOLLOWING:
      Serial.println("Task 2: FOLLOWING state");
      // Use TOF sensors for wall following
      break;
      
    case T2_CORNER_DETECTED:
      Serial.println("Task 2: CORNER_DETECTED state");
      // Add your code here
      break;
      
    case T2_COMPLETED:
      Serial.println("Task 2: COMPLETED");
      taskActive = false;
      break;
  }
}

void Task2WallFollow::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 2: Wall Follow",
    "State: " + getSubStateName(),
    "Time: " + String((millis() - subStateStartTime) / 1000) + "s"
  );
}

void Task2WallFollow::setSubState(Task2SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 2 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task2SubState Task2WallFollow::getSubState() {
  return currentSubState;
}

String Task2WallFollow::getSubStateName() {
  switch(currentSubState) {
    case T2_INIT: return "INIT";
    case T2_FIND_WALL: return "FIND_WALL";
    case T2_ALIGN: return "ALIGN";
    case T2_FOLLOWING: return "FOLLOWING";
    case T2_CORNER_DETECTED: return "CORNER_DETECTED";
    case T2_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task2WallFollow::start() {
  Serial.println("Starting Task 2: Wall Following");
  taskActive = true;
  setSubState(T2_INIT);
}

void Task2WallFollow::stop() {
  Serial.println("Stopping Task 2: Wall Following");
  taskActive = false;
  stopAllMotors();
}

bool Task2WallFollow::isActive() {
  return taskActive;
}

bool Task2WallFollow::isCompleted() {
  return (currentSubState == T2_COMPLETED);
}

void Task2WallFollow::reset() {
  currentSubState = T2_INIT;
  subStateStartTime = millis();
  taskActive = false;
}
