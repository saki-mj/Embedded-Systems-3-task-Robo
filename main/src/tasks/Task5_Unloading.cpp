/*********************************************************************
 * Task 5: Unloading Balls Implementation (revised)
 * - Uses BACK TOF to detect unloading zone
 * - Moves backward into position and aligns
 * - Non-blocking servo sequence using ballCollector positions
 * - Uses barcode parity rule to choose basket (chooseBasket())
 *********************************************************************/

#include "Task5_Unloading.h"
#include "../Motors.h"
#include "../TOFSensors.h"
#include "../OLEDDisplay.h"
#include "../ColorSensors.h"
#include "BallCollector.h" // for servo positions & testServo()

extern Task5Unloading task5Unloading;

Task5Unloading task5Unloading;

// --- Navigation and Unloading Helpers (auto-generated) ---
// Tuning constants — calibrate for your robot
const float MS_PER_CM = 40.0;   // ms per cm (tune)
const unsigned long TURN_MS_90 = 450; // ms for ~90° turn (tune)
const uint16_t APPROACH_DISTANCE_CM = 30; // desired TOF stop distance (cm)

static inline bool safeMoveBackwardUsingBackTOF(uint16_t stopDistanceMm, uint16_t timeoutMs) {
  unsigned long start = millis();
  robotBackward();  // Uses global baseSpeed
  while (millis() - start < timeoutMs) {
    readTOFSensors();
    if (tofSensors.isBackValid()) {
      uint16_t d = tofSensors.getBackDistance();
      if (d <= stopDistanceMm) {
        stopAllMotors();
        return true;
      }
    }
    delay(10);
  }
  stopAllMotors();
  return false;
}
static inline void moveBackwardCmTimed(float cm) {
  unsigned long ms = (unsigned long)(cm * MS_PER_CM);
  robotBackward();  // Uses global baseSpeed
  delay(ms);
  stopAllMotors();
}
static inline void moveForwardCmTimed(float cm) {
  unsigned long ms = (unsigned long)(cm * MS_PER_CM);
  robotForward();  // Uses global baseSpeed
  delay(ms);
  stopAllMotors();
}
static inline void turnRight90Timed() {
  robotTurnRight();  // Uses global rotateSpeed
  delay(TURN_MS_90);
  stopAllMotors();
}
static inline void turnLeft90Timed() {
  robotTurnLeft();  // Uses global rotateSpeed
  delay(TURN_MS_90);
  stopAllMotors();
}
static inline void performUnloadServo(BasketColor basket) {
  int servoHome = ballCollector.getSortingPos1();
  int servoBlue = ballCollector.getSortingPos0();
  int servoRed  = ballCollector.getSortingPos2();
  int targetAngle = (basket == BASKET_BLUE) ? servoBlue : servoRed;
  oledDisplay.show("Unloading", (basket==BASKET_RED)?"RED":"BLUE");
  ballCollector.testServo("SORTING", targetAngle);
  delay(ballCollector.getSortingDelay());
  ballCollector.testServo("SORTING", servoHome);
  delay(ballCollector.getSortingDelay());
}
static inline BasketColor mapColorToBasket(bool barcodeIsEven, DetectedColor color) {
  if (barcodeIsEven) {
    if (color == COLOR_YELLOW) return BASKET_RED;
    else                      return BASKET_BLUE;
  } else {
    if (color == COLOR_YELLOW) return BASKET_BLUE;
    else                      return BASKET_RED;
  }
}
void navigateAndUnload(bool barcodeIsEven, DetectedColor color) {
  BasketColor basket = mapColorToBasket(barcodeIsEven, color);
  Serial.print("navigateAndUnload: parityEven=");
  Serial.print(barcodeIsEven ? "true" : "false");
  Serial.print(" color=");
  Serial.println(color == COLOR_YELLOW ? "YELLOW" : "WHITE");
  Serial.print(" -> target = ");
  Serial.println(basket == BASKET_RED ? "RED" : "BLUE");
  const uint16_t stopMm = (uint16_t)(APPROACH_DISTANCE_CM * 10);
  Serial.println("Approaching back zone (sensor-driven)...");
  bool ok = safeMoveBackwardUsingBackTOF(stopMm, 5000);
  if (!ok) {
    Serial.println("Back TOF approach timeout — falling back to time-driven approach");
    moveBackwardCmTimed(30.0);
  }
  robotBackward();  // Uses global baseSpeed
  delay((unsigned long) ( (float) task5Unloading.getAlignDuration() ));
  stopAllMotors();
  if (basket == BASKET_BLUE) {
    Serial.println("Going to BLUE basket (left)");
    turnLeft90Timed();
    moveForwardCmTimed(45.0);
    robotForward();  // Uses global baseSpeed
    delay(300);
    stopAllMotors();
  } else {
    Serial.println("Going to RED basket (right)");
    turnRight90Timed();
    moveForwardCmTimed(45.0);
    robotForward();  // Uses global baseSpeed
    delay(300);
    stopAllMotors();
  }
  Serial.println("Unloading servo action...");
  performUnloadServo(basket);
  Serial.println("Retreating and re-orienting to collection position");
  robotBackward();  // Uses global baseSpeed
  delay(400);
  stopAllMotors();
  if (basket == BASKET_BLUE) {
    turnRight90Timed();
  } else {
    turnLeft90Timed();
  }
  moveForwardCmTimed(30.0);
  Serial.println("navigateAndUnload: done");
}
void navigateAndUnloadFromLastCollection(DetectedColor color) {
  extern bool lastBarcodeEven;
  navigateAndUnload(lastBarcodeEven, color);
}
// --- End Navigation and Unloading Helpers ---

// Local sequencing states for non-blocking unload
namespace {
  enum UnloadStep { US_IDLE = 0, US_MOVED_TO_TARGET, US_WAIT_AFTER_MOVE, US_RETURNED_HOME, US_WAIT_AFTER_HOME };
  UnloadStep unloadStep = US_IDLE;
  unsigned long stepEndTime = 0;

  // Debounce for back TOF detection (require N consecutive good readings)
  const uint8_t BACK_DEBOUNCE_REQUIRED = 3;
  uint8_t backGoodCount = 0;
}

// ---------------- Constructor ----------------
Task5Unloading::Task5Unloading() {
  currentSubState = T5_INIT;
  subStateStartTime = 0;
  taskActive = false;
  ballsUnloaded = 0;

  // Motion defaults (can be changed via Serial)
  unloadDuration        = 2000; // ms waiting after servo returns home
  alignDuration         = 1200; // ms of slow backward creep for final align
  zoneDetectionDistance = 150;  // mm threshold for back TOF to detect zone
  targetBallCount       = 1;    // how many items to unload by default
  
  // OUT Servo defaults
  outServoPos0 = 85;   // home/initial position
  outServoPos1 = 140;  // out yellow balls position
  outServoPos2 = 40;   // out white balls position

  // Sorting / barcode defaults
  barcodeBinary   = "";
  barcodeValue    = 0;
  barcodeValid    = false;
  potatoQuality   = POTATO_GOOD;
  targetBasket    = BASKET_RED;
  hasSortedThisCycle = false;
}

// ---------------- Helpers ----------------

uint16_t Task5Unloading::binaryToValue(const String &bin) const {
  uint16_t value = 0;
  for (int i = 0; i < bin.length(); i++) {
    char c = bin.charAt(i);
    if (c == '0' || c == '1') {
      value = (value << 1) | (c == '1' ? 1 : 0);
    }
  }
  return value;
}

BasketColor Task5Unloading::chooseBasket() const {
  bool isEven = (barcodeValue % 2 == 0);
  if (isEven) {
    return (potatoQuality == POTATO_GOOD) ? BASKET_RED : BASKET_BLUE;
  } else {
    return (potatoQuality == POTATO_GOOD) ? BASKET_BLUE : BASKET_RED;
  }
}

// ---------------- Init / Start / Stop ----------------

void Task5Unloading::init() {
  Serial.println("=== Task 5: Unloading - Initializing ===");
  
  // Initialize OUT servo
  outServo.attach(OUT_SERVO_PIN);
  outServo.write(outServoPos0);
  Serial.print("OUT Servo initialized on pin ");
  Serial.print(OUT_SERVO_PIN);
  Serial.print(" at position ");
  Serial.println(outServoPos0);
  
  currentSubState = T5_INIT;
  subStateStartTime = millis();
  taskActive = false;
  ballsUnloaded = 0;
  hasSortedThisCycle = false;
  unloadStep = US_IDLE;
  backGoodCount = 0;

  oledDisplay.show("Task 5", "Unloading", "Initialized");
  delay(250);
}

void Task5Unloading::start() {
  Serial.println("Starting Task 5: Unloading");

  // If barcode was not set explicitly, default to 0
  if (!barcodeValid) {
    Serial.println("Task 5: WARNING - barcode not set, using 0");
    barcodeValue = 0;
    barcodeBinary = "0000";
    barcodeValid = true;
  }

  // Pre-compute the target basket from current barcode and quality
  targetBasket = chooseBasket();

  taskActive = true;
  setSubState(T5_INIT);
}

void Task5Unloading::stop() {
  Serial.println("Stopping Task 5: Unloading");
  taskActive = false;
  stopAllMotors();
  unloadStep = US_IDLE;
}

bool Task5Unloading::isActive()    { return taskActive; }
bool Task5Unloading::isCompleted() { return currentSubState == T5_COMPLETED; }

void Task5Unloading::reset() {
  currentSubState = T5_INIT;
  subStateStartTime = millis();
  taskActive = false;
  ballsUnloaded = 0;
  barcodeBinary = "";
  barcodeValue = 0;
  barcodeValid = false;
  hasSortedThisCycle = false;
  unloadStep = US_IDLE;
  backGoodCount = 0;
}

int Task5Unloading::getBallsUnloaded() {
  return ballsUnloaded;
}

// ---------------- Barcode / Quality setters -------------
void Task5Unloading::setBarcodeBinary(const String &binary) {
  barcodeBinary = binary;
  barcodeValue  = binaryToValue(binary);
  barcodeValid  = true;
  Serial.print("Task 5: Barcode binary set to ");
  Serial.print(barcodeBinary);
  Serial.print(" -> value = ");
  Serial.println(barcodeValue);
  targetBasket = chooseBasket();
}

void Task5Unloading::setBarcodeValue(uint16_t value) {
  barcodeValue  = value;
  barcodeBinary = String(value, BIN);
  barcodeValid  = true;
  Serial.print("Task 5: Barcode value set to ");
  Serial.print(barcodeValue);
  Serial.print(" (binary ");
  Serial.print(barcodeBinary);
  Serial.println(")");
  targetBasket = chooseBasket();
}

void Task5Unloading::setPotatoQuality(PotatoQuality q) {
  potatoQuality = q;
  Serial.print("Task 5: Potato quality set to ");
  Serial.println((q == POTATO_GOOD) ? "GOOD" : "BAD");
}

// ---------------- Main execute() ----------------
// Uses BACK TOF: tofSensors.getBackDistance() and tofSensors.isBackValid()
void Task5Unloading::execute() {
  if (!taskActive) return;

  // keep display updated
  updateDisplay();

  switch (currentSubState) {

    // INIT – start moving backward to approach rear unloading zone
    case T5_INIT: {
      Serial.println("Task 5: INIT -> moving backward to find back zone");
      ballsUnloaded = 0;
      hasSortedThisCycle = false;
      unloadStep = US_IDLE;
      backGoodCount = 0;

      // start moving backward
      robotBackward();  // Uses global baseSpeed
      setSubState(T5_NAVIGATE_TO_ZONE);
      break;
    }

    // NAVIGATE_TO_ZONE – monitor BACK TOF and debounce detection
    case T5_NAVIGATE_TO_ZONE: {
      readTOFSensors();
      uint16_t backDist = tofSensors.getBackDistance();
      bool backOk = tofSensors.isBackValid();

      // debug print occasionally (not every cycle)
      static unsigned long lastNavPrint = 0;
      if (millis() - lastNavPrint > 300) {
        Serial.print("T5 NAV (back): ");
        if (backOk) Serial.print(backDist);
        else Serial.print("INVALID");
        Serial.print(" mm  threshold=");
        Serial.println(zoneDetectionDistance);
        lastNavPrint = millis();
      }

      if (backOk && backDist <= zoneDetectionDistance) {
        backGoodCount++;
      } else {
        backGoodCount = 0;
      }

      if (backGoodCount >= BACK_DEBOUNCE_REQUIRED) {
        // confirmed detection
        stopAllMotors();
        Serial.print("Task 5: Back zone confirmed at ");
        Serial.print(backDist);
        Serial.println(" mm -> ALIGN");
        setSubState(T5_ALIGN);
      } else {
        // keep moving backward (ensure speed set)
        robotBackward();  // Uses global baseSpeed
      }
      break;
    }

    // ALIGN – small backward creep for alignDuration (non-blocking)
    case T5_ALIGN: {
      unsigned long elapsed = millis() - subStateStartTime;
      if (elapsed == 0) {
        Serial.print("Task 5: ALIGN (backward creep) for ");
        Serial.print(alignDuration);
        Serial.println(" ms");
      }

      if (elapsed < alignDuration) {
        robotBackward();  // Uses global baseSpeed
      } else {
        stopAllMotors();
        Serial.println("Task 5: ALIGN done -> UNLOADING");
        setSubState(T5_UNLOADING);
      }
      break;
    }

    // UNLOADING – display barcode info and proceed to backup sequence
    case T5_UNLOADING: {
      Serial.println("Task 5: UNLOADING - Displaying barcode info");
      Serial.print("Barcode Value: ");
      Serial.print(barcodeValue);
      Serial.print(" (Binary: ");
      Serial.print(barcodeBinary);
      Serial.print(") - ");
      Serial.println((barcodeValue % 2 == 0) ? "EVEN" : "ODD");
      
      // Show barcode on OLED
      oledDisplay.show(
        "Task 5: Unload",
        "Val: " + String(barcodeValue) + " (" + String((barcodeValue % 2 == 0) ? "Even" : "Odd") + ")",
        "Bin: " + barcodeBinary
      );
      
      delay(1000);  // Show barcode for 1 second
      setSubState(T5_BACKUP_AFTER_ALIGN);
      break;
    }
    
    // BACKUP_AFTER_ALIGN - backup for 1000ms
    case T5_BACKUP_AFTER_ALIGN: {
      unsigned long elapsed = millis() - subStateStartTime;
      if (elapsed == 0) {
        Serial.println("Task 5: BACKUP_AFTER_ALIGN - backing up for 1000ms");
      }
      
      if (elapsed < 1000) {
        robotBackward();  // Uses global baseSpeed
      } else {
        stopAllMotors();
        Serial.println("Task 5: Backup complete -> ROTATE_LEFT");
        setSubState(T5_ROTATE_LEFT);
      }
      break;
    }
    
    // ROTATE_LEFT - left motor backward for 1800ms
    case T5_ROTATE_LEFT: {
      unsigned long elapsed = millis() - subStateStartTime;
      if (elapsed == 0) {
        Serial.println("Task 5: ROTATE_LEFT - rotating left (left motor back) for 1800ms");
      }
      
      if (elapsed < 1800) {
        // Rotate: left motor backward, right motor stopped
        setLeftMotorSpeed(getRotateSpeed());
        setRightMotorSpeed(0);
        leftMotorBackward();
        setMotorB(0, true);
      } else {
        stopAllMotors();
        Serial.println("Task 5: Rotation complete -> BACKUP_TO_WALL");
        setSubState(T5_BACKUP_TO_WALL);
      }
      break;
    }
    
    // BACKUP_TO_WALL - backup until back TOF reads below 50mm
    case T5_BACKUP_TO_WALL: {
      readTOFSensors();
      uint16_t backDist = tofSensors.getBackDistance();
      bool backOk = tofSensors.isBackValid();
      
      // Print occasionally (not every cycle)
      static unsigned long lastBackupPrint = 0;
      if (millis() - lastBackupPrint > 300) {
        Serial.print("T5 BACKUP_TO_WALL (back): ");
        if (backOk) Serial.print(backDist);
        else Serial.print("INVALID");
        Serial.println(" mm (target: <50mm)");
        lastBackupPrint = millis();
      }
      
      if (backOk && backDist <= 50) {
        stopAllMotors();
        Serial.print("Task 5: Back wall reached at ");
        Serial.print(backDist);
        Serial.println(" mm -> CHECK_COLOR_YELLOW");
        setSubState(T5_CHECK_COLOR_YELLOW);
      } else {
        robotBackward();  // Uses global baseSpeed
      }
      break;
    }
    
    // CHECK_COLOR_YELLOW - read back color sensor and drop yellow ball if condition met
    case T5_CHECK_COLOR_YELLOW: {
      Serial.println("Task 5: CHECK_COLOR_YELLOW - Reading back color sensor");
      
      // Read back color sensor
      colorSensors.readBackSensor();
      DetectedColor backColor = colorSensors.getBackColor();
      
      Serial.print("Back color detected: ");
      switch(backColor) {
        case COLOR_RED: Serial.println("RED"); break;
        case COLOR_BLUE: Serial.println("BLUE"); break;
        case COLOR_GREEN: Serial.println("GREEN"); break;
        case COLOR_YELLOW: Serial.println("YELLOW"); break;
        case COLOR_WHITE: Serial.println("WHITE"); break;
        case COLOR_BLACK: Serial.println("BLACK"); break;
        default: Serial.println("UNKNOWN"); break;
      }
      
      // Show on OLED
      oledDisplay.show(
        "Task 5: Color Check",
        "Val: " + String(barcodeValue) + " (" + String((barcodeValue % 2 == 0) ? "Even" : "Odd") + ")",
        "Back: " + String(backColor == COLOR_RED ? "RED" : backColor == COLOR_BLUE ? "BLUE" : "OTHER")
      );
      
      bool isEven = (barcodeValue % 2 == 0);
      bool shouldDropYellow = false;
      
      // Logic: Even + RED -> drop yellow, Odd + BLUE -> drop yellow
      if (isEven && backColor == COLOR_RED) {
        Serial.println("Condition met: EVEN barcode + RED color -> Dropping YELLOW ball");
        shouldDropYellow = true;
      } else if (!isEven && backColor == COLOR_BLUE) {
        Serial.println("Condition met: ODD barcode + BLUE color -> Dropping YELLOW ball");
        shouldDropYellow = true;
      } else {
        Serial.println("Yellow ball condition NOT met, skipping");
      }
      
      if (shouldDropYellow) {
        releaseYellowBall();
        ballsUnloaded++;
      }
      
      delay(500);
      setSubState(T5_CHECK_COLOR_WHITE);
      break;
    }
    
    // CHECK_COLOR_WHITE - read back color sensor and drop white ball if condition met
    case T5_CHECK_COLOR_WHITE: {
      Serial.println("Task 5: CHECK_COLOR_WHITE - Reading back color sensor");
      
      // Read back color sensor again
      colorSensors.readBackSensor();
      DetectedColor backColor = colorSensors.getBackColor();
      
      Serial.print("Back color detected: ");
      switch(backColor) {
        case COLOR_RED: Serial.println("RED"); break;
        case COLOR_BLUE: Serial.println("BLUE"); break;
        case COLOR_GREEN: Serial.println("GREEN"); break;
        case COLOR_YELLOW: Serial.println("YELLOW"); break;
        case COLOR_WHITE: Serial.println("WHITE"); break;
        case COLOR_BLACK: Serial.println("BLACK"); break;
        default: Serial.println("UNKNOWN"); break;
      }
      
      // Show on OLED
      oledDisplay.show(
        "Task 5: Color Check",
        "Val: " + String(barcodeValue) + " (" + String((barcodeValue % 2 == 0) ? "Even" : "Odd") + ")",
        "Back: " + String(backColor == COLOR_RED ? "RED" : backColor == COLOR_BLUE ? "BLUE" : "OTHER")
      );
      
      bool isEven = (barcodeValue % 2 == 0);
      bool shouldDropWhite = false;
      
      // Logic: Even + BLUE -> drop white, Odd + RED -> drop white
      if (isEven && backColor == COLOR_BLUE) {
        Serial.println("Condition met: EVEN barcode + BLUE color -> Dropping WHITE ball");
        shouldDropWhite = true;
      } else if (!isEven && backColor == COLOR_RED) {
        Serial.println("Condition met: ODD barcode + RED color -> Dropping WHITE ball");
        shouldDropWhite = true;
      } else {
        Serial.println("White ball condition NOT met, skipping");
      }
      
      if (shouldDropWhite) {
        releaseWhiteBall();
        ballsUnloaded++;
      }
      
      delay(500);
      setSubState(T5_VERIFY);
      break;
    }

    // VERIFY – simple; extend with sensors if you want robust checks
    case T5_VERIFY: {
      Serial.println("Task 5: VERIFY - assuming success");
      setSubState(T5_COMPLETED);
      break;
    }

    // COMPLETED – finish
    case T5_COMPLETED: {
      Serial.println("Task 5: COMPLETED");
      Serial.print("  Balls/Items unloaded: ");
      Serial.println(ballsUnloaded);
      stopAllMotors();
      taskActive = false;
      unloadStep = US_IDLE;
      break;
    }

    default: {
      Serial.println("Task 5: Unknown state - resetting");
      setSubState(T5_INIT);
      break;
    }
  } // switch
}

// ---------------- Display & helpers ----------------

void Task5Unloading::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 5: Unloading",
    "State: " + getSubStateName(),
    "Count: " + String(ballsUnloaded)
  );
}

void Task5Unloading::setSubState(Task5SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 5 sub-state -> ");
    Serial.println(getSubStateName());

    // Reset sequencing if leaving UNLOADING states
    if (newSubState != T5_UNLOADING && 
        newSubState != T5_BACKUP_AFTER_ALIGN &&
        newSubState != T5_ROTATE_LEFT &&
        newSubState != T5_BACKUP_TO_WALL &&
        newSubState != T5_CHECK_COLOR_YELLOW &&
        newSubState != T5_CHECK_COLOR_WHITE) {
      unloadStep = US_IDLE;
      backGoodCount = 0;
    }

    updateDisplay();
  }
}

Task5SubState Task5Unloading::getSubState() {
  return currentSubState;
}

String Task5Unloading::getSubStateName() {
  switch (currentSubState) {
    case T5_INIT:             return "INIT";
    case T5_NAVIGATE_TO_ZONE: return "NAVIGATE";
    case T5_ALIGN:            return "ALIGN";
    case T5_UNLOADING:        return "UNLOADING";
    case T5_BACKUP_AFTER_ALIGN: return "BACKUP";
    case T5_ROTATE_LEFT:      return "ROTATE";
    case T5_BACKUP_TO_WALL:   return "BACKUP_WALL";
    case T5_CHECK_COLOR_YELLOW: return "CHK_YELLOW";
    case T5_CHECK_COLOR_WHITE:  return "CHK_WHITE";
    case T5_VERIFY:           return "VERIFY";
    case T5_COMPLETED:        return "COMPLETED";
    default:                  return "UNKNOWN";
  }
}

// ---------------- Config setters / getters --------------

void Task5Unloading::setUnloadDuration(unsigned long timeMs) {
  unloadDuration = timeMs;
  Serial.print("T5 Unload duration set to ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task5Unloading::setAlignDuration(unsigned long timeMs) {
  alignDuration = timeMs;
  Serial.print("T5 Align duration set to: ");
  Serial.print(timeMs);
  Serial.println(" ms");
}

void Task5Unloading::setZoneDetectionDistance(uint16_t distance) {
  zoneDetectionDistance = distance;
  Serial.print("T5 Zone detection distance set to: ");
  Serial.print(distance);
  Serial.println(" mm");
}

void Task5Unloading::setTargetBallCount(uint16_t count) {
  targetBallCount = count;
  Serial.print("T5 Target ball count set to: ");
  Serial.println(count);
}

unsigned long Task5Unloading::getUnloadDuration()    { return unloadDuration; }
unsigned long Task5Unloading::getAlignDuration()     { return alignDuration; }
uint16_t Task5Unloading::getZoneDetectionDistance()  { return zoneDetectionDistance; }
uint16_t Task5Unloading::getTargetBallCount()        { return targetBallCount; }

// ---------------- OUT Servo Control ----------------

void Task5Unloading::setOutServoPositions(int pos0, int pos1, int pos2) {
  outServoPos0 = pos0;
  outServoPos1 = pos1;
  outServoPos2 = pos2;
  Serial.print("OUT Servo positions updated - Home: ");
  Serial.print(pos0);
  Serial.print("°, Yellow: ");
  Serial.print(pos1);
  Serial.print("°, White: ");
  Serial.print(pos2);
  Serial.println("°");
}

void Task5Unloading::moveOutServoTo(int position) {
  position = constrain(position, 0, 180);
  outServo.write(position);
  Serial.print("OUT Servo moved to: ");
  Serial.print(position);
  Serial.println("°");
}

void Task5Unloading::testOutServo(int angle) {
  angle = constrain(angle, 0, 180);
  outServo.write(angle);
  Serial.print("OUT Servo test: ");
  Serial.print(angle);
  Serial.println("°");
}

void Task5Unloading::releaseYellowBall() {
  Serial.println("=== Releasing YELLOW Ball ===");
  Serial.print("Moving OUT servo to yellow position: ");
  Serial.print(outServoPos1);
  Serial.println("°");
  
  // Move to yellow ball position
  outServo.write(outServoPos1);
  delay(1000);  // Wait for servo to reach position and ball to drop
  
  // Return to home position
  Serial.print("Returning to home position: ");
  Serial.print(outServoPos0);
  Serial.println("°");
  outServo.write(outServoPos0);
  delay(500);
  
  Serial.println("Yellow ball released successfully");
}

void Task5Unloading::releaseWhiteBall() {
  Serial.println("=== Releasing WHITE Ball ===");
  Serial.print("Moving OUT servo to white position: ");
  Serial.print(outServoPos2);
  Serial.println("°");
  
  // Move to white ball position
  outServo.write(outServoPos2);
  delay(1000);  // Wait for servo to reach position and ball to drop
  
  // Return to home position
  Serial.print("Returning to home position: ");
  Serial.print(outServoPos0);
  Serial.println("°");
  outServo.write(outServoPos0);
  delay(500);
  
  Serial.println("White ball released successfully");
}
