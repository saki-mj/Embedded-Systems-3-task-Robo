/*********************************************************************
 * Serial Commands Library Implementation
 *********************************************************************/

#include "SerialCommands.h"
#include "Motors.h"
#include "IRReading.h"
#include "LineFollow.h"
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
    oledDisplay.show("Robot", "STOPPED");
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
