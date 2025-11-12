// -------------------------------------------------------------------------
// MotorConfig.cpp - TB6612 Motor Driver Library Implementation
// -------------------------------------------------------------------------

#include "MotorConfig.h"

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------
int currentSpeed = 512; // Default to mid-range speed (~50%)

// -------------------------------------------------------------------------
// Motor Initialization
// -------------------------------------------------------------------------

void initMotors() {
  // Initialize Direction/Standby Pins as OUTPUTs
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // Enable the driver (STBY is active HIGH)
  digitalWrite(STBY, HIGH);
  
  // PWM Configuration for analogWrite()
  analogWriteResolution(PWMA, 10); 
  analogWriteFrequency(PWMA, freq); 
  analogWriteResolution(PWMB, 10); 
  analogWriteFrequency(PWMB, freq);
  
  Serial.println("TB6612 Motor Driver Initialized.");
}

// -------------------------------------------------------------------------
// Low-Level Motor Control Functions
// -------------------------------------------------------------------------

void setMotorA(int speed, bool forward) {
  // Clamp speed to valid range
  if (speed > maxSpeed) speed = maxSpeed;
  if (speed < 0) speed = 0;

  if (speed == 0) {
    // Stop: Low-Low braking/coasting
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
  } else if (forward) {
    // Forward direction
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
  } else {
    // Reverse direction
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
  }

  // Write PWM duty cycle (speed magnitude)
  analogWrite(PWMA, speed);
}

void setMotorB(int speed, bool forward) {
  // Clamp speed to valid range
  if (speed > maxSpeed) speed = maxSpeed;
  if (speed < 0) speed = 0;

  if (speed == 0) {
    // Stop: Low-Low braking/coasting
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
  } else if (forward) {
    // Forward direction
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
  } else {
    // Reverse direction
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
  }

  // Write PWM duty cycle (speed magnitude)
  analogWrite(PWMB, speed);
}

// -------------------------------------------------------------------------
// Individual Motor Control Functions
// -------------------------------------------------------------------------

void leftMotorForward() {
  setMotorA(currentSpeed, true);
  Serial.println("Left Motor Forward");
}

void leftMotorBackward() {
  setMotorA(currentSpeed, false);
  Serial.println("Left Motor Backward");
}

void rightMotorForward() {
  setMotorB(currentSpeed, true);
  Serial.println("Right Motor Forward");
}

void rightMotorBackward() {
  setMotorB(currentSpeed, false);
  Serial.println("Right Motor Backward");
}

// -------------------------------------------------------------------------
// Robot Movement Functions
// -------------------------------------------------------------------------

void robotForward() {
  setMotorA(currentSpeed, true);
  setMotorB(currentSpeed, true);
  Serial.println("Robot Forward");
}

void robotBackward() {
  setMotorA(currentSpeed, false);
  setMotorB(currentSpeed, false);
  Serial.println("Robot Backward");
}

void robotTurnLeft() {
  // Left motor backward, right motor forward (spin turn)
  setMotorA(currentSpeed / 2, false);
  setMotorB(currentSpeed, true);
  Serial.println("Robot Turn Left");
}

void robotTurnRight() {
  // Left motor forward, right motor backward (spin turn)
  setMotorA(currentSpeed, true);
  setMotorB(currentSpeed / 2, false);
  Serial.println("Robot Turn Right");
}

void stopAllMotors() {
  setMotorA(0, true);
  setMotorB(0, true);
  Serial.println("All Motors Stopped");
}

// -------------------------------------------------------------------------
// Speed Control Functions
// -------------------------------------------------------------------------

void setSpeedLevel(int level) {
  // Clamp level to valid range (1-10)
  if (level < 1) level = 1;
  if (level > speedLevels) level = speedLevels;
  
  currentSpeed = mapSpeedLevelToPWM(level);
  Serial.print("Speed Level Set to: ");
  Serial.print(level);
  Serial.print(" (PWM: ");
  Serial.print(currentSpeed);
  Serial.println(")");
}

int getCurrentSpeed() {
  return currentSpeed;
}

int mapSpeedLevelToPWM(int level) {
  // Map level (1-10) to speed range (100-255), then to PWM (0-1023)
  // First map to 8-bit range (100-255)
  int speed8bit = map(level, 1, speedLevels, minSpeedValue, maxSpeedValue);
  // Then map to 10-bit PWM range (0-1023)
  int speedPWM = map(speed8bit, 0, 255, 0, maxSpeed);
  return speedPWM;
}

// -------------------------------------------------------------------------
// Serial Command Processing
// -------------------------------------------------------------------------

void processCommand(String command) {
  command.trim();
  command.toUpperCase();
  
  // Speed Control Commands (SPEED1 to SPEED10)
  if (command.startsWith("SPEED")) {
    int level = command.substring(5).toInt();
    if (level >= 1 && level <= speedLevels) {
      setSpeedLevel(level);
    } else {
      Serial.println("Invalid speed level. Use SPEED1 to SPEED10.");
    }
  }
  // Individual Motor Control
  else if (command == "LMF") {
    leftMotorForward();
  }
  else if (command == "LMB") {
    leftMotorBackward();
  }
  else if (command == "RMF") {
    rightMotorForward();
  }
  else if (command == "RMB") {
    rightMotorBackward();
  }
  // Robot Movement
  else if (command == "RF") {
    robotForward();
  }
  else if (command == "RB") {
    robotBackward();
  }
  else if (command == "RTL") {
    robotTurnLeft();
  }
  else if (command == "RTR") {
    robotTurnRight();
  }
  else if (command == "STOP") {
    stopAllMotors();
  }
  else if (command == "HELP" || command == "?") {
    printCommands();
  }
  else {
    Serial.println("Unknown command. Type HELP for available commands.");
  }
}

void printCommands() {
  Serial.println("\n========================================");
  Serial.println("Serial Commands:");
  Serial.println("========================================");
  Serial.println("Speed Control:");
  Serial.println("  SPEED1 to SPEED10 (100-255, evenly divided)");
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
  Serial.println("Other:");
  Serial.println("  HELP or ? - Show this help menu");
  Serial.println("========================================\n");
}
