/*********************************************************************
 * LED Library - External LED Control
 * 
 * Controls external LEDs connected to GPIO pins
 * LED1: GPIO 3
 * LED2: GPIO 17
 *********************************************************************/

#ifndef LED_H
#define LED_H

#include <Arduino.h>

// LED Pin Definitions
#define LED1_PIN 3   // First external LED
#define LED2_PIN 17  // Second external LED

class LED {
  private:
    bool led1State;
    bool led2State;
    
  public:
    // Constructor
    LED();
    
    // Initialize LED pins
    void begin();
    
    // LED1 Control
    void led1On();
    void led1Off();
    void led1Toggle();
    bool getLed1State();
    
    // LED2 Control
    void led2On();
    void led2Off();
    void led2Toggle();
    bool getLed2State();
    
    // Control both LEDs
    void allOn();
    void allOff();
    void setLed1(bool state);
    void setLed2(bool state);
    
    // Blink functions
    void led1Blink(int times, int delayMs = 200);
    void led2Blink(int times, int delayMs = 200);
    void allBlink(int times, int delayMs = 200);
};

// Global LED object
extern LED led;

// Initialization function
void initLED();

#endif
