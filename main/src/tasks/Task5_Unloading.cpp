/*********************************************************************
 * Task 5: Unloading Balls Implementation
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task5_Unloading.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

Task5Unloading task5Unloading;

Task5Unloading::Task5Unloading() {
  currentSubState = T5_INIT;
  subStateStartTime = 0;
  taskActive = false;
  ballsUnloaded = 0;
  
  // Default configuration (can be changed via serial commands)
  unloadDuration = 3000;  // 3 seconds to unload balls
  alignDuration = 1500;  // 1.5 seconds alignment
  zoneDetectionDistance = 150;  // mm
  targetBallCount = 3;  // Expected number of balls
}

void Task5Unloading::init() {
  Serial.println("=== Task 5: Unloading - Initializing ===");
  currentSubState = T5_INIT;
  subStateStartTime = millis();
  taskActive = false;
  ballsUnloaded = 0;
  oledDisplay.show("Task 5", "Unloading", "Initialized");
  delay(1000);
}

void Task5Unloading::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case T5_INIT:
      Serial.println("Task 5: INIT state");
      // Add your code here
      break;
      
    case T5_NAVIGATE_TO_ZONE:
      Serial.println("Task 5: NAVIGATE_TO_ZONE state");
      // Add your code here
      break;
      
    case T5_ALIGN:
      Serial.println("Task 5: ALIGN state");
      // Add your code here
      break;
      
    case T5_UNLOADING:
      Serial.println("Task 5: UNLOADING state");
      // Add unloading mechanism control here
      break;
      
    case T5_VERIFY:
      Serial.println("Task 5: VERIFY state");
      // Verify unloading completed
      break;
      
    case T5_COMPLETED:
      Serial.println("Task 5: COMPLETED");
      Serial.print("Balls Unloaded: ");
      Serial.println(ballsUnloaded);
      taskActive = false;
      break;
  }
}

void Task5Unloading::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 5: Unloading",
    "State: " + getSubStateName(),
    "Balls: " + String(ballsUnloaded)
  );
}

void Task5Unloading::setSubState(Task5SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 5 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task5SubState Task5Unloading::getSubState() {
  return currentSubState;
}

String Task5Unloading::getSubStateName() {
  switch(currentSubState) {
    case T5_INIT: return "INIT";
    case T5_NAVIGATE_TO_ZONE: return "NAVIGATE";
    case T5_ALIGN: return "ALIGN";
    case T5_UNLOADING: return "UNLOADING";
    case T5_VERIFY: return "VERIFY";
    case T5_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task5Unloading::start() {
  Serial.println("Starting Task 5: Unloading");
  taskActive = true;
  setSubState(T5_INIT);
}

void Task5Unloading::stop() {
  Serial.println("Stopping Task 5: Unloading");
  taskActive = false;
  stopAllMotors();
}

bool Task5Unloading::isActive() {
  return taskActive;
}

bool Task5Unloading::isCompleted() {
  return (currentSubState == T5_COMPLETED);
}

void Task5Unloading::reset() {
  currentSubState = T5_INIT;
  subStateStartTime = millis();
  taskActive = false;
  ballsUnloaded = 0;
}

int Task5Unloading::getBallsUnloaded() {
  return ballsUnloaded;
}

// Configuration setters
void Task5Unloading::setUnloadDuration(unsigned long timeMs) {
  unloadDuration = timeMs;
  Serial.print("T5 Unload duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task5Unloading::setAlignDuration(unsigned long timeMs) {
  alignDuration = timeMs;
  Serial.print("T5 Align duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task5Unloading::setZoneDetectionDistance(uint16_t distance) {
  zoneDetectionDistance = distance;
  Serial.print("T5 Zone detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task5Unloading::setTargetBallCount(uint16_t count) {
  targetBallCount = count;
  Serial.print("T5 Target ball count set to: ");
  Serial.println(count);
}

// Configuration getters
unsigned long Task5Unloading::getUnloadDuration() {
  return unloadDuration;
}

unsigned long Task5Unloading::getAlignDuration() {
  return alignDuration;
}

uint16_t Task5Unloading::getZoneDetectionDistance() {
  return zoneDetectionDistance;
}

uint16_t Task5Unloading::getTargetBallCount() {
  return targetBallCount;
}
