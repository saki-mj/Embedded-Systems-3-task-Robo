// -------------------------------------------------------------------------
// Motors.cpp - TB6612 Motor Driver Library Implementation
// -------------------------------------------------------------------------

#include "Motors.h"
#include "IRReading.h"
#include "LineFollow.h"

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------
int currentSpeed = 50; // Default to speed level 1 (50 PWM)

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
  // Map level (1-3) to PWM values (50, 100, 150)
  // Level 1 = 50 PWM
  // Level 2 = 100 PWM
  // Level 3 = 150 PWM
  switch(level) {
    case 1: return 50;
    case 2: return 100;
    case 3: return 150;
    default: return 50;  // Default to level 1
  }
}

