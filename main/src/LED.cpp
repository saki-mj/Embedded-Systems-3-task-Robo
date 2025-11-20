/*********************************************************************
 * LED Library Implementation
 *********************************************************************/

#include "LED.h"

// Global LED object
LED led;

// Constructor
LED::LED() {
  led1State = false;
  led2State = false;
}

// Initialize LED pins
void LED::begin() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  
  // Turn off both LEDs initially
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  
  Serial.println("External LEDs initialized:");
  Serial.print("  LED1: GPIO ");
  Serial.println(LED1_PIN);
  Serial.print("  LED2: GPIO ");
  Serial.println(LED2_PIN);
}

// LED1 Control - Simple digital HIGH/LOW
void LED::led1On() {
  digitalWrite(LED1_PIN, HIGH);
  led1State = true;
}

void LED::led1Off() {
  digitalWrite(LED1_PIN, LOW);
  led1State = false;
}

void LED::led1Toggle() {
  led1State = !led1State;
  digitalWrite(LED1_PIN, led1State);
}

bool LED::getLed1State() {
  return led1State;
}

// LED2 Control - Simple digital HIGH/LOW
void LED::led2On() {
  digitalWrite(LED2_PIN, HIGH);
  led2State = true;
}

void LED::led2Off() {
  digitalWrite(LED2_PIN, LOW);
  led2State = false;
}

void LED::led2Toggle() {
  led2State = !led2State;
  digitalWrite(LED2_PIN, led2State);
}

bool LED::getLed2State() {
  return led2State;
}

// Control both LEDs
void LED::allOn() {
  led1On();
  led2On();
}

void LED::allOff() {
  led1Off();
  led2Off();
}

void LED::setLed1(bool state) {
  if (state) {
    led1On();
  } else {
    led1Off();
  }
}

void LED::setLed2(bool state) {
  if (state) {
    led2On();
  } else {
    led2Off();
  }
}

// Blink functions
void LED::led1Blink(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    led1On();
    delay(delayMs);
    led1Off();
    delay(delayMs);
  }
}

void LED::led2Blink(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    led2On();
    delay(delayMs);
    led2Off();
    delay(delayMs);
  }
}

void LED::allBlink(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    allOn();
    delay(delayMs);
    allOff();
    delay(delayMs);
  }
}

// Global initialization function
void initLED() {
  led.begin();
}
