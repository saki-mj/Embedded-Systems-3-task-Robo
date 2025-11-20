// -------------------------------------------------------------------------
// Ball Collector Implementation
// -------------------------------------------------------------------------

#include "BallCollector.h"
#include "../OLEDDisplay.h"
#include "../IRReading.h"
#include "../TOFSensors.h"
#include "../ColorSensors.h"
#include "../Motors.h"
#include <ESP32Servo.h>

// Create global instance
BallCollector ballCollector;

BallCollector::BallCollector() {
  isCollecting = false;
  collectionComplete = false;
  collectionStartTime = 0;
  
  // Default threshold values (to be calibrated later)
  irThreshold = 500;
  tofThreshold = 100;
  colorThreshold = 1000;
  
  // Default servo positions (PLACEHOLDER VALUES - TO BE CALIBRATED)
  armPos0 = 125;      // Arm home position
  armPos1 = 22;       // Arm pickup position
  gripperPos0 = 0;    // Gripper open
  gripperPos1 = 180;  // Gripper closed
  gripperInitialPos = 130;  // Gripper initial position
  gripperDropPos = 150;     // Gripper drop position
  sortingPos0 = 45;   // Sorting yellow position
  sortingPos1 = 92;   // Sorting initial/home position
  sortingPos2 = 135;  // Sorting white position
  
  // Default timing delays (in milliseconds - TO BE CALIBRATED)
  servoMoveDelay = 500;      // 500ms delay after servo movement
  colorDetectDelay = 1000;   // 1000ms delay before color detection
  sortingDelay = 1500;       // 1500ms delay at sorting position
  completionDelay = 2000;    // 2000ms delay before DONE
}

void BallCollector::init() {
  Serial.println("Ball Collector: Initializing servos...");
  
  // Attach servos to pins
  armServo.attach(ARM_SERVO_PIN);
  gripperServo.attach(GRIPPER_SERVO_PIN);
  sortingServo.attach(SORTING_SERVO_PIN);
  
  // Set servos to initial positions
  armServo.write(armPos0);
  gripperServo.write(gripperPos0);
  sortingServo.write(sortingPos1);
  
  delay(500);
  
  Serial.println("Ball Collector: Initialized");
  oledDisplay.show("Ball Collector", "Ready");
  delay(500);
  
  isCollecting = false;
  collectionComplete = false;
}

bool BallCollector::collectingBall() {
  // If not currently collecting, start the process
  if (!isCollecting) {
    isCollecting = true;
    collectionComplete = false;
    collectionStartTime = millis();
    
    Serial.println("\n=== Ball Collection Started ===");
    oledDisplay.show("Collecting", "Ball...");
  }
  
  // Step 1: Move gripper to initial position, then move arm to pickup position
  Serial.println("Step 1: Moving arm to pickup position...");
  oledDisplay.show("Step 1", "Gripper Init");
  moveGripperTo(gripperInitialPos);
  delay(servoMoveDelay);  // Allow gripper to reach initial position
  
  oledDisplay.show("Step 1", "Arm Moving");
  moveArmTo(armPos1);
  delay(servoMoveDelay);  // Allow servo to reach position
  
  // Step 2: Close gripper to grab ball
  Serial.println("Step 2: Closing gripper to grab ball...");
  oledDisplay.show("Step 2", "Grabbing Ball");
  moveGripperTo(gripperPos1);
  delay(servoMoveDelay);  // Allow gripper to close
  
  // Step 3: Return arm to home position with ball
  Serial.println("Step 3: Returning arm to home position...");
  oledDisplay.show("Step 3", "Arm Returning");
  moveArmTo(armPos0);
  delay(servoMoveDelay);  // Allow servo to reach position
  
  // Step 4: Move gripper to drop position to drop ball into sorting area
  Serial.println("Step 4: Opening gripper to drop ball...");
  oledDisplay.show("Step 4", "Dropping Ball");
  moveGripperTo(gripperDropPos);
  delay(servoMoveDelay);  // Allow gripper to reach drop position
  
  // Wait 3 seconds at drop position
  Serial.println("Waiting 3 seconds at drop position...");
  delay(3000);
  
  // Return gripper to initial position
  Serial.println("Returning gripper to initial position...");
  oledDisplay.show("Step 4", "Gripper Init");
  moveGripperTo(gripperInitialPos);
  delay(servoMoveDelay);  // Allow gripper to reach initial position
  
  // Step 5: Wait before color detection
  Serial.println("Step 5: Waiting before color detection...");
  oledDisplay.show("Step 5", "Waiting...");
  delay(colorDetectDelay);
  
  // Step 6: Read color sensor to detect ball color
  Serial.println("Step 6: Detecting ball color...");
  oledDisplay.show("Step 6", "Color Detection");
  colorSensors.readTopSensor();
  DetectedColor ballColor = colorSensors.getTopColor();
  String colorName = colorSensors.getColorName(ballColor);

  // store last detected color so other modules (Task5) can use it
  lastDetectedColor = ballColor;

  Serial.print("Detected color: ");
  Serial.println(colorName);
  Serial.print("RGB values - R: ");
  Serial.print(colorSensors.getTopRed());
  Serial.print(", G: ");
  Serial.print(colorSensors.getTopGreen());
  Serial.print(", B: ");
  Serial.println(colorSensors.getTopBlue());
  
  // Step 7: Sort ball based on color
  if (ballColor == COLOR_YELLOW) {
    Serial.println("Step 7: Sorting YELLOW ball...");
    oledDisplay.show("Sorting", "YELLOW");
    
    // Move sorting servo from position 1 to 0
    moveSortingTo(sortingPos0);
    delay(sortingDelay);  // Wait at yellow position
    
    // Return to initial position
    moveSortingTo(sortingPos1);
    
  } else if (ballColor == COLOR_WHITE) {
    Serial.println("Step 7: Sorting WHITE ball...");
    oledDisplay.show("Sorting", "WHITE");
    
    // Move sorting servo from position 1 to 2
    moveSortingTo(sortingPos2);
    delay(sortingDelay);  // Wait at white position
    
    // Return to initial position
    moveSortingTo(sortingPos1);
    
  } else {
    Serial.print("Step 7: Unknown color detected (");
    Serial.print(colorName);
    Serial.println(") - Sorting to YELLOW basket");
    oledDisplay.show("Sorting", "UNKNOWN->YEL");
    
    // Move sorting servo to yellow position (same as yellow ball)
    moveSortingTo(sortingPos0);
    delay(sortingDelay);  // Wait at yellow position
    
    // Return to initial position
    moveSortingTo(sortingPos1);
  }
  
  // Wait after sorting before completing
  Serial.println("Waiting before completion...");
  delay(completionDelay);
  
  // Mark collection as complete
  collectionComplete = true;
  isCollecting = false;
  
  Serial.println("=== Ball Collection Complete ===");
  Serial.println(">>> DONE <<<");  // Return message for state machine
  oledDisplay.show("Collection", "DONE!", "Ready for next");
  delay(1000);
  
  return true;  // Return true to indicate completion
}

bool BallCollector::isActive() {
  return isCollecting;
}

void BallCollector::reset() {
  isCollecting = false;
  collectionComplete = false;
  collectionStartTime = 0;
  
  Serial.println("Ball Collector: Reset");
  oledDisplay.show("Ball Collector", "Reset");
  delay(300);
}

void BallCollector::stop() {
  if (isCollecting) {
    Serial.println("Ball Collector: Stopped");
    oledDisplay.show("Collection", "Stopped");
  }
  
  isCollecting = false;
  collectionComplete = false;
  
  // Return servos to initial positions
  armServo.write(armPos0);
  gripperServo.write(gripperInitialPos);
  sortingServo.write(sortingPos1);
  
  Serial.println("Servos returned to initial positions");
}

bool BallCollector::isComplete() {
  return collectionComplete;
}

void BallCollector::setIRThreshold(int threshold) {
  irThreshold = threshold;
  Serial.print("Ball Collector: IR Threshold set to ");
  Serial.println(threshold);
}

void BallCollector::setTOFThreshold(int threshold) {
  tofThreshold = threshold;
  Serial.print("Ball Collector: TOF Threshold set to ");
  Serial.println(threshold);
}

void BallCollector::setColorThreshold(int threshold) {
  colorThreshold = threshold;
  Serial.print("Ball Collector: Color Threshold set to ");
  Serial.println(threshold);
}

// Servo position calibration methods
void BallCollector::setArmPositions(int pos0, int pos1) {
  armPos0 = pos0;
  armPos1 = pos1;
  Serial.print("Ball Collector: Arm positions set - Home: ");
  Serial.print(pos0);
  Serial.print(", Pickup: ");
  Serial.println(pos1);
}

void BallCollector::setGripperPositions(int pos0, int pos1, int initialPos, int dropPos) {
  gripperPos0 = pos0;
  gripperPos1 = pos1;
  gripperInitialPos = initialPos;
  gripperDropPos = dropPos;
  Serial.print("Ball Collector: Gripper positions set - Open: ");
  Serial.print(pos0);
  Serial.print(", Closed: ");
  Serial.print(pos1);
  Serial.print(", Initial: ");
  Serial.print(initialPos);
  Serial.print(", Drop: ");
  Serial.println(dropPos);
}

void BallCollector::setSortingPositions(int pos0, int pos1, int pos2) {
  sortingPos0 = pos0;
  sortingPos1 = pos1;
  sortingPos2 = pos2;
  Serial.print("Ball Collector: Sorting positions set - Yellow: ");
  Serial.print(pos0);
  Serial.print(", Home: ");
  Serial.print(pos1);
  Serial.print(", White: ");
  Serial.println(pos2);
}

// Servo control methods
void BallCollector::moveArmTo(int position) {
  Serial.print("Moving arm servo to position: ");
  Serial.println(position);
  armServo.write(position);
}

void BallCollector::moveGripperTo(int position) {
  Serial.print("Moving gripper servo to position: ");
  Serial.println(position);
  gripperServo.write(position);
}

void BallCollector::moveSortingTo(int position) {
  Serial.print("Moving sorting servo to position: ");
  Serial.println(position);
  sortingServo.write(position);
}

void BallCollector::testServo(const String& servoName, int angle) {
  // Constrain angle to valid servo range
  angle = constrain(angle, 0, 180);
  
  if (servoName == "ARM") {
    Serial.print("Testing ARM servo at ");
    Serial.print(angle);
    Serial.println(" degrees");
    armServo.write(angle);
    oledDisplay.show("ARM Servo", String(angle) + " deg");
  } 
  else if (servoName == "GRIPPER") {
    Serial.print("Testing GRIPPER servo at ");
    Serial.print(angle);
    Serial.println(" degrees");
    gripperServo.write(angle);
    oledDisplay.show("GRIPPER Servo", String(angle) + " deg");
  } 
  else if (servoName == "SORTING") {
    Serial.print("Testing SORTING servo at ");
    Serial.print(angle);
    Serial.println(" degrees");
    sortingServo.write(angle);
    oledDisplay.show("SORTING Servo", String(angle) + " deg");
  } 
  else {
    Serial.print("Unknown servo: ");
    Serial.println(servoName);
    Serial.println("Valid servos: ARM, GRIPPER, SORTING");
  }
}
