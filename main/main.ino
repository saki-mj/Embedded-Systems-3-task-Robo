// -------------------------------------------------------------------------
// Motor Control with IR Sensors and TOF Sensors - Complete Robot Control
// -------------------------------------------------------------------------

#include <ESP32Servo.h>
#include "src/Motors.h"
#include "src/IRReading.h"
#include "src/LineFollow.h"
#include "src/WallFollow.h"
#include "src/TOFSensors.h"
#include "src/ColorSensors.h"
#include "src/PushButton.h"
#include "src/OLEDDisplay.h"
#include "src/SerialCommands.h"
#include "src/StateMachine.h"
#include "src/tasks/Task1_Plantation.h"
#include "src/tasks/Task2_WallFollow.h"
#include "src/tasks/Task3_Ramp.h"
#include "src/tasks/Task4_Barcode.h"
#include "src/tasks/Task5_Unloading.h"
#include "src/tasks/BallCollector.h"

// -------------------------------------------------------------------------
// Activity Tracking for QR Code Display
// -------------------------------------------------------------------------

unsigned long lastActivityTime = 0;
const unsigned long INACTIVITY_TIMEOUT = 30000; // 30 seconds in milliseconds
bool qrCodeDisplayed = false;
const String GITHUB_REPO_URL = "https://github.com/saki-mj/Embedded-Systems-3-task-Robo.git";

void updateActivity() {
  lastActivityTime = millis();
  qrCodeDisplayed = false;
}

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
  
  // Initialize color sensors
  initColorSensors();
  
  // Initialize push button
  initPushButton();
  
  // Initialize line following
  initLineFollow();
  
  // Initialize wall following
  wallFollow.init();
  
  // Initialize state machine
  initStateMachine();
  
  // Initialize all tasks
  task1Plantation.init();
  task2WallFollow.init();
  task3Ramp.init();
  task4Barcode.init();
  task5Unloading.init();
  
  // Initialize ball collector
  ballCollector.init();
  
  // Print available commands
  printSerialCommands();
  
  // Set default speed to level 1 (50 PWM)
  setSpeedLevel(1);
  
  Serial.println("\n=== Robot Ready ===\n");
  
  // Show ready status
  oledDisplay.show("Robot Ready", "Type HELP");
  delay(2000);
  oledDisplay.showStatus("Idle", getCurrentSpeed());
  
  // Initialize activity tracking
  updateActivity();
}

void loop() {
  // Update push button state
  pushButton.update();
  
  // Handle push button controls
  handlePushButtonControls();
  
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    updateActivity(); // Reset inactivity timer on serial command
    processSerialCommand(command);
  }
  
  // Check for inactivity and display QR code
  unsigned long currentTime = millis();
  if (!qrCodeDisplayed && (currentTime - lastActivityTime >= INACTIVITY_TIMEOUT)) {
    oledDisplay.showQRCode(GITHUB_REPO_URL);
    qrCodeDisplayed = true;
    Serial.println("Displaying QR code due to 30 seconds of inactivity");
  }
  
  // Update state machine
  stateMachine.update();
  
  // Execute current task based on state
  switch(stateMachine.getState()) {
    case STATE_TASK1_PLANTATION:
      updateActivity(); // Reset timer when task is active
      task1Plantation.execute();
      break;
    case STATE_TASK2_WALL_FOLLOW:
      updateActivity(); // Reset timer when task is active
      task2WallFollow.execute();
      break;
    case STATE_TASK3_RAMP:
      updateActivity(); // Reset timer when task is active
      task3Ramp.execute();
      break;
    case STATE_TASK4_BARCODE:
      updateActivity(); // Reset timer when task is active
      task4Barcode.execute();
      break;
    case STATE_TASK5_UNLOADING:
      updateActivity(); // Reset timer when task is active
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
  // If TOF continuous reading is active, continuously print TOF values
  else if (tofSensors.isContinuousReadingActive()) {
    readTOFSensors();
    printTOFValues();
  }
  // If color sensor continuous reading is active, print values
  else if (colorSensors.isBottomContinuousReading()) {
    colorSensors.readBottomSensor();
    ColorData data = colorSensors.getBottomColorData();
    Serial.print("Bottom: R="); Serial.print(data.r);
    Serial.print(" G="); Serial.print(data.g);
    Serial.print(" B="); Serial.print(data.b);
    Serial.print(" C="); Serial.print(data.c);
    Serial.print(" Color="); Serial.println(colorSensors.getColorName(colorSensors.getBottomColor()));
    delay(200);
  }
  else if (colorSensors.isTopContinuousReading()) {
    colorSensors.readTopSensor();
    ColorData data = colorSensors.getTopColorData();
    Serial.print("Top: R="); Serial.print(data.r);
    Serial.print(" G="); Serial.print(data.g);
    Serial.print(" B="); Serial.print(data.b);
    Serial.print(" C="); Serial.print(data.c);
    Serial.print(" Color="); Serial.println(colorSensors.getColorName(colorSensors.getTopColor()));
    delay(200);
  }
  else if (colorSensors.isBackContinuousReading()) {
    colorSensors.readBackSensor();
    ColorData data = colorSensors.getBackColorData();
    Serial.print("Back: R="); Serial.print(data.r);
    Serial.print(" G="); Serial.print(data.g);
    Serial.print(" B="); Serial.print(data.b);
    Serial.print(" C="); Serial.print(data.c);
    Serial.print(" Color="); Serial.println(colorSensors.getColorName(colorSensors.getBackColor()));
    delay(200);
  }
}

// -------------------------------------------------------------------------
// Push Button Control Handler
// -------------------------------------------------------------------------

void handlePushButtonControls() {
  // DOWN button - Emergency Stop
  if (pushButton.wasPressed(BTN_DOWN)) {
    updateActivity(); // Reset timer on button press
    Serial.println("\n*** EMERGENCY STOP - Button Pressed ***");
    stateMachine.emergencyStop();
    // Stop all motors
    stopAllMotors();
    // Stop all tasks
    task1Plantation.stop();
    task2WallFollow.stop();
    task3Ramp.stop();
    task4Barcode.stop();
    task5Unloading.stop();
    // Stop ball collector
    ballCollector.stop();
    // Stop line following
    if (isLineFollowActive()) {
      toggleLineFollow();
    }
    // Stop wall following
    if (wallFollow.isActive()) {
      wallFollow.stop();
    }
    oledDisplay.show("EMERGENCY", "STOP", "BTN DOWN");
    delay(1000);
  }
  
  // RIGHT button - Change Tasks (cycle through tasks)
  if (pushButton.wasPressed(BTN_RIGHT)) {
    updateActivity(); // Reset timer on button press
    RobotState currentState = stateMachine.getState();
    RobotState nextState;
    
    // Only allow task switching if not in emergency or standby
    if (currentState == STATE_EMERGENCY_STOP || currentState == STATE_STANDBY) {
      Serial.println("Cannot switch tasks - Resume or complete initialization first");
      oledDisplay.show("Cannot Switch", "Resume First");
      delay(1000);
      return;
    }
    
    // Stop current task
    task1Plantation.stop();
    task2WallFollow.stop();
    task3Ramp.stop();
    task4Barcode.stop();
    task5Unloading.stop();
    
    // Cycle through tasks
    switch(currentState) {
      case STATE_IDLE:
      case STATE_TASK1_PLANTATION:
        nextState = STATE_TASK2_WALL_FOLLOW;
        task2WallFollow.start();
        Serial.println("*** Switching to Task 2: Wall Following ***");
        oledDisplay.show("Switch Task", "Task 2", "Wall Follow");
        break;
        
      case STATE_TASK2_WALL_FOLLOW:
        nextState = STATE_TASK3_RAMP;
        task3Ramp.start();
        Serial.println("*** Switching to Task 3: Ramp ***");
        oledDisplay.show("Switch Task", "Task 3", "Ramp");
        break;
        
      case STATE_TASK3_RAMP:
        nextState = STATE_TASK4_BARCODE;
        task4Barcode.start();
        Serial.println("*** Switching to Task 4: Barcode ***");
        oledDisplay.show("Switch Task", "Task 4", "Barcode");
        break;
        
      case STATE_TASK4_BARCODE:
        nextState = STATE_TASK5_UNLOADING;
        task5Unloading.start();
        Serial.println("*** Switching to Task 5: Unloading ***");
        oledDisplay.show("Switch Task", "Task 5", "Unloading");
        break;
        
      case STATE_TASK5_UNLOADING:
        nextState = STATE_TASK1_PLANTATION;
        task1Plantation.start();
        Serial.println("*** Switching to Task 1: Plantation ***");
        oledDisplay.show("Switch Task", "Task 1", "Plantation");
        break;
        
      default:
        nextState = STATE_TASK1_PLANTATION;
        task1Plantation.start();
        Serial.println("*** Starting Task 1: Plantation ***");
        oledDisplay.show("Switch Task", "Task 1", "Plantation");
        break;
    }
    
    stateMachine.setState(nextState);
    delay(1500);  // Show message for 1.5 seconds
  }
}