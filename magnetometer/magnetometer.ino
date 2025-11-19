#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

// calibration offset variables
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
float gyroOffsetX  = 0, gyroOffsetY  = 0, gyroOffsetZ  = 0;

const int16_t ACCEL_SENSITIVITY = 16384; // for ±2 g
const int16_t GYRO_SENSITIVITY  = 131;   // for ±250 deg/s

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL
  delay(100);

  Serial.println("Initializing MPU6050...");
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed!");
    while (1) { delay(1000); }
  }
  Serial.println("MPU6050 connected.");

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
}

void loop() {
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

  Serial.print("Pitch: "); Serial.print(pitch, 2);
  Serial.print("   Roll: "); Serial.print(roll, 2);
  Serial.print("   GyroZ: "); Serial.print(GyroZ, 2);
  Serial.println();

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
