/*********************************************************************
 * Task 1: Plantation Task Implementation
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task1_Plantation.h"
#include "BallCollector.h"
#include "../Motors.h"
#include "../IRReading.h"
#include "../LineFollow.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"

// Global task object
Task1Plantation task1Plantation;

// Constructor
Task1Plantation::Task1Plantation() {
  currentSubState = T1_INIT;
  subStateStartTime = 0;
  taskActive = false;
  
  // Default configuration (can be changed via serial commands)
  searchSpeed = 60;
  followSpeed = 70;
  turnSpeed = 55;
  turnDuration = 1000;  // 1 second for 90° turn
  searchDuration = 5000;  // 5 seconds search time
  collectDuration = 2000;  // 2 seconds collection time
  ballDetectionThreshold = 100;  // Default threshold
}

// Initialize task
void Task1Plantation::init() {
  Serial.println("=== Task 1: Plantation - Initializing ===");
  currentSubState = T1_INIT;
  subStateStartTime = millis();
  taskActive = false;
  
  // Add your initialization code here
  // Example: Set initial motor speeds, calibrate sensors, etc.
  
  oledDisplay.show("Task 1", "Plantation", "Initialized");
  delay(1000);
}

// Execute task
void Task1Plantation::execute() {
  if (!taskActive) return;
  
  // Main task logic - customize based on your requirements
  switch(currentSubState) {
    case T1_INIT:
      // Initialization logic
      Serial.println("Task 1: INIT state");
      // Add your code here
      // Example: setSubState(T1_SEARCHING);
      break;
      
    case T1_SEARCHING:
      // Searching logic
      Serial.println("Task 1: SEARCHING state");
      // Add your code here
      break;
      
    case T1_FOLLOWING:
      // Following logic
      Serial.println("Task 1: FOLLOWING state");
      // Add your code here
      break;
      
    case T1_TURNING:
      // Turning logic
      Serial.println("Task 1: TURNING state");
      // Add your code here
      break;
      
    case T1_LINE_FOLLOWING:
      // Line following logic
      Serial.println("Task 1: LINE_FOLLOWING state");
      // You can use existing line follow functionality
      // executeLineFollow();
      break;
      
    case T1_COLLECTING:
      // Ball collecting logic
      Serial.println("Task 1: COLLECTING state");
      // Call ballCollector to collect the ball
      if (ballCollector.collectingBall()) {
        // When collection is complete, move to next state
        // Example: setSubState(T1_PLANTING);
        // or continue to next collection point
      }
      break;
      
    case T1_PLANTING:
      // Planting logic
      Serial.println("Task 1: PLANTING state");
      // Add your code here
      break;
      
    case T1_COMPLETED:
      // Task completed
      Serial.println("Task 1: COMPLETED");
      taskActive = false;
      break;
  }
}

// Update OLED display
void Task1Plantation::updateDisplay() {
  if (!taskActive) return;
  
  oledDisplay.show(
    "Task 1: Plantation",
    "State: " + getSubStateName(),
    "Time: " + String((millis() - subStateStartTime) / 1000) + "s"
  );
}

// Set sub-state
void Task1Plantation::setSubState(Task1SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    
    Serial.print("Task 1 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

// Get current sub-state
Task1SubState Task1Plantation::getSubState() {
  return currentSubState;
}

// Get sub-state name
String Task1Plantation::getSubStateName() {
  switch(currentSubState) {
    case T1_INIT: return "INIT";
    case T1_SEARCHING: return "SEARCHING";
    case T1_FOLLOWING: return "FOLLOWING";
    case T1_TURNING: return "TURNING";
    case T1_LINE_FOLLOWING: return "LINE_FOLLOWING";
    case T1_COLLECTING: return "COLLECTING";
    case T1_PLANTING: return "PLANTING";
    case T1_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

// Start task
void Task1Plantation::start() {
  Serial.println("Starting Task 1: Plantation");
  taskActive = true;
  setSubState(T1_INIT);
}

// Stop task
void Task1Plantation::stop() {
  Serial.println("Stopping Task 1: Plantation");
  taskActive = false;
  stopAllMotors();
}

// Check if active
bool Task1Plantation::isActive() {
  return taskActive;
}

// Check if completed
bool Task1Plantation::isCompleted() {
  return (currentSubState == T1_COMPLETED);
}

// Reset task
void Task1Plantation::reset() {
  currentSubState = T1_INIT;
  subStateStartTime = millis();
  taskActive = false;

    
  
}

// Configuration setters
void Task1Plantation::setSearchSpeed(uint16_t speed) {
  searchSpeed = speed;
  Serial.print("T1 Search speed set to: ");
  Serial.println(speed);
}

void Task1Plantation::setFollowSpeed(uint16_t speed) {
  followSpeed = speed;
  Serial.print("T1 Follow speed set to: ");
  Serial.println(speed);
}

void Task1Plantation::setTurnSpeed(uint16_t speed) {
  turnSpeed = speed;
  Serial.print("T1 Turn speed set to: ");
  Serial.println(speed);
}

void Task1Plantation::setTurnDuration(unsigned long timeMs) {
  turnDuration = timeMs;
  Serial.print("T1 Turn duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setSearchDuration(unsigned long timeMs) {
  searchDuration = timeMs;
  Serial.print("T1 Search duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setCollectDuration(unsigned long timeMs) {
  collectDuration = timeMs;
  Serial.print("T1 Collect duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setBallDetectionThreshold(uint16_t threshold) {
  ballDetectionThreshold = threshold;
  Serial.print("T1 Ball detection threshold set to: ");
  Serial.println(threshold);
}

// Configuration getters
uint16_t Task1Plantation::getSearchSpeed() {
  return searchSpeed;
}

uint16_t Task1Plantation::getFollowSpeed() {
  return followSpeed;
}

uint16_t Task1Plantation::getTurnSpeed() {
  return turnSpeed;
}

unsigned long Task1Plantation::getTurnDuration() {
  return turnDuration;
}

unsigned long Task1Plantation::getSearchDuration() {
  return searchDuration;
}

unsigned long Task1Plantation::getCollectDuration() {
  return collectDuration;
}

uint16_t Task1Plantation::getBallDetectionThreshold() {
  return ballDetectionThreshold;
}
