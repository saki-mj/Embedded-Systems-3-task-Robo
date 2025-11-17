// -------------------------------------------------------------------------
// Ball Collector Implementation
// -------------------------------------------------------------------------

#include "BallCollector.h"
#include "../OLEDDisplay.h"
#include "../IRReading.h"
#include "../TOFSensors.h"
#include "../ColorSensors.h"
#include "../Motors.h"

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
  
  // Default servo positions (to be defined later)
  servoOpenPosition = 90;
  servoClosePosition = 0;
}

void BallCollector::init() {
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
  
  // TODO: Implement actual ball collection logic here
  // This is a placeholder implementation showing the flow
  
  // Step 1: Read IR sensors (to detect ball proximity)
  // int irValue = readIRValue();  // To be implemented
  // hadapan huththooooooooooo
  Serial.println("Step 1: Reading IR sensors...");
  oledDisplay.show("Step 1", "IR Sensors");
  delay(500);  // Placeholder delay
  
  // Step 2: Read TOF sensors (to measure distance to ball)
  // int tofDistance = readTOFDistance();  // To be implemented
  Serial.println("Step 2: Reading TOF sensors...");
  oledDisplay.show("Step 2", "TOF Sensors");
  delay(500);  // Placeholder delay
  
  // Step 3: Read color sensors (to verify ball color/presence)
  // int colorValue = readColorSensor();  // To be implemented
  Serial.println("Step 3: Reading Color sensors...");
  oledDisplay.show("Step 3", "Color Check");
  delay(500);  // Placeholder delay
  
  // Step 4: Approach the ball (motor control)
  // approachBall();  // To be implemented
  Serial.println("Step 4: Approaching ball...");
  oledDisplay.show("Step 4", "Approaching");
  delay(500);  // Placeholder delay
  
  // Step 5: Activate gripper/collector mechanism (servo control)
  // activateGripper();  // To be implemented
  Serial.println("Step 5: Activating gripper...");
  oledDisplay.show("Step 5", "Gripper Active");
  delay(500);  // Placeholder delay
  
  // Step 6: Verify collection (sensor feedback)
  // bool ballCollected = verifyCollection();  // To be implemented
  Serial.println("Step 6: Verifying collection...");
  oledDisplay.show("Step 6", "Verifying");
  delay(500);  // Placeholder delay
  
  // Mark collection as complete
  collectionComplete = true;
  isCollecting = false;
  
  Serial.println("=== Ball Collection Complete ===\n");
  Serial.println(">>> DONE <<<");  // Return message for Task1 state machine
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
  
  // TODO: Stop motors and return servos to default position
  // stopAllMotors();  // Already available
  // resetServo();  // To be implemented
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

void BallCollector::setServoPositions(int openPos, int closePos) {
  servoOpenPosition = openPos;
  servoClosePosition = closePos;
  Serial.print("Ball Collector: Servo positions set - Open: ");
  Serial.print(openPos);
  Serial.print(", Close: ");
  Serial.println(closePos);
}
