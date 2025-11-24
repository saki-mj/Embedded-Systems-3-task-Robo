/*********************************************************************
 * Task 4: Read Barcode Implementation
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task4_Barcode.h"
#include "Task5_Unloading.h"
#include "ReadBarcode.h"
#include "../Motors.h"
#include "../IRReading.h"
#include "../OLEDDisplay.h"
#include "../TOFSensors.h"
#include "../WallFollow.h"

Task4Barcode task4Barcode;

Task4Barcode::Task4Barcode() {
  currentSubState = T4_INIT;
  subStateStartTime = 0;
  taskActive = false;
  barcodeData = "";
  
  // Default configuration (all can be changed via serial commands)
  wallDetectionDistance = 200;  // 20cm in mm
  wallFollowDistance = 150;     // 15cm target for wall following
  wallLossThreshold = 300;      // 30cm - wall is lost if distance > this
  wallLossForwardDuration = 500; // 0.5 second forward after wall loss
  turnRightDuration = 1000;     // 1 second for right turn 90 degrees
  turnLeftDuration = 1000;      // 1 second for left turn 90 degrees
  straightDuration = 1000;      // 1 second for straight movement
  reverseDuration = 500;        // 0.5 second for reverse
  irWhiteThreshold = 2000;      // IR threshold: above=white(1), below=black(0)
}

void Task4Barcode::init() {
  Serial.println("=== Task 4: Barcode - Initializing ===");
  currentSubState = T4_INIT;
  subStateStartTime = millis();
  taskActive = false;
  barcodeData = "";
  
  // Initialize barcode reader with default settings
  barcodeReader.init();
  
  oledDisplay.show("Task 4", "Barcode", "Initialized");
  delay(1000);
}

void Task4Barcode::execute() {
  if (!taskActive) return;
  
  unsigned long currentTime = millis();
  
  switch(currentSubState) {
    case T4_INIT:
      Serial.println("Task 4: Initializing barcode reading");
      readTOFSensors();  // Start TOF sensor reading
      setSubState(T4_SEARCHING_WALL1);
      break;
      
    case T4_SEARCHING_WALL1:
      // Follow left wall using PD control and check front TOF sensor
      readTOFSensors();
      
      // Start wall following if not already active
      if (!wallFollow.isActive()) {
        Serial.print("Following left wall until front TOF reads ");
        Serial.print(wallDetectionDistance);
        Serial.println(" mm");
        wallFollow.start();
      }
      
      // Use wall following with left TOF sensor (uses global baseSpeed)
      wallFollow.setTargetDistance(wallFollowDistance);
      wallFollow.executeWallFollow(tofSensors.getLeftDistance());
      
      // Only change state when FRONT TOF detects wall
      if (tofSensors.getFrontDistance() <= wallDetectionDistance) {
        Serial.print("Wall 1 detected at ");
        Serial.print(tofSensors.getFrontDistance());
        Serial.println(" mm");
        wallFollow.stop();
        stopAllMotors();
        delay(300);
        setSubState(T4_TURNING_RIGHT1);
        turnStartTime = millis();
      }
      break;
      
    case T4_TURNING_RIGHT1:
      // Turn right 90 degrees
      {
        static bool printedOnce = false;
        if (!printedOnce) {
          Serial.print("Turning right 90° for ");
          Serial.print(turnRightDuration);
          Serial.println(" ms");
          printedOnce = true;
        }
        
        robotTurnRight();  // Uses global rotateSpeed
        
        if (isTurnComplete()) {
          stopAllMotors();
          delay(300);
          printedOnce = false;
          setSubState(T4_SEARCHING_WALL2);
        }
      }
      break;
      
    case T4_SEARCHING_WALL2:
      // Follow left wall until wall is lost
      readTOFSensors();
      
      // Start wall following if not already active
      if (!wallFollow.isActive()) {
        Serial.print("Following left wall until wall is lost (distance > ");
        Serial.print(wallLossThreshold);
        Serial.println(" mm)");
        wallFollow.start();
      }
      
      // Use wall following with left TOF sensor (uses global baseSpeed)
      wallFollow.setTargetDistance(wallFollowDistance);
      wallFollow.executeWallFollow(tofSensors.getLeftDistance());
      
      // Detect when left wall is lost
      if (tofSensors.getLeftDistance() > wallLossThreshold) {
        Serial.print("Left wall lost at ");
        Serial.print(tofSensors.getLeftDistance());
        Serial.println(" mm");
        wallFollow.stop();
        setSubState(T4_WALL_LOSS_FORWARD);
        wallLossForwardStartTime = millis();
      }
      break;
      
    case T4_WALL_LOSS_FORWARD:
      // Go forward for configured time after wall loss
      {
        static bool printedOnce = false;
        if (!printedOnce) {
          Serial.print("Moving forward for ");
          Serial.print(wallLossForwardDuration);
          Serial.println(" ms after wall loss");
          printedOnce = true;
        }
        
        robotForward();  // Uses global baseSpeed
        
        if (currentTime - wallLossForwardStartTime >= wallLossForwardDuration) {
          stopAllMotors();
          delay(300);
          printedOnce = false;
          setSubState(T4_TURNING_LEFT2);
          turnStartTime = millis();
        }
      }
      break;
      
    case T4_TURNING_LEFT2:
      // Turn left 90 degrees
      {
        static bool printedOnce = false;
        if (!printedOnce) {
          Serial.print("Turning left 90° for ");
          Serial.print(turnLeftDuration);
          Serial.println(" ms");
          printedOnce = true;
        }
        
        robotTurnLeft();  // Uses global rotateSpeed
        
        if (isTurnComplete()) {
          stopAllMotors();
          delay(300);
          printedOnce = false;
          setSubState(T4_MOVING_REVERSE);
          reverseStartTime = millis();
        }
      }
      break;
      
    case T4_MOVING_REVERSE:
      // Move reverse for configured time
      {
        static bool printedOnce = false;
        if (!printedOnce) {
          Serial.print("Moving reverse for ");
          Serial.print(reverseDuration);
          Serial.println(" ms");
          printedOnce = true;
        }
        
        robotBackward();  // Uses global baseSpeed
        
        if (currentTime - reverseStartTime >= reverseDuration) {
          stopAllMotors();
          delay(300);
          printedOnce = false;
          setSubState(T4_ALIGNING);
        }
      }
      break;
      
    case T4_ALIGNING:
      // Brief alignment phase
      Serial.println("Task 4: Preparing to read barcode");
      delay(500);
      
      // Initialize barcode reader
      barcodeReader.reset();
      barcodeReader.startReading();
      
      setSubState(T4_READING);
      break;
      
    case T4_READING:
      // Move forward at constant speed and read barcode using ReadBarcode class
      {
        static bool printedOnce = false;
        if (!printedOnce) {
          Serial.println("Reading barcode while moving forward...");
          printedOnce = true;
        }
        
        robotForward();  // Uses global baseSpeed
        
        // Process barcode reading
        barcodeReader.processReading();
        
        // Check if barcode reading is complete
        if (barcodeReader.isComplete()) {
          stopAllMotors();
          Serial.println("Barcode reading complete!");
          printedOnce = false;
          setSubState(T4_PROCESSING);
        }
      }
      break;
      
    case T4_PROCESSING: {
      Serial.println("Task 4: Processing barcode data");
      
      // Get barcode result from ReadBarcode class
      barcodeData = barcodeReader.getBinaryResult();
      int barcodeValue = barcodeReader.getDecimalValue();
      
      Serial.println("\n=== Barcode Processing Complete ===");
      Serial.print("Binary: ");
      Serial.println(barcodeData);
      Serial.print("Decimal: ");
      Serial.println(barcodeValue);
      
      // Store barcode but DO NOT start Task 5 automatically
      task5Unloading.setBarcodeValue((uint16_t)barcodeValue);
      task5Unloading.setBarcodeBinary(barcodeData);
      
      Serial.println("Task4: Barcode stored for Task5");
      Serial.print("  value = ");
      Serial.print(barcodeValue);
      Serial.print("  binary = ");
      Serial.println(barcodeData);
      Serial.println("Use TASK5 command to start unloading when ready");
      
      setSubState(T4_COMPLETED);
      break;
    }
      
    case T4_COMPLETED:
      Serial.println("Task 4: COMPLETED");
      Serial.print("Barcode Result: ");
      Serial.println(barcodeData);
      stopAllMotors();
      taskActive = false;
      break;
  }
}

// Removed - barcode reading now handled by ReadBarcode class

bool Task4Barcode::isTurnComplete() {
  // Use gyro or encoder feedback for accurate 90-degree turn detection
  // Example: Replace with actual gyro/encoder check
  // return (abs(gyro.getAngle() - turnStartAngle) >= 90);

  // Time-based turn detection using configurable threshold
  // Check which turn we're doing based on current state
  unsigned long requiredDuration;
  
  if (currentSubState == T4_TURNING_RIGHT1) {
    requiredDuration = turnRightDuration;
  } else if (currentSubState == T4_TURNING_LEFT2) {
    requiredDuration = turnLeftDuration;
  } else {
    requiredDuration = turnRightDuration;  // Default to right turn duration
  }
  
  return (millis() - turnStartTime >= requiredDuration);
}

void Task4Barcode::updateDisplay() {
  if (!taskActive) return;
  
  String line3 = "";
  if (barcodeData.length() > 0) {
    line3 = "Code: " + barcodeData;
  } else if (barcodeReader.isActive()) {
    // Show strips detected while reading
    line3 = "Strips: " + String(barcodeReader.getStripCount()) + "/4";
  } else {
    line3 = "Ready";
  }
  
  oledDisplay.show(
    "Task 4: Barcode",
    "State: " + getSubStateName(),
    line3
  );
}

void Task4Barcode::setSubState(Task4SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 4 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task4SubState Task4Barcode::getSubState() {
  return currentSubState;
}

String Task4Barcode::getSubStateName() {
  switch(currentSubState) {
    case T4_INIT: return "INIT";
    case T4_SEARCHING_WALL1: return "SEARCH_WALL1";
    case T4_TURNING_RIGHT1: return "TURN_RIGHT1";
    case T4_SEARCHING_WALL2: return "SEARCH_WALL2";
    case T4_WALL_LOSS_FORWARD: return "WALL_LOSS_FWD";
    case T4_TURNING_LEFT2: return "TURN_LEFT2";
    case T4_MOVING_REVERSE: return "MOVE_REVERSE";
    case T4_ALIGNING: return "ALIGNING";
    case T4_READING: return "READING";
    case T4_PROCESSING: return "PROCESSING";
    case T4_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task4Barcode::start() {
  Serial.println("Starting Task 4: Barcode");
  taskActive = true;
  setSubState(T4_INIT);
}

void Task4Barcode::stop() {
  Serial.println("Stopping Task 4: Barcode");
  taskActive = false;
  stopAllMotors();
}

bool Task4Barcode::isActive() {
  return taskActive;
}

bool Task4Barcode::isCompleted() {
  return (currentSubState == T4_COMPLETED);
}

void Task4Barcode::reset() {
  currentSubState = T4_INIT;
  subStateStartTime = millis();
  taskActive = false;
  barcodeData = "";
  barcodeReader.reset();
}

String Task4Barcode::getBarcodeData() {
  return barcodeData;
}

// Configuration setters
void Task4Barcode::setWallDetectionDistance(uint16_t distance) {
  wallDetectionDistance = distance;
  Serial.print("T4 Wall detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task4Barcode::setWallFollowDistance(uint16_t distance) {
  wallFollowDistance = distance;
  Serial.print("T4 Wall follow distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task4Barcode::setWallLossThreshold(uint16_t threshold) {
  wallLossThreshold = threshold;
  Serial.print("T4 Wall loss threshold set to: ");
  Serial.print(threshold);
  Serial.println(" mm");
}

void Task4Barcode::setWallLossForwardDuration(unsigned long timeMs) {
  wallLossForwardDuration = timeMs;
  Serial.print("T4 Wall loss forward duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task4Barcode::setBarcodeSpeed(uint16_t speed) {
  barcodeSpeed = speed;
  Serial.print("T4 Barcode reading speed set to: ");
  Serial.println(speed);
}

void Task4Barcode::setTurnRightDuration(unsigned long timeMs) {
  turnRightDuration = timeMs;
  Serial.print("T4 Turn RIGHT duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task4Barcode::setTurnLeftDuration(unsigned long timeMs) {
  turnLeftDuration = timeMs;
  Serial.print("T4 Turn LEFT duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task4Barcode::setStraightDuration(unsigned long timeMs) {
  straightDuration = timeMs;
  Serial.print("T4 Straight duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task4Barcode::setReverseDuration(unsigned long timeMs) {
  reverseDuration = timeMs;
  Serial.print("T4 Reverse duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task4Barcode::setIRWhiteThreshold(uint16_t threshold) {
  irWhiteThreshold = threshold;
  Serial.print("T4 IR white threshold set to: ");
  Serial.println(threshold);
}

// Configuration getters
uint16_t Task4Barcode::getWallDetectionDistance() {
  return wallDetectionDistance;
}

uint16_t Task4Barcode::getWallFollowDistance() {
  return wallFollowDistance;
}

uint16_t Task4Barcode::getWallLossThreshold() {
  return wallLossThreshold;
}

unsigned long Task4Barcode::getWallLossForwardDuration() {
  return wallLossForwardDuration;
}

unsigned long Task4Barcode::getTurnRightDuration() {
  return turnRightDuration;
}

unsigned long Task4Barcode::getTurnLeftDuration() {
  return turnLeftDuration;
}

unsigned long Task4Barcode::getStraightDuration() {
  return straightDuration;
}

unsigned long Task4Barcode::getReverseDuration() {
  return reverseDuration;
}

uint16_t Task4Barcode::getIRWhiteThreshold() {
  return irWhiteThreshold;
}
