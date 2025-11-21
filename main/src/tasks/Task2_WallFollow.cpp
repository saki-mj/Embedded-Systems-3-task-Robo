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
  stateMessagePrinted = false;
  
  // Default configuration (can be changed via serial commands)
  wallNearThreshold = 200;       // mm - distance to detect wall nearby
  wallFarThreshold = 150;        // mm - distance to detect wall ended
  cornerDelay = 1500;            // ms - delay after corner detected
  turnDuration = 3500;           // ms - duration for 90° turn
  targetWallDistance = 85;       // mm - target distance to maintain
  
  lastError = 0;
}

void Task2WallFollow::init() {
  Serial.println("=== Task 2: Wall Following - Initializing ===");
  currentSubState = T2_INIT;
  subStateStartTime = millis();
  taskActive = false;
  stateMessagePrinted = false;
  oledDisplay.show("Task 2", "Wall Follow", "Initialized");
  delay(1000);
}

void Task2WallFollow::execute() {
  if (!taskActive) return;
  
  // Read all TOF sensors at the start of execute
  tofSensors.readAll();
  
  switch(currentSubState) {
    case T2_INIT:
      if (!stateMessagePrinted) {
        Serial.println("Task 2: INIT - Starting right wall follow");
        stateMessagePrinted = true;
      }
      // Move to right wall following
      setSubState(T2_RIGHT_WALL_FOLLOW);
      break;
      
    case T2_RIGHT_WALL_FOLLOW: {
      if (!stateMessagePrinted) {
        Serial.println("Task 2: Following right wall until distance > 100mm");
        stateMessagePrinted = true;
      }
      
      uint16_t rightDist = tofSensors.getRightDistance();
      
      // Check if wall has ended (distance > wallFarThreshold)
      if (rightDist > wallFarThreshold) {
        stopAllMotors();
        setSubState(T2_RIGHT_CORNER_DELAY);
        break;
      }
      
      // Wall following with simple proportional control
      if (rightDist < wallNearThreshold) {
        // Too close to wall - turn slightly left
        setLeftMotorSpeed(getBaseSpeed() - 100);
        setRightMotorSpeed(getBaseSpeed());
        robotForward();
      } else if (rightDist > targetWallDistance + 10) {
        // Too far from wall - turn slightly right
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed() - 100);
        robotForward();
      } else {
        // Good distance - go straight
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed());
        robotForward();
      }
      break;
    }
      
    case T2_RIGHT_CORNER_DELAY:
      if (!stateMessagePrinted) {
        Serial.print("Task 2: Right corner detected - delaying for ");
        Serial.print(cornerDelay);
        Serial.println(" ms");
        stateMessagePrinted = true;
      }
      
      robotForward();
      
      if (millis() - subStateStartTime >= cornerDelay) {
        stopAllMotors();
        setSubState(T2_RIGHT_TURN);
      }
      break;
      
    case T2_RIGHT_TURN:
      if (!stateMessagePrinted) {
        Serial.print("Task 2: Turning right for ");
        Serial.print(turnDuration);
        Serial.println(" ms");
        stateMessagePrinted = true;
        robotTurnRight();
      }
      
      if (millis() - subStateStartTime >= turnDuration) {
        stopAllMotors();
        setSubState(T2_FORWARD_TO_FRONT_WALL);
      }
      break;
      
    case T2_FORWARD_TO_FRONT_WALL: {
      if (!stateMessagePrinted) {
        Serial.println("Task 2: Moving forward until front wall < 70mm");
        stateMessagePrinted = true;
      }
      
      robotForward();
      
      uint16_t frontDist = tofSensors.getFrontDistance();
      if (frontDist < wallNearThreshold) {
        stopAllMotors();
        setSubState(T2_FRONT_TURN);
      }
      break;
    }
      
    case T2_FRONT_TURN: {
      if (!stateMessagePrinted) {
        Serial.print("Task 2: Front wall detected - turning right for ");
        Serial.print(turnDuration);
        Serial.println(" ms");
        stateMessagePrinted = true;
        robotTurnRight();
      }
      
      if (millis() - subStateStartTime >= turnDuration) {
        stopAllMotors();
        setSubState(T2_LEFT_WALL_FOLLOW);
      }
      break;
    }
      
    case T2_LEFT_WALL_FOLLOW: {
      if (!stateMessagePrinted) {
        Serial.println("Task 2: Following left wall until distance > 100mm");
        stateMessagePrinted = true;
      }
      
      uint16_t leftDist = tofSensors.getLeftDistance();
      
      // Check if wall has ended (distance > wallFarThreshold)
      if (leftDist > wallFarThreshold) {
        stopAllMotors();
        setSubState(T2_LEFT_CORNER_DELAY);
        break;
      }
      
      // Wall following with simple proportional control
      if (leftDist < wallNearThreshold) {
        // Too close to wall - turn slightly right
        setLeftMotorSpeed(getBaseSpeed());
        setRightMotorSpeed(getBaseSpeed() - 100);
        robotForward();
      } else if (leftDist > targetWallDistance + 10) {
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
      break;
    }
      
    case T2_LEFT_CORNER_DELAY: {
      if (!stateMessagePrinted) {
        Serial.print("Task 2: Left corner detected - delaying for ");
        Serial.print(cornerDelay);
        Serial.println(" ms");
        stateMessagePrinted = true;
      }
      
      robotForward();
      
      if (millis() - subStateStartTime >= cornerDelay) {
        stopAllMotors();
        setSubState(T2_LEFT_TURN);
      }
      break;
    }
      
    case T2_LEFT_TURN: {
      if (!stateMessagePrinted) {
        Serial.print("Task 2: Turning left for ");
        Serial.print(turnDuration);
        Serial.println(" ms");
        stateMessagePrinted = true;
        robotTurnLeft();
      }
      
      if (millis() - subStateStartTime >= turnDuration) {
        stopAllMotors();
        setSubState(T2_FINAL_FORWARD);
      }
      break;
    }
      
    case T2_FINAL_FORWARD: {
      if (!stateMessagePrinted) {
        Serial.println("Task 2: Moving forward - task continuing");
        stateMessagePrinted = true;
      }
      
      robotForward();
      
      // You can add a condition here to complete the task
      // For now, it continues forward indefinitely
      // Uncomment below to auto-complete after 3 seconds:
      // if (millis() - subStateStartTime >= 3000) {
      //   stopAllMotors();
      //   setSubState(T2_COMPLETED);
      // }
      break;
    }
      
    case T2_COMPLETED: {
      if (!stateMessagePrinted) {
        Serial.println("Task 2: COMPLETED");
        stateMessagePrinted = true;
      }
      stopAllMotors();
      taskActive = false;
      break;
    }
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
    stateMessagePrinted = false;  // Reset flag for new state
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
    case T2_RIGHT_WALL_FOLLOW: return "RIGHT_WALL_FOLLOW";
    case T2_RIGHT_CORNER_DELAY: return "RIGHT_CORNER_DELAY";
    case T2_RIGHT_TURN: return "RIGHT_TURN";
    case T2_FORWARD_TO_FRONT_WALL: return "FORWARD_TO_FRONT";
    case T2_FRONT_TURN: return "FRONT_TURN";
    case T2_LEFT_WALL_FOLLOW: return "LEFT_WALL_FOLLOW";
    case T2_LEFT_CORNER_DELAY: return "LEFT_CORNER_DELAY";
    case T2_LEFT_TURN: return "LEFT_TURN";
    case T2_FINAL_FORWARD: return "FINAL_FORWARD";
    case T2_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task2WallFollow::start() {
  Serial.println("Starting Task 2: Wall Following");
  taskActive = true;
  stateMessagePrinted = false;
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
  stateMessagePrinted = false;
}

// Configuration setters
void Task2WallFollow::setWallNearThreshold(uint16_t distance) {
  wallNearThreshold = distance;
  Serial.print("T2 Wall near threshold set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task2WallFollow::setWallFarThreshold(uint16_t distance) {
  wallFarThreshold = distance;
  Serial.print("T2 Wall far threshold set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task2WallFollow::setCornerDelay(unsigned long timeMs) {
  cornerDelay = timeMs;
  Serial.print("T2 Corner delay set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task2WallFollow::setTurnDuration(unsigned long timeMs) {
  turnDuration = timeMs;
  Serial.print("T2 Turn duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task2WallFollow::setTargetWallDistance(uint16_t distance) {
  targetWallDistance = distance;
  Serial.print("T2 Target wall distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

// Configuration getters
uint16_t Task2WallFollow::getWallNearThreshold() {
  return wallNearThreshold;
}

uint16_t Task2WallFollow::getWallFarThreshold() {
  return wallFarThreshold;
}

unsigned long Task2WallFollow::getCornerDelay() {
  return cornerDelay;
}

unsigned long Task2WallFollow::getTurnDuration() {
  return turnDuration;
}

uint16_t Task2WallFollow::getTargetWallDistance() {
  return targetWallDistance;
}