// -------------------------------------------------------------------------
// Motor Control with IR Sensors and TOF Sensors - Complete Robot Control
// -------------------------------------------------------------------------

#include "src/Motors.h"
#include "src/IRReading.h"
#include "src/LineFollow.h"
#include "src/TOFSensors.h"

// -------------------------------------------------------------------------
// Setup and Loop
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  
  Serial.println("\n=== Robot Initialization ===");
  
  // Initialize motors
  initMotors();
  
  // Initialize IR sensors
  initIRSensors();
  
  // Initialize TOF sensors
  initTOFSensors();
  
  // Initialize line following
  initLineFollow();
  
  // Print available commands
  printCommands();
  
  // Set default speed to level 5 (mid-range)
  setSpeedLevel(5);
  
  Serial.println("\n=== Robot Ready ===\n");
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
  
  // Execute line following if active
  if (isLineFollowActive()) {
    executeLineFollow();
  }
  // If IR reading mode is active (and not line following), continuously print IR values
  else if (isIRReadingActive()) {
    // If calibrated, show binary values; otherwise show raw values
    extern bool irCalibrated;
    if (irCalibrated) {
      printIRBinary();  // Show 0/1 for line following
    } else {
      printIRValues();  // Show raw values
    }
    // Optional: Add a small delay to control data rate
    // delay(100); // Uncomment for slower updates (100ms between scans)
  }
  
  // Optional: Add your autonomous behavior here
}