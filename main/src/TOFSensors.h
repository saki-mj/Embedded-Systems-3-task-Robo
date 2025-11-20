#ifndef TOFSENSORS_H
#define TOFSENSORS_H

#include <Arduino.h>
#include "Adafruit_VL53L0X.h"
#include "I2CMux.h"

// TOF Sensor positions (I2C Mux Channels)
#define TOF_LEFT   0  // Channel 0
#define TOF_FRONT  4  // Channel 4
#define TOF_RIGHT  3  // Channel 3
#define TOF_BACK   7  // Channel 7

// Distance thresholds (in mm)
#define TOF_OBSTACLE_THRESHOLD 200  // Default obstacle detection distance
#define TOF_MAX_RANGE 2000          // Maximum reliable range

class TOFSensors {
  private:
    I2CMux* mux;
    Adafruit_VL53L0X* loxLeft;
    Adafruit_VL53L0X* loxFront;
    Adafruit_VL53L0X* loxRight;
    Adafruit_VL53L0X* loxBack;
    
    uint16_t distanceLeft;
    uint16_t distanceFront;
    uint16_t distanceRight;
    uint16_t distanceBack;
    
    bool leftValid;
    bool frontValid;
    bool rightValid;
    bool backValid;
    
    // Calibration errors (offset to subtract from readings)
    int16_t errorLeft;
    int16_t errorFront;
    int16_t errorRight;
    int16_t errorBack;
    
    bool calibrated;
    
    uint16_t obstacleThreshold;
    
    bool continuousReadingActive;

  public:
    // Constructor
    TOFSensors();
    
    // Initialize all TOF sensors
    bool begin();
    
    // Read all sensors
    void readAll();
    
    // Get individual distances (with error correction)
    uint16_t getLeftDistance();
    uint16_t getFrontDistance();
    uint16_t getRightDistance();
    uint16_t getBackDistance();
    
    // Check if obstacle detected
    bool isObstacleLeft();
    bool isObstacleFront();
    bool isObstacleRight();
    bool isObstacleBack();
    
    // Set obstacle detection threshold
    void setObstacleThreshold(uint16_t threshold);
    
    // Print sensor values
    void printDistances();
    
    // Check if sensor reading is valid
    bool isLeftValid();
    bool isFrontValid();
    bool isRightValid();
    bool isBackValid();
    
    // Calibration methods
    void calibrate();
    void setErrorLeft(int16_t error);
    void setErrorFront(int16_t error);
    void setErrorRight(int16_t error);
    void setErrorBack(int16_t error);
    int16_t getErrorLeft();
    int16_t getErrorFront();
    int16_t getErrorRight();
    int16_t getErrorBack();
    bool isCalibrated();
    
    // Toggle continuous reading mode
    void toggleContinuousReading();
    bool isContinuousReadingActive();
    
    // Read only back sensor
    void readBack();
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
