/*
 * ServoController.h
 * Library for controlling SG90 servo with ESP32-S3
 * Provides position control, speed control, and serial command interface
 */

#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class ServoController {
private:
  Servo servo;
  int pin;
  int currentPosition;
  int targetPosition;
  int minPulseWidth;
  int maxPulseWidth;
  int minAngle;
  int maxAngle;
  int speed;  // degrees per step
  unsigned long lastMoveTime;
  int moveDelay;  // milliseconds between steps
  
public:
  // Constructor
  ServoController(int servoPin);
  
  // Initialization
  void begin();
  void begin(int minPulse, int maxPulse);
  
  // Configuration
  void setMinMaxPulseWidth(int minPulse, int maxPulse);
  void setMinMaxAngles(int minDegree, int maxDegree);
  void setSpeed(int degreesPerSecond);
  
  // Position control
  void setPosition(int angle);
  void setPositionImmediate(int angle);
  void moveToPosition(int angle);  // Smooth movement with speed control
  
  // Getters
  int getCurrentPosition();
  int getTargetPosition();
  bool isMoving();
  
  // Update function (call in loop for smooth movement)
  void update();
  
  // Serial command interface
  void handleSerialCommand();
  void printStatus();
  void printHelp();
};

#endif
