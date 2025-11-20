/*********************************************************************
 * Task 3: Ramp Navigation Implementation (Gyro + TOF)
 *********************************************************************/

#include "Task3_Ramp.h"
#include "../Gyroscope.h"
#include "../TOFSensors.h"
#include "../Motors.h"
#include "../OLEDDisplay.h"
#include <Arduino.h>
#include <cmath>

Task3Ramp task3Ramp;

// External singletons (these exist elsewhere in your project)
extern Gyroscope gyroscope;
extern TOFSensors tofSensors;
extern OLEDDisplay oledDisplay; // adjust name if your display object differs


// Local tuning constants (adjust on your robot)
static const int BASE_SPEED_DEFAULT = 60;     // base speed (0-255)
static const unsigned long TURN_90_MS = 450;  // ms to approximate 90° turn (tune)
static const uint16_t WALL_AFTER_RAMP_MM = 100; // mm threshold to detect wall after ramp

// Pitch thresholds (degrees) - tune these values on the real ramp
static const float PITCH_CLIMB_START = 8.0f;   // when pitch > this, consider climbing started
static const float PITCH_NEAR_ZERO = 5.0f;     // within +/- this is considered near-flat/top
static const float PITCH_DESCEND = -3.0f;      // when pitch <= this, consider descending started

// Helper: safe constrain for speed
static inline int safeSpeed(int s) {
  if (s < 0) return 0;
  if (s > 255) return 255;
  return s;
}

// ---------------- Constructor ----------------
Task3Ramp::Task3Ramp() {
// reset state machine
  currentSubState = T3_INIT;
  subStateStartTime = 0;
  taskActive = false;

  // sensible defaults (can be changed via setters)
  climbDuration = 0;
  descendDuration = 0;
  rampDetectionDistance = 300; // mm - optional (not required when using gyro)
  topDetectionThreshold = 50;  // mm - optional
}

void Task3Ramp::init() {
  Serial.println("=== Task 3: Ramp - Initializing ===");

  currentSubState = T3_INIT;
  subStateStartTime = millis();
  taskActive = false;

  // defaults
  climbDuration = 0;
  descendDuration = 0;
  rampDetectionDistance = 300; // mm
  topDetectionThreshold = 50;  // mm

  oledDisplay.show("Task 3", "Ramp", "Initialized");
  delay(150);
}

// ---------------- Main execute() ----------------
void Task3Ramp::execute() {
  if (!taskActive) return;

  // Refresh sensors each cycle
  gyroscope.read();    // make sure gyroscope values are current
  tofSensors.readAll(); // update TOF values

  float pitch = gyroscope.getPitch(); // degrees, positive when climbing
  uint16_t frontDist = tofSensors.getFrontDistance();
  bool     frontOk = tofSensors.isFrontValid();

  // For debug: print pitch occasionally (not every loop)
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 500) {
    Serial.print("T3: state=");
    Serial.print(getSubStateName());
    Serial.print(" pitch=");
    Serial.print(pitch);
    Serial.print(" frontDist=");
    if (frontOk) Serial.print(frontDist); else Serial.print("INV");
    Serial.println();
    lastDebug = millis();
  }

  switch (currentSubState) {
    case T3_INIT: {
      Serial.println("Task 3: INIT -> APPROACH");
      setSubState(T3_APPROACH);
      break;
    }

    case T3_APPROACH: {
      setCurrentSpeed(BASE_SPEED_DEFAULT);
      robotForward();

      // If gyro indicates climb start OR front distance indicates ramp presence, go to CLIMBING
      bool rampDetectedByTOF = frontOk && (frontDist <= rampDetectionDistance);
      if (pitch >= PITCH_CLIMB_START || rampDetectedByTOF) {
        Serial.print("Task 3: Detected ramp start. pitch=");
        Serial.println(pitch);
        setSubState(T3_CLIMBING);
      }
      break;
    }

    case T3_CLIMBING: {
      // While climbing (pitch positive and above threshold), increase speed to 3x base.
      if (pitch >= PITCH_CLIMB_START) {
        int sp = safeSpeed(BASE_SPEED_DEFAULT * 3);
        setCurrentSpeed(sp);
        robotForward();
        Serial.print("Task 3: CLIMBING at speed ");
        Serial.println(sp);
      } else {
        // If pitch fell below climb start (rare), slow to base
        setCurrentSpeed(BASE_SPEED_DEFAULT);
        robotForward();
      }

      // If pitch approaches zero (top of ramp), transition to DESCENDING state where we will
      // observe negative pitch to detect descent. We treat near-zero as top.
      if (fabs(pitch) <= PITCH_NEAR_ZERO) {
        Serial.print("Task 3: Near top (pitch=");
        Serial.print(pitch);
        Serial.println(") -> consider top/descend");
        setSubState(T3_DESCENDING);
      }
      break;
    }

    case T3_DESCENDING: {
      // When descending, pitch becomes negative. While negative, reduce speed to half base.
      if (pitch <= PITCH_DESCEND) {
        int sp = safeSpeed(std::max(1, BASE_SPEED_DEFAULT / 2));
        setCurrentSpeed(sp);
        robotForward();
        Serial.print("Task 3: DESCENDING at reduced speed ");
        Serial.println(sp);
      } else if (fabs(pitch) <= PITCH_NEAR_ZERO) {
        // When near flat after descent, move at base speed and prepare to finish
        setCurrentSpeed(BASE_SPEED_DEFAULT);
        robotForward();
      } else {
        // If still slightly positive or ambiguous, continue at base speed - wait for stabilization
        setCurrentSpeed(BASE_SPEED_DEFAULT);
        robotForward();
      }

      // Finishing condition: once the robot is roughly flat and stable, we can mark ramp done
      // and move forward until a wall is seen, then perform the right turn.
      // Here we look for flat pitch and then switch to COMPLETED where we check for wall detection.
      if (fabs(pitch) <= PITCH_NEAR_ZERO) {
        // Give it a short stabilization period first (use subStateStartTime)
        if (millis() - subStateStartTime >= 300) { // 300 ms settle
          Serial.println("Task 3: Ramp finished (flat) -> proceed to search wall");
          setSubState(T3_COMPLETED);
        }
      }
      break;
    }

    case T3_COMPLETED: {
      setCurrentSpeed(BASE_SPEED_DEFAULT);
      robotForward();

      if (frontOk && frontDist <= WALL_AFTER_RAMP_MM) {
        Serial.print("Task 3: Wall detected after ramp at ");
        Serial.print(frontDist);
        Serial.println(" mm -> stopping and turning right 90deg");

        stopAllMotors();
        delay(100);

        robotTurnRight();
        delay(TURN_90_MS);
        stopAllMotors();

        Serial.println("Task 3: Turn complete; stopping task");
        taskActive = false;
        setSubState(T3_COMPLETED);
      }
      break;
    }

    default: {
      Serial.println("Task 3: Unknown state -> reset");
      setSubState(T3_INIT);
      break;
    }
  } // switch
}

// ---------------- Display & state helpers ----------------
void Task3Ramp::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 3: Ramp",
    "State: " + getSubStateName(),
    "Time: " + String((millis() - subStateStartTime) / 1000) + "s"
  );
}

void Task3Ramp::setSubState(Task3SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 3 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task3SubState Task3Ramp::getSubState() {
  return currentSubState;
}

String Task3Ramp::getSubStateName() {
  switch(currentSubState) {
    case T3_INIT: return "INIT";
    case T3_APPROACH: return "APPROACH";
    case T3_CLIMBING: return "CLIMBING";
    case T3_DESCENDING: return "DESCENDING";
    case T3_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task3Ramp::start() {
  Serial.println("Starting Task 3: Ramp");
  taskActive = true;
  setSubState(T3_INIT);
}

void Task3Ramp::stop() {
  Serial.println("Stopping Task 3: Ramp");
  taskActive = false;
  stopAllMotors();
}

bool Task3Ramp::isActive() {
  return taskActive;
}

bool Task3Ramp::isCompleted() {
  return (currentSubState == T3_COMPLETED);
}

void Task3Ramp::reset() {
  currentSubState = T3_INIT;
  subStateStartTime = millis();
  taskActive = false;
}

// ---------------- Configuration setters ----------------
void Task3Ramp::setClimbDuration(unsigned long timeMs) {
  climbDuration = timeMs;
  Serial.print("T3 Climb duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task3Ramp::setDescendDuration(unsigned long timeMs) {
  descendDuration = timeMs;
  Serial.print("T3 Descend duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task3Ramp::setRampDetectionDistance(uint16_t distance) {
  rampDetectionDistance = distance;
  Serial.print("T3 Ramp detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task3Ramp::setTopDetectionThreshold(uint16_t threshold) {
  topDetectionThreshold = threshold;
  Serial.print("T3 Top detection threshold set to: ");
  Serial.print(threshold);
  Serial.println(" mm");
}

// ---------------- Configuration getters ----------------
unsigned long Task3Ramp::getClimbDuration() {
  return climbDuration;
}

unsigned long Task3Ramp::getDescendDuration() {
  return descendDuration;
}

uint16_t Task3Ramp::getRampDetectionDistance() {
  return rampDetectionDistance;
}

uint16_t Task3Ramp::getTopDetectionThreshold() {
  return topDetectionThreshold;
}