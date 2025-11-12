// -------------------------------------------------------------------------
// Motors.cpp - TB6612 Motor Driver Library Implementation
// -------------------------------------------------------------------------

#include "Motors.h"
#include "IRReading.h"
#include "LineFollow.h"

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------
int currentSpeed = 25; // Default to speed level 1 (25 PWM)

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
  // Clamp level to valid range (1-12)
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
  // Speed mapping as requested:
  // Level 1 = 25 PWM
  // Level 2 = 35 PWM
  // Level 3 = 50 PWM
  // Level 4 = 100 PWM
  // Level 5 = 150 PWM
  // Level 6 = 200 PWM
  // Levels 7-12 evenly distributed from 200 to 1023
  switch(level) {
    case 1: return 40;
    case 2: return 45;
    case 3: return 50;
    case 4: return 75;
    case 5: return 100;
    case 6: return 200;
    case 7: return 337;   // 200 + (1023-200)/6 * 1
    case 8: return 474;   // 200 + (1023-200)/6 * 2
    case 9: return 611;   // 200 + (1023-200)/6 * 3
    case 10: return 748;  // 200 + (1023-200)/6 * 4
    case 11: return 885;  // 200 + (1023-200)/6 * 5
    case 12: return 1023; // Maximum speed
    default: return 25;   // Default to level 1
  }
}

