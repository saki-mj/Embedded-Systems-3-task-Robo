// -------------------------------------------------------------------------
// Motors.cpp - TB6612 Motor Driver Library Implementation
// -------------------------------------------------------------------------
// DESIGN PRINCIPLE: Low-level motor control functions DO NOT print serial output
// to prevent spam when called repeatedly in loops or tasks.
// High-level task code and manual serial commands handle status messages.
// -------------------------------------------------------------------------

#include "Motors.h"
#include "IRReading.h"
#include "LineFollow.h"

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
  setMotorA(baseSpeed, true);
  // Serial output removed to prevent spam - only used for manual commands via serial
}

void leftMotorBackward() {
  setMotorA(baseSpeed, false);
  // Serial output removed to prevent spam - only used for manual commands via serial
}

void rightMotorForward() {
  setMotorB(baseSpeed, true);
  // Serial output removed to prevent spam - only used for manual commands via serial
}

void rightMotorBackward() {
  setMotorB(baseSpeed, false);
  // Serial output removed to prevent spam - only used for manual commands via serial
}

// -------------------------------------------------------------------------
// Robot Movement Functions
// -------------------------------------------------------------------------

void robotForward() {
  setMotorA(baseSpeed, true);
  setMotorB(baseSpeed, true);
  // Serial output removed to prevent spam in loops - use task-level status messages instead
}

void robotBackward() {
  setMotorA(baseSpeed, false);
  setMotorB(baseSpeed, false);
  // Serial output removed to prevent spam in loops - use task-level status messages instead
}

void robotTurnLeft() {
  // Left motor backward, right motor forward (spin turn)
  setMotorA(rotateSpeed, false);
  setMotorB(rotateSpeed, true);
  // Serial output removed to prevent spam in loops - use task-level status messages instead
}

void robotTurnRight() {
  // Left motor forward, right motor backward (spin turn)
  setMotorA(rotateSpeed, true);
  setMotorB(rotateSpeed, false);
  // Serial output removed to prevent spam in loops - use task-level status messages instead
}

void stopAllMotors() {
  setMotorA(0, true);
  setMotorB(0, true);
  // NOTE: Serial output removed to prevent spam when called in loops/tasks
  // Tasks should print their own contextual status messages
  // Manual STOP command in SerialCommands.cpp will print status
}

// -------------------------------------------------------------------------
// Speed Control Functions
// -------------------------------------------------------------------------

void setMotorSpeed(int speed) {
  // Used for directly setting motor speed in specific scenarios
  speed = constrain(speed, 0, maxSpeed);
  setMotorA(speed, true);
  setMotorB(speed, true);
}

void setLeftMotorSpeed(int speed) {
  // Left motor speed control (Motor A)
  speed = constrain(speed, 0, maxSpeed);
  // Keep current direction, just change speed
  // This will be used in conjunction with robotForward() or other direction commands
  analogWrite(PWMA, speed);
}

void setRightMotorSpeed(int speed) {
  // Right motor speed control (Motor B)
  speed = constrain(speed, 0, maxSpeed);
  // Keep current direction, just change speed
  // This will be used in conjunction with robotForward() or other direction commands
  analogWrite(PWMB, speed);
}

int getBaseSpeed() {
  return baseSpeed;
}

int getRotateSpeed() {
  return rotateSpeed;
}

