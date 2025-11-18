/*********************************************************************
 * Task 2: Wall Following Implementation
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task2_WallFollow.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

Task2WallFollow task2WallFollow;

Task2WallFollow::Task2WallFollow() {
  currentSubState = T2_INIT;
  subStateStartTime = 0;
  taskActive = false;
  
  // Default configuration (can be changed via serial commands)
  approachSpeed = 70;
  followSpeed = 65;
  turnSpeed = 55;
  wallDetectionDistance = 150;  // mm
  targetWallDistance = 120;  // mm
  turnDuration = 1000;  // 1 second for 90° turn
  alignDuration = 2000;  // 2 seconds alignment
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

// Configuration setters
void Task2WallFollow::setApproachSpeed(uint16_t speed) {
  approachSpeed = speed;
  Serial.print("T2 Approach speed set to: ");
  Serial.println(speed);
}

void Task2WallFollow::setFollowSpeed(uint16_t speed) {
  followSpeed = speed;
  Serial.print("T2 Follow speed set to: ");
  Serial.println(speed);
}

void Task2WallFollow::setTurnSpeed(uint16_t speed) {
  turnSpeed = speed;
  Serial.print("T2 Turn speed set to: ");
  Serial.println(speed);
}

void Task2WallFollow::setWallDetectionDistance(uint16_t distance) {
  wallDetectionDistance = distance;
  Serial.print("T2 Wall detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task2WallFollow::setTargetWallDistance(uint16_t distance) {
  targetWallDistance = distance;
  Serial.print("T2 Target wall distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task2WallFollow::setTurnDuration(unsigned long timeMs) {
  turnDuration = timeMs;
  Serial.print("T2 Turn duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task2WallFollow::setAlignDuration(unsigned long timeMs) {
  alignDuration = timeMs;
  Serial.print("T2 Align duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

// Configuration getters
uint16_t Task2WallFollow::getApproachSpeed() {
  return approachSpeed;
}

uint16_t Task2WallFollow::getFollowSpeed() {
  return followSpeed;
}

uint16_t Task2WallFollow::getTurnSpeed() {
  return turnSpeed;
}

uint16_t Task2WallFollow::getWallDetectionDistance() {
  return wallDetectionDistance;
}

uint16_t Task2WallFollow::getTargetWallDistance() {
  return targetWallDistance;
}

unsigned long Task2WallFollow::getTurnDuration() {
  return turnDuration;
}

unsigned long Task2WallFollow::getAlignDuration() {
  return alignDuration;
}
