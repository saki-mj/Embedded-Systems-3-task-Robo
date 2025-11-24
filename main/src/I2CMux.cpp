#include "I2CMux.h"

// Constructor
I2CMux::I2CMux(uint8_t address, uint8_t pinSDA, uint8_t pinSCL) {
  muxAddress = address;
  sdaPin = pinSDA;
  sclPin = pinSCL;
  currentChannel = 255; // Invalid channel initially
}

// Initialize the multiplexer
void I2CMux::begin() {
  // Initialize I2C bus with ESP32-S3 specific pins
  Wire.begin(sdaPin, sclPin);
  delay(100);
  
  Serial.print("I2C initialized - SDA:");
  Serial.print(sdaPin);
  Serial.print(" SCL:");
  Serial.println(sclPin);
  
  // Check if PCA9548A is connected
  if (isConnected()) {
    Serial.print("PCA9548A found at address 0x");
    Serial.println(muxAddress, HEX);
  } else {
    Serial.print("ERROR: PCA9548A not found at address 0x");
    Serial.println(muxAddress, HEX);
  }
  
  // Start with all channels disabled
  disableAll();
  delay(50);
}

// Select a specific channel (0-7)
void I2CMux::selectChannel(uint8_t channel) {
  if (channel > 7) {
    return; // Invalid channel
  }
  
  // Write to PCA9548A to select channel
  Wire.beginTransmission(muxAddress);
  Wire.write(1 << channel); // Set the bit for the desired channel
  Wire.endTransmission();
  
  currentChannel = channel;
  delay(10); // Small delay for channel to stabilize
}

// Disable all channels
void I2CMux::disableAll() {
  Wire.beginTransmission(muxAddress);
  Wire.write(0); // All channels off
  Wire.endTransmission();
  currentChannel = 255;
  delay(10);
}

// Get current channel
uint8_t I2CMux::getCurrentChannel() {
  return currentChannel;
}

// Check if PCA9548A is responding
bool I2CMux::isConnected() {
  Wire.beginTransmission(muxAddress);
  return (Wire.endTransmission() == 0);
}
