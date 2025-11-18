/*********************************************************************
 * Task 3: Ramp Navigation Implementation
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task3_Ramp.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

Task3Ramp task3Ramp;

Task3Ramp::Task3Ramp() {
  currentSubState = T3_INIT;
  subStateStartTime = 0;
  taskActive = false;
  
  // Default configuration (can be changed via serial commands)
  approachSpeed = 65;
  climbSpeed = 90;  // Higher speed for climbing
  descendSpeed = 50;  // Lower speed for descending
  climbDuration = 3000;  // 3 seconds climb time
  descendDuration = 2500;  // 2.5 seconds descend time
  rampDetectionDistance = 100;  // mm
  topDetectionThreshold = 300;  // TOF reading at top
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

// Configuration setters
void Task3Ramp::setApproachSpeed(uint16_t speed) {
  approachSpeed = speed;
  Serial.print("T3 Approach speed set to: ");
  Serial.println(speed);
}

void Task3Ramp::setClimbSpeed(uint16_t speed) {
  climbSpeed = speed;
  Serial.print("T3 Climb speed set to: ");
  Serial.println(speed);
}

void Task3Ramp::setDescendSpeed(uint16_t speed) {
  descendSpeed = speed;
  Serial.print("T3 Descend speed set to: ");
  Serial.println(speed);
}

void Task3Ramp::setClimbDuration(unsigned long timeMs) {
  climbDuration = timeMs;
  Serial.print("T3 Climb duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task3Ramp::setDescendDuration(unsigned long timeMs) {
  descendDuration = timeMs;
  Serial.print("T3 Descend duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task3Ramp::setRampDetectionDistance(uint16_t distance) {
  rampDetectionDistance = distance;
  Serial.print("T3 Ramp detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task3Ramp::setTopDetectionThreshold(uint16_t threshold) {
  topDetectionThreshold = threshold;
  Serial.print("T3 Top detection threshold set to: ");
  Serial.print(threshold);
  Serial.println(" mm");
}

// Configuration getters
uint16_t Task3Ramp::getApproachSpeed() {
  return approachSpeed;
}

uint16_t Task3Ramp::getClimbSpeed() {
  return climbSpeed;
}

uint16_t Task3Ramp::getDescendSpeed() {
  return descendSpeed;
}

unsigned long Task3Ramp::getClimbDuration() {
  return climbDuration;
}

unsigned long Task3Ramp::getDescendDuration() {
  return descendDuration;
}

uint16_t Task3Ramp::getRampDetectionDistance() {
  return rampDetectionDistance;
}

uint16_t Task3Ramp::getTopDetectionThreshold() {
  return topDetectionThreshold;
}
