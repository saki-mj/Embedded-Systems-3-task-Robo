/*********************************************************************
 * Task 4: Read Barcode Implementation
 *********************************************************************/

#include "Task4_Barcode.h"
#include "../Motors.h"
#include "../IRReading.h"
#include "../OLEDDisplay.h"

Task4Barcode task4Barcode;

Task4Barcode::Task4Barcode() {
  currentSubState = T4_INIT;
  subStateStartTime = 0;
  taskActive = false;
  barcodeData = "";
}

void Task4Barcode::init() {
  Serial.println("=== Task 4: Barcode - Initializing ===");
  currentSubState = T4_INIT;
  subStateStartTime = millis();
  taskActive = false;
  barcodeData = "";
  oledDisplay.show("Task 4", "Barcode", "Initialized");
  delay(1000);
}

void Task4Barcode::execute() {
  if (!taskActive) return;
  
  switch(currentSubState) {
    case T4_INIT:
      Serial.println("Task 4: INIT state");
      // Add your code here
      break;
      
    case T4_SEARCHING:
      Serial.println("Task 4: SEARCHING state");
      // Add your code here
      break;
      
    case T4_ALIGNING:
      Serial.println("Task 4: ALIGNING state");
      // Add your code here
      break;
      
    case T4_READING:
      Serial.println("Task 4: READING state");
      // Add barcode reading logic here
      break;
      
    case T4_PROCESSING:
      Serial.println("Task 4: PROCESSING state");
      // Process barcode data
      break;
      
    case T4_COMPLETED:
      Serial.println("Task 4: COMPLETED");
      Serial.print("Barcode Data: ");
      Serial.println(barcodeData);
      taskActive = false;
      break;
  }
}

void Task4Barcode::updateDisplay() {
  if (!taskActive) return;
  oledDisplay.show(
    "Task 4: Barcode",
    "State: " + getSubStateName(),
    "Data: " + barcodeData
  );
}

void Task4Barcode::setSubState(Task4SubState newSubState) {
  if (currentSubState != newSubState) {
    currentSubState = newSubState;
    subStateStartTime = millis();
    Serial.print("Task 4 sub-state: ");
    Serial.println(getSubStateName());
    updateDisplay();
  }
}

Task4SubState Task4Barcode::getSubState() {
  return currentSubState;
}

String Task4Barcode::getSubStateName() {
  switch(currentSubState) {
    case T4_INIT: return "INIT";
    case T4_SEARCHING: return "SEARCHING";
    case T4_ALIGNING: return "ALIGNING";
    case T4_READING: return "READING";
    case T4_PROCESSING: return "PROCESSING";
    case T4_COMPLETED: return "COMPLETED";
    default: return "UNKNOWN";
  }
}

void Task4Barcode::start() {
  Serial.println("Starting Task 4: Barcode");
  taskActive = true;
  setSubState(T4_INIT);
}

void Task4Barcode::stop() {
  Serial.println("Stopping Task 4: Barcode");
  taskActive = false;
  stopAllMotors();
}

bool Task4Barcode::isActive() {
  return taskActive;
}

bool Task4Barcode::isCompleted() {
  return (currentSubState == T4_COMPLETED);
}

void Task4Barcode::reset() {
  currentSubState = T4_INIT;
  subStateStartTime = millis();
  taskActive = false;
  barcodeData = "";
}

String Task4Barcode::getBarcodeData() {
  return barcodeData;
}
