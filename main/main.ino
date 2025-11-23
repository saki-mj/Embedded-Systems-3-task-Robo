// -------------------------------------------------------------------------
// Motor Control with IR Sensors and TOF Sensors - Complete Robot Control
// -------------------------------------------------------------------------

#include <ESP32Servo.h>
#include "src/Motors.h"

// -------------------------------------------------------------------------
// Global Speed Variables
// -------------------------------------------------------------------------
int baseSpeed = 350;  // Base speed for forward movement (PWM: 0-1023)
int rotateSpeed = 350;  // Speed for turning/rotation (PWM: 0-1023)

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
#include "src/Gyroscope.h"
#include "src/LED.h"

// -------------------------------------------------------------------------
// Activity Tracking for QR Code Display
// -------------------------------------------------------------------------

unsigned long lastActivityTime = 0;
const unsigned long INACTIVITY_TIMEOUT = 30000; // 30 seconds in milliseconds
bool qrCodeDisplayed = false;
const String GITHUB_REPO_URL = "https://github.com/saki-mj/Embedded-Systems-3-task-Robo.git";

// -------------------------------------------------------------------------
// Auto-start Timer
// -------------------------------------------------------------------------

unsigned long robotReadyTime = 0;
const unsigned long AUTO_START_TIMEOUT = 10000; // 10 seconds - auto-start Task 1
bool autoStartTriggered = false;

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
  
  // Initialize external LEDs
  initLED();
  
  // Initialize gyroscope
  initGyroscope();
  
  // Initialize line following
  initLineFollow();
  
  // Initialize wall following
  wallFollow.init();
  
  // Initialize state machine
  initStateMachine();
  
  // Set to AUTOMATIC mode for task progression
  stateMachine.setMode(MODE_AUTOMATIC);
  stateMachine.setState(STATE_IDLE);
  Serial.println("Robot set to AUTOMATIC mode - tasks will progress automatically");
  
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
  
  Serial.println("\n=== Robot Ready ===\n");
  Serial.print("Base Speed: ");
  Serial.println(baseSpeed);
  Serial.print("Rotate Speed: ");
  Serial.println(rotateSpeed);
  
  // Show ready status
  oledDisplay.show("Robot Ready", "Type HELP");
  delay(2000);
  oledDisplay.showStatus("Idle", baseSpeed);
  
  // Initialize activity tracking
  updateActivity();
  
  // Initialize auto-start timer
  robotReadyTime = millis();
  autoStartTriggered = false;
}

void loop() {
  // *** PRIORITY 1: PUSH BUTTONS - Always check first ***
  pushButton.update();
  handlePushButtonControls();
  
  // *** PRIORITY 2: Serial commands ***
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    updateActivity(); // Reset inactivity timer on serial command
    processSerialCommand(command);
  }
  
  // *** PRIORITY 3: Auto-start (only if no buttons pressed) ***
  unsigned long currentTime = millis();
  if (!autoStartTriggered && 
      stateMachine.getState() == STATE_IDLE && 
      (currentTime - robotReadyTime >= AUTO_START_TIMEOUT)) {
    autoStartTriggered = true;
    Serial.println("\n*** AUTO-START (10s): Starting Task 1 in AUTOMATIC mode ***");
    Serial.println("    Tasks will progress automatically: T1→T2→T3→T4→T5");
    oledDisplay.show("AUTO START", "Task 1", "Automatic");
    delay(1500);
    stateMachine.setState(STATE_TASK1_PLANTATION);
    task1Plantation.start();
  }
  
  // Update state machine
  stateMachine.update();
  
  // Update OLED with real-time speed (every 500ms when idle)
  static unsigned long lastSpeedUpdate = 0;
  if (millis() - lastSpeedUpdate > 500 && stateMachine.getState() == STATE_IDLE) {
    lastSpeedUpdate = millis();
    oledDisplay.showStatus("Idle", baseSpeed);
  }
  
  // Check for inactivity and display QR code
  if (!qrCodeDisplayed && (currentTime - lastActivityTime >= INACTIVITY_TIMEOUT)) {
    oledDisplay.showQRCode(GITHUB_REPO_URL);
    qrCodeDisplayed = true;
    Serial.println("Displaying QR code due to 30 seconds of inactivity");
  }
  
  // Execute current task based on state
  switch(stateMachine.getState()) {
    case STATE_TASK1_PLANTATION:
      updateActivity(); // Reset timer when task is active
      task1Plantation.execute();
      // Auto-progress to Task 2 when complete (automatic mode)
      if (task1Plantation.isCompleted() && stateMachine.isAutomaticMode()) {
        Serial.println("*** Task 1 Complete - Auto-progressing to Task 2 ***");
        stateMachine.markTaskCompleted();
        stateMachine.setState(STATE_TASK2_WALL_FOLLOW);
        task2WallFollow.start();
        oledDisplay.show("Task 1 Done", "Starting Task 2");
        delay(1000);
      }
      break;
    case STATE_TASK2_WALL_FOLLOW:
      updateActivity(); // Reset timer when task is active
      task2WallFollow.execute();
      // Auto-progress to Task 3 when complete (automatic mode)
      if (task2WallFollow.isCompleted() && stateMachine.isAutomaticMode()) {
        Serial.println("*** Task 2 Complete - Auto-progressing to Task 3 ***");
        stateMachine.markTaskCompleted();
        stateMachine.setState(STATE_TASK3_RAMP);
        task3Ramp.start();
        oledDisplay.show("Task 2 Done", "Starting Task 3");
        delay(1000);
      }
      break;
    case STATE_TASK3_RAMP:
      updateActivity(); // Reset timer when task is active
      task3Ramp.execute();
      // Auto-progress to Task 4 when complete (automatic mode)
      if (task3Ramp.isCompleted() && stateMachine.isAutomaticMode()) {
        Serial.println("*** Task 3 Complete - Auto-progressing to Task 4 ***");
        stateMachine.markTaskCompleted();
        stateMachine.setState(STATE_TASK4_BARCODE);
        task4Barcode.start();
        oledDisplay.show("Task 3 Done", "Starting Task 4");
        delay(1000);
      }
      break;
    case STATE_TASK4_BARCODE:
      updateActivity(); // Reset timer when task is active
      led.led1On(); // LED1 on during Task 4
      task4Barcode.execute();
      // Auto-progress to Task 5 when complete (automatic mode)
      if (task4Barcode.isCompleted() && stateMachine.isAutomaticMode()) {
        Serial.println("*** Task 4 Complete - Auto-progressing to Task 5 ***");
        stateMachine.markTaskCompleted();
        stateMachine.setState(STATE_TASK5_UNLOADING);
        task5Unloading.start();
        oledDisplay.show("Task 4 Done", "Starting Task 5");
        delay(1000);
      }
      break;
    case STATE_TASK5_UNLOADING:
      updateActivity(); // Reset timer when task is active
      led.led1On(); // LED1 on during Task 5
      task5Unloading.execute();
      // Mark complete and turn off LED
      if (task5Unloading.isCompleted()) {
        led.led1Off();
        if (stateMachine.isAutomaticMode()) {
          Serial.println("*** Task 5 Complete - All tasks finished! ***");
          stateMachine.markTaskCompleted();
          stateMachine.setState(STATE_IDLE);
          oledDisplay.show("All Tasks", "COMPLETED!");
          delay(2000);
        }
      }
      break;
    default:
      // For STANDBY, IDLE, and EMERGENCY_STOP states, no task execution
      led.led1Off(); // Turn off LED when not in Task 4 or 5
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
  // Execute wall following if active
  else if (wallFollow.isActive()) {
    readTOFSensors();
    uint16_t leftDist = tofSensors.getLeftDistance();
    wallFollow.executeWallFollow(leftDist);
    
    // Update OLED periodically during wall following
    static unsigned long lastWallOLEDUpdate = 0;
    if (millis() - lastWallOLEDUpdate > 500) {
      lastWallOLEDUpdate = millis();
      oledDisplay.show("Wall Follow", "Active", "L: " + String(leftDist) + "mm");
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
  // If gyroscope continuous reading is active, print values
  else if (gyroscope.isContinuousReadingActive()) {
    gyroscope.read();
    gyroscope.printValues();
    delay(100);
  }
  // If button continuous reading is active, print values
  else if (pushButton.isContinuousReadingActive()) {
    pushButton.printState();
    delay(100);
  }
}

// -------------------------------------------------------------------------
// Push Button Control Handler
// -------------------------------------------------------------------------

void handlePushButtonControls() {
  // Any button press cancels auto-start and resets timer
  Button currentBtn = pushButton.getCurrentButton();
  if (currentBtn != BTN_NONE) {
    autoStartTriggered = true; // Cancel auto-start
    robotReadyTime = millis(); // Reset timer
    updateActivity(); // Reset inactivity timer
  }
  
  // UP button - Start Task 2 (Wall Follow) and continue to next tasks
  if (pushButton.wasPressed(BTN_UP)) {
    stopAllMotors();
    if (isLineFollowActive()) toggleLineFollow();
    if (wallFollow.isActive()) wallFollow.stop();
    
    task1Plantation.stop();
    task2WallFollow.stop();
    task3Ramp.stop();
    task4Barcode.stop();
    task5Unloading.stop();
    
    Serial.println("*** UP Button: Starting Task 2 (Wall Follow) and continuing ***");
    stateMachine.setState(STATE_TASK2_WALL_FOLLOW);
    task2WallFollow.start();
    oledDisplay.show("UP Button", "Task 2", "Wall Follow");
    delay(1500);
  }
  
  // LEFT button - Ball Collector Control (execute BALLCOLLECT command)
  if (pushButton.wasPressed(BTN_LEFT)) {
    Serial.println("*** LEFT Button: Executing BALLCOLLECT command ***");
    oledDisplay.show("LEFT Button", "Ball", "Collecting");
    ballCollector.collectingBall();  // Execute ball collection
    delay(1500);
  }
  
  // DOWN button - Start Task 4 (Barcode)
  if (pushButton.wasPressed(BTN_DOWN)) {
    stopAllMotors();
    if (isLineFollowActive()) toggleLineFollow();
    if (wallFollow.isActive()) wallFollow.stop();
    
    task1Plantation.stop();
    task2WallFollow.stop();
    task3Ramp.stop();
    task4Barcode.stop();
    task5Unloading.stop();
    
    Serial.println("*** DOWN Button: Starting Task 4 (Barcode) ***");
    stateMachine.setState(STATE_TASK4_BARCODE);
    task4Barcode.start();
    led.led1On();
    oledDisplay.show("DOWN Button", "Task 4", "Barcode");
    delay(1500);
  }
  
  // RIGHT button - DISABLED (not used)
  // if (pushButton.wasPressed(BTN_RIGHT)) { }
  
  // MIDDLE button - DISABLED (not used)
  // if (pushButton.wasPressed(BTN_MIDDLE)) { }
}