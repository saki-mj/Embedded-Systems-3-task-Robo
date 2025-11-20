#include "Gyroscope.h"

// Global gyroscope object
Gyroscope gyroscope;

// Constructor
Gyroscope::Gyroscope() {
  accelOffsetX = 0.0;
  accelOffsetY = 0.0;
  accelOffsetZ = 0.0;
  gyroOffsetX = 0.0;
  gyroOffsetY = 0.0;
  gyroOffsetZ = 0.0;
  
  accelX = 0.0;
  accelY = 0.0;
  accelZ = 0.0;
  gyroX = 0.0;
  gyroY = 0.0;
  gyroZ = 0.0;
  
  roll = 0.0;
  pitch = 0.0;
  
  calibrated = false;
  initialized = false;
  continuousReadingActive = false;
}

// Initialize MPU6050 sensor
bool Gyroscope::begin() {
  Serial.println("Initializing MPU6050...");
  
  // MPU6050 library uses Wire (main I2C bus) by default
  // Wire is already initialized by other sensors (TOF, Color)
  
  mpu.initialize();
  
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    Serial.println("Check wiring: SDA->Pin 21, SCL->Pin 22, VCC->3.3V, GND->GND");
    initialized = false;
    return false;
  }
  
  Serial.println("MPU6050 connected.");
  initialized = true;
  
  // Give time to stabilize
  delay(1000);
  
  return true;
}

// Calibrate sensor (must be still during calibration)
void Gyroscope::calibrate() {
  if (!initialized) {
    Serial.println("MPU6050 not initialized. Cannot calibrate.");
    return;
  }
  
  Serial.println("Keep the sensor absolutely still for calibration...");
  delay(2000);
  
  long sumAx = 0, sumAy = 0, sumAz = 0;
  long sumGx = 0, sumGy = 0, sumGz = 0;

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    sumAx += ax;
    sumAy += ay;
    sumAz += az;
    sumGx += gx;
    sumGy += gy;
    sumGz += gz;
    delay(CALIBRATION_DELAY_MS);
  }

  accelOffsetX = (float)sumAx / CALIBRATION_SAMPLES;
  accelOffsetY = (float)sumAy / CALIBRATION_SAMPLES;
  // Subtract 1g on Z for static gravity
  accelOffsetZ = (float)sumAz / CALIBRATION_SAMPLES - ACCEL_SENSITIVITY;

  gyroOffsetX = (float)sumGx / CALIBRATION_SAMPLES;
  gyroOffsetY = (float)sumGy / CALIBRATION_SAMPLES;
  gyroOffsetZ = (float)sumGz / CALIBRATION_SAMPLES;
  
  calibrated = true;
  
  Serial.println("Calibration done.");
  printCalibration();
}

// Read sensor data
void Gyroscope::read() {
  if (!initialized) {
    return;
  }
  
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Apply calibration offsets
  accelX = (ax - accelOffsetX) / float(ACCEL_SENSITIVITY);
  accelY = (ay - accelOffsetY) / float(ACCEL_SENSITIVITY);
  accelZ = (az - accelOffsetZ) / float(ACCEL_SENSITIVITY);

  gyroX = (gx - gyroOffsetX) / float(GYRO_SENSITIVITY);
  gyroY = (gy - gyroOffsetY) / float(GYRO_SENSITIVITY);
  gyroZ = (gz - gyroOffsetZ) / float(GYRO_SENSITIVITY);

  // Compute roll & pitch (swapped due to MPU6050 orientation)
  pitch = atan2(accelY, accelZ) * 180.0 / PI;
  roll = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180.0 / PI;
}

// Get accelerometer readings (in g)
float Gyroscope::getAccelX() {
  return accelX;
}

float Gyroscope::getAccelY() {
  return accelY;
}

float Gyroscope::getAccelZ() {
  return accelZ;
}

// Get gyroscope readings (in deg/s)
float Gyroscope::getGyroX() {
  return gyroX;
}

float Gyroscope::getGyroY() {
  return gyroY;
}

float Gyroscope::getGyroZ() {
  return gyroZ;
}

// Get roll and pitch angles (in degrees)
float Gyroscope::getRoll() {
  return roll;
}

float Gyroscope::getPitch() {
  return pitch;
}

// Get calibration offsets
float Gyroscope::getAccelOffsetX() {
  return accelOffsetX;
}

float Gyroscope::getAccelOffsetY() {
  return accelOffsetY;
}

float Gyroscope::getAccelOffsetZ() {
  return accelOffsetZ;
}

float Gyroscope::getGyroOffsetX() {
  return gyroOffsetX;
}

float Gyroscope::getGyroOffsetY() {
  return gyroOffsetY;
}

float Gyroscope::getGyroOffsetZ() {
  return gyroOffsetZ;
}

// Set calibration offsets manually
void Gyroscope::setAccelOffset(float x, float y, float z) {
  accelOffsetX = x;
  accelOffsetY = y;
  accelOffsetZ = z;
  calibrated = true;
}

void Gyroscope::setGyroOffset(float x, float y, float z) {
  gyroOffsetX = x;
  gyroOffsetY = y;
  gyroOffsetZ = z;
}

// Check if initialized and calibrated
bool Gyroscope::isInitialized() {
  return initialized;
}

bool Gyroscope::isCalibrated() {
  return calibrated;
}

// Print sensor values
void Gyroscope::printValues() {
  Serial.print("Pitch: "); Serial.print(pitch, 2);
  Serial.print("   Roll: "); Serial.print(roll, 2);
  Serial.print("   GyroZ: "); Serial.print(gyroZ, 2);
  Serial.println();
}

void Gyroscope::printCalibration() {
  Serial.println("Calibration Offsets:");
  Serial.print("  accelOffsetX = "); Serial.print(accelOffsetX);
  Serial.print(", accelOffsetY = "); Serial.print(accelOffsetY);
  Serial.print(", accelOffsetZ = "); Serial.println(accelOffsetZ);
  Serial.print("  gyroOffsetX  = "); Serial.print(gyroOffsetX);
  Serial.print(", gyroOffsetY  = "); Serial.print(gyroOffsetY);
  Serial.print(", gyroOffsetZ  = "); Serial.println(gyroOffsetZ);
}

// Global helper functions
void initGyroscope() {
  gyroscope.begin();
}

void readGyroscope() {
  gyroscope.read();
}

void printGyroscopeValues() {
  gyroscope.printValues();
}

// Toggle continuous reading mode
void Gyroscope::toggleContinuousReading() {
  continuousReadingActive = !continuousReadingActive;
  if (continuousReadingActive) {
    Serial.println("Gyroscope Continuous Reading: ON");
  } else {
    Serial.println("Gyroscope Continuous Reading: OFF");
  }
}

bool Gyroscope::isContinuousReadingActive() {
  return continuousReadingActive;
}
