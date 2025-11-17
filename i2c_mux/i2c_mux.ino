// PCA9548A I2C Multiplexer Sample Code for ESP32-S3
// Optimized for ESP32-S3 with custom I2C pins

#include <Wire.h>

// Define your I2C pins (change these to match your wiring)
#define I2C_SDA 8   // GPIO8 - Change to your SDA pin
#define I2C_SCL 9   // GPIO9 - Change to your SCL pin
#define I2C_FREQ 100000  // 100kHz standard mode (can use 400000 for fast mode)

#define PCA9548A_ADDRESS 0x70  // Default address (A0-A2 = GND or floating)

// Function to select a channel on the PCA9548A (0-7)
void selectChannel(uint8_t channel) {
  if (channel > 7) return;  // Invalid channel
  
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(1 << channel);  // Send control byte to enable the channel
  Wire.endTransmission();
}

// Function to disable all channels
void disableAllChannels() {
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(0);  // 0x00 disables all channels
  Wire.endTransmission();
}

// Function to enable multiple channels at once (use with caution!)
void selectMultipleChannels(uint8_t channelMask) {
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(channelMask);  // e.g., 0b00000011 enables channels 0 and 1
  Wire.endTransmission();
}

// Function to read which channels are currently enabled
uint8_t readEnabledChannels() {
  Wire.requestFrom(PCA9548A_ADDRESS, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give time for serial monitor to connect
  
  // Initialize I2C with custom pins for ESP32-S3
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
  
  Serial.println("\n=== PCA9548A I2C Multiplexer Test (ESP32-S3) ===");
  Serial.printf("I2C initialized: SDA=GPIO%d, SCL=GPIO%d\n", I2C_SDA, I2C_SCL);
  
  // Test: Disable all channels first
  disableAllChannels();
  Serial.println("All channels disabled\n");
}

void loop() {
  // Example: Scan each channel for I2C devices
  for (uint8_t channel = 0; channel < 8; channel++) {
    selectChannel(channel);
    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(": ");
    
    scanI2CDevices();
    delay(100);
  }
  
  disableAllChannels();
  Serial.println("\n--- Scan complete ---\n");
  delay(3000);
}

// Helper function to scan for I2C devices on the current channel
void scanI2CDevices() {
  uint8_t deviceCount = 0;
  
  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.print(" ");
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.print("No devices found");
  }
  Serial.println();
}

// Example: Reading from a specific sensor on a specific channel
void readSensorOnChannel(uint8_t channel, uint8_t sensorAddress) {
  selectChannel(channel);
  
  // Now communicate with your sensor
  Wire.beginTransmission(sensorAddress);
  // ... your sensor-specific code here ...
  Wire.endTransmission();
  
  // Don't forget to disable the channel when done (optional)
  // disableAllChannels();
}