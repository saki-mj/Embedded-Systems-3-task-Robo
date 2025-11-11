// -------------------------------------------------------------------------
// IRReading.cpp - CD74HC4067 MUX IR Sensor Array Library Implementation
// -------------------------------------------------------------------------

#include "IRReading.h"

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------

int irValues[NUM_IR_SENSORS] = {0};       // Array to store IR sensor readings
int irThresholds[NUM_IR_SENSORS] = {0};   // Array to store calibrated thresholds
int irBinary[NUM_IR_SENSORS] = {0};       // Array to store binary values (0 or 1)
bool irReadingActive = false;             // Flag for continuous IR reading mode
bool irCalibrated = false;                // Flag indicating if calibration is complete

// Array to hold the select pin numbers
const int selectPins[4] = {S0, S1, S2, S3};

// -------------------------------------------------------------------------
// IR Sensor Initialization
// -------------------------------------------------------------------------

void initIRSensors() {
  // Set the MUX select pins as OUTPUTs
  for (int i = 0; i < 4; i++) {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], LOW); // Initialize to channel 0
  }
  
  // Set the MUX SIG pin as an INPUT
  pinMode(SIG_PIN, INPUT);
  
  // Load preset threshold values
  irThresholds[0] = 2425;
  irThresholds[1] = 2511;
  irThresholds[2] = 2762;
  irThresholds[3] = 2921;
  irThresholds[4] = 3180;
  irThresholds[5] = 3232;
  irThresholds[6] = 3067;
  irThresholds[7] = 3024;
  irThresholds[8] = 3067;
  irThresholds[9] = 3259;
  irThresholds[10] = 3117;
  irThresholds[11] = 2881;
  irThresholds[12] = 2735;
  irThresholds[13] = 2777;
  irThresholds[14] = 2631;
  irThresholds[15] = 2471;
  
  irCalibrated = true;  // Mark as calibrated with preset values
  
  Serial.println("CD74HC4067 MUX IR Sensors Initialized (16 Channels)");
  Serial.println("Preset threshold values loaded.");
}

// -------------------------------------------------------------------------
// MUX Channel Selection
// -------------------------------------------------------------------------

void setMuxChannel(int channel) {
  if (channel < 0 || channel > 15) {
    return;
  }
  
  // Set the four select pins (S0-S3) based on the binary representation of the channel number
  for (int i = 0; i < 4; i++) {
    digitalWrite(selectPins[i], bitRead(channel, i));
  }
}

// -------------------------------------------------------------------------
// IR Sensor Reading Functions
// -------------------------------------------------------------------------

int readIRSensor(int channel) {
  if (channel < 0 || channel >= NUM_IR_SENSORS) {
    return 0;
  }
  
  setMuxChannel(channel);
  
  // Tiny delay for the MUX to settle and the signal to stabilize
  delayMicroseconds(MUX_SETTLE_TIME);
  
  // Read the analog value from the common SIG pin
  return analogRead(SIG_PIN);
}

void readAllIRSensors() {
  // Loop through all 16 channels (0 to 15)
  for (int channel = 0; channel < NUM_IR_SENSORS; channel++) {
    irValues[channel] = readIRSensor(channel);
  }
}

void readAllIRSensorsBinary() {
  // First read all raw values
  readAllIRSensors();
  
  // Convert to binary based on thresholds
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    // If value is above threshold, it's white (1), otherwise black (0)
    irBinary[i] = (irValues[i] > irThresholds[i]) ? 1 : 0;
  }
}

void calibrateIRSensors() {
  int minValues[NUM_IR_SENSORS];
  int maxValues[NUM_IR_SENSORS];
  
  // Initialize min and max arrays
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    minValues[i] = 4095;  // Start with max possible value (12-bit ADC)
    maxValues[i] = 0;     // Start with min possible value
  }
  
  Serial.println("\n========================================");
  Serial.println("IR SENSOR CALIBRATION");
  Serial.println("========================================");
  Serial.println("Starting 10-second calibration...");
  Serial.println("Move sensors over BLACK and WHITE surfaces!");
  Serial.println();
  
  unsigned long startTime = millis();
  unsigned long lastSecond = 0;
  int secondsElapsed = 0;
  
  // Calibrate for 10 seconds
  while (millis() - startTime < 10000) {
    // Read all sensors
    readAllIRSensors();
    
    // Update min and max for each sensor
    for (int i = 0; i < NUM_IR_SENSORS; i++) {
      if (irValues[i] < minValues[i]) {
        minValues[i] = irValues[i];
      }
      if (irValues[i] > maxValues[i]) {
        maxValues[i] = irValues[i];
      }
    }
    
    // Print progress every second
    unsigned long elapsed = millis() - startTime;
    int currentSecond = elapsed / 1000;
    if (currentSecond > secondsElapsed) {
      secondsElapsed = currentSecond;
      Serial.print("Calibrating... ");
      Serial.print(secondsElapsed);
      Serial.println("s");
    }
    
    delay(10);  // Small delay for stability
  }
  
  Serial.println("Calibration complete!");
  Serial.println();
  
  // Calculate thresholds as midpoint between min and max
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    irThresholds[i] = (minValues[i] + maxValues[i]) / 2;
  }
  
  irCalibrated = true;
  
  // Print calibration results with min, max, and threshold
  printIRThresholds(minValues, maxValues);
  
  Serial.println("Calibration data stored.");
  Serial.println("========================================\n");
}

void printIRValues() {
  // Read all sensors first
  readAllIRSensors();
  
  // Print all values on one line with separators
  for (int channel = 0; channel < NUM_IR_SENSORS; channel++) {
    Serial.print(irValues[channel]);
    
    // Print a separator, but not after the last sensor
    if (channel < NUM_IR_SENSORS - 1) {
      Serial.print(" | ");
    }
  }
  
  // Move to the next line after all sensors have been printed
  Serial.println();
}

void printIRBinary() {
  // Read all sensors and convert to binary
  readAllIRSensorsBinary();
  
  // Print all binary values on one line with separators
  for (int channel = 0; channel < NUM_IR_SENSORS; channel++) {
    Serial.print(irBinary[channel]);
    
    // Print a separator, but not after the last sensor
    if (channel < NUM_IR_SENSORS - 1) {
      Serial.print(" | ");
    }
  }
  
  // Move to the next line after all sensors have been printed
  Serial.println();
}

void printIRThresholds(int minVals[], int maxVals[]) {
  Serial.println("Calibrated Thresholds:");
  Serial.println("Sensor |  Min  |  Max  | Threshold");
  Serial.println("-------|-------|-------|----------");
  
  for (int i = 0; i < NUM_IR_SENSORS; i++) {
    Serial.print("  ");
    if (i < 10) Serial.print(" ");  // Alignment for single digits
    Serial.print(i);
    Serial.print("   | ");
    Serial.print(minVals[i]);
    if (minVals[i] < 1000) Serial.print(" ");
    if (minVals[i] < 100) Serial.print(" ");
    if (minVals[i] < 10) Serial.print(" ");
    Serial.print(" | ");
    Serial.print(maxVals[i]);
    if (maxVals[i] < 1000) Serial.print(" ");
    if (maxVals[i] < 100) Serial.print(" ");
    if (maxVals[i] < 10) Serial.print(" ");
    Serial.print(" | ");
    Serial.println(irThresholds[i]);
  }
  Serial.println();
}

// -------------------------------------------------------------------------
// IR Reading Mode Control
// -------------------------------------------------------------------------

void toggleIRReading() {
  irReadingActive = !irReadingActive;
  
  if (irReadingActive) {
    Serial.println("IR Continuous Reading: ENABLED");
    if (irCalibrated) {
      Serial.println("Format: Binary values (0=Black, 1=White)");
    } else {
      Serial.println("Format: Raw values (Calibration recommended!)");
    }
  } else {
    Serial.println("IR Continuous Reading: DISABLED");
  }
}

bool isIRReadingActive() {
  return irReadingActive;
}
