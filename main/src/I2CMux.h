#ifndef I2CMUX_H
#define I2CMUX_H

#include <Arduino.h>
#include <Wire.h>

// PCA9548A I2C Multiplexer Address
#define PCA9548A_ADDRESS 0x70

// ESP32-S3 I2C Pins
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
#define I2C_FREQ 100000  // 100kHz standard mode (can use 400000 for fast mode)

// I2C Multiplexer Channels
#define MUX_CHANNEL_0 0  // SCL0/SDA0 - Left TOF
#define MUX_CHANNEL_1 1  // SCL1/SDA1 - Color Sensor Top
#define MUX_CHANNEL_3 3  // SCL3/SDA3 - Right TOF
#define MUX_CHANNEL_4 4  // SCL4/SDA4 - Front TOF
#define MUX_CHANNEL_5 5  // SCL5/SDA5 - Bottom Color Sensor

class I2CMux {
  private:
    uint8_t muxAddress;
    uint8_t sdaPin;
    uint8_t sclPin;
    uint32_t i2cFrequency;
    uint8_t currentChannel;

  public:
    // Constructor
    I2CMux(uint8_t address = PCA9548A_ADDRESS, uint8_t pinSDA = I2C_SDA_PIN, uint8_t pinSCL = I2C_SCL_PIN, uint32_t frequency = I2C_FREQ);
    
    // Initialize the multiplexer
    void begin();
    
    // Select a specific channel (0-7)
    void selectChannel(uint8_t channel);
    
    // Enable multiple channels at once (use with caution!)
    void selectMultipleChannels(uint8_t channelMask);
    
    // Read which channels are currently enabled
    uint8_t readEnabledChannels();
    
    // Disable all channels
    void disableAll();
    
    // Get current channel
    uint8_t getCurrentChannel();
    
    // Check if PCA9548A is responding
    bool isConnected();
};

#endif
