/*********************************************************************
 * Task 4: Read Barcode Implementation
 *********************************************************************/

#include "Task4_Barcode.h"
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
  barcodeSpeed = 50;            // Moderate speed for reading
  approachSpeed = 70;           // Speed while navigating
  turnSpeed = 60;               // Speed while turning
  turnRightDuration = 1000;     // 1 second for right turn 90 degrees
  turnLeftDuration = 1000;      // 1 second for left turn 90 degrees
  straightDuration = 1000;      // 1 second for straight movement
  reverseDuration = 500;        // 0.5 second for reverse
  irWhiteThreshold = 2000;      // IR threshold: above=white(1), below=black(0)
  
  // Initialize barcode reading
  for(int i = 0; i < 4; i++) {
    barcodeBits[i] = -1;
  }
  for(int i = 0; i < 16; i++) {
    barcodeCounts[i] = 0;
  }
  barcodeReadComplete = false;
}

void Task4Barcode::init() {
  Serial.println("=== Task 4: Barcode - Initializing ===");
  currentSubState = T4_INIT;
  subStateStartTime = millis();
  taskActive = false;
  barcodeData = "";
  for(int i = 0; i < 4; i++) {
    barcodeBits[i] = -1;
  }
  for(int i = 0; i < 16; i++) {
    barcodeCounts[i] = 0;
  }
  barcodeReadComplete = false;
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
        wallFollow.start();
      }
      
      // Use wall following with left TOF sensor
      wallFollow.setTargetDistance(wallFollowDistance);
      wallFollow.setBaseSpeed(approachSpeed);
      wallFollow.executeWallFollow(tofSensors.getLeftDistance());
      
      // Only change state when FRONT TOF detects wall
      if (tofSensors.getFrontDistance() <= wallDetectionDistance) {
        Serial.print("Wall 1 detected at: ");
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
      setCurrentSpeed(turnSpeed);
      robotTurnRight();
      
      if (isTurnComplete()) {
        stopAllMotors();
        delay(300);
        setSubState(T4_SEARCHING_WALL2);
      }
      break;
      
    case T4_SEARCHING_WALL2:
      // Follow left wall using PD control and check front TOF sensor
      readTOFSensors();
      
      // Start wall following if not already active
      if (!wallFollow.isActive()) {
        wallFollow.start();
      }
      
      // Use wall following with left TOF sensor
      wallFollow.setTargetDistance(wallFollowDistance);
      wallFollow.setBaseSpeed(approachSpeed);
      wallFollow.executeWallFollow(tofSensors.getLeftDistance());
      
      // Only change state when FRONT TOF detects wall
      if (tofSensors.getFrontDistance() <= wallDetectionDistance) {
        Serial.print("Wall 2 detected at: ");
        Serial.print(tofSensors.getFrontDistance());
        Serial.println(" mm");
        wallFollow.stop();
        stopAllMotors();
        delay(300);
        setSubState(T4_TURNING_LEFT2);
        turnStartTime = millis();
      }
      break;
      
    case T4_TURNING_LEFT2:
      // Turn left 90 degrees
      setCurrentSpeed(turnSpeed);
      robotTurnLeft();
      
      if (isTurnComplete()) {
        stopAllMotors();
        delay(300);
        setSubState(T4_MOVING_REVERSE);
        reverseStartTime = millis();
      }
      break;
      
    case T4_MOVING_REVERSE:
      // Move reverse for configured time
      setCurrentSpeed(approachSpeed);
      robotBackward();
      
      if (currentTime - reverseStartTime >= reverseDuration) {
        stopAllMotors();
        delay(300);
        setSubState(T4_ALIGNING);
      }
      break;
      
    case T4_ALIGNING:
      // Brief alignment phase
      Serial.println("Task 4: Preparing to read barcode");
      delay(500);
      for(int i = 0; i < 4; i++) {
        barcodeBits[i] = -1;
      }
      for(int i = 0; i < 16; i++) {
        barcodeCounts[i] = 0;
      }
      barcodeReadComplete = false;
      setSubState(T4_READING);
      break;
      
    case T4_READING:
      // Move forward at constant speed and read barcode
      setCurrentSpeed(barcodeSpeed);
      robotForward();
      readBarcodeBar();
      
      // Check if barcode reading is complete
      if (barcodeReadComplete) {
        stopAllMotors();
        Serial.println("Barcode reading complete!");
        setSubState(T4_PROCESSING);
      }
      break;
      
    case T4_PROCESSING:
      Serial.println("Task 4: Processing barcode data");
      processBarcodeData();
      setSubState(T4_COMPLETED);
      break;
      
    case T4_COMPLETED:
      Serial.println("Task 4: COMPLETED");
      Serial.print("Barcode Result: ");
      Serial.println(barcodeData);
      stopAllMotors();
      taskActive = false;
      break;
  }
}

void Task4Barcode::readBarcodeBar() {
  // Read all IR sensors
  readAllIRSensors();
  
  // Read barcode using sensors 6, 7, 8, 9 (indices 6, 7, 8, 9)
  // Each sensor reads one bar independently
  // IR value > threshold = white = 1
  // IR value <= threshold = black = 0
  
  // Sample interval to avoid reading too fast
  static unsigned long lastReadTime = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastReadTime < 100) {  // Read every 100ms
    return;
  }
  lastReadTime = currentTime;
  
  // Read current barcode value from 4 sensors
  int currentBarcode = 0;
  
  // Sensor 6 = bit 3 (MSB)
  if (irValues[6] > irWhiteThreshold) {
    currentBarcode |= (1 << 3);  // Set bit 3
  }
  
  // Sensor 7 = bit 2
  if (irValues[7] > irWhiteThreshold) {
    currentBarcode |= (1 << 2);  // Set bit 2
  }
  
  // Sensor 8 = bit 1
  if (irValues[8] > irWhiteThreshold) {
    currentBarcode |= (1 << 1);  // Set bit 1
  }
  
  // Sensor 9 = bit 0 (LSB)
  if (irValues[9] > irWhiteThreshold) {
    currentBarcode |= (1 << 0);  // Set bit 0
  }
  
  // Increment count for this barcode value
  barcodeCounts[currentBarcode]++;
  
  // Debug output
  Serial.print("Barcode reading: ");
  Serial.print(currentBarcode, BIN);
  Serial.print(" (");
  Serial.print(currentBarcode);
  Serial.print(") - IR[6-9]: ");
  Serial.print(irValues[6]);
  Serial.print(", ");
  Serial.print(irValues[7]);
  Serial.print(", ");
  Serial.print(irValues[8]);
  Serial.print(", ");
  Serial.println(irValues[9]);
  
  // Check if we have enough samples (e.g., 20 readings)
  int totalSamples = 0;
  for (int i = 0; i < 16; i++) {
    totalSamples += barcodeCounts[i];
  }
  
  if (totalSamples >= 20) {
    barcodeReadComplete = true;
  }
}

void Task4Barcode::processBarcodeData() {
  // Find the barcode value that appears most frequently
  int maxCount = 0;
  int mostFrequentBarcode = 0;
  
  for (int i = 0; i < 16; i++) {
    Serial.print("Barcode ");
    Serial.print(i);
    Serial.print(" (");
    Serial.print(i, BIN);
    Serial.print("): ");
    Serial.print(barcodeCounts[i]);
    Serial.println(" samples");
    
    if (barcodeCounts[i] > maxCount) {
      maxCount = barcodeCounts[i];
      mostFrequentBarcode = i;
    }
  }
  
  // Convert the most frequent barcode to binary string
  barcodeData = "";
  for (int i = 3; i >= 0; i--) {
    if (mostFrequentBarcode & (1 << i)) {
      barcodeData += "1";
    } else {
      barcodeData += "0";
    }
  }
  
  Serial.println("\n=== Barcode Processing Complete ===");
  Serial.print("Most Frequent Barcode: ");
  Serial.print(mostFrequentBarcode);
  Serial.print(" (appeared ");
  Serial.print(maxCount);
  Serial.println(" times)");
  Serial.print("Binary: ");
  Serial.println(barcodeData);
  Serial.print("Decimal: ");
  Serial.println(mostFrequentBarcode);
}

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
  } else {
    // Show total samples collected
    int totalSamples = 0;
    for (int i = 0; i < 16; i++) {
      totalSamples += barcodeCounts[i];
    }
    line3 = "Samples: " + String(totalSamples);
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
  for(int i = 0; i < 4; i++) {
    barcodeBits[i] = -1;
  }
  for(int i = 0; i < 16; i++) {
    barcodeCounts[i] = 0;
  }
  barcodeReadComplete = false;
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

void Task4Barcode::setBarcodeSpeed(uint16_t speed) {
  barcodeSpeed = speed;
  Serial.print("T4 Barcode reading speed set to: ");
  Serial.println(speed);
}

void Task4Barcode::setApproachSpeed(uint16_t speed) {
  approachSpeed = speed;
  Serial.print("T4 Approach speed set to: ");
  Serial.println(speed);
}

void Task4Barcode::setTurnSpeed(uint16_t speed) {
  turnSpeed = speed;
  Serial.print("T4 Turn speed set to: ");
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

uint16_t Task4Barcode::getBarcodeSpeed() {
  return barcodeSpeed;
}

uint16_t Task4Barcode::getApproachSpeed() {
  return approachSpeed;
}

uint16_t Task4Barcode::getTurnSpeed() {
  return turnSpeed;
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
