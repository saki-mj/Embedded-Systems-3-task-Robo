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

void processSerialCommand(String command) {
  command.trim();
  command.toUpperCase();
  
  // Speed Control Commands (SPEED1 to SPEED10)
  if (command.startsWith("SPEED")) {
    int level = command.substring(5).toInt();
    if (level >= 1 && level <= 10) {
      setSpeedLevel(level);
      oledDisplay.showStatus("Manual", getCurrentSpeed());
    } else {
      Serial.println("Invalid speed level. Use SPEED1 to SPEED10.");
    }
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
    colorSensors.readBottomSensor();
    ColorData data = colorSensors.getBottomColorData();
    Serial.print("Bottom: R="); Serial.print(data.r);
    Serial.print(" G="); Serial.print(data.g);
    Serial.print(" B="); Serial.print(data.b);
    Serial.print(" Color="); Serial.println(colorSensors.getColorName(colorSensors.getBottomColor()));
    oledDisplay.show("Bottom Color", colorSensors.getColorName(colorSensors.getBottomColor()));
  }
  else if (command == "COLORTOP") {
    colorSensors.readTopSensor();
    ColorData data = colorSensors.getTopColorData();
    Serial.print("Top: R="); Serial.print(data.r);
    Serial.print(" G="); Serial.print(data.g);
    Serial.print(" B="); Serial.print(data.b);
    Serial.print(" Color="); Serial.println(colorSensors.getColorName(colorSensors.getTopColor()));
    oledDisplay.show("Top Color", colorSensors.getColorName(colorSensors.getTopColor()));
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
  else if (command.startsWith("T4BARSPEED ")) {
    uint16_t speed = command.substring(11).toInt();
    task4Barcode.setBarcodeSpeed(speed);
    oledDisplay.show("T4 Bar Speed", String(speed));
  }
  else if (command.startsWith("T4APPSPEED ")) {
    uint16_t speed = command.substring(11).toInt();
    task4Barcode.setApproachSpeed(speed);
    oledDisplay.show("T4 App Speed", String(speed));
  }
  else if (command.startsWith("T4TURNSPEED ")) {
    uint16_t speed = command.substring(12).toInt();
    task4Barcode.setTurnSpeed(speed);
    oledDisplay.show("T4 Turn Speed", String(speed));
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
  // Task 1 Plantation Configuration
  else if (command.startsWith("T1SEARCHSPD ")) {
    uint16_t speed = command.substring(12).toInt();
    task1Plantation.setSearchSpeed(speed);
    oledDisplay.show("T1 Search Spd", String(speed));
  }
  else if (command.startsWith("T1FOLLOWSPD ")) {
    uint16_t speed = command.substring(12).toInt();
    task1Plantation.setFollowSpeed(speed);
    oledDisplay.show("T1 Follow Spd", String(speed));
  }
  else if (command.startsWith("T1TURNSPD ")) {
    uint16_t speed = command.substring(10).toInt();
    task1Plantation.setTurnSpeed(speed);
    oledDisplay.show("T1 Turn Spd", String(speed));
  }
  else if (command.startsWith("T1TURNDUR ")) {
    unsigned long dur = command.substring(10).toInt();
    task1Plantation.setTurnDuration(dur);
    oledDisplay.show("T1 Turn Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T1SEARCHDUR ")) {
    unsigned long dur = command.substring(12).toInt();
    task1Plantation.setSearchDuration(dur);
    oledDisplay.show("T1 Search Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T1COLLECTDUR ")) {
    unsigned long dur = command.substring(13).toInt();
    task1Plantation.setCollectDuration(dur);
    oledDisplay.show("T1 Collect Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T1BALLTHRESH ")) {
    uint16_t thresh = command.substring(13).toInt();
    task1Plantation.setBallDetectionThreshold(thresh);
    oledDisplay.show("T1 Ball Thresh", String(thresh));
  }
  // Task 2 Wall Follow Configuration
  else if (command.startsWith("T2APPSPD ")) {
    uint16_t speed = command.substring(9).toInt();
    task2WallFollow.setApproachSpeed(speed);
    oledDisplay.show("T2 App Spd", String(speed));
  }
  else if (command.startsWith("T2FOLLOWSPD ")) {
    uint16_t speed = command.substring(12).toInt();
    task2WallFollow.setFollowSpeed(speed);
    oledDisplay.show("T2 Follow Spd", String(speed));
  }
  else if (command.startsWith("T2TURNSPD ")) {
    uint16_t speed = command.substring(10).toInt();
    task2WallFollow.setTurnSpeed(speed);
    oledDisplay.show("T2 Turn Spd", String(speed));
  }
  else if (command.startsWith("T2WALLDIST ")) {
    uint16_t dist = command.substring(11).toInt();
    task2WallFollow.setWallDetectionDistance(dist);
    oledDisplay.show("T2 Wall Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T2TARGETDIST ")) {
    uint16_t dist = command.substring(13).toInt();
    task2WallFollow.setTargetWallDistance(dist);
    oledDisplay.show("T2 Target Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T2TURNDUR ")) {
    unsigned long dur = command.substring(10).toInt();
    task2WallFollow.setTurnDuration(dur);
    oledDisplay.show("T2 Turn Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T2ALIGNDUR ")) {
    unsigned long dur = command.substring(11).toInt();
    task2WallFollow.setAlignDuration(dur);
    oledDisplay.show("T2 Align Dur", String(dur) + " ms");
  }
  // Task 3 Ramp Configuration
  else if (command.startsWith("T3APPSPD ")) {
    uint16_t speed = command.substring(9).toInt();
    task3Ramp.setApproachSpeed(speed);
    oledDisplay.show("T3 App Spd", String(speed));
  }
  else if (command.startsWith("T3CLIMBSPD ")) {
    uint16_t speed = command.substring(11).toInt();
    task3Ramp.setClimbSpeed(speed);
    oledDisplay.show("T3 Climb Spd", String(speed));
  }
  else if (command.startsWith("T3DESCSPD ")) {
    uint16_t speed = command.substring(10).toInt();
    task3Ramp.setDescendSpeed(speed);
    oledDisplay.show("T3 Descend Spd", String(speed));
  }
  else if (command.startsWith("T3CLIMBDUR ")) {
    unsigned long dur = command.substring(11).toInt();
    task3Ramp.setClimbDuration(dur);
    oledDisplay.show("T3 Climb Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T3DESCDUR ")) {
    unsigned long dur = command.substring(10).toInt();
    task3Ramp.setDescendDuration(dur);
    oledDisplay.show("T3 Descend Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T3RAMPDIST ")) {
    uint16_t dist = command.substring(11).toInt();
    task3Ramp.setRampDetectionDistance(dist);
    oledDisplay.show("T3 Ramp Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T3TOPTHRESH ")) {
    uint16_t thresh = command.substring(12).toInt();
    task3Ramp.setTopDetectionThreshold(thresh);
    oledDisplay.show("T3 Top Thresh", String(thresh) + " mm");
  }
  // Task 5 Unloading Configuration
  else if (command.startsWith("T5NAVSPD ")) {
    uint16_t speed = command.substring(9).toInt();
    task5Unloading.setNavigateSpeed(speed);
    oledDisplay.show("T5 Nav Spd", String(speed));
  }
  else if (command.startsWith("T5ALIGNSPD ")) {
    uint16_t speed = command.substring(11).toInt();
    task5Unloading.setAlignSpeed(speed);
    oledDisplay.show("T5 Align Spd", String(speed));
  }
  else if (command.startsWith("T5UNLOADSPD ")) {
    uint16_t speed = command.substring(12).toInt();
    task5Unloading.setUnloadSpeed(speed);
    oledDisplay.show("T5 Unload Spd", String(speed));
  }
  else if (command.startsWith("T5UNLOADDUR ")) {
    unsigned long dur = command.substring(12).toInt();
    task5Unloading.setUnloadDuration(dur);
    oledDisplay.show("T5 Unload Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T5ALIGNDUR ")) {
    unsigned long dur = command.substring(11).toInt();
    task5Unloading.setAlignDuration(dur);
    oledDisplay.show("T5 Align Dur", String(dur) + " ms");
  }
  else if (command.startsWith("T5ZONEDIST ")) {
    uint16_t dist = command.substring(11).toInt();
    task5Unloading.setZoneDetectionDistance(dist);
    oledDisplay.show("T5 Zone Dist", String(dist) + " mm");
  }
  else if (command.startsWith("T5BALLCOUNT ")) {
    uint16_t count = command.substring(12).toInt();
    task5Unloading.setTargetBallCount(count);
    oledDisplay.show("T5 Ball Count", String(count));
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
  Serial.println("Speed Control:");
  Serial.println("  SPEED1 to SPEED12");
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
  Serial.println("  TOFREAD - Read and display TOF distances");
  Serial.println("  TOFTHRESHOLD <mm> - Set obstacle threshold");
  Serial.println();
  Serial.println("Color Sensors:");
  Serial.println("  COLORREAD - Read both color sensors");
  Serial.println("  COLORBOTTOM - Read bottom color sensor (Ch 4)");
  Serial.println("  COLORTOP - Read top color sensor (Ch 2)");
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
  Serial.print("  T1SEARCHSPD <speed> - Search speed (");
  Serial.print(task1Plantation.getSearchSpeed());
  Serial.println(")");
  Serial.print("  T1FOLLOWSPD <speed> - Follow speed (");
  Serial.print(task1Plantation.getFollowSpeed());
  Serial.println(")");
  Serial.print("  T1TURNSPD <speed> - Turn speed (");
  Serial.print(task1Plantation.getTurnSpeed());
  Serial.println(")");
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
  Serial.print("  T2APPSPD <speed> - Approach speed (");
  Serial.print(task2WallFollow.getApproachSpeed());
  Serial.println(")");
  Serial.print("  T2FOLLOWSPD <speed> - Follow speed (");
  Serial.print(task2WallFollow.getFollowSpeed());
  Serial.println(")");
  Serial.print("  T2TURNSPD <speed> - Turn speed (");
  Serial.print(task2WallFollow.getTurnSpeed());
  Serial.println(")");
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
  Serial.print("  T3APPSPD <speed> - Approach speed (");
  Serial.print(task3Ramp.getApproachSpeed());
  Serial.println(")");
  Serial.print("  T3CLIMBSPD <speed> - Climb speed (");
  Serial.print(task3Ramp.getClimbSpeed());
  Serial.println(")");
  Serial.print("  T3DESCSPD <speed> - Descend speed (");
  Serial.print(task3Ramp.getDescendSpeed());
  Serial.println(")");
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
  Serial.print("  T4BARSPEED <speed> - Barcode reading speed (");
  Serial.print(task4Barcode.getBarcodeSpeed());
  Serial.println(")");
  Serial.print("  T4APPSPEED <speed> - Approach speed (");
  Serial.print(task4Barcode.getApproachSpeed());
  Serial.println(")");
  Serial.print("  T4TURNSPEED <speed> - Turn speed (");
  Serial.print(task4Barcode.getTurnSpeed());
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
  Serial.print("  T5NAVSPD <speed> - Navigate speed (");
  Serial.print(task5Unloading.getNavigateSpeed());
  Serial.println(")");
  Serial.print("  T5ALIGNSPD <speed> - Align speed (");
  Serial.print(task5Unloading.getAlignSpeed());
  Serial.println(")");
  Serial.print("  T5UNLOADSPD <speed> - Unload speed (");
  Serial.print(task5Unloading.getUnloadSpeed());
  Serial.println(")");
  Serial.print("  T5UNLOADDUR <ms> - Unload duration (");
  Serial.print(task5Unloading.getUnloadDuration());
  Serial.println(")");
  Serial.print("  T5ALIGNDUR <ms> - Alignment duration (");
  Serial.print(task5Unloading.getAlignDuration());
  Serial.println(")");
  Serial.print("  T5ZONEDIST <mm> - Zone detection distance (");
  Serial.print(task5Unloading.getZoneDetectionDistance());
  Serial.println(")");
  Serial.print("  T5BALLCOUNT <count> - Target ball count (");
  Serial.print(task5Unloading.getTargetBallCount());
  Serial.println(")");
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
