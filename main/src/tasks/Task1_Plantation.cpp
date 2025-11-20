/*********************************************************************
 * Task 1: Plantation Task Implementation
 * Path:
 *  - Find main line, 90° right, backup 1000 ms.
 *  - For each of 4 lines:
 *      * Go DOWN: line follow, 3 intersections, read color+OLED.
 *      * On 3rd: 180° right, come back UP (no color).
 *      * On 3rd again (top):
 *          - If line 1..3: 90° right, move to next line, 90° right,
 *            backup 1000 ms, repeat.
 *          - If line 4: 90° right, forward 3000 ms, COMPLETED.
 *********************************************************************/
// SERIAL OUTPUT GUIDELINES:
// - Print status ONCE when entering a new state (use static bool or state tracking)
// - For time-based actions: print "Action for X ms" ONCE at start
// - For condition-based actions: print "Action until condition" ONCE at start
// - Avoid printing inside loops that run every cycle
// - Low-level motor/sensor functions don't print - task prints context
/**********************************************************************/

#include "Task1_Plantation.h"
// #include "BallCollector.h"   // Not used in this version
#include "../Motors.h"
#include "../IRReading.h"
#include "../LineFollow.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"
#include "../ColorSensors.h"

// Global task object
Task1Plantation task1Plantation;

// ---------- Plantation task helpers & state variables ----------

// Turn command type for T1_TURNING
enum T1TurnCommand {
  T1_TURN_NONE,
  T1_TURN_RIGHT_90,
  T1_TURN_LEFT_90,
  T1_TURN_180
};

// Tunable timings (milliseconds) - Global definitions for Task1
// These match the extern declarations in Task1_Plantation.h
unsigned long T1_TURN_90_TIME_MS      = 2000;   // 90° turn time
unsigned long T1_TURN_180_TIME_MS     = 3500;   // 180° turn time
unsigned long T1_BACKUP_TIME_MS       = 1500;   // backup after aligning with a line
unsigned long T1_EXIT_FORWARD_TIME_MS = 3000;   // final straight exit time

// Task1 runtime-configurable defaults
int T1_SPEED_LEVEL = 6;                          // default speed level (1-12)
int T1_INTERSECTION_WHITE_MIN = 7;               // minimum white sensors to detect intersection

// Plantation grid settings
const int T1_INTERSECTIONS_PER_LINE   = 3;   // 3 intersections in each vertical line
const int T1_NUM_LINES                = 4;   // 4 vertical lines total
// T1_INTERSECTION_WHITE_MIN is defined above with other tunable parameters

// State for current plantation sweep
static T1TurnCommand  t1PendingTurn        = T1_TURN_NONE;
static Task1SubState  t1NextStateAfterTurn = T1_LINE_FOLLOWING;

static int  t1CurrentLine          = 0;   // 0..3 (4 lines)
static int  t1IntersectionCount    = 0;   // number of intersections in current direction
static bool t1ReturningAlongLine   = false; // false = going down, true = coming back up
static bool t1IntersectionLatched  = false; // avoid double-counting same intersection

// ---------- SETTER FUNCTIONS FOR SERIAL TUNING ----------

void T1_setTurn90Time(unsigned long ms) {
  T1_TURN_90_TIME_MS = ms;
  Serial.print("Updated T1 90° Turn Time: ");
  Serial.println(ms);
}

void T1_setTurn180Time(unsigned long ms) {
  T1_TURN_180_TIME_MS = ms;
  Serial.print("Updated T1 180° Turn Time: ");
  Serial.println(ms);
}

void T1_setBackupTime(unsigned long ms) {
  T1_BACKUP_TIME_MS = ms;
  Serial.print("Updated T1 Backup Time: ");
  Serial.println(ms);
}

void T1_setExitForwardTime(unsigned long ms) {
  T1_EXIT_FORWARD_TIME_MS = ms;
  Serial.print("Updated T1 Exit Forward Time: ");
  Serial.println(ms);
}

void T1_setSpeedLevel(int level) {
  T1_SPEED_LEVEL = level;
  Serial.print("Updated T1 Speed Level: ");
  Serial.println(level);
}

void T1_setIntersectionWhiteMin(int min) {
  T1_INTERSECTION_WHITE_MIN = min;
  Serial.print("Updated T1 Intersection White Min: ");
  Serial.println(min);
}

// ---------- Plantation task main methods ----------

// Constructor
Task1Plantation::Task1Plantation() {
  currentSubState = T1_INIT;
  subStateStartTime = 0;
  taskActive = false;
  
  // Default configuration (can be changed via serial commands)
  turnDuration = 1000;  // 1 second for 90° turn
  searchDuration = 5000;  // 5 seconds search time
  collectDuration = 2000;  // 2 seconds collection time
  ballDetectionThreshold = 100;  // Default threshold
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
  t1CurrentLine        = 0;
  t1IntersectionCount  = 0;
  t1ReturningAlongLine = false;
  t1IntersectionLatched= false;

  // Using global baseSpeed and rotateSpeed from main.ino

  oledDisplay.show("Task 1", "Plantation", "Initialized");
  delay(1000);
}

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

      t1PendingTurn        = T1_TURN_NONE;
      t1NextStateAfterTurn = T1_LINE_FOLLOWING;
      t1CurrentLine        = 0;
      t1IntersectionCount  = 0;
      t1ReturningAlongLine = false;
      t1IntersectionLatched= false;

      T1_setSpeedLevel(T1_SPEED_LEVEL);   // moderate speed
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
      // detect white line by IR sensor 3 to 15
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
        t1NextStateAfterTurn = T1_FOLLOWING;   // backup 1 s then line follow on line 1
        setSubState(T1_TURNING);
      }
      break;
    }

    // -----------------------------------------------------------
    // 3) FOLLOWING (used here as "BACKUP then go to LINE_FOLLOWING")
    //    - after each 90° turn into a vertical line: go back 1000 ms
    // -----------------------------------------------------------
    case T1_FOLLOWING: {
      unsigned long elapsed = millis() - subStateStartTime;

      if (elapsed < T1_BACKUP_TIME_MS) {
        // "robo car come back 1000ms"
        robotBackward();
      } else {
        // Backup finished -> start moving forward along current line
        Serial.println("Task 1: backup done, start LINE_FOLLOWING on line");
        t1IntersectionCount    = 0;
        t1ReturningAlongLine   = false;  // going DOWN the line
        t1IntersectionLatched  = false;

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
    // 5) LINE_FOLLOWING on a vertical line:
    //    - DOWN first (t1ReturningAlongLine = false)
    //      * read color + OLED at every intersection
    //      * at 3rd intersection: 180° and come back up
    //    - UP after 180° (t1ReturningAlongLine = true)
    //      * only count intersections, no color
    //      * at 3rd intersection again (top):
    //          - lines 1..3: move to next line (two 90° rights)
    //          - line 4: 90° right and exit forward 3000 ms
    // -----------------------------------------------------------
    case T1_LINE_FOLLOWING: {
      // 1) Keep line following on current line
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
        Serial.print(" on line ");
        Serial.print(t1CurrentLine + 1);
        Serial.print(" (returning = ");
        Serial.print(t1ReturningAlongLine ? "true" : "false");
        Serial.println(")");

        // ---------- DOWN direction (away from top corridor) ----------
        if (!t1ReturningAlongLine) {
          // Read bottom color sensor and show on OLED
          DetectedColor col = COLOR_UNKNOWN;

          if (colorSensors.isBottomReady() && colorSensors.readBottomSensor()) {
            col = colorSensors.getBottomColor();
            String colorName = colorSensors.getColorName(col);
            Serial.print("  Color at intersection: ");
            Serial.println(colorName);

            String lineInfo  = "Line " + String(t1CurrentLine + 1) +
                               " Int " + String(t1IntersectionCount);
            oledDisplay.show("Task1 Plantation",
                             lineInfo,
                             "Color: " + colorName);
          } else {
            Serial.println("  Bottom color sensor not ready!");
          }

          // If this is the 3rd intersection -> bottom of this line
          if (t1IntersectionCount == T1_INTERSECTIONS_PER_LINE) {
            Serial.println("  Reached bottom of line -> 180 deg turn and come back");
            stopAllMotors();
            t1PendingTurn        = T1_TURN_180;
            t1NextStateAfterTurn = T1_LINE_FOLLOWING;   // continue but in reverse direction
            t1ReturningAlongLine = true;                // now we are "coming back up"
            t1IntersectionCount  = 0;                   // recount intersections on way back
            setSubState(T1_TURNING);
            break;
          }
          // For intersections 1..2 while going down: nothing special
        }
        // ---------- UP direction (coming back to top) ----------
        else {
          // When we hit the 3rd intersection again, we are back at top.
          if (t1IntersectionCount == T1_INTERSECTIONS_PER_LINE) {
            Serial.println("  Back at top of this line");

            stopAllMotors();
            t1ReturningAlongLine = false;
            t1IntersectionCount  = 0;

            // Prepare to either go to next line or exit
            if (t1CurrentLine < T1_NUM_LINES - 1) {
              // Not the last line -> move to next line
              t1CurrentLine++;
              Serial.print("  Moving to next line: ");
              Serial.println(t1CurrentLine + 1);

              // We are still on this intersection; latch it so MOVE_TO_NEXT_LINE
              // ignores it until the robot leaves it.
              t1IntersectionLatched = true;

              t1PendingTurn        = T1_TURN_RIGHT_90;       // first 90° right at top intersection
              t1NextStateAfterTurn = T1_MOVE_TO_NEXT_LINE;   // then move along top corridor
            } else {
              // Last line finished -> go to exit
              Serial.println("  Finished last line -> exit to right and forward 3000ms");
              t1PendingTurn        = T1_TURN_RIGHT_90;
              t1NextStateAfterTurn = T1_EXIT_FORWARD;
            }

            setSubState(T1_TURNING);
            break;
          }
        }

        // If we reached here: intersection handled but no special turn,
        // just keep following the line.
      }
      else if (!intersectionNow && t1IntersectionLatched) {
        // Left the intersection area -> arm detection for the next one
        t1IntersectionLatched = false;
      }

      break;
    }

    // -----------------------------------------------------------
    // 6) MOVE_TO_NEXT_LINE:
    //    - after finishing a line and turning right at top intersection,
    //      follow the top horizontal line until the next intersection,
    //      then turn right 90° into the new vertical line and
    //      go to T1_FOLLOWING (backup 1000 ms).
    // -----------------------------------------------------------
    case T1_MOVE_TO_NEXT_LINE: {
      // Follow the top corridor line
      executeLineFollow();

      readAllIRSensorsBinary();
      int whiteCount = 0;
      for (int i = 0; i < NUM_IR_SENSORS; i++) {
        if (irBinary[i] == 1) whiteCount++;
      }
      bool intersectionNow = (whiteCount >= T1_INTERSECTION_WHITE_MIN);

      if (intersectionNow && !t1IntersectionLatched) {
        // New intersection -> this is the next vertical line
        t1IntersectionLatched = true;

        Serial.print("Task 1: Top corridor intersection for line ");
        Serial.println(t1CurrentLine + 1);

        stopAllMotors();
        t1PendingTurn        = T1_TURN_RIGHT_90;  // turn into the new vertical line
        t1NextStateAfterTurn = T1_FOLLOWING;      // then backup 1000 ms and start line following
        setSubState(T1_TURNING);
      }
      else if (!intersectionNow && t1IntersectionLatched) {
        // Left the previous intersection region
        t1IntersectionLatched = false;
      }

      break;
    }

    // -----------------------------------------------------------
    // 7) EXIT_FORWARD:
    //    - after finishing line 4 and turning right at top intersection,
    //      go forwards for 3000 ms and then complete the task.
    // -----------------------------------------------------------
    case T1_EXIT_FORWARD: {
      unsigned long elapsed = millis() - subStateStartTime;

      if (elapsed < T1_EXIT_FORWARD_TIME_MS) {
        robotForward();
      } else {
        stopAllMotors();
        Serial.println("Task 1: EXIT_FORWARD complete -> COMPLETED");
        setSubState(T1_COMPLETED);
      }
      break;
    }

    // -----------------------------------------------------------
    // 8) COLLECTING: reserved / not used in this version
    // -----------------------------------------------------------
    case T1_COLLECTING: {
      // No collecting logic in this plantation version
      break;
    }

    // -----------------------------------------------------------
    // 9) PLANTING: reserved for future actions
    // -----------------------------------------------------------
    case T1_PLANTING: {
      // Add planting logic here if needed
      break;
    }

    // -----------------------------------------------------------
    // 10) COMPLETED: plantation task finished
    // -----------------------------------------------------------
    case T1_COMPLETED: {
      Serial.println("Task 1: COMPLETED");
      taskActive = false;
      stopAllMotors();
      break;
    }
  } // end switch
}


// ---------- Utility methods ----------

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
    
    Serial.print("Task 1 sub-state -> ");
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
    case T1_INIT:            return "INIT";
    case T1_SEARCHING:       return "SEARCHING";
    case T1_FOLLOWING:       return "BACKUP_ON_LINE";
    case T1_TURNING:         return "TURNING";
    case T1_LINE_FOLLOWING:  return "LINE_FOLLOWING";
    case T1_MOVE_TO_NEXT_LINE:return "MOVE_TO_NEXT_LINE";
    case T1_EXIT_FORWARD:    return "EXIT_FORWARD";
    case T1_COLLECTING:      return "COLLECTING";
    case T1_PLANTING:        return "PLANTING";
    case T1_COMPLETED:       return "COMPLETED";
    default:                 return "UNKNOWN";
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

// Configuration setters
void Task1Plantation::setTurnDuration(unsigned long timeMs) {
  turnDuration = timeMs;
  Serial.print("T1 Turn duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setSearchDuration(unsigned long timeMs) {
  searchDuration = timeMs;
  Serial.print("T1 Search duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setCollectDuration(unsigned long timeMs) {
  collectDuration = timeMs;
  Serial.print("T1 Collect duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task1Plantation::setBallDetectionThreshold(uint16_t threshold) {
  ballDetectionThreshold = threshold;
  Serial.print("T1 Ball detection threshold set to: ");
  Serial.println(threshold);
}

// Configuration getters
unsigned long Task1Plantation::getTurnDuration() {
  return turnDuration;
}

unsigned long Task1Plantation::getSearchDuration() {
  return searchDuration;
}

unsigned long Task1Plantation::getCollectDuration() {
  return collectDuration;
}

uint16_t Task1Plantation::getBallDetectionThreshold() {
  return ballDetectionThreshold;
}
