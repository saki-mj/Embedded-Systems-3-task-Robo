// -------------------------------------------------------------------------
// Motor Control with IR Sensors and TOF Sensors - Complete Robot Control
// -------------------------------------------------------------------------

#include "src/Motors.h"
#include "src/IRReading.h"
#include "src/LineFollow.h"
#include "src/TOFSensors.h"
#include "src/OLEDDisplay.h"
#include "src/SerialCommands.h"
#include "src/StateMachine.h"
#include "src/tasks/Task1_Plantation.h"
#include "src/tasks/Task2_WallFollow.h"
#include "src/tasks/Task3_Ramp.h"
#include "src/tasks/Task4_Barcode.h"
#include "src/tasks/Task5_Unloading.h"

// -------------------------------------------------------------------------
// Setup and Loop
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  
  Serial.println("\n=== Robot Initialization ===");
  
  // Initialize OLED Display
  initOLED();
  oledDisplay.show("Initializing", "Robot...");
  
  // Initialize motors
  initMotors();
  
  // Initialize IR sensors
  initIRSensors();
  
  // Initialize TOF sensors
  initTOFSensors();
  
  // Initialize line following
  initLineFollow();
  
  // Initialize state machine
  initStateMachine();
  
  // Initialize all tasks
  task1Plantation.init();
  task2WallFollow.init();
  task3Ramp.init();
  task4Barcode.init();
  task5Unloading.init();
  
  // Print available commands
  printSerialCommands();
  
  // Set default speed to level 1 (50 PWM)
  setSpeedLevel(1);
  
  Serial.println("\n=== Robot Ready ===\n");
  
  // Show ready status
  oledDisplay.show("Robot Ready", "Type HELP");
  delay(2000);
  oledDisplay.showStatus("Idle", getCurrentSpeed());
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    processSerialCommand(command);
  }
  
  // Update state machine
  stateMachine.update();
  
  // Execute current task based on state
  switch(stateMachine.getCurrentState()) {
    case TASK1_PLANTATION:
      task1Plantation.execute();
      break;
    case TASK2_WALL_FOLLOW:
      task2WallFollow.execute();
      break;
    case TASK3_RAMP:
      task3Ramp.execute();
      break;
    case TASK4_BARCODE:
      task4Barcode.execute();
      break;
    case TASK5_UNLOADING:
      task5Unloading.execute();
      break;
    default:
      // For STANDBY, IDLE, and EMERGENCY_STOP states, no task execution
      break;
  }
  
  // Execute line following if active (can be used within tasks)
  if (isLineFollowActive()) {
    executeLineFollow();
    
    // Update OLED periodically during line following
    static unsigned long lastOLEDUpdate = 0;
    if (millis() - lastOLEDUpdate > 500) {
      lastOLEDUpdate = millis();
      extern float lastError;
      oledDisplay.showLineFollowing(true, lastError);
    }
  }
  // If raw IR reading mode is active, continuously print raw IR values
  else if (isIRReadingActive()) {
    printIRValues();
  }
  // If binary IR reading mode is active, continuously print binary values
  else if (isIRReadingBinaryActive()) {
    printIRBinary();
  }
}