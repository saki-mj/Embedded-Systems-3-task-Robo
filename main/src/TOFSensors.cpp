#include "TOFSensors.h"

// Global TOF sensor object
TOFSensors tofSensors;

// Constructor
TOFSensors::TOFSensors() {
  mux = new I2CMux();
  loxLeft = new Adafruit_VL53L0X();
  loxFront = new Adafruit_VL53L0X();
  loxRight = new Adafruit_VL53L0X();
  loxBack = new Adafruit_VL53L0X();
  
  distanceLeft = 0;
  distanceFront = 0;
  distanceRight = 0;
  distanceBack = 0;
  
  leftValid = false;
  frontValid = false;
  rightValid = false;
  backValid = false;
  
  // Initialize calibration errors to 0
  errorLeft = 0;
  errorFront = 0;
  errorRight = 0;
  errorBack = 0;
  calibrated = false;
  
  obstacleThreshold = TOF_OBSTACLE_THRESHOLD;
  continuousReadingActive = false;
}

// Initialize all TOF sensors
bool TOFSensors::begin() {
  Serial.println("=== Initializing TOF Sensors ===");
  
  // Initialize I2C Multiplexer
  mux->begin();
  
  if (!mux->isConnected()) {
    Serial.println("ERROR: Failed to initialize I2C Multiplexer!");
    return false;
  }
  
  // Initialize Left TOF on Channel 0
  Serial.println("Initializing Left TOF (Channel 0)...");
  mux->selectChannel(MUX_CHANNEL_0);
  delay(50);
  if (!loxLeft->begin()) {
    Serial.println("ERROR: Failed to initialize Left TOF!");
    return false;
  }
  Serial.println("Left TOF initialized successfully");
  
  // Initialize Front TOF on Channel 4
  Serial.println("Initializing Front TOF (Channel 4)...");
  mux->selectChannel(MUX_CHANNEL_4);
  delay(50);
  if (!loxFront->begin()) {
    Serial.println("ERROR: Failed to initialize Front TOF!");
    return false;
  }
  Serial.println("Front TOF initialized successfully");
  
  // Initialize Right TOF on Channel 3
  Serial.println("Initializing Right TOF (Channel 3)...");
  mux->selectChannel(MUX_CHANNEL_3);
  delay(50);
  if (!loxRight->begin()) {
    Serial.println("ERROR: Failed to initialize Right TOF!");
    return false;
  }
  Serial.println("Right TOF initialized successfully");
  
  // Initialize Back TOF on Channel 7
  Serial.println("Initializing Back TOF (Channel 7)...");
  mux->selectChannel(MUX_CHANNEL_7);
  delay(50);
  if (!loxBack->begin()) {
    Serial.println("ERROR: Failed to initialize Back TOF!");
    return false;
  }
  Serial.println("Back TOF initialized successfully");
  
  Serial.println("=== All TOF Sensors Ready ===");
  Serial.println("Run TOFCALIBRATE command to calibrate sensors\n");
  return true;
}

// Read all sensors
void TOFSensors::readAll() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Read Left TOF
  mux->selectChannel(MUX_CHANNEL_0);
  loxLeft->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceLeft = measure.RangeMilliMeter - errorLeft;
    if (distanceLeft < 0) distanceLeft = 0;
    leftValid = true;
  } else {
    distanceLeft = TOF_MAX_RANGE;
    leftValid = false;
  }
  
  // Read Front TOF
  mux->selectChannel(MUX_CHANNEL_4);
  loxFront->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceFront = measure.RangeMilliMeter - errorFront;
    if (distanceFront < 0) distanceFront = 0;
    frontValid = true;
  } else {
    distanceFront = TOF_MAX_RANGE;
    frontValid = false;
  }
  
  // Read Right TOF
  mux->selectChannel(MUX_CHANNEL_3);
  loxRight->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceRight = measure.RangeMilliMeter - errorRight;
    if (distanceRight < 0) distanceRight = 0;
    rightValid = true;
  } else {
    distanceRight = TOF_MAX_RANGE;
    rightValid = false;
  }
  
  // Read Back TOF
  mux->selectChannel(MUX_CHANNEL_7);
  loxBack->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceBack = measure.RangeMilliMeter - errorBack;
    if (distanceBack < 0) distanceBack = 0;
    backValid = true;
  } else {
    distanceBack = TOF_MAX_RANGE;
    backValid = false;
  }
}

// Get individual distances
uint16_t TOFSensors::getLeftDistance() {
  return distanceLeft;
}

uint16_t TOFSensors::getFrontDistance() {
  return distanceFront;
}

uint16_t TOFSensors::getRightDistance() {
  return distanceRight;
}

uint16_t TOFSensors::getBackDistance() {
  return distanceBack;
}

// Check if obstacle detected
bool TOFSensors::isObstacleLeft() {
  return (leftValid && distanceLeft < obstacleThreshold);
}

bool TOFSensors::isObstacleFront() {
  return (frontValid && distanceFront < obstacleThreshold);
}

bool TOFSensors::isObstacleRight() {
  return (rightValid && distanceRight < obstacleThreshold);
}

bool TOFSensors::isObstacleBack() {
  return (backValid && distanceBack < obstacleThreshold);
}

// Set obstacle detection threshold
void TOFSensors::setObstacleThreshold(uint16_t threshold) {
  obstacleThreshold = threshold;
  Serial.print("TOF obstacle threshold set to: ");
  Serial.print(threshold);
  Serial.println(" mm");
}

// Print sensor values
void TOFSensors::printDistances() {
  Serial.print("Left: ");
  if (leftValid) {
    Serial.print(distanceLeft);
    Serial.print(" mm");
  } else {
    Serial.print("Out of range");
  }
  
  Serial.print(" | Front: ");
  if (frontValid) {
    Serial.print(distanceFront);
    Serial.print(" mm");
  } else {
    Serial.print("Out of range");
  }
  
  Serial.print(" | Right: ");
  if (rightValid) {
    Serial.print(distanceRight);
    Serial.print(" mm");
  } else {
    Serial.print("Out of range");
  }
  
  Serial.print(" | Back: ");
  if (backValid) {
    Serial.print(distanceBack);
    Serial.print(" mm");
  } else {
    Serial.print("Out of range");
  }
  Serial.println();
}

// Check if sensor reading is valid
bool TOFSensors::isLeftValid() {
  return leftValid;
}

bool TOFSensors::isFrontValid() {
  return frontValid;
}

bool TOFSensors::isRightValid() {
  return rightValid;
}

bool TOFSensors::isBackValid() {
  return backValid;
}

// Calibration methods
void TOFSensors::calibrate() {
  Serial.println("\n=== TOF Sensor Calibration ===");
  Serial.println("Place robot in open area with no obstacles nearby.");
  Serial.println("Calibrating in 3 seconds...");
  delay(3000);
  
  VL53L0X_RangingMeasurementData_t measure;
  
  // Calibrate Left TOF
  Serial.print("Calibrating Left TOF... ");
  mux->selectChannel(MUX_CHANNEL_0);
  delay(50);
  loxLeft->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    errorLeft = measure.RangeMilliMeter - TOF_MAX_RANGE;
    Serial.print("Error: ");
    Serial.print(errorLeft);
    Serial.println(" mm");
  } else {
    errorLeft = 0;
    Serial.println("Out of range - setting error to 0");
  }
  
  // Calibrate Front TOF
  Serial.print("Calibrating Front TOF... ");
  mux->selectChannel(MUX_CHANNEL_4);
  delay(50);
  loxFront->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    errorFront = measure.RangeMilliMeter - TOF_MAX_RANGE;
    Serial.print("Error: ");
    Serial.print(errorFront);
    Serial.println(" mm");
  } else {
    errorFront = 0;
    Serial.println("Out of range - setting error to 0");
  }
  
  // Calibrate Right TOF
  Serial.print("Calibrating Right TOF... ");
  mux->selectChannel(MUX_CHANNEL_3);
  delay(50);
  loxRight->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    errorRight = measure.RangeMilliMeter - TOF_MAX_RANGE;
    Serial.print("Error: ");
    Serial.print(errorRight);
    Serial.println(" mm");
  } else {
    errorRight = 0;
    Serial.println("Out of range - setting error to 0");
  }
  
  // Calibrate Back TOF
  Serial.print("Calibrating Back TOF... ");
  mux->selectChannel(MUX_CHANNEL_7);
  delay(50);
  loxBack->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    errorBack = measure.RangeMilliMeter - TOF_MAX_RANGE;
    Serial.print("Error: ");
    Serial.print(errorBack);
    Serial.println(" mm");
  } else {
    errorBack = 0;
    Serial.println("Out of range - setting error to 0");
  }
  
  calibrated = true;
  Serial.println("=== Calibration Complete ===\n");
}

void TOFSensors::setErrorLeft(int16_t error) {
  errorLeft = error;
  Serial.print("Left TOF error set to: ");
  Serial.print(errorLeft);
  Serial.println(" mm");
}

void TOFSensors::setErrorFront(int16_t error) {
  errorFront = error;
  Serial.print("Front TOF error set to: ");
  Serial.print(errorFront);
  Serial.println(" mm");
}

void TOFSensors::setErrorRight(int16_t error) {
  errorRight = error;
  Serial.print("Right TOF error set to: ");
  Serial.print(errorRight);
  Serial.println(" mm");
}

void TOFSensors::setErrorBack(int16_t error) {
  errorBack = error;
  Serial.print("Back TOF error set to: ");
  Serial.print(errorBack);
  Serial.println(" mm");
}

int16_t TOFSensors::getErrorLeft() {
  return errorLeft;
}

int16_t TOFSensors::getErrorFront() {
  return errorFront;
}

int16_t TOFSensors::getErrorRight() {
  return errorRight;
}

int16_t TOFSensors::getErrorBack() {
  return errorBack;
}

bool TOFSensors::isCalibrated() {
  return calibrated;
}

void TOFSensors::toggleContinuousReading() {
  continuousReadingActive = !continuousReadingActive;
  if (continuousReadingActive) {
    Serial.println("TOF Continuous Reading: ON");
  } else {
    Serial.println("TOF Continuous Reading: OFF");
  }
}

bool TOFSensors::isContinuousReadingActive() {
  return continuousReadingActive;
}

// Read only back sensor
void TOFSensors::readBack() {
  VL53L0X_RangingMeasurementData_t measure;
  
  mux->selectChannel(MUX_CHANNEL_7);
  loxBack->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceBack = measure.RangeMilliMeter - errorBack;
    if (distanceBack < 0) distanceBack = 0;
    backValid = true;
  } else {
    distanceBack = TOF_MAX_RANGE;
    backValid = false;
  }
}

// Global functions

void initTOFSensors() {
  tofSensors.begin();
}

void readTOFSensors() {
  tofSensors.readAll();
}

void printTOFValues() {
  tofSensors.printDistances();
}
