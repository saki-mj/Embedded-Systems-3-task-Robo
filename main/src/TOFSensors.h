#ifndef TOFSENSORS_H
#define TOFSENSORS_H

#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include "I2CMux.h"

// TOF Sensor positions (I2C Mux Channels)
#define TOF_LEFT   0  // Channel 0
#define TOF_FRONT  4  // Channel 4
#define TOF_RIGHT  3  // Channel 3

// Distance thresholds (in mm)
#define TOF_OBSTACLE_THRESHOLD 200  // Default obstacle detection distance
#define TOF_MAX_RANGE 2000          // Maximum reliable range

class TOFSensors {
  private:
    I2CMux* mux;
    Adafruit_VL53L0X* loxLeft;
    Adafruit_VL53L0X* loxFront;
    Adafruit_VL53L0X* loxRight;
    
    uint16_t distanceLeft;
    uint16_t distanceFront;
    uint16_t distanceRight;
    
    bool leftValid;
    bool frontValid;
    bool rightValid;
    
    uint16_t obstacleThreshold;
    
    bool continuousReadingActive;

  public:
    // Constructor
    TOFSensors();
    
    // Initialize all TOF sensors
    bool begin();
    
    // Read all sensors
    void readAll();
    
    // Get individual distances
    uint16_t getLeftDistance();
    uint16_t getFrontDistance();
    uint16_t getRightDistance();
    
    // Check if obstacle detected
    bool isObstacleLeft();
    bool isObstacleFront();
    bool isObstacleRight();
    
    // Set obstacle detection threshold
    void setObstacleThreshold(uint16_t threshold);
    
    // Print sensor values
    void printDistances();
    
    // Check if sensor reading is valid
    bool isLeftValid();
    bool isFrontValid();
    bool isRightValid();
    
    // Toggle continuous reading mode
    void toggleContinuousReading();
    bool isContinuousReadingActive();
};

// Global TOF sensor object
extern TOFSensors tofSensors;

// Initialization function
void initTOFSensors();

// Get TOF readings
void readTOFSensors();

// Print TOF values
void printTOFValues();

#endif
