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
bool irReadingActive = false;             // Flag for continuous raw IR reading mode
bool irReadingBinaryActive = false;       // Flag for continuous binary IR reading mode
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
  
  // Load preset threshold values (calibrated)
  irThresholds[0] = 2163;
  irThresholds[1] = 2279;
  irThresholds[2] = 2460;
  irThresholds[3] = 2460;
  irThresholds[4] = 2580;
  irThresholds[5] = 2668;
  irThresholds[6] = 2607;
  irThresholds[7] = 2567;
  irThresholds[8] = 2574;
  irThresholds[9] = 2696;
  irThresholds[10] = 2585;
  irThresholds[11] = 2464;
  irThresholds[12] = 2382;
  irThresholds[13] = 2423;
  irThresholds[14] = 2414;
  irThresholds[15] = 2249;
  
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
  
  // Turn off binary mode if raw mode is being enabled
  if (irReadingActive) {
    irReadingBinaryActive = false;
    Serial.println("IR Raw Reading: ENABLED");
    Serial.println("Format: Raw analog values (0-4095)");
  } else {
    Serial.println("IR Raw Reading: DISABLED");
  }
}

void toggleIRReadingBinary() {
  irReadingBinaryActive = !irReadingBinaryActive;
  
  // Turn off raw mode if binary mode is being enabled
  if (irReadingBinaryActive) {
    irReadingActive = false;
    Serial.println("IR Binary Reading: ENABLED");
    Serial.println("Format: Binary values (0=Black, 1=White)");
  } else {
    Serial.println("IR Binary Reading: DISABLED");
  }
}

bool isIRReadingActive() {
  return irReadingActive;
}

bool isIRReadingBinaryActive() {
  return irReadingBinaryActive;
}
