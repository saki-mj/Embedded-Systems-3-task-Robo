/*********************************************************************
 * Task 1: Plantation Task Implementation
 *********************************************************************/

#include "Task1_Plantation.h"
#include "BallCollector.h"
#include "../Motors.h"
#include "../IRReading.h"
#include "../LineFollow.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"
#include "../ColorSensors.h"

// Global task object
Task1Plantation task1Plantation;

// Constructor
Task1Plantation::Task1Plantation() {
  currentSubState = T1_INIT;
  subStateStartTime = 0;
  taskActive = false;
}

// Initialize task
void Task1Plantation::init() {
  Serial.println("=== Task 1: Plantation - Initializing ===");
  currentSubState = T1_INIT;
  subStateStartTime = millis();
  taskActive = false;

  // Reset plantation sweep state
  t1PendingTurn        = T1_TURN_NONE;
  t1NextStateAfterTurn = T1_LINE_FOLLOWING;
  t1ColumnIndex        = 0;
  t1IntersectionCount  = 0;
  t1ReturningAlongColumn = false;
  t1IntersectionLatched  = false;

  setSpeedLevel(6);   // medium speed (tune if needed)

  oledDisplay.show("Task 1", "Plantation", "Initialized");
  delay(1000);
}
// ---------- Plantation task helpers & state variables ----------

// Turn command type for T1_TURNING
enum T1TurnCommand {
  T1_TURN_NONE,
  T1_TURN_RIGHT_90,
  T1_TURN_LEFT_90,
  T1_TURN_180
};

// Tunable timings (milliseconds)  --- you will adjust on the robot ---
const unsigned long T1_TURN_90_TIME_MS  = 500;   // right 90° (tune)
const unsigned long T1_TURN_180_TIME_MS = 1000;  // 180° (tune)
const unsigned long T1_BACKUP_TIME_MS   = 1000;  // as you said (1 s)

const int T1_NUM_INTERSECTIONS = 3;   // your grid: 3 intersections per line
const int T1_INTERSECTION_WHITE_MIN = 7; // how many white sensors = "intersection" (tune)

// State for current plantation sweep
static T1TurnCommand  t1PendingTurn      = T1_TURN_NONE;
static Task1SubState  t1NextStateAfterTurn = T1_LINE_FOLLOWING;

static int  t1ColumnIndex          = 0;   // 0 = first column, 1 = second column
static int  t1IntersectionCount    = 0;   // number of intersections seen in this direction
static bool t1ReturningAlongColumn = false; // false = going "forward", true = coming back
static bool t1IntersectionLatched  = false; // to avoid double-counting one intersection
// ---------- Plantation task main methods ----------


// Execute task
void Task1Plantation::execute() {
  if (!taskActive) return;

  // ---------- MAIN SUB-STATE MACHINE ----------
  switch (currentSubState) {

    // -----------------------------------------------------------
    // 1) INIT: reset everything and start searching for main line
    // -----------------------------------------------------------
    case T1_INIT: {
      Serial.println("Task 1: INIT state");

      // Reset plantation sweep state
      t1PendingTurn        = T1_TURN_NONE;
      t1NextStateAfterTurn = T1_LINE_FOLLOWING;
      t1ColumnIndex        = 0;   // first column
      t1IntersectionCount  = 0;
      t1ReturningAlongColumn = false;
      t1IntersectionLatched  = false;

      setSpeedLevel(6);   // moderate speed
      robotForward();     // move from yellow start head into arena

      // Go to SEARCHING: move forward until IR 3..15 see the white line
      setSubState(T1_SEARCHING);
      break;
    }

    // -----------------------------------------------------------
    // 2) SEARCHING: move forward until IR sensors 3..15 see white line
    // -----------------------------------------------------------
    case T1_SEARCHING: {
      // Move forward and watch the IR array
      readAllIRSensorsBinary();

      bool lineDetected = false;
      // User said: detect white line by IR sensor 3 to 15
      for (int i = 3; i <= 15; i++) {
        if (irBinary[i] == 1) {
          lineDetected = true;
          break;
        }
      }

      if (!lineDetected) {
        // Still not on the main grid line -> keep moving forward
        robotForward();
      } else {
        // Found the main grid line -> stop and prepare right 90° turn
        Serial.println("Task 1: main line detected, prepare 90 deg right");
        stopAllMotors();
        t1PendingTurn        = T1_TURN_RIGHT_90;
        t1NextStateAfterTurn = T1_FOLLOWING;   // backup 1s then line follow
        setSubState(T1_TURNING);
      }
      break;
    }

    // -----------------------------------------------------------
    // 3) FOLLOWING (used here as "BACKUP then go to LINE_FOLLOWING")
    //    - after first 90° turn: come back 1000 ms then go forward
    // -----------------------------------------------------------
    case T1_FOLLOWING: {
      Serial.println("Task 1: FOLLOWING (BACKUP) state");
      unsigned long elapsed = millis() - subStateStartTime;

      if (elapsed < T1_BACKUP_TIME_MS) {
        // "robo car come back 1000ms"
        robotBackward();
      } else {
        // Backup finished -> start moving forward along first column
        Serial.println("Task 1: backup done, start LINE_FOLLOWING");
        t1IntersectionCount    = 0;
        t1ReturningAlongColumn = false;
        t1IntersectionLatched  = false;

        // start line follow on the column
        setSubState(T1_LINE_FOLLOWING);
      }
      break;
    }

    // -----------------------------------------------------------
    // 4) TURNING: perform pending 90° / 180° turn using time control
    // -----------------------------------------------------------
    case T1_TURNING: {
      unsigned long elapsed = millis() - subStateStartTime;

      switch (t1PendingTurn) {
        case T1_TURN_RIGHT_90:
          if (elapsed < T1_TURN_90_TIME_MS) {
            robotTurnRight();
          } else {
            stopAllMotors();
            Serial.println("Task 1: 90 deg right turn finished");
            t1PendingTurn = T1_TURN_NONE;
            setSubState(t1NextStateAfterTurn);
          }
          break;

        case T1_TURN_LEFT_90:
          if (elapsed < T1_TURN_90_TIME_MS) {
            robotTurnLeft();
          } else {
            stopAllMotors();
            Serial.println("Task 1: 90 deg left turn finished");
            t1PendingTurn = T1_TURN_NONE;
            setSubState(t1NextStateAfterTurn);
          }
          break;

        case T1_TURN_180:
          if (elapsed < T1_TURN_180_TIME_MS) {
            // 180° = spin in place; reuse right spin
            robotTurnRight();
          } else {
            stopAllMotors();
            Serial.println("Task 1: 180 deg turn finished");
            t1PendingTurn = T1_TURN_NONE;
            setSubState(t1NextStateAfterTurn);
          }
          break;

        case T1_TURN_NONE:
        default:
          // Nothing to do, just go to next state
          setSubState(t1NextStateAfterTurn);
          break;
      }

      break;
    }

    // -----------------------------------------------------------
    // 5) LINE_FOLLOWING:
    //    - follow the line
    //    - detect intersections (3 per column)
    //    - at each intersection: read color sensor
    //      * if GREEN -> go to COLLECTING (BallCollector.cpp)
    //      * else, at 3rd intersection with no green:
    //          - turn 180°
    //          - come back along same column
    //      * when coming back and hitting 3rd intersection again:
    //          - turn right 90°
    //          - move into next column (or finish after 2nd col)
    // -----------------------------------------------------------
    case T1_LINE_FOLLOWING: {
      // 1) Keep line following
      executeLineFollow();    // PD line follow from LineFollow.h

      // 2) Detect intersection with IR array
      readAllIRSensorsBinary();
      int whiteCount = 0;
      for (int i = 0; i < NUM_IR_SENSORS; i++) {
        if (irBinary[i] == 1) whiteCount++;
      }

      bool intersectionNow = (whiteCount >= T1_INTERSECTION_WHITE_MIN);

      if (intersectionNow && !t1IntersectionLatched) {
        // New intersection detected
        t1IntersectionLatched = true;
        t1IntersectionCount++;

        Serial.print("Task 1: Intersection #");
        Serial.print(t1IntersectionCount);
        Serial.print(" on column ");
        Serial.print(t1ColumnIndex);
        Serial.print(" (returning = ");
        Serial.print(t1ReturningAlongColumn ? "true" : "false");
        Serial.println(")");

        // 3) At every intersection: read color (bottom sensor)
        DetectedColor col = COLOR_UNKNOWN;
        if (colorSensors.isBottomReady() && colorSensors.readBottomSensor()) {
          col = colorSensors.getBottomColor();
          Serial.print("  Color at intersection: ");
          Serial.println(colorSensors.getColorName(col));
        } else {
          Serial.println("  Bottom color sensor not ready!");
        }

        // ---- Case A: GREEN detected -> BallCollector.cpp task ----
        if (col == COLOR_GREEN) {
          Serial.println("  GREEN detected -> go to COLLECTING");
          stopAllMotors();
          setSubState(T1_COLLECTING);
          break;
        }

        // ---- Case B: No green - handle logic depending on direction ----
        // Forward along column (going from start side towards end)
        if (!t1ReturningAlongColumn) {
          // Last intersection (3rd one) and still no green here:
          // -> turn 180° and come back along same column
          if (t1IntersectionCount == T1_NUM_INTERSECTIONS) {
            Serial.println("  3rd intersection with no GREEN -> 180 deg turn and come back");
            stopAllMotors();
            t1PendingTurn        = T1_TURN_180;
            t1NextStateAfterTurn = T1_LINE_FOLLOWING;   // continue but in reverse direction
            t1ReturningAlongColumn = true;              // now we are "coming back"
            t1IntersectionCount    = 0;                 // recount intersections on way back
            setSubState(T1_TURNING);
            break;
          }
          // For intersection 1 & 2 with no green: just continue line follow
        }
        else {
          // We are coming back along the same column after 180°
          // When we hit the 3rd intersection again, we are back at the
          // first intersection point (start of column).
          if (t1IntersectionCount == T1_NUM_INTERSECTIONS) {
            Serial.println("  Back at 1st intersection of this column -> 90 deg right turn");

            stopAllMotors();
            t1PendingTurn        = T1_TURN_RIGHT_90;
            t1NextStateAfterTurn = T1_LINE_FOLLOWING;
            t1ReturningAlongColumn = false;
            t1IntersectionCount    = 0;   // reset for new column / exit

            // If we were in first column, go to second column.
            // If we were in second column, we can mark task completed.
            if (t1ColumnIndex == 0) {
              t1ColumnIndex = 1;     // moving into second column
              Serial.println("  Moving into COLUMN 1 (second column)");
            } else {
              Serial.println("  Finished second column -> Task COMPLETED");
              // After the turn and a bit of line follow, you can switch to COMPLETED
              // For now we just schedule COMPLETED directly after the turn.
              t1NextStateAfterTurn = T1_COMPLETED;
            }

            setSubState(T1_TURNING);
            break;
          }
        }

        // If we reached here: intersection handled but no special turn,
        // just keep following the line.
      }
      else if (!intersectionNow && t1IntersectionLatched) {
        // Left the intersection area -> arm the detection for the next one
        t1IntersectionLatched = false;
      }

      break;
    }

    // -----------------------------------------------------------
    // 6) COLLECTING: run BallCollector.cpp until it returns true
    // -----------------------------------------------------------
    case T1_COLLECTING: {
      Serial.println("Task 1: COLLECTING state (BallCollector)");
      // Call ballCollector to collect the ball (assumes global ballCollector object)
      if (ballCollector.collectingBall()) {
        Serial.println("  BallCollector finished -> back to LINE_FOLLOWING");
        // After collection, return to line following in current direction
        setSubState(T1_LINE_FOLLOWING);
      }
      break;
    }

    // -----------------------------------------------------------
    // 7) PLANTING: reserved for future (if you want separate planting action)
    // -----------------------------------------------------------
    case T1_PLANTING: {
      Serial.println("Task 1: PLANTING state");
      // Add planting logic here if needed
      break;
    }

    // -----------------------------------------------------------
    // 8) COMPLETED: plantation task finished
    // -----------------------------------------------------------
    case T1_COMPLETED: {
      Serial.println("Task 1: COMPLETED");
      taskActive = false;
      stopAllMotors();
      break;
    }
  } // end switch
}



// Update OLED display
void Task1Plantation::updateDisplay() {
  if (!taskActive) return;
  
  oledDisplay.show(
    "Task 1: Plantation",
    "State: " + getSubStateName(),
    "Time: " + String((millis() - subStateStartTime) / 1000) + "s"
  );
}

// Set sub-state
void Task1Plantation::setSubState(Task1SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    
    Serial.print("Task 1 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

// Get current sub-state
Task1SubState Task1Plantation::getSubState() {
  return currentSubState;
}

// Get sub-state name
String Task1Plantation::getSubStateName() {
  switch(currentSubState) {
    case T1_INIT: return "INIT";
    case T1_SEARCHING: return "SEARCHING";
    case T1_FOLLOWING: return "FOLLOWING";
    case T1_TURNING: return "TURNING";
    case T1_LINE_FOLLOWING: return "LINE_FOLLOWING";
    case T1_COLLECTING: return "COLLECTING";
    case T1_PLANTING: return "PLANTING";
    case T1_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

// Start task
void Task1Plantation::start() {
  Serial.println("Starting Task 1: Plantation");
  taskActive = true;
  setSubState(T1_INIT);
}

// Stop task
void Task1Plantation::stop() {
  Serial.println("Stopping Task 1: Plantation");
  taskActive = false;
  stopAllMotors();
}

// Check if active
bool Task1Plantation::isActive() {
  return taskActive;
}

// Check if completed
bool Task1Plantation::isCompleted() {
  return (currentSubState == T1_COMPLETED);
}

// Reset task
void Task1Plantation::reset() {
  currentSubState = T1_INIT;
  subStateStartTime = millis();
  taskActive = false;
}
