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
    oledDisplay.show("Left Motor", "Forward");
  }
  else if (command == "LMB") {
    leftMotorBackward();
    oledDisplay.show("Left Motor", "Backward");
  }
  else if (command == "RMF") {
    rightMotorForward();
    oledDisplay.show("Right Motor", "Forward");
  }
  else if (command == "RMB") {
    rightMotorBackward();
    oledDisplay.show("Right Motor", "Backward");
  }
  // Robot Movement
  else if (command == "RF") {
    robotForward();
    oledDisplay.show("Robot", "Forward");
  }
  else if (command == "RB") {
    robotBackward();
    oledDisplay.show("Robot", "Backward");
  }
  else if (command == "RTL") {
    robotTurnLeft();
    oledDisplay.show("Robot", "Turn Left");
  }
  else if (command == "RTR") {
    robotTurnRight();
    oledDisplay.show("Robot", "Turn Right");
  }
  else if (command == "STOP") {
    stopAllMotors();
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

  // Task1 Plantation tunable parameters (via serial)
  else if (command.startsWith("T1TURN90 ")) {
    unsigned long dur = command.substring(9).toInt();
    T1_TURN_90_TIME_MS = dur;
    oledDisplay.show("T1 Turn90", String(dur) + " ms");
  }
  else if (command.startsWith("T1TURN180 ")) {
    unsigned long dur = command.substring(10).toInt();
    T1_TURN_180_TIME_MS = dur;
    oledDisplay.show("T1 Turn180", String(dur) + " ms");
  }
  else if (command.startsWith("T1BACKUP ")) {
    unsigned long dur = command.substring(8).toInt();
    T1_BACKUP_TIME_MS = dur;
    oledDisplay.show("T1 Backup", String(dur) + " ms");
  }
  else if (command.startsWith("T1EXIT ")) {
    unsigned long dur = command.substring(6).toInt();
    T1_EXIT_FORWARD_TIME_MS = dur;
    oledDisplay.show("T1 Exit", String(dur) + " ms");
  }
  else if (command.startsWith("T1SPEED ")) {
    int lvl = command.substring(7).toInt();
    if (lvl >= 1 && lvl <= 12) {
      T1_SPEED_LEVEL = lvl;
      setSpeedLevel(T1_SPEED_LEVEL);
      oledDisplay.show("T1 Speed", String(T1_SPEED_LEVEL));
    } else {
      Serial.println("Invalid T1 speed level. Use 1-12.");
    }
  }
  else if (command.startsWith("T1ITHRESH ")) {
    int v = command.substring(9).toInt();
    T1_INTERSECTION_WHITE_MIN = v;
    oledDisplay.show("T1 IR Thresh", String(v));
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
  Serial.println("Task 1 Plantation Configuration:");
  Serial.print("  T1TURN90 <ms> - 90° turn duration (current: "); Serial.print(T1_TURN_90_TIME_MS); Serial.println(" ms)");
  Serial.print("  T1TURN180 <ms> - 180° turn duration (current: "); Serial.print(T1_TURN_180_TIME_MS); Serial.println(" ms)");
  Serial.print("  T1BACKUP <ms> - Backup duration after turn (current: "); Serial.print(T1_BACKUP_TIME_MS); Serial.println(" ms)");
  Serial.print("  T1EXIT <ms> - Exit forward duration after last line (current: "); Serial.print(T1_EXIT_FORWARD_TIME_MS); Serial.println(" ms)");
  Serial.print("  T1SPEED <1-12> - Task1 default speed level (current: "); Serial.print(T1_SPEED_LEVEL); Serial.println(")");
  Serial.print("  T1ITHRESH <value> - IR white count threshold for intersections (current: "); Serial.print(T1_INTERSECTION_WHITE_MIN); Serial.println(")");
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
