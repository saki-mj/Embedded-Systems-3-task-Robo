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
#include "../IRReading.h"

Task3Ramp task3Ramp;

Task3Ramp::Task3Ramp() {
  currentSubState = T3_INIT;
  subStateStartTime = 0;
  taskActive = false;
  stateMessagePrinted = false;
  
  // Default configuration (can be changed via serial commands)
  climbDuration = 100;  // 3 seconds climb time
  descendDuration = 200;  // 2.5 seconds descend time
  rampDetectionDistance = 100;  // mm
  topDetectionThreshold = 300;  // TOF reading at top
  turnDuration = 2000;  //  0second for 90° turn
  stateChangeInterval = 25;  // 2 seconds minimum between state changes
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
        Serial.println("Task 3: APPROACH - Going forward at 2x base speed");
        stateMessagePrinted = true;
      }
      
      // Go forward at 2x base speed
      setLeftMotorSpeed(getBaseSpeed() * 2);
      setRightMotorSpeed(getBaseSpeed() * 2);
      robotForward();
      
      // Check if ramp is detected (pitch > +7 degrees) and minimum interval passed
      if (pitch > 7.0 && (millis() - subStateStartTime >= stateChangeInterval)) {
        Serial.print("Ramp detected! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_CLIMBING);
      }
      break;
      
    case T3_CLIMBING:
      if (!stateMessagePrinted) {
        Serial.println("Task 3: CLIMBING - Speed = 600 only when pitch > 8.0");
        stateMessagePrinted = true;
      }
      
      // Climb at high speed only when pitch > 8.0
      if (pitch > 8.0) {
        setMotorA(600, true);  // Left motor forward at high speed
        setMotorB(600, true);  // Right motor forward at high speed
      } else {
        // Immediately transition to AT_TOP when pitch drops below 8.0
        Serial.print("Top reached! Pitch dropped below 8.0: ");
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
      
      // Check if descending (pitch < -10 degrees) and minimum interval passed
      if (pitch < -10.0 && (millis() - subStateStartTime >= stateChangeInterval)) {
        Serial.print("Descending detected! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_DESCENDING);
      }
      break;
      
    case T3_DESCENDING: {
      if (!stateMessagePrinted) {
        Serial.println("Task 3: DESCENDING - Speed = 0.25x base speed until flat");
        stateMessagePrinted = true;
      }
      
      // Descend at 0.25x base speed (slower for control) - use setMotorA/B directly
      int descendSpeed = getBaseSpeed() * 0.25;
      setMotorA(descendSpeed, true);  // Left motor forward at reduced speed
      setMotorB(descendSpeed, true);  // Right motor forward at reduced speed
      
      // Immediately transition when pitch becomes flat (above -8 degrees)
      if (pitch > -8.0) {
        Serial.print("Flat ground detected! Pitch: ");
        Serial.println(pitch);
        setSubState(T3_AFTER_RAMP);
      }
      break;
    }
      
    case T3_AFTER_RAMP: {
      if (!stateMessagePrinted) {
        Serial.println("Task 3: AFTER_RAMP - Moving forward to IR detection area");
        stateMessagePrinted = true;
      }
      
      // Go forward at base speed
      setLeftMotorSpeed(getBaseSpeed()*0.7);
      setRightMotorSpeed(getBaseSpeed()*0.7);
      robotForward();
      
      // Wait for state change interval before moving to next state
      if (millis() - subStateStartTime >= stateChangeInterval) {
        stopAllMotors();
        setSubState(T3_IR_WHITE_DETECT);
      }
      break;
    }
    
    case T3_IR_WHITE_DETECT: {
      if (!stateMessagePrinted) {
        Serial.println("Task 3: IR_WHITE_DETECT - Reading IR sensors for white line");
        stateMessagePrinted = true;
      }
      
      // Go forward slowly while checking IR sensors
      setLeftMotorSpeed(getBaseSpeed());
      setRightMotorSpeed(getBaseSpeed());
      robotForward();
      
      // Read IR sensors and check for white detection
      readAllIRSensors();
      bool whiteDetected = false;
      
      // Check multiple IR sensors for white (assuming white = high value)
      for (int i = 0; i < 16; i++) {
        if (irValues[i] > 2000) {  // Adjust threshold as needed
          whiteDetected = true;
          break;
        }
      }
      
      if (whiteDetected && (millis() - subStateStartTime >= stateChangeInterval)) {
        Serial.println("White line detected on IR sensors");
        stopAllMotors();
        setSubState(T3_TURN_RIGHT_90_FIRST);
      }
      break;
    }
      
    case T3_TURN_RIGHT_90_FIRST:
      if (!stateMessagePrinted) {
        Serial.print("Task 3: TURN_RIGHT_90_FIRST - Turning right (right motor only) for ");
        Serial.print(turnDuration);
        Serial.println(" ms");
        stateMessagePrinted = true;
        
        // Turn right: only right motor backward, left motor stopped
        setLeftMotorSpeed(0);
        setRightMotorSpeed(getRotateSpeed());
        setMotorA(0, true);  // Stop left motor
        rightMotorBackward();
      }
      
      if (millis() - subStateStartTime >= turnDuration) {
        stopAllMotors();
        delay(100);  // Brief pause after turn
        setSubState(T3_FORWARD_TO_WALL);
      }
      break;
      
    case T3_FORWARD_TO_WALL: {
      static unsigned long lastCorrectionTime = 0;
      
      if (!stateMessagePrinted) {
        Serial.println("Task 3: FORWARD_TO_WALL - Moving forward with white line correction");
        stateMessagePrinted = true;
        lastCorrectionTime = 0;
      }
      
      // Read IR sensors to check for white line on sensors 0 or 1
      readAllIRSensors();
      bool whiteOnEdge = (irValues[0] > 2000) || (irValues[1] > 2000);
      
      // If white detected on edge sensors and enough time since last correction
      if (whiteOnEdge && (millis() - lastCorrectionTime > 100)) {
        Serial.println("White line detected on edge - correcting left");
        
        // Quick left correction: 0.2 * turnDuration
        unsigned long correctionTime = (unsigned long)(turnDuration * 0.2);
        
        setLeftMotorSpeed(getRotateSpeed());
        setRightMotorSpeed(getRotateSpeed());
        leftMotorForward();
        rightMotorBackward();
        
        delay(correctionTime);
        
        stopAllMotors();
        delay(50);
        
        lastCorrectionTime = millis();
      } else {
        // Go forward at base speed
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed());
        robotForward();
      }
      
      // Check front TOF sensor for wall detection
      uint16_t frontDist = tofSensors.getFrontDistance();
      if (frontDist < 150 && (millis() - subStateStartTime >= stateChangeInterval)) {
        Serial.print("Front wall detected at ");
        Serial.print(frontDist);
        Serial.println(" mm");
        stopAllMotors();
        lastCorrectionTime = 0;  // Reset for next time
        setSubState(T3_TURN_RIGHT_90_SECOND);
      }
      break;
    }
    
    case T3_TURN_RIGHT_90_SECOND:
      if (!stateMessagePrinted) {
        Serial.print("Task 3: TURN_RIGHT_90_SECOND - Turning right for ");
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
        Serial.println("Task 3: WALL_FOLLOW_LEFT - Following left wall with PD until white line detected");
        stateMessagePrinted = true;
      }
      
      // Use wall following with PD control
      if (!wallFollow.isActive()) {
        wallFollow.start();
      }
      
      uint16_t leftDist = tofSensors.getLeftDistance();
      wallFollow.setTargetDistance(85);  // 8.5cm from left wall
      wallFollow.executeWallFollow(leftDist);
      
      // Check IR sensors for white line detection
      readAllIRSensors();
      bool whiteDetected = false;
      
      for (int i = 0; i < 16; i++) {
        if (irValues[i] > 2000) {  // Adjust threshold as needed
          whiteDetected = true;
          break;
        }
      }
      
      // Complete task when white line detected after minimum interval
      if (whiteDetected && (millis() - subStateStartTime >= stateChangeInterval)) {
        Serial.println("White line detected - Task 3 complete");
        wallFollow.stop();
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
    case T3_IR_WHITE_DETECT: return "IR_WHITE_DETECT";
    case T3_TURN_RIGHT_90_FIRST: return "TURN_RIGHT_1";
    case T3_FORWARD_TO_WALL: return "FWD_TO_WALL";
    case T3_TURN_RIGHT_90_SECOND: return "TURN_RIGHT_2";
    case T3_WALL_FOLLOW_LEFT: return "WALL_FOLLOW";
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

void Task3Ramp::setStateChangeInterval(unsigned long timeMs) {
  stateChangeInterval = timeMs;
  Serial.print("T3 State change interval set to: ");
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

unsigned long Task3Ramp::getStateChangeInterval() {
  return stateChangeInterval;
}
