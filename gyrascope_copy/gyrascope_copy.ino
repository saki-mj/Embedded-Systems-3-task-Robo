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

// Kalman filter variables for Roll
float Q_angle_roll = 0.001;
float Q_bias_roll = 0.003;
float R_measure_roll = 0.03;
float angle_roll = 0;
float bias_roll = 0;
float P_roll[2][2] = {{0, 0}, {0, 0}};

// Kalman filter variables for Pitch
float Q_angle_pitch = 0.001;
float Q_bias_pitch = 0.003;
float R_measure_pitch = 0.03;
float angle_pitch = 0;
float bias_pitch = 0;
float P_pitch[2][2] = {{0, 0}, {0, 0}};

// Kalman filter variables for Yaw
float Q_angle_yaw = 0.001;
float Q_bias_yaw = 0.003;
float R_measure_yaw = 0.03;
float angle_yaw = 0;
float bias_yaw = 0;
float P_yaw[2][2] = {{0, 0}, {0, 0}};

// Compass variables
float compassYaw = 0;
float compassOffsetYaw = 0;  // Calibration offset for compass

unsigned long lastTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(8, 9); // SDA, SCL
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

  // Initialize QMC5883L Compass
  Serial.println("Initializing QMC5883L Compass...");
  compass.init();
  compass.setSmoothing(10, true);  // Smooth readings
  Serial.println("QMC5883L Compass connected.");

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
  
  // Calibrate compass yaw offset
  Serial.println("Calibrating compass for initial heading...");
  compass.read();
  compassOffsetYaw = compass.getAzimuth();
  Serial.print("Compass offset yaw = "); Serial.println(compassOffsetYaw);
  
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

  // compute raw roll & pitch from accelerometer
  float roll_acc  = atan2(AccY, AccZ) * 180.0 / PI;
  float pitch_acc = atan2(-AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0 / PI;
  
  // Apply Kalman filter for Roll
  float roll = kalmanFilter(roll_acc, GyroX, dt, 
                             &angle_roll, &bias_roll, P_roll,
                             Q_angle_roll, Q_bias_roll, R_measure_roll);
  
  // Apply Kalman filter for Pitch
  float pitch = kalmanFilter(pitch_acc, GyroY, dt,
                              &angle_pitch, &bias_pitch, P_pitch,
                              Q_angle_pitch, Q_bias_pitch, R_measure_pitch);
  
  // Read compass for absolute yaw heading
  compass.read();
  compassYaw = compass.getAzimuth() - compassOffsetYaw;
  
  // Normalize compass yaw to -180 to 180 range
  if (compassYaw > 180) compassYaw -= 360;
  if (compassYaw < -180) compassYaw += 360;
  
  // Apply Kalman filter for Yaw using compass measurement (more accurate than gyro drift)
  float yaw = kalmanFilter(compassYaw, GyroZ, dt,
                           &angle_yaw, &bias_yaw, P_yaw,
                           Q_angle_yaw, Q_bias_yaw, R_measure_yaw);

  // Format for Serial Plotter
  Serial.print("Pitch:");
  Serial.print(pitch, 2);
  Serial.print(",Roll:");
  Serial.print(roll, 2);
  Serial.print(",Yaw:");
  Serial.print(yaw, 2);
  Serial.print(",CompassYaw:");
  Serial.print(compassYaw, 2);
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

// Kalman filter function
float kalmanFilter(float newAngle, float newRate, float dt,
                   float* angle, float* bias, float P[2][2],
                   float Q_angle, float Q_bias, float R_measure) {
  // Predict
  float rate = newRate - *bias;
  *angle += dt * rate;
  
  P[0][0] += dt * (dt * P[1][1] - P[0][1] - P[1][0] + Q_angle);
  P[0][1] -= dt * P[1][1];
  P[1][0] -= dt * P[1][1];
  P[1][1] += Q_bias * dt;
  
  // Update
  float S = P[0][0] + R_measure;
  float K[2];
  K[0] = P[0][0] / S;
  K[1] = P[1][0] / S;
  
  float y = newAngle - *angle;
  *angle += K[0] * y;
  *bias += K[1] * y;
  
  float P00_temp = P[0][0];
  float P01_temp = P[0][1];
  
  P[0][0] -= K[0] * P00_temp;
  P[0][1] -= K[0] * P01_temp;
  P[1][0] -= K[1] * P00_temp;
  P[1][1] -= K[1] * P01_temp;
  
  return *angle;
}
