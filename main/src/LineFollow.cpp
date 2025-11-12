// -------------------------------------------------------------------------
// LineFollow.cpp - PD Controller Line Following Implementation
// -------------------------------------------------------------------------

#include "LineFollow.h"
#include "IRReading.h"
#include "Motors.h"

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------

float Kp = 15.0;              // Proportional gain (default)
float Kd = 5.0;               // Derivative gain (default)
bool lineFollowActive = false; // Flag for line following mode
bool crossDetected = false;    // Flag for cross detection
bool invertLine = false;       // Flag for line color (false = white on black, true = black on white)

float lastError = 0;          // Previous error for derivative calculation

// Sensor weights for position calculation
// Center is between sensors 7 and 8 (position 0)
const int sensorWeights[16] = {
  -7, -6, -5, -4, -3, -2, -1, 0,  // Sensors 0-7
   0, 1,  2,  3,  4,  5,  6,  7   // Sensors 8-15
};

// -------------------------------------------------------------------------
// Line Following Initialization
// -------------------------------------------------------------------------

void initLineFollow() {
  lineFollowActive = false;
  crossDetected = false;
  lastError = 0;
  Serial.println("Line Following System Initialized.");
  Serial.print("Default Kp: "); Serial.println(Kp);
  Serial.print("Default Kd: "); Serial.println(Kd);
}

// -------------------------------------------------------------------------
// Line Position Calculation
// -------------------------------------------------------------------------

float calculateLinePosition() {
  // Read binary sensor values
  readAllIRSensorsBinary();
  
  float weightedSum = 0;
  int activeCount = 0;
  
  // Determine which value represents the line (1 or 0)
  int lineValue = invertLine ? 0 : 1;  // If inverted, look for 0 (black), else look for 1 (white)
  
  // Calculate weighted average of sensor positions
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    if (irBinary[i] == lineValue) {  // Line detected
      weightedSum += sensorWeights[i];
      activeCount++;
    }
  }
  
  // If no sensors detect the line, return last known position
  if (activeCount == 0) {
    return lastError;  // Lost line - use last error
  }
  
  // Return average position
  return weightedSum / activeCount;
}

// -------------------------------------------------------------------------
// Cross Detection
// -------------------------------------------------------------------------

bool detectCross() {
  // Read binary sensor values
  readAllIRSensorsBinary();
  
  // Determine which value represents the line (1 or 0)
  int lineValue = invertLine ? 0 : 1;  // If inverted, look for 0 (black), else look for 1 (white)
  
  // Check if all sensors from 3 to 12 detect the line
  for (int i = CROSS_START; i <= CROSS_END; i++) {
    if (irBinary[i] != lineValue) {
      return false;  // Not all sensors detect line
    }
  }
  
  return true;  // All sensors 3-12 detect line = CROSS!
}

// -------------------------------------------------------------------------
// PD Controller
// -------------------------------------------------------------------------

int calculatePD(float position) {
  // Position error (how far line is from center)
  float error = position;
  
  // Derivative (rate of change of error)
  float derivative = error - lastError;
  
  // PD control calculation
  float correction = (Kp * error) + (Kd * derivative);
  
  // Store error for next iteration
  lastError = error;
  
  // Return correction value as integer
  return (int)correction;
}

// -------------------------------------------------------------------------
// Line Following Execution
// -------------------------------------------------------------------------

void executeLineFollow() {
  // Check for cross detection first
  if (detectCross()) {
    if (!crossDetected) {  // Only trigger once
      crossDetected = true;
      stopAllMotors();
      Serial.println("\n*** CROSS DETECTED! ***");
      Serial.println("Line following paused. Waiting for command...");
      lineFollowActive = false;  // Stop line following
      return;
    }
  }
  
  // Calculate line position (-7 to +7)
  float position = calculateLinePosition();
  
  // Calculate PD correction
  int correction = calculatePD(position);
  
  // Get current base speed from motor config
  int baseSpeed = getCurrentSpeed();
  
  // Calculate left and right motor speeds
  int leftSpeed = baseSpeed + correction;
  int rightSpeed = baseSpeed - correction;
  
  // Clamp speeds to valid range (0 to maxSpeed)
  if (leftSpeed > maxSpeed) leftSpeed = maxSpeed;
  if (leftSpeed < 0) leftSpeed = 0;
  if (rightSpeed > maxSpeed) rightSpeed = maxSpeed;
  if (rightSpeed < 0) rightSpeed = 0;
  
  // Set motor speeds
  setMotorA(leftSpeed, true);   // Left motor
  setMotorB(rightSpeed, true);  // Right motor
}

// -------------------------------------------------------------------------
// Line Following Control
// -------------------------------------------------------------------------

void toggleLineFollow() {
  lineFollowActive = !lineFollowActive;
  
  if (lineFollowActive) {
    // Check if IR sensors are calibrated
    extern bool irCalibrated;
    if (!irCalibrated) {
      Serial.println("ERROR: IR sensors not calibrated!");
      Serial.println("Run IRCALIBRATE first.");
      lineFollowActive = false;
      return;
    }
    
    lastError = 0;  // Reset error
    crossDetected = false;  // Reset cross detection
    Serial.println("Line Following: ENABLED");
    Serial.print("Kp: "); Serial.print(Kp);
    Serial.print(" | Kd: "); Serial.println(Kd);
    Serial.print("Base Speed: "); Serial.println(getCurrentSpeed());
  } else {
    stopAllMotors();
    Serial.println("Line Following: DISABLED");
  }
}

// -------------------------------------------------------------------------
// PD Gain Control
// -------------------------------------------------------------------------

void setKp(float value) {
  Kp = value;
  Serial.print("Kp set to: ");
  Serial.println(Kp);
}

void setKd(float value) {
  Kd = value;
  Serial.print("Kd set to: ");
  Serial.println(Kd);
}

float getKp() {
  return Kp;
}

float getKd() {
  return Kd;
}

bool isLineFollowActive() {
  return lineFollowActive;
}

void resetCrossDetection() {
  crossDetected = false;
  Serial.println("Cross detection reset.");
}

void toggleLineColor() {
  invertLine = !invertLine;
  
  if (invertLine) {
    Serial.println("Line Mode: BLACK line on WHITE background");
  } else {
    Serial.println("Line Mode: WHITE line on BLACK background");
  }
}

bool isLineInverted() {
  return invertLine;
}
