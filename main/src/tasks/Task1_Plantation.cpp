/*********************************************************************
 * Task 1: Plantation Task Implementation - Simplified Version
 * Logic:
 *  - Go forward until cross detected
 *  - Cross 1: Turn right 90°, backup 0.75s, line follow
 *  - Cross 2: Turn left, backup 1s, line follow
 *  - Crosses 3-7: Go forward without stopping
 *  - Cross 8: Turn left, backup 1s, line follow
 *  - Cross 9: Forward 0.5s, turn right blindly, forward until left wall detected
 *  - When left wall detected: Move to Task 2
 *********************************************************************/

#include "Task1_Plantation.h"
#include "../Motors.h"
#include "../IRReading.h"
#include "../LineFollow.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"
#include "../ColorSensors.h"

// Global task object
Task1Plantation task1Plantation;

// ---------- Task1 Timings & Settings ----------

// Tunable timings (milliseconds) - Global definitions for Task1
unsigned long T1_TURN_90_TIME_MS      = 1500;   // 90° turn time
unsigned long T1_TURN_180_TIME_MS     = 3500;   // 180° turn time (not used in new logic)
unsigned long T1_BACKUP_TIME_MS       = 1000;   // backup time after turns
unsigned long T1_EXIT_FORWARD_TIME_MS = 500;    // forward time after cross 9

// Task1 runtime-configurable defaults
uint8_t T1_SPEED_LEVEL = 6;                      // default speed level (1-12)
int T1_INTERSECTION_WHITE_MIN = 7;               // minimum white sensors to detect intersection

// Cross counting state
static int  t1CrossCount          = 0;   // Total crosses detected (0-9)
static bool t1CrossLatched        = false; // avoid double-counting same cross
static unsigned long t1ActionStartTime = 0; // Timer for timed actions
static bool t1ActionComplete      = false; // Flag for action completion
static int  t1CurrentAction       = 0;   // Current action step (0=idle, 1=turning, 2=backing, 3=following, etc)

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

void T1_setIntersectionWhiteMin(int min) {
  T1_INTERSECTION_WHITE_MIN = min;
  Serial.print("Updated T1 Intersection White Min: ");
  Serial.println(min);
}

// ---------- HELPER FUNCTION: Read and display color ----------
void T1_readAndDisplayColor(int crossNum) {
  if (colorSensors.isBottomReady() && colorSensors.readBottomSensor()) {
    DetectedColor col = colorSensors.getBottomColor();
    String colorName = colorSensors.getColorName(col);
    Serial.print("  Cross ");
    Serial.print(crossNum);
    Serial.print(" Color: ");
    Serial.println(colorName);
    
    oledDisplay.show("Task 1 Cross " + String(crossNum),
                     "Color: " + colorName,
                     "Continuing...");
  } else {
    Serial.println("  Bottom color sensor not ready!");
  }
}

// ---------- Task1 main methods ----------

// Constructor
Task1Plantation::Task1Plantation() {
  currentSubState = T1_INIT;
  subStateStartTime = 0;
  taskActive = false;
  
  // Default configuration (can be changed via serial commands)
  turnDuration = 1000;
  searchDuration = 5000;
  collectDuration = 2000;
  ballDetectionThreshold = 100;
}

// Initialize task
void Task1Plantation::init() {
  Serial.println("=== Task 1: Plantation - Initializing ===");
  currentSubState   = T1_INIT;
  subStateStartTime = millis();
  taskActive        = false;

  // Reset cross counting state
  t1CrossCount      = 0;
  t1CrossLatched    = false;
  t1ActionStartTime = 0;
  t1ActionComplete  = false;
  t1CurrentAction   = 0;

  oledDisplay.show("Task 1", "Plantation", "Initialized");
  delay(1000);
}

// Execute task - Simplified sequential logic
void Task1Plantation::execute() {
  if (!taskActive) return;

  // Read IR sensors for cross detection
  readAllIRSensorsBinary();
  int whiteCount = 0;
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    if (irBinary[i] == 1) whiteCount++;
  }
  bool crossDetected = (whiteCount >= T1_INTERSECTION_WHITE_MIN);

  // ---------- MAIN LOGIC BASED ON CROSS COUNT ----------

  // ACTION 0: Idle - go forward until first cross
  if (t1CurrentAction == 0) {
    if (!crossDetected) {
      executeLineFollow();  // Follow line or move forward
    } else {
      if (!t1CrossLatched) {
        t1CrossLatched = true;
        t1CrossCount++;
        Serial.print("=== Cross ");
        Serial.print(t1CrossCount);
        Serial.println(" detected ===");
        
        // Read and display color
        T1_readAndDisplayColor(t1CrossCount);
        
        // Determine action based on cross number
        if (t1CrossCount == 1) {
          // Cross 1: Turn right, backup 0.75s
          Serial.println("Action: Turn right 90°");
          stopAllMotors();
          t1CurrentAction = 1;  // Turn right
          t1ActionStartTime = millis();
        } else if (t1CrossCount == 2) {
          // Cross 2: Turn left, backup 1s
          Serial.println("Action: Turn left 90°");
          stopAllMotors();
          t1CurrentAction = 3;  // Turn left
          t1ActionStartTime = millis();
        } else if (t1CrossCount >= 3 && t1CrossCount <= 7) {
          // Crosses 3-7: Go forward without stopping
          Serial.println("Action: Continue forward");
          // Don't change action, just keep going
        } else if (t1CrossCount == 8) {
          // Cross 8: Turn left, backup 1s
          Serial.println("Action: Turn left 90°");
          stopAllMotors();
          t1CurrentAction = 5;  // Turn left for cross 8
          t1ActionStartTime = millis();
        } else if (t1CrossCount == 9) {
          // Cross 9: Forward 0.5s, turn right, forward until left wall
          Serial.println("Action: Forward 0.5s");
          t1CurrentAction = 7;  // Forward before blind turn
          t1ActionStartTime = millis();
        }
      }
    }
    
    // Reset latch when leaving cross
    if (!crossDetected && t1CrossLatched) {
      t1CrossLatched = false;
      // For crosses 3-7, return to action 0 to continue
      if (t1CrossCount >= 3 && t1CrossCount <= 7 && t1CurrentAction == 0) {
        // Already in action 0, just continue
      }
    }
  }

  // ACTION 1: Turn right 90° (Cross 1)
  else if (t1CurrentAction == 1) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_TURN_90_TIME_MS) {
      robotTurnRight();
    } else {
      stopAllMotors();
      Serial.println("Turn right complete, backing up 0.75s");
      t1CurrentAction = 2;  // Backup
      t1ActionStartTime = millis();
    }
  }

  // ACTION 2: Backup 0.75s (after Cross 1)
  else if (t1CurrentAction == 2) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < 750) {  // 0.75s backup
      robotBackward();
    } else {
      stopAllMotors();
      Serial.println("Backup complete, resuming line follow");
      t1CurrentAction = 0;  // Return to forward/line follow
      t1CrossLatched = false;  // Reset for next cross
    }
  }

  // ACTION 3: Turn left 90° (Cross 2)
  else if (t1CurrentAction == 3) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_TURN_90_TIME_MS) {
      robotTurnLeft();
    } else {
      stopAllMotors();
      Serial.println("Turn left complete, backing up 1s");
      t1CurrentAction = 4;  // Backup
      t1ActionStartTime = millis();
    }
  }

  // ACTION 4: Backup 1s (after Cross 2)
  else if (t1CurrentAction == 4) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_BACKUP_TIME_MS) {
      robotBackward();
    } else {
      stopAllMotors();
      Serial.println("Backup complete, resuming line follow");
      t1CurrentAction = 0;  // Return to forward/line follow
      t1CrossLatched = false;
    }
  }

  // ACTION 5: Turn left 90° (Cross 8)
  else if (t1CurrentAction == 5) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_TURN_90_TIME_MS) {
      robotTurnLeft();
    } else {
      stopAllMotors();
      Serial.println("Turn left complete, backing up 1s");
      t1CurrentAction = 6;  // Backup
      t1ActionStartTime = millis();
    }
  }

  // ACTION 6: Backup 1s (after Cross 8)
  else if (t1CurrentAction == 6) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_BACKUP_TIME_MS) {
      robotBackward();
    } else {
      stopAllMotors();
      Serial.println("Backup complete, resuming line follow");
      t1CurrentAction = 0;  // Return to forward/line follow
      t1CrossLatched = false;
    }
  }

  // ACTION 7: Forward 0.5s (after Cross 9)
  else if (t1CurrentAction == 7) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_EXIT_FORWARD_TIME_MS) {
      robotForward();
    } else {
      stopAllMotors();
      Serial.println("Forward complete, turning right blindly");
      t1CurrentAction = 8;  // Blind right turn
      t1ActionStartTime = millis();
    }
  }

  // ACTION 8: Blind right turn (after Cross 9)
  else if (t1CurrentAction == 8) {
    unsigned long elapsed = millis() - t1ActionStartTime;
    if (elapsed < T1_TURN_90_TIME_MS) {
      robotTurnRight();
    } else {
      stopAllMotors();
      Serial.println("Blind turn complete, moving forward until left wall");
      t1CurrentAction = 9;  // Forward until wall
      t1ActionStartTime = millis();
    }
  }

  // ACTION 9: Forward until left wall detected
  else if (t1CurrentAction == 9) {
    // Check left TOF sensor
    readTOFSensors();  // Update TOF readings
    uint16_t leftDistance = tofSensors.getLeftDistance();
    
    if (leftDistance > 0 && leftDistance < 150) {  // Left wall detected within 15cm
      stopAllMotors();
      Serial.println("=== Left wall detected - Task 1 COMPLETED ===");
      Serial.println("=== Moving to Task 2 ===");
      taskActive = false;
      setSubState(T1_COMPLETED);
    } else {
      // Keep moving forward
      robotForward();
    }
  }
}


// ---------- Utility methods ----------

// Update OLED display
void Task1Plantation::updateDisplay() {
  if (!taskActive) return;
  
  oledDisplay.show(
    "Task 1: Plantation",
    "Cross: " + String(t1CrossCount) + "/9",
    "Action: " + String(t1CurrentAction)
  );
}

// Set sub-state (kept for compatibility)
void Task1Plantation::setSubState(Task1SubState newSubState) {
  currentSubState   = newSubState;
  subStateStartTime = millis();
}

// Get current sub-state
Task1SubState Task1Plantation::getSubState() {
  return currentSubState;
}

// Get sub-state name (simplified)
String Task1Plantation::getSubStateName() {
  if (currentSubState == T1_COMPLETED) {
    return "COMPLETED";
  }
  return "RUNNING (Cross " + String(t1CrossCount) + ")";
}

// Start task
void Task1Plantation::start() {
  Serial.println("=== Starting Task 1: Plantation (Simplified) ===");
  taskActive = true;
  t1CrossCount = 0;
  t1CrossLatched = false;
  t1ActionStartTime = 0;
  t1ActionComplete = false;
  t1CurrentAction = 0;
  currentSubState = T1_INIT;
  subStateStartTime = millis();
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
  currentSubState   = T1_INIT;
  subStateStartTime = millis();
  taskActive        = false;
  t1CrossCount      = 0;
  t1CrossLatched    = false;
  t1ActionStartTime = 0;
  t1ActionComplete  = false;
  t1CurrentAction   = 0;
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
