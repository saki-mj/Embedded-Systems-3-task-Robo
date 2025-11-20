#ifndef GYROSCOPE_H
#define GYROSCOPE_H

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

// Main I2C Bus pins for MPU6050 (shared with other I2C devices)
#define GYRO_I2C_SDA 21  // Main I2C SDA pin
#define GYRO_I2C_SCL 22  // Main I2C SCL pin

// Sensitivity constants for MPU6050
#define ACCEL_SENSITIVITY 16384  // for ±2g
#define GYRO_SENSITIVITY  131    // for ±250 deg/s

// Calibration settings
#define CALIBRATION_SAMPLES 200
#define CALIBRATION_DELAY_MS 10

class Gyroscope {
  private:
    MPU6050 mpu;
    
    // Calibration offset variables
    float accelOffsetX;
    float accelOffsetY;
    float accelOffsetZ;
    float gyroOffsetX;
    float gyroOffsetY;
    float gyroOffsetZ;
    
    // Current sensor readings (calibrated)
    float accelX;
    float accelY;
    float accelZ;
    float gyroX;
    float gyroY;
    float gyroZ;
    
    // Roll and pitch angles
    float roll;
    float pitch;
    
    bool calibrated;
    bool initialized;
    bool continuousReadingActive;
    
  public:
    // Constructor
    Gyroscope();
    
    // Initialize MPU6050 sensor
    bool begin();
    
    // Calibrate sensor (must be still during calibration)
    void calibrate();
    
    // Read sensor data
    void read();
    
    // Get accelerometer readings (in g)
    float getAccelX();
    float getAccelY();
    float getAccelZ();
    
    // Get gyroscope readings (in deg/s)
    float getGyroX();
    float getGyroY();
    float getGyroZ();
    
    // Get roll and pitch angles (in degrees)
    float getRoll();
    float getPitch();
    
    // Get calibration offsets
    float getAccelOffsetX();
    float getAccelOffsetY();
    float getAccelOffsetZ();
    float getGyroOffsetX();
    float getGyroOffsetY();
    float getGyroOffsetZ();
    
    // Set calibration offsets manually
    void setAccelOffset(float x, float y, float z);
    void setGyroOffset(float x, float y, float z);
    
    // Check if initialized and calibrated
    bool isInitialized();
    bool isCalibrated();
    
    // Print sensor values
    void printValues();
    void printCalibration();
    
    // Toggle continuous reading mode
    void toggleContinuousReading();
    bool isContinuousReadingActive();
};

// Global gyroscope object
extern Gyroscope gyroscope;

// Initialization function
void initGyroscope();

// Read gyroscope
void readGyroscope();

// Print gyroscope values
void printGyroscopeValues();

#endif
