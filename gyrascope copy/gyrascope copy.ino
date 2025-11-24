#include <Wire.h>
#include <MPU6050.h>
#include <QMC5883LCompass.h>

MPU6050 mpu;
QMC5883LCompass compass;

// calibration offset variables
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
float gyroOffsetX  = 0, gyroOffsetY  = 0, gyroOffsetZ  = 0;

const int16_t ACCEL_SENSITIVITY = 16384; // for ±2 g
const int16_t GYRO_SENSITIVITY  = 131;   // for ±250 deg/s

// Magnetometer offsets (calibration)
float magOffsetX = 0, magOffsetY = 0, magOffsetZ = 0;
float magScaleX = 1.0, magScaleY = 1.0, magScaleZ = 1.0;

// Complementary filter for yaw
float yaw = 0.0;
float magYaw = 0.0;
unsigned long lastTime = 0;
const float ALPHA = 0.98; // Complementary filter coefficient

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  delay(100);

  Serial.println("Initializing MPU6050...");
  
  // Scan I2C bus for devices
  Serial.println("Scanning I2C bus...");
  byte error, address;
  int nDevices = 0;
  for(address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found!");
  } else {
    Serial.print("Found ");
    Serial.print(nDevices);
    Serial.println(" I2C device(s)");
  }
  
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    Serial.println("Check wiring: SDA->21, SCL->22, VCC->3.3V, GND->GND");
    while (1) { delay(1000); }
  }
  Serial.println("MPU6050 connected.");
  
  // Initialize HMC5883L/QMC5883L
  Serial.println("Initializing Magnetometer...");
  compass.init();
  compass.setCalibration(-1362, 1318, -1254, 1559, -1256, 1396); // Default calibration
  Serial.println("Magnetometer initialized.");
  Serial.println("Note: For best accuracy, calibrate magnetometer by rotating 360° in all axes");

  // give time to stabilize
  delay(1000);

  Serial.println("Keep the sensor absolutely still for calibration in 5 seconds...");
  delay(5000);
  calibrateSensor();
  Serial.println("Calibration done. Offsets:");
  Serial.print("accelOffsetX = "); Serial.print(accelOffsetX);
  Serial.print(", accelOffsetY = "); Serial.print(accelOffsetY);
  Serial.print(", accelOffsetZ = "); Serial.println(accelOffsetZ);
  Serial.print("gyroOffsetX  = "); Serial.print(gyroOffsetX);
  Serial.print(", gyroOffsetY  = "); Serial.print(gyroOffsetY);
  Serial.print(", gyroOffsetZ  = "); Serial.println(gyroOffsetZ);
  
  lastTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // Convert to seconds
  lastTime = currentTime;
  
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // apply calibration offsets
  float AccX = (ax   - accelOffsetX) / float(ACCEL_SENSITIVITY);
  float AccY = (ay   - accelOffsetY) / float(ACCEL_SENSITIVITY);
  float AccZ = (az   - accelOffsetZ) / float(ACCEL_SENSITIVITY);

  float GyroX = (gx   - gyroOffsetX)  / float(GYRO_SENSITIVITY);
  float GyroY = (gy   - gyroOffsetY)  / float(GYRO_SENSITIVITY);
  float GyroZ = (gz   - gyroOffsetZ)  / float(GYRO_SENSITIVITY);

  // compute roll & pitch
  float roll  = atan2(AccY, AccZ) * 180.0 / PI;
  float pitch = atan2(-AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0 / PI;
  
  // Read magnetometer
  compass.read();
  int mx = compass.getX();
  int my = compass.getY();
  int mz = compass.getZ();
  
  // Calculate tilt-compensated magnetic yaw
  float rollRad = roll * PI / 180.0;
  float pitchRad = pitch * PI / 180.0;
  
  // Tilt compensation
  float magX = mx * cos(pitchRad) + mz * sin(pitchRad);
  float magY = mx * sin(rollRad) * sin(pitchRad) + my * cos(rollRad) - mz * sin(rollRad) * cos(pitchRad);
  
  // Calculate magnetic yaw (0-360 degrees)
  magYaw = atan2(magY, magX) * 180.0 / PI;
  if (magYaw < 0) magYaw += 360;
  
  // Integrate gyroscope for smooth yaw
  float gyroYaw = yaw + GyroZ * dt;
  
  // Complementary filter: combine gyro (fast) with magnetometer (accurate)
  // Handle 0/360 degree wrap-around
  float yawDiff = magYaw - yaw;
  if (yawDiff > 180) yawDiff -= 360;
  if (yawDiff < -180) yawDiff += 360;
  
  yaw = ALPHA * gyroYaw + (1 - ALPHA) * (yaw + yawDiff);
  
  // Keep yaw in 0-360 range
  if (yaw < 0) yaw += 360;
  if (yaw >= 360) yaw -= 360;

  // Format for Serial Plotter
  Serial.print("Pitch:");
  Serial.print(pitch, 2);
  Serial.print(",Roll:");
  Serial.print(roll, 2);
  Serial.print(",Yaw:");
  Serial.print(yaw, 2);
  Serial.print(",GyroX:");
  Serial.print(GyroX, 2);
  Serial.print(",GyroY:");
  Serial.print(GyroY, 2);
  Serial.print(",GyroZ:");
  Serial.println(GyroZ, 2);

  delay(50);
}

void calibrateSensor() {
  const int CALIBRATION_SAMPLES = 200;
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
    delay(10);
  }

  accelOffsetX = (float)sumAx / CALIBRATION_SAMPLES;
  accelOffsetY = (float)sumAy / CALIBRATION_SAMPLES;
  accelOffsetZ = (float)sumAz / CALIBRATION_SAMPLES - ACCEL_SENSITIVITY;  
    // subtract 1g on Z for static gravity

  gyroOffsetX  = (float)sumGx / CALIBRATION_SAMPLES;
  gyroOffsetY  = (float)sumGy / CALIBRATION_SAMPLES;
  gyroOffsetZ  = (float)sumGz / CALIBRATION_SAMPLES;
}
