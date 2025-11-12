#include "Adafruit_VL53L0X.h"
#include "I2CMux.h"
#include <Wire.h>

// Create PCA9548A I2C Multiplexer object
// Default address 0x70, ESP32-S3 I2C pins: SDA=8, SCL=9
I2CMux mux;

// Create TOF sensor objects for each sensor
Adafruit_VL53L0X loxLeft = Adafruit_VL53L0X();
Adafruit_VL53L0X loxFront = Adafruit_VL53L0X();
Adafruit_VL53L0X loxRight = Adafruit_VL53L0X();

void scanI2C() {
  Serial.println("Scanning I2C bus...");
  byte error, address;
  int nDevices = 0;
  
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      nDevices++;
    }
  }
  
  if (nDevices == 0)
    Serial.println("No I2C devices found!");
  else
    Serial.println("I2C scan complete");
}

void setup() {
  Serial.begin(115200)
  
  // Wait for serial port to connect
  while (!Serial) {
    delay(1);
  }
  
  Serial.println("=== VL53L0X with PCA9548A I2C Mux - ESP32-S3 ===");
  Serial.println("I2C on SDA=8, SCL=9");
  Serial.println("3 TOF Sensors: Channel 0 (Left), 1 (Front), 3 (Right)");
  
  // Small delay for system to stabilize
  delay(100);
  
  // Initialize the PCA9548A I2C multiplexer
  mux.begin();
  
  // Scan I2C bus
  Serial.println("\nScanning I2C bus:");
  scanI2C();
  
  // Initialize Left TOF on Channel 0
  Serial.println("\n=== Initializing Left TOF (Channel 0) ===");
  mux.selectChannel(MUX_CHANNEL_0);
  delay(50);
  if (!loxLeft.begin()) {
    Serial.println("Failed to boot Left VL53L0X on Channel 0");
    while(1);
  }
  Serial.println("Left TOF ready!");
  
  // Initialize Front TOF on Channel 1
  Serial.println("\n=== Initializing Front TOF (Channel 1) ===");
  mux.selectChannel(MUX_CHANNEL_1);
  delay(50);
  if (!loxFront.begin()) {
    Serial.println("Failed to boot Front VL53L0X on Channel 1");
    while(1);
  }
  Serial.println("Front TOF ready!");
  
  // Initialize Right TOF on Channel 3
  Serial.println("\n=== Initializing Right TOF (Channel 3) ===");
  mux.selectChannel(MUX_CHANNEL_3);
  delay(50);
  if (!loxRight.begin()) {
    Serial.println("Failed to boot Right VL53L0X on Channel 3");
    while(1);
  }
  Serial.println("Right TOF ready!");
  
  Serial.println("\n=== All TOF sensors initialized successfully! ===\n");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measure;
  
  // Read Left TOF (Channel 0)
  mux.selectChannel(MUX_CHANNEL_0);
  loxLeft.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    Serial.print("Left: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.print(" mm | ");
  } else {
    Serial.print("Left: Out of range | ");
  }
  
  // Read Front TOF (Channel 1)
  mux.selectChannel(MUX_CHANNEL_1);
  loxFront.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    Serial.print("Front: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.print(" mm | ");
  } else {
    Serial.print("Front: Out of range | ");
  }
  
  // Read Right TOF (Channel 3)
  mux.selectChannel(MUX_CHANNEL_3);
  loxRight.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    Serial.print("Right: ");
    Serial.print(measure.RangeMilliMeter);
    Serial.println(" mm");
  } else {
    Serial.println("Right: Out of range");
  }
  
  delay(100);
}
