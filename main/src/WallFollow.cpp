/*********************************************************************
 * Wall Following Library Implementation
 *********************************************************************/
// DESIGN PRINCIPLE: Control functions (start/stop/execute) DO NOT print
// repetitive serial output. Tasks using wall following should print
// meaningful context-specific status messages at state transitions.
/**********************************************************************/

#include "WallFollow.h"
#include "Motors.h"
#include "TOFSensors.h"

WallFollow wallFollow;

WallFollow::WallFollow() {
  kp = 1.0;
  kd = 0.5;
  targetDistance = 150;  // 15cm default
  baseSpeed = 60;
  maxCorrection = 100;
  lastError = 0;
  lastUpdateTime = 0;
  active = false;
}

void WallFollow::init() {
  kp = 1.0;
  kd = 0.5;
  targetDistance = 150;
  baseSpeed = 60;
  maxCorrection = 100;
  lastError = 0;
  lastUpdateTime = millis();
  active = false;
  Serial.println("Wall Following initialized");
}

void WallFollow::setKp(float newKp) {
  if (kp != newKp) {
    kp = newKp;
    Serial.print("Wall Follow Kp set to: ");
    Serial.println(kp, 3);
  }
}

void WallFollow::setKd(float newKd) {
  if (kd != newKd) {
    kd = newKd;
    Serial.print("Wall Follow Kd set to: ");
    Serial.println(kd, 3);
  }
}

void WallFollow::setTargetDistance(uint16_t distance) {
  if (targetDistance != distance) {
    targetDistance = distance;
    Serial.print("Wall Follow target distance set to: ");
    Serial.print(targetDistance);
    Serial.println(" mm");
  }
}

void WallFollow::setBaseSpeed(uint16_t speed) {
  if (baseSpeed != speed) {
    baseSpeed = speed;
    Serial.print("Wall Follow base speed set to: ");
    Serial.println(baseSpeed);
  }
}

void WallFollow::setMaxCorrection(int correction) {
  maxCorrection = correction;
  Serial.print("Wall Follow max correction set to: ");
  Serial.println(maxCorrection);
}

float WallFollow::getKp() {
  return kp;
}

float WallFollow::getKd() {
  return kd;
}

uint16_t WallFollow::getTargetDistance() {
  return targetDistance;
}

void WallFollow::executeWallFollow(uint16_t leftDistance) {
  if (!active) return;
  
  // Calculate error: positive = too far from wall, negative = too close
  float error = leftDistance - targetDistance;
  
  // Calculate time delta
  unsigned long currentTime = millis();
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0;  // Convert to seconds
  
  // Calculate derivative
  float derivative = 0;
  if (deltaTime > 0) {
    derivative = (error - lastError) / deltaTime;
  }
  
  // PD control calculation
  float correction = (kp * error) + (kd * derivative);
  
  // Limit correction
  correction = constrain(correction, -maxCorrection, maxCorrection);
  
  // Calculate motor speeds
  int leftSpeed = baseSpeed - correction;
  int rightSpeed = baseSpeed + correction;
  
  // Constrain speeds to valid range
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
  
  // Apply motor speeds
  setCurrentSpeed(baseSpeed);  // Set base speed
  setLeftMotorSpeed(leftSpeed);
  setRightMotorSpeed(rightSpeed);
  robotForward();
  
  // Update for next iteration
  lastError = error;
  lastUpdateTime = currentTime;
  
  // Debug output (optional, can be commented out)
  /*
  Serial.print("Wall Follow - Distance: ");
  Serial.print(leftDistance);
  Serial.print(" mm, Error: ");
  Serial.print(error);
  Serial.print(", Correction: ");
  Serial.print(correction);
  Serial.print(", L: ");
  Serial.print(leftSpeed);
  Serial.print(", R: ");
  Serial.println(rightSpeed);
  */
}

void WallFollow::start() {
  active = true;
  lastError = 0;
  lastUpdateTime = millis();
  // NOTE: Serial output removed to prevent spam in task loops
  // Tasks print their own status when starting wall following
}

void WallFollow::stop() {
  active = false;
  stopAllMotors();
  // NOTE: Serial output removed to prevent spam when called in task state transitions
  // Tasks should print their own contextual status messages when stopping wall follow
}

bool WallFollow::isActive() {
  return active;
}

float WallFollow::getLastError() {
  return lastError;
}
