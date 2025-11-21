/*********************************************************************
 * Serial Commands Library Implementation
 *********************************************************************/

#include "SerialCommands.h"
#include "Motors.h"
#include "IRReading.h"
#include "LineFollow.h"
#include "WallFollow.h"
#include "TOFSensors.h"
#include "ColorSensors.h"
#include "PushButton.h"
#include "OLEDDisplay.h"
#include "StateMachine.h"
#include "tasks/Task1_Plantation.h"
#include "tasks/Task2_WallFollow.h"
#include "tasks/Task3_Ramp.h"
#include "tasks/Task4_Barcode.h"
#include "tasks/Task5_Unloading.h"
#include "tasks/BallCollector.h"
#include "Gyroscope.h"
#include "LED.h"

void processSerialCommand(String command) {
  command.trim();
  command.toUpperCase();
  
  // Global Speed Control Commands
  if (command.startsWith("SETBASESPEED ")) {
    int speed = command.substring(13).toInt();
    if (speed >= 0 && speed <= 1023) {
      baseSpeed = speed;
      Serial.print("Base speed set to: ");
      Serial.println(baseSpeed);
      oledDisplay.showStatus("Base Speed", baseSpeed);
    } else {
      Serial.println("Invalid speed. Use 0-1023.");
    }
  }
  else if (command.startsWith("SETROTATESPEED ")) {
    int speed = command.substring(15).toInt();
    if (speed >= 0 && speed <= 1023) {
      rotateSpeed = speed;
      Serial.print("Rotate speed set to: ");
      Serial.println(rotateSpeed);
      oledDisplay.showStatus("Rotate Speed", rotateSpeed);
    } else {
      Serial.println("Invalid speed. Use 0-1023.");
    }
  }
  else if (command == "SHOWSPEEDS") {
    Serial.print("Base Speed: ");
    Serial.println(baseSpeed);
    Serial.print("Rotate Speed: ");
    Serial.println(rotateSpeed);
    oledDisplay.show("Speeds", "Base: " + String(baseSpeed), "Rot: " + String(rotateSpeed));
  }
  // Individual Motor Control
  else if (command == "LMF") {
    leftMotorForward();
    Serial.println("Left Motor Forward");
    oledDisplay.show("Left Motor", "Forward");
  }
  else if (command == "LMB") {
    leftMotorBackward();
    Serial.println("Left Motor Backward");
    oledDisplay.show("Left Motor", "Backward");
  }
  else if (command == "RMF") {
    rightMotorForward();
    Serial.println("Right Motor Forward");
    oledDisplay.show("Right Motor", "Forward");
  }
  else if (command == "RMB") {
    rightMotorBackward();
    Serial.println("Right Motor Backward");
    oledDisplay.show("Right Motor", "Backward");
  }
  // Robot Movement
  else if (command == "RF") {
    robotForward();
    Serial.println("Robot Forward");
    oledDisplay.show("Robot", "Forward");
  }
  else if (command == "RB") {
    robotBackward();
    Serial.println("Robot Backward");
    oledDisplay.show("Robot", "Backward");
  }
  else if (command == "RTL") {
    robotTurnLeft();
    Serial.println("Robot Turn Left");
    oledDisplay.show("Robot", "Turn Left");
  }
  else if (command == "RTR") {
    robotTurnRight();
    Serial.println("Robot Turn Right");
    oledDisplay.show("Robot", "Turn Right");
  }
  else if (command == "STOP") {
    stopAllMotors();
    Serial.println("All Motors Stopped");
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
      Serial.println("Wall Following stopped");
    }
    // Stop IR reading modes
    if (isIRReadingActive()) {
      toggleIRReading();
    }
    if (isIRReadingBinaryActive()) {
      toggleIRReadingBinary();
    }
    // Stop TOF continuous reading
    if (tofSensors.isContinuousReadingActive()) {
      tofSensors.toggleContinuousReading();
    }
    // Stop gyroscope continuous reading
    if (gyroscope.isContinuousReadingActive()) {
      gyroscope.toggleContinuousReading();
    }
    // Stop button continuous reading
    if (pushButton.isContinuousReadingActive()) {
      pushButton.toggleContinuousReading();
    }
    // Stop all color sensor continuous reading
    colorSensors.stopAllContinuousReading();
    
    Serial.println("All tasks and operations stopped");
    oledDisplay.show("Robot", "STOPPED", "All Tasks");
  }
  // IR Sensor Reading
  else if (command == "IRREAD") {
    toggleIRReading();
  }
  else if (command == "IRREADB") {
    toggleIRReadingBinary();
  }
  else if (command == "IRCALIBRATE") {
    oledDisplay.show("Calibrating", "IR Sensors...");
    calibrateIRSensors();
    extern bool irCalibrated;
    oledDisplay.showIRStatus(irCalibrated, 8);
  }
  // Line Following
  else if (command == "LINEFOLLOW") {
    toggleLineFollow();
    if (isLineFollowActive()) {
      oledDisplay.show("Line Follow", "ACTIVE");
    } else {
      oledDisplay.show("Line Follow", "STOPPED");
    }
  }
  else if (command == "INVERTLINE") {
    toggleLineColor();
  }
  else if (command.startsWith("SETP ")) {
    float value = command.substring(5).toFloat();
    setKp(value);
    oledDisplay.show("PD Tuning", "Kp updated");
  }
  else if (command.startsWith("SETD ")) {
    float value = command.substring(5).toFloat();
    setKd(value);
    oledDisplay.show("PD Tuning", "Kd updated");
  }
  // Wall Following
  else if (command == "WALLFOLLOW") {
    if (wallFollow.isActive()) {
      wallFollow.stop();
      Serial.println("Wall Following: STOPPED");
      oledDisplay.show("Wall Follow", "STOPPED");
    } else {
      wallFollow.start();
      Serial.println("Wall Following: ACTIVE");
      oledDisplay.show("Wall Follow", "ACTIVE");
    }
  }
  // TOF Sensors
  else if (command == "TOFREAD") {
    tofSensors.toggleContinuousReading();
    if (!tofSensors.isContinuousReadingActive()) {
      oledDisplay.show("TOF Reading", "Stopped");
    } else {
      oledDisplay.show("TOF Reading", "Continuous ON");
    }
  }
  else if (command == "TOFBACKREAD") {
    tofSensors.readBack();
    Serial.print("Back TOF: ");
    if (tofSensors.isBackValid()) {
      Serial.print(tofSensors.getBackDistance());
      Serial.println(" mm");
    } else {
      Serial.println("Out of range");
    }
    oledDisplay.show("Back TOF", String(tofSensors.getBackDistance()) + " mm");
  }
  else if (command == "TOFCALIBRATE") {
    oledDisplay.show("Calibrating", "TOF Sensors...");
    tofSensors.calibrate();
    oledDisplay.show("TOF Calibrated", "Check Serial");
  }
  else if (command.startsWith("TOFERRORLEFT ")) {
    int16_t error = command.substring(13).toInt();
    tofSensors.setErrorLeft(error);
    oledDisplay.show("Left TOF Error", String(error) + " mm");
  }
  else if (command.startsWith("TOFERRORFRONT ")) {
    int16_t error = command.substring(14).toInt();
    tofSensors.setErrorFront(error);
    oledDisplay.show("Front TOF Error", String(error) + " mm");
  }
  else if (command.startsWith("TOFERRORRIGHT ")) {
    int16_t error = command.substring(14).toInt();
    tofSensors.setErrorRight(error);
    oledDisplay.show("Right TOF Error", String(error) + " mm");
  }
  else if (command.startsWith("TOFERRORBACK ")) {
    int16_t error = command.substring(13).toInt();
    tofSensors.setErrorBack(error);
    oledDisplay.show("Back TOF Error", String(error) + " mm");
  }
  else if (command == "TOFSHOWERRORS") {
    Serial.println("TOF Sensor Calibration Errors:");
    Serial.print("  Left:  ");
    Serial.print(tofSensors.getErrorLeft());
    Serial.println(" mm");
    Serial.print("  Front: ");
    Serial.print(tofSensors.getErrorFront());
    Serial.println(" mm");
    Serial.print("  Right: ");
    Serial.print(tofSensors.getErrorRight());
    Serial.println(" mm");
    Serial.print("  Back:  ");
    Serial.print(tofSensors.getErrorBack());
    Serial.println(" mm");
    oledDisplay.show("TOF Errors", "Check Serial");
  }
  else if (command.startsWith("TOFTHRESHOLD ")) {
    int threshold = command.substring(13).toInt();
    tofSensors.setObstacleThreshold(threshold);
    oledDisplay.show("TOF Threshold", String(threshold) + " mm");
  }
  // LED Control
  else if (command == "LED1ON") {
    led.led1On();
    Serial.println("LED1: ON");
    oledDisplay.show("LED1", "ON");
  }
  else if (command == "LED1OFF") {
    led.led1Off();
    Serial.println("LED1: OFF");
    oledDisplay.show("LED1", "OFF");
  }
  else if (command == "LED2ON") {
    led.led2On();
    Serial.println("LED2: ON");
    oledDisplay.show("LED2", "ON");
  }
  else if (command == "LED2OFF") {
    led.led2Off();
    Serial.println("LED2: OFF");
    oledDisplay.show("LED2", "OFF");
  }
  else if (command == "LEDON") {
    led.allOn();
    Serial.println("All LEDs: ON");
    oledDisplay.show("All LEDs", "ON");
  }
  else if (command == "LEDOFF") {
    led.allOff();
    Serial.println("All LEDs: OFF");
    oledDisplay.show("All LEDs", "OFF");
  }
  else if (command.startsWith("LED1BLINK ")) {
    int times = command.substring(10).toInt();
    led.led1Blink(times);
    Serial.print("LED1 blinked ");
    Serial.print(times);
    Serial.println(" times");
  }
  else if (command.startsWith("LED2BLINK ")) {
    int times = command.substring(10).toInt();
    led.led2Blink(times);
    Serial.print("LED2 blinked ");
    Serial.print(times);
    Serial.println(" times");
  }
  // Gyroscope
  else if (command == "GYROREAD") {
    gyroscope.toggleContinuousReading();
    if (!gyroscope.isContinuousReadingActive()) {
      oledDisplay.show("Gyroscope", "Reading OFF");
    } else {
      oledDisplay.show("Gyroscope", "Continuous ON");
    }
  }
  else if (command == "GYROCALIBRATE") {
    oledDisplay.show("Calibrating", "Gyroscope", "Keep Steady!");
    Serial.println("\n*** KEEP ROBOT ABSOLUTELY STILL ***");
    Serial.println("*** Gyroscope calibration starting... ***");
    gyroscope.calibrate();
    Serial.println("Gyroscope calibration complete!\n");
    oledDisplay.show("Gyro Calibrated", "Check Serial");
  }
  else if (command == "GYROOFFSETS") {
    gyroscope.printCalibration();
    oledDisplay.show("Gyro Offsets", "Check Serial");
  }
  // Color Sensors
  else if (command == "COLORREAD") {
    printColorValues();
  }
  else if (command == "COLORBOTTOM") {
    colorSensors.toggleBottomContinuousReading();
    if (!colorSensors.isBottomContinuousReading()) {
      oledDisplay.show("Bottom Color", "Stopped");
    } else {
      oledDisplay.show("Bottom Color", "Continuous ON");
    }
  }
  else if (command == "COLORTOP") {
    colorSensors.toggleTopContinuousReading();
    if (!colorSensors.isTopContinuousReading()) {
      oledDisplay.show("Top Color", "Stopped");
    } else {
      oledDisplay.show("Top Color", "Continuous ON");
    }
  }
  else if (command == "COLORBACK") {
    colorSensors.toggleBackContinuousReading();
    if (!colorSensors.isBackContinuousReading()) {
      oledDisplay.show("Back Color", "Stopped");
    } else {
      oledDisplay.show("Back Color", "Continuous ON");
    }
  }
  else if (command == "COLORSCAN") {
    Serial.println("Scanning all I2C mux channels for color sensors...");
    oledDisplay.show("Scanning", "Color Sensors");
    scanColorSensors();
    oledDisplay.show("Scan Complete", "Check Serial");
  }
  // Push Button
  else if (command == "BUTTONREAD") {
    pushButton.toggleContinuousReading();
    if (!pushButton.isContinuousReadingActive()) {
      oledDisplay.show("Button Reading", "OFF");
    } else {
      oledDisplay.show("Button Reading", "Continuous ON");
    }
  }
  // OLED Display
  else if (command == "OLEDCLEAR") {
    oledDisplay.clear();
  }
  // State Machine Commands
  else if (command == "START") {
    stateMachine.setState(STATE_IDLE);
    oledDisplay.show("State", "IDLE", "Ready");
  }
  else if (command == "AUTO") {
    stateMachine.setMode(MODE_AUTOMATIC);
    oledDisplay.show("Mode", "Automatic");
  }
  else if (command == "MANUAL") {
    stateMachine.setMode(MODE_MANUAL);
    oledDisplay.show("Mode", "Manual");
  }
  else if (command == "TASK1") {
    stateMachine.setState(STATE_TASK1_PLANTATION);
    task1Plantation.start();
    oledDisplay.show("Starting", "Task 1", "Plantation");
  }
  else if (command == "TASK2") {
    stateMachine.setState(STATE_TASK2_WALL_FOLLOW);
    task2WallFollow.start();
    oledDisplay.show("Starting", "Task 2", "Wall Follow");
  }
  else if (command == "TASK3") {
    stateMachine.setState(STATE_TASK3_RAMP);
    task3Ramp.start();
    oledDisplay.show("Starting", "Task 3", "Ramp");
  }
  else if (command == "TASK4") {
    stateMachine.setState(STATE_TASK4_BARCODE);
    task4Barcode.start();
    oledDisplay.show("Starting", "Task 4", "Barcode");
  }
  else if (command == "TASK5") {
    stateMachine.setState(STATE_TASK5_UNLOADING);
    task5Unloading.start();
    oledDisplay.show("Starting", "Task 5", "Unloading");
  }
  else if (command == "EMERGENCY") {
    stateMachine.emergencyStop();
    task1Plantation.stop();
    task2WallFollow.stop();
    task3Ramp.stop();
    task4Barcode.stop();
    task5Unloading.stop();
    oledDisplay.show("EMERGENCY", "STOP", "All Stopped");
  }
  else if (command == "RESUME") {
    if (stateMachine.getState() == STATE_EMERGENCY_STOP) {
      stateMachine.setState(STATE_IDLE);
      oledDisplay.show("Resumed", "to IDLE");
    }
  }
  else if (command == "STATUS") {
    Serial.print("Current State: ");
    Serial.println(stateMachine.getStateName(stateMachine.getState()));
    Serial.print("Current Mode: ");
    Serial.println(stateMachine.getMode() == MODE_MANUAL ? "Manual" : "Automatic");
    oledDisplay.show("State", stateMachine.getStateName(stateMachine.getState()), 
                    stateMachine.getMode() == MODE_MANUAL ? "Manual" : "Auto");
  }
  // Wall Following Configuration
  else if (command.startsWith("WFKP ")) {
    float kp = command.substring(5).toFloat();
    wallFollow.setKp(kp);
    oledDisplay.show("Wall Follow", "Kp: " + String(kp, 3));
  }
  else if (command.startsWith("WFKD ")) {
    float kd = command.substring(5).toFloat();
    wallFollow.setKd(kd);
    oledDisplay.show("Wall Follow", "Kd: " + String(kd, 3));
  }
  else if (command.startsWith("WFDIST ")) {
    uint16_t dist = command.substring(7).toInt();
    wallFollow.setTargetDistance(dist);
    oledDisplay.show("Wall Follow", "Target: " + String(dist) + "mm");
  }
  // Task 2 Wall Following Configuration
  else if (command.startsWith("T2DETECTTHR ")) {
    uint16_t threshold = command.substring(12).toInt();
    task2WallFollow.setWallNearThreshold(threshold);
    Serial.print("T2 Wall near threshold set to: ");
    Serial.print(threshold);
    Serial.println(" mm");
    oledDisplay.show("T2 Detect Thr", String(threshold) + " mm");
  }
  else if (command.startsWith("T2ENDTHR ")) {
    uint16_t threshold = command.substring(9).toInt();
    task2WallFollow.setWallFarThreshold(threshold);
    Serial.print("T2 Wall far threshold set to: ");
    Serial.print(threshold);
    Serial.println(" mm");
    oledDisplay.show("T2 End Thr", String(threshold) + " mm");
  }
  else if (command.startsWith("T2ENDDELAY ")) {
    unsigned long delay = command.substring(11).toInt();
    task2WallFollow.setCornerDelay(delay);
    Serial.print("T2 Corner delay set to: ");
    Serial.print(delay);
    Serial.println(" ms");
    oledDisplay.show("T2 Corner Delay", String(delay) + " ms");
  }
  else if (command.startsWith("T2TURNDUR ")) {
    unsigned long dur = command.substring(10).toInt();
    task2WallFollow.setTurnDuration(dur);
    Serial.print("T2 Turn duration set to: ");
    Serial.print(dur);
    Serial.println(" ms");
    oledDisplay.show("T2 Turn Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T2TARGETWALL ")) {
    uint16_t dist = command.substring(13).toInt();
    task2WallFollow.setTargetWallDistance(dist);
    Serial.print("T2 Target wall distance set to: ");
    Serial.print(dist);
    Serial.println(" mm");
    oledDisplay.show("T2 Target Wall", String(dist) + " mm");
  }
  // Task 4 Barcode Configuration
  else if (command.startsWith("T4WALLDIST ")) {
    uint16_t dist = command.substring(11).toInt();
    task4Barcode.setWallDetectionDistance(dist);
    oledDisplay.show("T4 Wall Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T4WFOLLOW ")) {
    uint16_t dist = command.substring(10).toInt();
    task4Barcode.setWallFollowDistance(dist);
    oledDisplay.show("T4 WF Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T4TURNRDUR ")) {
    unsigned long dur = command.substring(11).toInt();
    task4Barcode.setTurnRightDuration(dur);
    oledDisplay.show("T4 TurnR Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T4TURNLDUR ")) {
    unsigned long dur = command.substring(11).toInt();
    task4Barcode.setTurnLeftDuration(dur);
    oledDisplay.show("T4 TurnL Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T4STRAIGHTDUR ")) {
    unsigned long dur = command.substring(14).toInt();
    task4Barcode.setStraightDuration(dur);
    oledDisplay.show("T4 Str Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T4REVDUR ")) {
    unsigned long dur = command.substring(9).toInt();
    task4Barcode.setReverseDuration(dur);
    oledDisplay.show("T4 Rev Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T4IRTHRESH ")) {
    uint16_t thresh = command.substring(11).toInt();
    task4Barcode.setIRWhiteThreshold(thresh);
    oledDisplay.show("T4 IR Thresh", String(thresh));
  }
  // Task 5 Unloading: manual barcode and quality
  else if (command.startsWith("T5BARCODEBIN ")) {
    String bin = command.substring(13);
    task5Unloading.setBarcodeBinary(bin);
    oledDisplay.show("T5 Barcode", bin);
  }
  else if (command.startsWith("T5BARCODEVAL ")) {
    uint16_t val = command.substring(13).toInt();
    task5Unloading.setBarcodeValue(val);
    oledDisplay.show("T5 Barcode", String(val));
  }
  else if (command.startsWith("T5QUALITY ")) {
    String q = command.substring(9);
    q.trim();
    q.toUpperCase();
    if (q == "GOOD") task5Unloading.setPotatoQuality(POTATO_GOOD);
    else if (q == "BAD") task5Unloading.setPotatoQuality(POTATO_BAD);
    else Serial.println("T5QUALITY must be GOOD or BAD");
  }
  // --- Custom: Odd/Even Barcode Command for Task 5 ---
  else if (command == "ODD") {
    task5Unloading.setBarcodeValue(1); // Odd barcode value
    task5Unloading.setPotatoQuality(POTATO_GOOD); // Default to GOOD, or let user set
    task5Unloading.start();
    Serial.println("Task 5: Started with ODD barcode value (1)");
    oledDisplay.show("T5 Barcode", "ODD (1)", "Started");
  }
  else if (command == "EVEN") {
    task5Unloading.setBarcodeValue(0); // Even barcode value
    task5Unloading.setPotatoQuality(POTATO_GOOD); // Default to GOOD, or let user set
    task5Unloading.start();
    Serial.println("Task 5: Started with EVEN barcode value (0)");
    oledDisplay.show("T5 Barcode", "EVEN (0)", "Started");
  }
  // Ball Collector Commands
  else if (command == "BALLCOLLECT") {
    Serial.println("Starting ball collection sequence...");
    oledDisplay.show("Ball Collect", "Starting...");
    ballCollector.collectingBall();
  }
  else if (command.startsWith("BCARMPOS ")) {
    // Format: BCARMPOS pos0 pos1
    int spaceIndex = command.indexOf(' ', 9);
    if (spaceIndex > 0) {
      int pos0 = command.substring(9, spaceIndex).toInt();
      int pos1 = command.substring(spaceIndex + 1).toInt();
      ballCollector.setArmPositions(pos0, pos1);
      Serial.print("Arm positions updated - Home: ");
      Serial.print(pos0);
      Serial.print("°, Pickup: ");
      Serial.print(pos1);
      Serial.println("°");
      oledDisplay.show("Arm Pos", String(pos0) + "," + String(pos1));
    } else {
      Serial.print("Current - Home: ");
      Serial.print(ballCollector.getArmPos0());
      Serial.print("°, Pickup: ");
      Serial.print(ballCollector.getArmPos1());
      Serial.println("°");
      Serial.println("Usage: BCARMPOS <pos0> <pos1>");
    }
  }
  else if (command.startsWith("BCGRIPPOS ")) {
    // Format: BCGRIPPOS pos0 pos1 initialPos dropPos
    String params = command.substring(10);
    params.trim();
    
    int firstSpace = params.indexOf(' ');
    int secondSpace = params.indexOf(' ', firstSpace + 1);
    int thirdSpace = params.indexOf(' ', secondSpace + 1);
    
    if (firstSpace > 0 && secondSpace > 0 && thirdSpace > 0) {
      int pos0 = params.substring(0, firstSpace).toInt();
      int pos1 = params.substring(firstSpace + 1, secondSpace).toInt();
      int initialPos = params.substring(secondSpace + 1, thirdSpace).toInt();
      int dropPos = params.substring(thirdSpace + 1).toInt();
      ballCollector.setGripperPositions(pos0, pos1, initialPos, dropPos);
      Serial.print("Gripper positions updated - Open: ");
      Serial.print(pos0);
      Serial.print("°, Closed: ");
      Serial.print(pos1);
      Serial.print("°, Initial: ");
      Serial.print(initialPos);
      Serial.print("°, Drop: ");
      Serial.print(dropPos);
      Serial.println("°");
      oledDisplay.show("Grip Pos", String(pos0) + "," + String(pos1) + "," + String(initialPos) + "," + String(dropPos));
    } else {
      Serial.print("Current - Open: ");
      Serial.print(ballCollector.getGripperPos0());
      Serial.print("°, Closed: ");
      Serial.print(ballCollector.getGripperPos1());
      Serial.print("°, Initial: ");
      Serial.print(ballCollector.getGripperInitialPos());
      Serial.print("°, Drop: ");
      Serial.print(ballCollector.getGripperDropPos());
      Serial.println("°");
      Serial.println("Usage: BCGRIPPOS <pos0> <pos1> <initialPos> <dropPos>");
    }
  }
  else if (command.startsWith("BCSORTPOS ")) {
    // Format: BCSORTPOS pos0 pos1 pos2
    int space1 = command.indexOf(' ', 10);
    int space2 = command.indexOf(' ', space1 + 1);
    if (space1 > 0 && space2 > 0) {
      int pos0 = command.substring(10, space1).toInt();
      int pos1 = command.substring(space1 + 1, space2).toInt();
      int pos2 = command.substring(space2 + 1).toInt();
      ballCollector.setSortingPositions(pos0, pos1, pos2);
      Serial.print("Sorting positions updated - Yellow: ");
      Serial.print(pos0);
      Serial.print("°, Home: ");
      Serial.print(pos1);
      Serial.print("°, White: ");
      Serial.print(pos2);
      Serial.println("°");
      oledDisplay.show("Sort Pos", String(pos0) + "," + String(pos1) + "," + String(pos2));
    } else {
      Serial.print("Current - Yellow: ");
      Serial.print(ballCollector.getSortingPos0());
      Serial.print("°, Home: ");
      Serial.print(ballCollector.getSortingPos1());
      Serial.print("°, White: ");
      Serial.print(ballCollector.getSortingPos2());
      Serial.println("°");
      Serial.println("Usage: BCSORTPOS <pos0> <pos1> <pos2>");
    }
  }
  else if (command.startsWith("CONFIGARM ")) {
    int angle = command.substring(10).toInt();
    ballCollector.testServo("ARM", angle);
  }
  else if (command.startsWith("CONFIGGRIP ")) {
    int angle = command.substring(11).toInt();
    ballCollector.testServo("GRIPPER", angle);
  }
  else if (command.startsWith("CONFIGSORT ")) {
    int angle = command.substring(11).toInt();
    ballCollector.testServo("SORTING", angle);
  }
  else if (command.startsWith("CONFIGOUT ")) {
    int angle = command.substring(10).toInt();
    task5Unloading.testOutServo(angle);
  }
  else if (command.startsWith("T5OUTPOS ")) {
    // Format: T5OUTPOS pos0 pos1 pos2
    int space1 = command.indexOf(' ', 9);
    int space2 = command.indexOf(' ', space1 + 1);
    if (space1 > 0 && space2 > 0) {
      int pos0 = command.substring(9, space1).toInt();
      int pos1 = command.substring(space1 + 1, space2).toInt();
      int pos2 = command.substring(space2 + 1).toInt();
      task5Unloading.setOutServoPositions(pos0, pos1, pos2);
      Serial.print("OUT Servo positions updated - Home: ");
      Serial.print(pos0);
      Serial.print("°, Yellow: ");
      Serial.print(pos1);
      Serial.print("°, White: ");
      Serial.print(pos2);
      Serial.println("°");
      oledDisplay.show("OUT Servo Pos", String(pos0) + "," + String(pos1) + "," + String(pos2));
    } else {
      Serial.print("Current - Home: ");
      Serial.print(task5Unloading.getOutServoPos0());
      Serial.print("°, Yellow: ");
      Serial.print(task5Unloading.getOutServoPos1());
      Serial.print("°, White: ");
      Serial.print(task5Unloading.getOutServoPos2());
      Serial.println("°");
      Serial.println("Usage: T5OUTPOS <pos0> <pos1> <pos2>");
    }
  }
  else if (command.startsWith("BCSERVODELAY ")) {
    unsigned long delay = command.substring(13).toInt();
    ballCollector.setServoMoveDelay(delay);
    Serial.print("Servo move delay set to ");
    Serial.print(delay);
    Serial.println(" ms");
    oledDisplay.show("Servo Delay", String(delay) + " ms");
  }
  else if (command.startsWith("BCCOLORDELAY ")) {
    unsigned long delay = command.substring(13).toInt();
    ballCollector.setColorDetectDelay(delay);
    Serial.print("Color detection delay set to ");
    Serial.print(delay);
    Serial.println(" ms");
    oledDisplay.show("Color Delay", String(delay) + " ms");
  }
  else if (command.startsWith("BCSORTDELAY ")) {
    unsigned long delay = command.substring(12).toInt();
    ballCollector.setSortingDelay(delay);
    Serial.print("Sorting delay set to ");
    Serial.print(delay);
    Serial.println(" ms");
    oledDisplay.show("Sort Delay", String(delay) + " ms");
  }
  else if (command.startsWith("BCDONEDELAY ")) {
    unsigned long delay = command.substring(12).toInt();
    ballCollector.setCompletionDelay(delay);
    Serial.print("Completion delay set to ");
    Serial.print(delay);
    Serial.println(" ms");
    oledDisplay.show("Done Delay", String(delay) + " ms");
  }
  else if (command == "HELP" || command == "?") {
    printSerialCommands();
  }
  else {
    Serial.println("Unknown command. Type HELP for available commands.");
  }
}

void printSerialCommands() {
  Serial.println("\n========================================");
  Serial.println("Serial Commands:");
  Serial.println("========================================");
  Serial.println("Global Speed Control:");
  Serial.println("  SETBASESPEED <0-1023> - Set base speed for forward movement");
  Serial.println("  SETROTATESPEED <0-1023> - Set speed for turning/rotation");
  Serial.println("  SHOWSPEEDS - Display current speed settings");
  Serial.println();
  Serial.println("Individual Motor Control:");
  Serial.println("  LMF - Left Motor Forward");
  Serial.println("  LMB - Left Motor Backward");
  Serial.println("  RMF - Right Motor Forward");
  Serial.println("  RMB - Right Motor Backward");
  Serial.println();
  Serial.println("Robot Movement:");
  Serial.println("  RF - Robot Forward");
  Serial.println("  RB - Robot Backward");
  Serial.println("  RTL - Robot Turn Left");
  Serial.println("  RTR - Robot Turn Right");
  Serial.println("  STOP - Stop All Motors");
  Serial.println();
  Serial.println("IR Sensors:");
  Serial.println("  IRCALIBRATE - Calibrate IR sensors (10s)");
  Serial.println("  IRREAD - Toggle continuous raw IR reading");
  Serial.println("  IRREADB - Toggle continuous binary IR reading");
  Serial.println();
  Serial.println("Line Following:");
  Serial.println("  LINEFOLLOW - Toggle autonomous line following");
  Serial.println("  INVERTLINE - Toggle line color (white/black)");
  Serial.println("  SETP <value> - Set proportional gain");
  Serial.println("  SETD <value> - Set derivative gain");
  Serial.println();
  Serial.println("Wall Following:");
  Serial.println("  WALLFOLLOW - Toggle wall following (uses left TOF)");
  Serial.println("TOF Sensors:");
  Serial.println("  TOFREAD - Toggle continuous reading (all sensors)");
  Serial.println("  TOFBACKREAD - Read back TOF sensor once");
  Serial.println("  TOFCALIBRATE - Calibrate all TOF sensors");
  Serial.println("  TOFERRORLEFT <mm> - Set left TOF error offset");
  Serial.println("  TOFERRORFRONT <mm> - Set front TOF error offset");
  Serial.println("  TOFERRORRIGHT <mm> - Set right TOF error offset");
  Serial.println("  TOFERRORBACK <mm> - Set back TOF error offset");
  Serial.println("  TOFSHOWERRORS - Display current calibration errors");
  Serial.println("  TOFTHRESHOLD <mm> - Set obstacle detection threshold");
  Serial.println();
  Serial.println("Gyroscope:");
  Serial.println("  GYROREAD - Toggle continuous gyroscope reading");
  Serial.println("  GYROCALIBRATE - Calibrate gyroscope (keep steady)");
  Serial.println("  GYROOFFSETS - Show calibration offsets");
  Serial.println();
  Serial.println("Color Sensors:");
  Serial.println("  COLORREAD - Read all color sensors (one time)");
  Serial.println("  COLORSCAN - Scan all channels to find color sensors");
  Serial.println("  COLORBOTTOM - Toggle continuous bottom sensor (Ch 5)");
  Serial.println("  COLORTOP - Toggle continuous top sensor (Ch 2 - Ball Detection)");
  Serial.println("  COLORBACK - Toggle continuous back sensor (Ch 1)");
  Serial.println();
  Serial.println("Push Button:");
  Serial.println("  BUTTONREAD - Toggle continuous button reading (analog + detected button)");
  Serial.println();
  Serial.println("External LEDs:");
  Serial.println("  LED1ON - Turn on LED1 (GPIO 3)");
  Serial.println("  LED1OFF - Turn off LED1 (GPIO 3)");
  Serial.println("  LED2ON - Turn on LED2 (GPIO 17)");
  Serial.println("  LED2OFF - Turn off LED2 (GPIO 17)");
  Serial.println("  LEDON - Turn on all LEDs");
  Serial.println("  LEDOFF - Turn off all LEDs");
  Serial.println("  LED1BLINK <times> - Blink LED1 specified times");
  Serial.println("  LED2BLINK <times> - Blink LED2 specified times");
  Serial.println();
  Serial.println("OLED Display:");
  Serial.println("  OLEDCLEAR - Clear OLED display");
  Serial.println();
  Serial.println("Wall Following Configuration:");
  Serial.println("  WFKP <value> - Set wall follow Kp gain (1.0)");
  Serial.println("  WFKD <value> - Set wall follow Kd gain (0.5)");
  Serial.println("  WFDIST <mm> - Set wall follow target distance (150)");
  Serial.println();
  Serial.println("Task 1 Plantation Configuration:");
  Serial.print("  T1TURNDUR <ms> - Turn duration for 90° (");
  Serial.print(task1Plantation.getTurnDuration());
  Serial.println(")");
  Serial.print("  T1SEARCHDUR <ms> - Search duration (");
  Serial.print(task1Plantation.getSearchDuration());
  Serial.println(")");
  Serial.print("  T1COLLECTDUR <ms> - Ball collection duration (");
  Serial.print(task1Plantation.getCollectDuration());
  Serial.println(")");
  Serial.print("  T1BALLTHRESH <value> - Ball detection threshold (");
  Serial.print(task1Plantation.getBallDetectionThreshold());
  Serial.println(")");
  Serial.println();
  Serial.println("Task 2 Wall Follow Configuration:");
  Serial.print("  T2DETECTTHR <mm> - Wall near threshold (");
  Serial.print(task2WallFollow.getWallNearThreshold());
  Serial.println(")");
  Serial.print("  T2ENDTHR <mm> - Wall far threshold (");
  Serial.print(task2WallFollow.getWallFarThreshold());
  Serial.println(")");
  Serial.print("  T2ENDDELAY <ms> - Corner delay (");
  Serial.print(task2WallFollow.getCornerDelay());
  Serial.println(")");
  Serial.print("  T2TURNDUR <ms> - Turn duration for 90° (");
  Serial.print(task2WallFollow.getTurnDuration());
  Serial.println(")");
  Serial.print("  T2TARGETWALL <mm> - Target wall distance (");
  Serial.print(task2WallFollow.getTargetWallDistance());
  Serial.println(")");
  Serial.println();
  Serial.println("Task 3 Ramp Configuration:");
  Serial.print("  T3CLIMBDUR <ms> - Climb duration (");
  Serial.print(task3Ramp.getClimbDuration());
  Serial.println(")");
  Serial.print("  T3DESCDUR <ms> - Descend duration (");
  Serial.print(task3Ramp.getDescendDuration());
  Serial.println(")");
  Serial.print("  T3RAMPDIST <mm> - Ramp detection distance (");
  Serial.print(task3Ramp.getRampDetectionDistance());
  Serial.println(")");
  Serial.print("  T3TOPTHRESH <mm> - Top detection threshold (");
  Serial.print(task3Ramp.getTopDetectionThreshold());
  Serial.println(")");
  Serial.print("  T3TURNDUR <ms> - Turn duration for 90° (");
  Serial.print(task3Ramp.getTurnDuration());
  Serial.println(")");
  Serial.println();
  Serial.println("Task 4 Barcode Configuration:");
  Serial.print("  T4WALLDIST <mm> - Wall detection distance (");
  Serial.print(task4Barcode.getWallDetectionDistance());
  Serial.println(")");
  Serial.print("  T4WFOLLOW <mm> - Wall following target distance (");
  Serial.print(task4Barcode.getWallFollowDistance());
  Serial.println(")");
  Serial.print("  T4TURNRDUR <ms> - Turn RIGHT duration for 90° (");
  Serial.print(task4Barcode.getTurnRightDuration());
  Serial.println(")");
  Serial.print("  T4TURNLDUR <ms> - Turn LEFT duration for 90° (");
  Serial.print(task4Barcode.getTurnLeftDuration());
  Serial.println(")");
  Serial.print("  T4STRAIGHTDUR <ms> - Straight movement duration (");
  Serial.print(task4Barcode.getStraightDuration());
  Serial.println(")");
  Serial.print("  T4REVDUR <ms> - Reverse movement duration (");
  Serial.print(task4Barcode.getReverseDuration());
  Serial.println(")");
  Serial.print("  T4IRTHRESH <value> - IR threshold: >value=white(1) (");
  Serial.print(task4Barcode.getIRWhiteThreshold());
  Serial.println(")");
  Serial.println();
  Serial.println("Task 5 Unloading Configuration:");
  Serial.println("  T5BARCODEBIN <bin> - Set barcode as binary string (e.g. 1010)");
  Serial.println("  T5BARCODEVAL <val> - Set barcode as integer value (e.g. 10)");
  Serial.println("  T5QUALITY <GOOD|BAD> - Set potato/ball quality");
  Serial.println("  ODD - Start Task 5 with ODD barcode value (1)");
  Serial.println("  EVEN - Start Task 5 with EVEN barcode value (0)");
  Serial.println();
  Serial.println("Ball Collector:");
  Serial.println("  BALLCOLLECT - Execute ball collection sequence");
  Serial.println();
  Serial.println("  Individual Servo Testing (0-180°):");
  Serial.println("    CONFIGARM <angle> - Test arm servo at specific angle");
  Serial.println("    CONFIGGRIP <angle> - Test gripper servo at specific angle");
  Serial.println("    CONFIGSORT <angle> - Test sorting servo at specific angle");
  Serial.println("    CONFIGOUT <angle> - Test OUT servo at specific angle (Task 5)");
  Serial.println();
  Serial.println("  OUT Servo Configuration (Task 5):");
  Serial.print("    T5OUTPOS <pos0> <pos1> <pos2> - Set OUT servo positions (Home: ");
  Serial.print(task5Unloading.getOutServoPos0());
  Serial.print("°, Yellow: ");
  Serial.print(task5Unloading.getOutServoPos1());
  Serial.print("°, White: ");
  Serial.print(task5Unloading.getOutServoPos2());
  Serial.println("°)");
  Serial.println();
  Serial.println("State Machine:");
  Serial.println("  START - Enter IDLE state (ready to run)");
  Serial.println("  AUTO - Enable automatic mode (tasks run sequentially)");
  Serial.println("  MANUAL - Enable manual mode (command-driven)");
  Serial.println("  TASK1 - Start Task 1 (Plantation)");
  Serial.println("  TASK2 - Start Task 2 (Wall Following)");
  Serial.println("  TASK3 - Start Task 3 (Ramp)");
  Serial.println("  TASK4 - Start Task 4 (Barcode Reading)");
  Serial.println("  TASK5 - Start Task 5 (Unloading Balls)");
  Serial.println("  EMERGENCY - Emergency stop all tasks");
  Serial.println("  RESUME - Resume from emergency stop");
  Serial.println("  STATUS - Show current state and mode");
  Serial.println();
  Serial.println("Other:");
  Serial.println("  HELP or ? - Show this help menu");
  Serial.println("========================================\n");
}
