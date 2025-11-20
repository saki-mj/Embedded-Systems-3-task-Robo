/*********************************************************************
 * Push Button Library Implementation
 *********************************************************************/

#include "PushButton.h"

PushButton pushButton;

PushButton::PushButton() {
  lastAnalogValue = 0;
  lastButton = BTN_NONE;
  currentButton = BTN_NONE;
  lastDebounceTime = 0;
  lastPressTime = 0;
  buttonPressed = false;
  continuousReadingActive = false;
}

void PushButton::begin() {
  pinMode(BUTTON_PIN, INPUT);
  
  // Set ADC resolution to 12-bit (0-4095 range)
  analogReadResolution(12);
  
  // Set ADC attenuation for 0V to 3.3V range
  analogSetAttenuation(ADC_11db);
  
  Serial.println("Push Button initialized on GPIO 19");
}

Button PushButton::detectButton(int analogValue) {
  if (analogValue >= BUTTON_UP_MIN && analogValue <= BUTTON_UP_MAX) {
    return BTN_UP;
  }
  else if (analogValue >= BUTTON_LEFT_MIN && analogValue <= BUTTON_LEFT_MAX) {
    return BTN_LEFT;
  }
  else if (analogValue >= BUTTON_MIDDLE_MIN && analogValue <= BUTTON_MIDDLE_MAX) {
    return BTN_MIDDLE;
  }
  else if (analogValue >= BUTTON_RIGHT_MIN && analogValue <= BUTTON_RIGHT_MAX) {
    return BTN_RIGHT;
  }
  else if (analogValue >= BUTTON_DOWN_MIN && analogValue <= BUTTON_DOWN_MAX) {
    return BTN_DOWN;
  }
  else {
    return BTN_NONE;
  }
}

void PushButton::update() {
  int analogValue = analogRead(BUTTON_PIN);
  lastAnalogValue = analogValue;
  
  Button detected = detectButton(analogValue);
  
  // Debouncing logic
  if (detected != lastButton) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {
    // Button state has been stable for debounce time
    if (detected != currentButton) {
      currentButton = detected;
      if (currentButton != BTN_NONE) {
        buttonPressed = true;
        lastPressTime = millis();
      }
    }
  }
  
  lastButton = detected;
}

Button PushButton::getCurrentButton() {
  return currentButton;
}

bool PushButton::isPressed(Button btn) {
  return (currentButton == btn && btn != BTN_NONE);
}

bool PushButton::wasPressed(Button btn) {
  if (buttonPressed && currentButton == btn) {
    buttonPressed = false;  // Reset flag
    return true;
  }
  return false;
}

int PushButton::getRawValue() {
  return lastAnalogValue;
}

String PushButton::getButtonName(Button btn) {
  switch(btn) {
    case BTN_UP: return "UP";
    case BTN_LEFT: return "LEFT";
    case BTN_MIDDLE: return "MIDDLE";
    case BTN_RIGHT: return "RIGHT";
    case BTN_DOWN: return "DOWN";
    case BTN_NONE: return "NONE";
    default: return "UNKNOWN";
  }
}

void PushButton::printState() {
  Serial.print("Analog: ");
  Serial.print(lastAnalogValue);
  Serial.print(" | Button: ");
  Serial.println(getButtonName(currentButton));
}

// Toggle continuous reading mode
void PushButton::toggleContinuousReading() {
  continuousReadingActive = !continuousReadingActive;
  if (continuousReadingActive) {
    Serial.println("Button Continuous Reading: ON");
    Serial.println("Format: Analog Value | Detected Button");
  } else {
    Serial.println("Button Continuous Reading: OFF");
  }
}

bool PushButton::isContinuousReadingActive() {
  return continuousReadingActive;
}

// Global initialization function
void initPushButton() {
  pushButton.begin();
}
