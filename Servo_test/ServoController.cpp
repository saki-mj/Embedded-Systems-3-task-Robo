/*
 * ServoController.cpp
 * Implementation of ServoController library
 */

#include "ServoController.h"

ServoController::ServoController(int servoPin) {
  pin = servoPin;
  currentPosition = 90;
  targetPosition = 90;
  minPulseWidth = 500;
  maxPulseWidth = 2400;
  minAngle = 0;
  maxAngle = 180;
  speed = 60;  // Default: 60 degrees per second
  lastMoveTime = 0;
  moveDelay = 50;  // 50ms between steps for smooth movement
}

void ServoController::begin() {
  begin(minPulseWidth, maxPulseWidth);
}

void ServoController::begin(int minPulse, int maxPulse) {
  minPulseWidth = minPulse;
  maxPulseWidth = maxPulse;
  
  // Allocate timers for ESP32
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  // Attach servo with specified pulse width range
  servo.setPeriodHertz(50);
  servo.attach(pin, minPulseWidth, maxPulseWidth);
  
  // Set to center position
  servo.write(currentPosition);
}

void ServoController::setMinMaxPulseWidth(int minPulse, int maxPulse) {
  minPulseWidth = constrain(minPulse, 400, 1000);
  maxPulseWidth = constrain(maxPulse, 2000, 2600);
  
  // Reattach with new values
  servo.detach();
  servo.setPeriodHertz(50);
  servo.attach(pin, minPulseWidth, maxPulseWidth);
  
  Serial.print("Pulse width set to: ");
  Serial.print(minPulseWidth);
  Serial.print("μs - ");
  Serial.print(maxPulseWidth);
  Serial.println("μs");
}

void ServoController::setMinMaxAngles(int minDegree, int maxDegree) {
  minAngle = constrain(minDegree, 0, 180);
  maxAngle = constrain(maxDegree, 0, 180);
  
  Serial.print("Angle range set to: ");
  Serial.print(minAngle);
  Serial.print("° - ");
  Serial.print(maxAngle);
  Serial.println("°");
}

void ServoController::setSpeed(int degreesPerSecond) {
  speed = constrain(degreesPerSecond, 1, 500);
  // Calculate delay between steps (assuming ~3 degrees per step for smoothness)
  moveDelay = (3000 / speed);  // milliseconds
  if (moveDelay < 10) moveDelay = 10;  // Minimum 10ms
  
  Serial.print("Speed set to: ");
  Serial.print(speed);
  Serial.println("°/s");
}

void ServoController::setPosition(int angle) {
  angle = constrain(angle, minAngle, maxAngle);
  targetPosition = angle;
}

void ServoController::setPositionImmediate(int angle) {
  angle = constrain(angle, minAngle, maxAngle);
  currentPosition = angle;
  targetPosition = angle;
  servo.write(angle);
}

void ServoController::moveToPosition(int angle) {
  setPosition(angle);
}

int ServoController::getCurrentPosition() {
  return currentPosition;
}

int ServoController::getTargetPosition() {
  return targetPosition;
}

bool ServoController::isMoving() {
  return currentPosition != targetPosition;
}

void ServoController::update() {
  if (!isMoving()) {
    return;
  }
  
  unsigned long currentTime = millis();
  if (currentTime - lastMoveTime < moveDelay) {
    return;
  }
  
  lastMoveTime = currentTime;
  
  // Calculate step size based on speed
  int step = (speed * moveDelay) / 1000;
  if (step < 1) step = 1;
  
  // Move towards target
  if (currentPosition < targetPosition) {
    currentPosition += step;
    if (currentPosition > targetPosition) {
      currentPosition = targetPosition;
    }
  } else if (currentPosition > targetPosition) {
    currentPosition -= step;
    if (currentPosition < targetPosition) {
      currentPosition = targetPosition;
    }
  }
  
  servo.write(currentPosition);
}

void ServoController::handleSerialCommand() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.length() == 0) {
      return;
    }
    
    // Parse command
    char cmd = command.charAt(0);
    
    switch (cmd) {
      case 'p':  // Set position: p90
      case 'P': {
        int angle = command.substring(1).toInt();
        moveToPosition(angle);
        Serial.print("Moving to ");
        Serial.print(angle);
        Serial.println("°");
        break;
      }
      
      case 'i':  // Immediate position: i90
      case 'I': {
        int angle = command.substring(1).toInt();
        setPositionImmediate(angle);
        Serial.print("Set immediate to ");
        Serial.print(angle);
        Serial.println("°");
        break;
      }
      
      case 's':  // Set speed: s120
      case 'S': {
        int newSpeed = command.substring(1).toInt();
        setSpeed(newSpeed);
        break;
      }
      
      case 'm':  // Set min/max pulse: m500,2400
      case 'M': {
        int commaIndex = command.indexOf(',');
        if (commaIndex > 0) {
          int minPulse = command.substring(1, commaIndex).toInt();
          int maxPulse = command.substring(commaIndex + 1).toInt();
          setMinMaxPulseWidth(minPulse, maxPulse);
        }
        break;
      }
      
      case 'a':  // Set min/max angles: a0,180
      case 'A': {
        int commaIndex = command.indexOf(',');
        if (commaIndex > 0) {
          int minDeg = command.substring(1, commaIndex).toInt();
          int maxDeg = command.substring(commaIndex + 1).toInt();
          setMinMaxAngles(minDeg, maxDeg);
        }
        break;
      }
      
      case '?':  // Status
        printStatus();
        break;
      
      case 'h':  // Help
      case 'H':
        printHelp();
        break;
      
      default:
        Serial.println("Unknown command. Send 'h' for help.");
    }
  }
}

void ServoController::printStatus() {
  Serial.println("\n=== Servo Status ===");
  Serial.print("Current Position: ");
  Serial.print(currentPosition);
  Serial.println("°");
  Serial.print("Target Position: ");
  Serial.print(targetPosition);
  Serial.println("°");
  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.println("°/s");
  Serial.print("Pulse Width Range: ");
  Serial.print(minPulseWidth);
  Serial.print("μs - ");
  Serial.print(maxPulseWidth);
  Serial.println("μs");
  Serial.print("Angle Range: ");
  Serial.print(minAngle);
  Serial.print("° - ");
  Serial.print(maxAngle);
  Serial.println("°");
  Serial.print("Moving: ");
  Serial.println(isMoving() ? "Yes" : "No");
  Serial.println("==================\n");
}

void ServoController::printHelp() {
  Serial.println("\n=== Servo Commands ===");
  Serial.println("p<angle>      - Move to position (e.g., p90)");
  Serial.println("i<angle>      - Set immediate position (e.g., i45)");
  Serial.println("s<speed>      - Set speed in °/s (e.g., s120)");
  Serial.println("m<min>,<max>  - Set pulse width μs (e.g., m500,2400)");
  Serial.println("a<min>,<max>  - Set angle range (e.g., a0,180)");
  Serial.println("?             - Show status");
  Serial.println("h             - Show this help");
  Serial.println("=====================\n");
}
