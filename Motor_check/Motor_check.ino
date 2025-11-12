// -------------------------------------------------------------------------
// Motor Control with Serial Commands - Using MotorConfig Library
// -------------------------------------------------------------------------

#include "MotorConfig.h"

// -------------------------------------------------------------------------
// Setup and Loop
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  
  // Initialize motors
  initMotors();
  
  // Print available commands
  printCommands();
  
  // Set default speed to level 5 (mid-range)
  setSpeedLevel(5);
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processCommand(command);
  }
  
  // Optional: Add your autonomous behavior here
  // For now, just wait for serial commands
}