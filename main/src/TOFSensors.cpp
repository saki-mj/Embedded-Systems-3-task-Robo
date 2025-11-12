#include "TOFSensors.h"

// Global TOF sensor object
TOFSensors tofSensors;

// Constructor
TOFSensors::TOFSensors() {
  mux = new I2CMux();
  loxLeft = new Adafruit_VL53L0X();
  loxFront = new Adafruit_VL53L0X();
  loxRight = new Adafruit_VL53L0X();
  
  distanceLeft = 0;
  distanceFront = 0;
  distanceRight = 0;
  
  leftValid = false;
  frontValid = false;
  rightValid = false;
  
  obstacleThreshold = TOF_OBSTACLE_THRESHOLD;
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
  
  // Initialize Front TOF on Channel 1
  Serial.println("Initializing Front TOF (Channel 1)...");
  mux->selectChannel(MUX_CHANNEL_1);
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
  
  Serial.println("=== All TOF Sensors Ready ===\n");
  return true;
}

// Read all sensors
void TOFSensors::readAll() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Read Left TOF
  mux->selectChannel(MUX_CHANNEL_0);
  loxLeft->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceLeft = measure.RangeMilliMeter;
    leftValid = true;
  } else {
    distanceLeft = TOF_MAX_RANGE;
    leftValid = false;
  }
  
  // Read Front TOF
  mux->selectChannel(MUX_CHANNEL_1);
  loxFront->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceFront = measure.RangeMilliMeter;
    frontValid = true;
  } else {
    distanceFront = TOF_MAX_RANGE;
    frontValid = false;
  }
  
  // Read Right TOF
  mux->selectChannel(MUX_CHANNEL_3);
  loxRight->rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    distanceRight = measure.RangeMilliMeter;
    rightValid = true;
  } else {
    distanceRight = TOF_MAX_RANGE;
    rightValid = false;
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
