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
    // Stop all color sensor continuous reading
    colorSensors.stopAllContinuousReading();
    
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
    pushButton.printState();
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
    // Format: BCGRIPPOS pos0 pos1
    int spaceIndex = command.indexOf(' ', 10);
    if (spaceIndex > 0) {
      int pos0 = command.substring(10, spaceIndex).toInt();
      int pos1 = command.substring(spaceIndex + 1).toInt();
      ballCollector.setGripperPositions(pos0, pos1);
      Serial.print("Gripper positions updated - Open: ");
      Serial.print(pos0);
      Serial.print("°, Closed: ");
      Serial.print(pos1);
      Serial.println("°");
      oledDisplay.show("Grip Pos", String(pos0) + "," + String(pos1));
    } else {
      Serial.print("Current - Open: ");
      Serial.print(ballCollector.getGripperPos0());
      Serial.print("°, Closed: ");
      Serial.print(ballCollector.getGripperPos1());
      Serial.println("°");
      Serial.println("Usage: BCGRIPPOS <pos0> <pos1>");
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
  Serial.println("Color Sensors:");
  Serial.println("  COLORREAD - Read all color sensors (one time)");
  Serial.println("  COLORSCAN - Scan all channels to find color sensors");
  Serial.println("  COLORBOTTOM - Toggle continuous bottom sensor (Ch 5)");
  Serial.println("  COLORTOP - Toggle continuous top sensor (Ch 2 - Ball Detection)");
  Serial.println("  COLORBACK - Toggle continuous back sensor (Ch 1)");
  Serial.println();
  Serial.println("Push Button:");
  Serial.println("  BUTTONREAD - Read current button state");
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
  Serial.print("  T2WALLDIST <mm> - Wall detection distance (");
  Serial.print(task2WallFollow.getWallDetectionDistance());
  Serial.println(")");
  Serial.print("  T2TARGETDIST <mm> - Target wall distance (");
  Serial.print(task2WallFollow.getTargetWallDistance());
  Serial.println(")");
  Serial.print("  T2TURNDUR <ms> - Turn duration for 90° (");
  Serial.print(task2WallFollow.getTurnDuration());
  Serial.println(")");
  Serial.print("  T2ALIGNDUR <ms> - Alignment duration (");
  Serial.print(task2WallFollow.getAlignDuration());
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
  Serial.println("Ball Collector:");
  Serial.println("  BALLCOLLECT - Execute ball collection sequence");
  Serial.println();
  Serial.println("  Servo Position Configuration:");
  Serial.print("    BCARMPOS <pos0> <pos1> - Set arm positions (Current: ");
  Serial.print(ballCollector.getArmPos0());
  Serial.print("°, ");
  Serial.print(ballCollector.getArmPos1());
  Serial.println("°)");
  Serial.print("    BCGRIPPOS <pos0> <pos1> - Set gripper positions (Current: ");
  Serial.print(ballCollector.getGripperPos0());
  Serial.print("°, ");
  Serial.print(ballCollector.getGripperPos1());
  Serial.println("°)");
  Serial.print("    BCSORTPOS <pos0> <pos1> <pos2> - Set sorting positions (Current: ");
  Serial.print(ballCollector.getSortingPos0());
  Serial.print("°, ");
  Serial.print(ballCollector.getSortingPos1());
  Serial.print("°, ");
  Serial.print(ballCollector.getSortingPos2());
  Serial.println("°)");
  Serial.println();
  Serial.println("  Individual Servo Testing (0-180°):");
  Serial.println("    CONFIGARM <angle> - Test arm servo at specific angle");
  Serial.println("    CONFIGGRIP <angle> - Test gripper servo at specific angle");
  Serial.println("    CONFIGSORT <angle> - Test sorting servo at specific angle");
  Serial.println();
  Serial.println("  Timing Configuration (milliseconds):");
  Serial.print("    BCSERVODELAY <ms> - Servo movement delay (Current: ");
  Serial.print(ballCollector.getServoMoveDelay());
  Serial.println(" ms)");
  Serial.print("    BCCOLORDELAY <ms> - Color detection delay (Current: ");
  Serial.print(ballCollector.getColorDetectDelay());
  Serial.println(" ms)");
  Serial.print("    BCSORTDELAY <ms> - Sorting position delay (Current: ");
  Serial.print(ballCollector.getSortingDelay());
  Serial.println(" ms)");
  Serial.print("    BCDONEDELAY <ms> - Completion delay before DONE (Current: ");
  Serial.print(ballCollector.getCompletionDelay());
  Serial.println(" ms)");
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
