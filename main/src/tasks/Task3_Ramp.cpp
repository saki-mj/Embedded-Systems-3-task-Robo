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
#include "../Gyroscope.h"
#include "../WallFollow.h"

Task3Ramp task3Ramp;

Task3Ramp::Task3Ramp() {
  currentSubState = T3_INIT;
  subStateStartTime = 0;
  taskActive = false;
  stateMessagePrinted = false;
  
  // Default configuration (can be changed via serial commands)
  climbDuration = 3000;  // 3 seconds climb time
  descendDuration = 2500;  // 2.5 seconds descend time
  rampDetectionDistance = 100;  // mm
  topDetectionThreshold = 300;  // TOF reading at top
  turnDuration = 1000;  // 1 second for 90° turn
}

void Task3Ramp::init() {
  Serial.println("=== Task 3: Ramp - Initializing ===");
  currentSubState = T3_INIT;
  subStateStartTime = millis();
  taskActive = false;
  stateMessagePrinted = false;
  oledDisplay.show("Task 3", "Ramp", "Initialized");
  delay(1000);
}

void Task3Ramp::execute() {
  if (!taskActive) return;
  
  // Read sensors with proper timing to avoid I2C bus conflicts
  // Read TOF sensors first (they use I2C mux)
  tofSensors.readAll();
  
  // Small delay before reading gyroscope on main I2C bus
  delay(10);
  
  // Read gyroscope
  gyroscope.read();
  float pitch = gyroscope.getPitch();
  
  switch(currentSubState) {
    case T3_INIT:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: INIT - Starting approach");
        stateMessagePrinted = true;
      }
      setSubState(T3_APPROACH);
      break;
      
    case T3_APPROACH:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: APPROACH - Going forward until pitch > +3 degrees");
        stateMessagePrinted = true;
      }
      
      // Go forward at base speed
      setLeftMotorSpeed(getBaseSpeed());
      setRightMotorSpeed(getBaseSpeed());
      robotForward();
      
      // Check if ramp is detected (pitch > +3 degrees)
      if (pitch > 3.0) {
        Serial.print("Ramp detected! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_CLIMBING);
      }
      break;
      
    case T3_CLIMBING:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: CLIMBING - Speed = 2x base speed until stable");
        stateMessagePrinted = true;
      }
      
      // Climb at 2x base speed
      setLeftMotorSpeed(getBaseSpeed() * 2);
      setRightMotorSpeed(getBaseSpeed() * 2);
      robotForward();
      
      // Check if reached top (pitch returns to stable, near 0 degrees)
      if (pitch >= -3.0 && pitch <= 3.0) {
        Serial.print("Top reached! Pitch stabilized: ");
        Serial.println(pitch);
        setSubState(T3_AT_TOP);
      }
      break;
      
    case T3_AT_TOP:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: AT_TOP - Going forward at base speed");
        stateMessagePrinted = true;
      }
      
      // Go forward at base speed
      setLeftMotorSpeed(getBaseSpeed());
      setRightMotorSpeed(getBaseSpeed());
      robotForward();
      
      // Check if descending (pitch < -3 degrees)
      if (pitch < -3.0) {
        Serial.print("Descending detected! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_DESCENDING);
      }
      break;
      
    case T3_DESCENDING:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: DESCENDING - Speed = 0.25x base speed until stable");
        stateMessagePrinted = true;
      }
      
      // Descend at 0.25x base speed (slower for control)
      setLeftMotorSpeed(getBaseSpeed() * 0.25);
      setRightMotorSpeed(getBaseSpeed() * 0.25);
      robotForward();
      
      // Check if stable again (pitch returns to near 0 degrees)
      if (pitch >= -3.0 && pitch <= 3.0) {
        Serial.print("Stable ground reached! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_AFTER_RAMP);
      }
      break;
      
    case T3_AFTER_RAMP: {
      if (!stateMessagePrinted) {
        Serial.println("Task 3: AFTER_RAMP - Going forward until front wall < 70mm");
        stateMessagePrinted = true;
      }
      
      // Go forward at base speed
      setLeftMotorSpeed(getBaseSpeed());
      setRightMotorSpeed(getBaseSpeed());
      robotForward();
      
      uint16_t frontDist = tofSensors.getFrontDistance();
      if (frontDist < 70) {
        Serial.print("Front wall detected at ");
        Serial.print(frontDist);
        Serial.println(" mm");
        stopAllMotors();
        setSubState(T3_TURN_RIGHT_90);
      }
      break;
    }
      
    case T3_TURN_RIGHT_90:
      if (!stateMessagePrinted) {
        Serial.print("Task 3: TURN_RIGHT_90 - Turning right for ");
        Serial.print(turnDuration);
        Serial.println(" ms");
        stateMessagePrinted = true;
        
        // Turn right: left motor forward, right motor backward
        setLeftMotorSpeed(getRotateSpeed());
        setRightMotorSpeed(getRotateSpeed());
        leftMotorForward();
        rightMotorBackward();
      }
      
      if (millis() - subStateStartTime >= turnDuration) {
        stopAllMotors();
        delay(100);  // Brief pause after turn
        setSubState(T3_WALL_FOLLOW_LEFT);
      }
      break;
      
    case T3_WALL_FOLLOW_LEFT: {
      if (!stateMessagePrinted) {
        Serial.println("Task 3: WALL_FOLLOW_LEFT - Following left wall for 1 second");
        stateMessagePrinted = true;
      }
      
      // Simple left wall following
      uint16_t leftDist = tofSensors.getLeftDistance();
      uint16_t targetDist = 85;  // Target distance from left wall
      
      if (leftDist < 70) {
        // Too close to wall - turn slightly right
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed() - 100);
        robotForward();
      } else if (leftDist > targetDist + 10) {
        // Too far from wall - turn slightly left
        setLeftMotorSpeed(getBaseSpeed() - 100);
        setRightMotorSpeed(getBaseSpeed());
        robotForward();
      } else {
        // Good distance - go straight
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed());
        robotForward();
      }
      
      // After 1 second, complete the task
      if (millis() - subStateStartTime >= 1000) {
        stopAllMotors();
        setSubState(T3_COMPLETED);
      }
      break;
    }
      
    case T3_COMPLETED:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: COMPLETED");
        stateMessagePrinted = true;
      }
      stopAllMotors();
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
    stateMessagePrinted = false;  // Reset flag for new state
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
    case T3_AT_TOP: return "AT_TOP";
    case T3_DESCENDING: return "DESCENDING";
    case T3_AFTER_RAMP: return "AFTER_RAMP";
    case T3_TURN_RIGHT_90: return "TURN_RIGHT_90";
    case T3_WALL_FOLLOW_LEFT: return "WALL_FOLLOW_LEFT";
    case T3_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task3Ramp::start() {
  Serial.println("Starting Task 3: Ramp");
  taskActive = true;
  stateMessagePrinted = false;
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
  stateMessagePrinted = false;
}

// Configuration setters
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

void Task3Ramp::setTurnDuration(unsigned long timeMs) {
  turnDuration = timeMs;
  Serial.print("T3 Turn duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

// Configuration getters
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

unsigned long Task3Ramp::getTurnDuration() {
  return turnDuration;
}
