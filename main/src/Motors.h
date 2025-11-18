// -------------------------------------------------------------------------
// Motors.h - TB6612 Motor Driver Library for ESP32-S3
// -------------------------------------------------------------------------

#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

// -------------------------------------------------------------------------
// TB6612 Motor Driver Pin Connections
// -------------------------------------------------------------------------

// --- Motor A (Left Motor) Connections ---
const int PWMA = 4;   // PWM Speed Control for Motor A
const int AIN2 = 5;   // Direction Control AIN2
const int AIN1 = 6;   // Direction Control AIN1

// --- Motor B (Right Motor) Connections ---
const int BIN1 = 15;  // Direction Control BIN1
const int BIN2 = 16;  // Direction Control BIN2
const int PWMB = 17;  // PWM Speed Control for Motor B

// --- Shared Control ---
const int STBY = 7;   // Standby Pin (Active HIGH)

// --- PWM Setup Constants ---
const int maxSpeed = 1023; // 10-bit resolution (0 to 1023)
const int freq = 30000;    // PWM frequency in Hz (30 kHz)

// --- Speed Settings ---
const int minSpeedPWM = 25;    // Minimum PWM value (speed level 1)
const int maxSpeedPWM = 1023;  // Maximum PWM value (10-bit)
const int speedLevels = 20;    // Number of speed levels (1-20)

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------
extern int currentSpeed;

// -------------------------------------------------------------------------
// Motor Control Functions
// -------------------------------------------------------------------------

/**
 * @brief Initialize motor driver pins and PWM settings
 */
void initMotors();

/**
 * @brief Controls Motor A (Left Motor) speed and direction
 * @param speed Magnitude of speed (0 to 1023)
 * @param forward True for forward, False for reverse
 */
void setMotorA(int speed, bool forward);

/**
 * @brief Controls Motor B (Right Motor) speed and direction
 * @param speed Magnitude of speed (0 to 1023)
 * @param forward True for forward, False for reverse
 */
void setMotorB(int speed, bool forward);

// -------------------------------------------------------------------------
// Individual Motor Control Functions
// -------------------------------------------------------------------------

/**
 * @brief Left Motor Forward
 */
void leftMotorForward();

/**
 * @brief Left Motor Backward
 */
void leftMotorBackward();

/**
 * @brief Right Motor Forward
 */
void rightMotorForward();

/**
 * @brief Right Motor Backward
 */
void rightMotorBackward();

// -------------------------------------------------------------------------
// Robot Movement Functions
// -------------------------------------------------------------------------

/**
 * @brief Move robot forward
 */
void robotForward();

/**
 * @brief Move robot backward
 */
void robotBackward();

/**
 * @brief Turn robot left (left motor slower/stopped, right motor forward)
 */
void robotTurnLeft();

/**
 * @brief Turn robot right (right motor slower/stopped, left motor forward)
 */
void robotTurnRight();

/**
 * @brief Stop all motors
 */
void stopAllMotors();

// -------------------------------------------------------------------------
// Speed Control Functions
// -------------------------------------------------------------------------

/**
 * @brief Set speed level (1-20)
 * @param level Speed level from 1 to 20
 */
void setSpeedLevel(int level);

/**
 * @brief Set current speed directly (for advanced control)
 * @param speed PWM value (0-1023)
 */
void setCurrentSpeed(int speed);

/**
 * @brief Set left motor speed independently (for wall following, etc.)
 * @param speed PWM value (0-1023)
 */
void setLeftMotorSpeed(int speed);

/**
 * @brief Set right motor speed independently (for wall following, etc.)
 * @param speed PWM value (0-1023)
 */
void setRightMotorSpeed(int speed);

/**
 * @brief Get current speed value in PWM units (0-1023)
 * @return Current speed value
 */
int getCurrentSpeed();

/**
 * @brief Map speed level (1-20) to PWM value
 * @param level Speed level from 1 to 20
 * @return PWM value
 */
int mapSpeedLevelToPWM(int level);

#endif // MOTORS_H
