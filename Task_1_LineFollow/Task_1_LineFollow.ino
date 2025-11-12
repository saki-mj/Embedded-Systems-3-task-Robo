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
  
<<<<<<< Updated upstream
<<<<<<< Updated upstream
  // Set default speed to level 5 (mid-range)
  setSpeedLevel(5);
  
  Serial.println("\n=== Robot Ready ===\n");
=======
  // Set default speed to level 1 (50 PWM)
  setSpeedLevel(1);
>>>>>>> Stashed changes
=======
  // Set default speed to level 1 (50 PWM)
  setSpeedLevel(1);
>>>>>>> Stashed changes
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
  // If raw IR reading mode is active, continuously print raw IR values
  else if (isIRReadingActive()) {
    printIRValues();  // Show raw analog values
    // Optional: Add a small delay to control data rate
    // delay(100); // Uncomment for slower updates (100ms between scans)
  }
  // If binary IR reading mode is active, continuously print binary values
  else if (isIRReadingBinaryActive()) {
    printIRBinary();  // Show 0/1 for line following
    // Optional: Add a small delay to control data rate
    // delay(100); // Uncomment for slower updates (100ms between scans)
  }
  
  // Optional: Add your autonomous behavior here
}