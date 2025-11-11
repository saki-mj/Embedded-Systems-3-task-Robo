// -------------------------------------------------------------------------
// IRReading.h - CD74HC4067 MUX IR Sensor Array Library for ESP32-S3
// -------------------------------------------------------------------------

#ifndef IR_READING_H
#define IR_READING_H

#include <Arduino.h>

// -------------------------------------------------------------------------
// MUX Pin Connections
// -------------------------------------------------------------------------

// Define the ESP32-S3 GPIO pins connected to the MUX
const int SIG_PIN = 1;  // Connects to the MUX's SIG (Common) pin - Analog Capable Pin
const int S0 = 40;      // MUX Select Pin S0
const int S1 = 41;      // MUX Select Pin S1
const int S2 = 42;      // MUX Select Pin S2
const int S3 = 2;       // MUX Select Pin S3

// -------------------------------------------------------------------------
// MUX Configuration
// -------------------------------------------------------------------------

const int NUM_IR_SENSORS = 16;  // Number of IR sensors (MUX channels)
const int MUX_SETTLE_TIME = 5;  // Microseconds for MUX to settle

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------

extern int irValues[NUM_IR_SENSORS];       // Array to store IR sensor readings
extern int irThresholds[NUM_IR_SENSORS];   // Array to store calibrated thresholds
extern int irBinary[NUM_IR_SENSORS];       // Array to store binary values (0 or 1)
extern bool irReadingActive;               // Flag for continuous IR reading mode
extern bool irCalibrated;                  // Flag indicating if calibration is complete

// -------------------------------------------------------------------------
// IR Sensor Functions
// -------------------------------------------------------------------------

/**
 * @brief Initialize IR sensor MUX pins
 */
void initIRSensors();

/**
 * @brief Set the MUX channel
 * @param channel Channel number (0-15)
 */
void setMuxChannel(int channel);

/**
 * @brief Read a specific IR sensor channel
 * @param channel Channel number (0-15)
 * @return Analog value from the IR sensor
 */
int readIRSensor(int channel);

/**
 * @brief Read all IR sensors and store in irValues array
 */
void readAllIRSensors();

/**
 * @brief Read all IR sensors and convert to binary based on thresholds
 */
void readAllIRSensorsBinary();

/**
 * @brief Calibrate IR sensors by reading min/max over 5 seconds
 * Manually move sensors over black and white surfaces during this time
 */
void calibrateIRSensors();

/**
 * @brief Print all IR sensor values to Serial in a formatted line
 */
void printIRValues();

/**
 * @brief Print binary IR sensor values (0 or 1) based on thresholds
 */
void printIRBinary();

/**
 * @brief Print calibration thresholds for all sensors
 * @param minVals Array of minimum values for each sensor
 * @param maxVals Array of maximum values for each sensor
 */
void printIRThresholds(int minVals[], int maxVals[]);

/**
 * @brief Toggle continuous IR reading mode on/off
 */
void toggleIRReading();

/**
 * @brief Get the current state of IR reading mode
 * @return true if IR reading is active, false otherwise
 */
bool isIRReadingActive();

#endif // IR_READING_H
