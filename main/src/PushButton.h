/*********************************************************************
 * Push Button Library - Multi-button Analog Switch
 * 
 * Five buttons connected to single analog pin (GPIO 19)
 * Uses voltage divider to detect which button is pressed
 * 
 * Buttons: UP, LEFT, MIDDLE, RIGHT, DOWN
 *********************************************************************/

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include <Arduino.h>

// Pin configuration
#define BUTTON_PIN 19

// Button detection ranges (analog values with ±100 tolerance)
#define BUTTON_UP_MIN 0
#define BUTTON_UP_MAX 183      // 83 ± 100

#define BUTTON_LEFT_MIN 1128
#define BUTTON_LEFT_MAX 1328   // 1228 ± 100

#define BUTTON_MIDDLE_MIN 1750
#define BUTTON_MIDDLE_MAX 1850

#define BUTTON_RIGHT_MIN 3747
#define BUTTON_RIGHT_MAX 3947  // 3847 ± 100

#define BUTTON_DOWN_MIN 3750
#define BUTTON_DOWN_MAX 4020

// Debounce time in milliseconds
#define DEBOUNCE_TIME 50

// Button enum
enum Button {
  BTN_NONE,
  BTN_UP,
  BTN_LEFT,
  BTN_MIDDLE,
  BTN_RIGHT,
  BTN_DOWN
};

class PushButton {
  private:
    int lastAnalogValue;
    Button lastButton;
    Button currentButton;
    unsigned long lastDebounceTime;
    unsigned long lastPressTime;
    bool buttonPressed;
    bool continuousReadingActive;
    
    // Detect which button is pressed based on analog value
    Button detectButton(int analogValue);
    
  public:
    PushButton();
    
    // Initialize button (sets ADC resolution and attenuation)
    void begin();
    
    // Read current button state
    void update();
    
    // Get current button (debounced)
    Button getCurrentButton();
    
    // Check if specific button is pressed
    bool isPressed(Button btn);
    
    // Check if button was just pressed (edge detection)
    bool wasPressed(Button btn);
    
    // Get raw analog value
    int getRawValue();
    
    // Get button name as string
    String getButtonName(Button btn);
    
    // Print current button state
    void printState();
    
    // Toggle continuous reading mode
    void toggleContinuousReading();
    bool isContinuousReadingActive();
};

// Global instance
extern PushButton pushButton;

// Initialization function
void initPushButton();

#endif
