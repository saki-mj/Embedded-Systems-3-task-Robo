/*********************************************************************
 * ReadBarcode - Binary Strip Detection Implementation
 *********************************************************************/

#include "ReadBarcode.h"
#include "../IRReading.h"

// Global barcode reader object
ReadBarcode barcodeReader;

// Constructor
ReadBarcode::ReadBarcode() {
  irSensorChannel = 6;        // Default to sensor 6 (7th sensor, 0-indexed)
  irThreshold = 2684;         // Default threshold
  timeThreshold = 505;        // Default time threshold (505ms)
  
  previousColor = false;
  currentColor = false;
  colorChangeCount = 0;
  whiteStartTime = 0;
  onWhiteStrip = false;
  readingActive = false;
  readingComplete = false;
  
  binaryResult = "";
  stripCount = 0;
  
  for (int i = 0; i < 4; i++) {
    stripDurations[i] = 0;
  }
}

// Initialize with custom parameters
void ReadBarcode::init(int sensorChannel, int threshold, unsigned long timeThresh) {
  irSensorChannel = sensorChannel;
  irThreshold = threshold;
  timeThreshold = timeThresh;
  reset();
  
  Serial.println("=== ReadBarcode Initialized ===");
  Serial.print("IR Sensor Channel: ");
  Serial.println(irSensorChannel);
  Serial.print("IR Threshold: ");
  Serial.println(irThreshold);
  Serial.print("Time Threshold: ");
  Serial.print(timeThreshold);
  Serial.println(" ms");
}

// Start barcode reading
void ReadBarcode::startReading() {
  if (readingActive) {
    Serial.println("Barcode reading already active!");
    return;
  }
  
  reset();
  
  // Initialize color state
  currentColor = readColorFromIR();
  previousColor = currentColor;
  
  readingActive = true;
  readingComplete = false;
  
  Serial.println("\n=== Barcode Reading STARTED ===");
  Serial.println("Detecting white strips...");
}

// Process barcode reading (call in loop while moving forward)
void ReadBarcode::processReading() {
  if (!readingActive || readingComplete) {
    return;
  }
  
  // Read current color from IR sensor
  currentColor = readColorFromIR();
  
  // Detect color change
  if (currentColor != previousColor) {
    colorChangeCount++;
    
    // Black to White transition
    if (currentColor == true && previousColor == false) {
      whiteStartTime = millis();
      onWhiteStrip = true;
      Serial.print("Strip detected (");
      Serial.print(stripCount + 1);
      Serial.println(") - Timer started");
    }
    
    // White to Black transition
    if (currentColor == false && previousColor == true && onWhiteStrip) {
      unsigned long whiteStripDuration = millis() - whiteStartTime;
      onWhiteStrip = false;
      
      // Store duration
      if (stripCount < 4) {
        stripDurations[stripCount] = whiteStripDuration;
      }
      stripCount++;
      
      // Determine binary value based on duration
      if (whiteStripDuration < timeThreshold) {
        binaryResult += "0";
        Serial.print("Binary 0 detected (Duration: ");
        Serial.print(whiteStripDuration);
        Serial.println(" ms)");
      } else {
        binaryResult += "1";
        Serial.print("Binary 1 detected (Duration: ");
        Serial.print(whiteStripDuration);
        Serial.println(" ms)");
      }
      
      Serial.print("Current binary sequence: ");
      Serial.println(binaryResult);
      Serial.print("Color changes: ");
      Serial.println(colorChangeCount);
      Serial.println();
    }
    
    previousColor = currentColor;
  }
  
  // Check if reading is complete (8 color changes = 4 strips)
  if (colorChangeCount >= 8) {
    readingComplete = true;
    readingActive = false;
    Serial.println("\n=== Barcode Reading COMPLETE ===");
    printResults();
  }
}

// Stop reading
void ReadBarcode::stopReading() {
  readingActive = false;
  Serial.println("Barcode reading stopped.");
}

// Reset for new reading
void ReadBarcode::reset() {
  previousColor = false;
  currentColor = false;
  colorChangeCount = 0;
  whiteStartTime = 0;
  onWhiteStrip = false;
  readingActive = false;
  readingComplete = false;
  binaryResult = "";
  stripCount = 0;
  
  for (int i = 0; i < 4; i++) {
    stripDurations[i] = 0;
  }
}

// Check if reading is complete
bool ReadBarcode::isComplete() {
  return readingComplete;
}

// Check if currently reading
bool ReadBarcode::isActive() {
  return readingActive;
}

// Get binary result as string
String ReadBarcode::getBinaryResult() {
  return binaryResult;
}

// Get number of strips detected
int ReadBarcode::getStripCount() {
  return stripCount;
}

// Get duration of specific strip
unsigned long ReadBarcode::getStripDuration(int index) {
  if (index >= 0 && index < 4) {
    return stripDurations[index];
  }
  return 0;
}

// Get color change count
int ReadBarcode::getColorChangeCount() {
  return colorChangeCount;
}

// Get decimal value from binary result
int ReadBarcode::getDecimalValue() {
  int decimalValue = 0;
  int length = binaryResult.length();
  
  for (int i = 0; i < length; i++) {
    if (binaryResult.charAt(i) == '1') {
      decimalValue |= (1 << (length - 1 - i));
    }
  }
  
  return decimalValue;
}

// Configuration setters
void ReadBarcode::setIRSensorChannel(int channel) {
  irSensorChannel = channel;
  Serial.print("Barcode IR sensor channel set to: ");
  Serial.println(channel);
}

void ReadBarcode::setIRThreshold(int threshold) {
  irThreshold = threshold;
  Serial.print("Barcode IR threshold set to: ");
  Serial.println(threshold);
}

void ReadBarcode::setTimeThreshold(unsigned long threshold) {
  timeThreshold = threshold;
  Serial.print("Barcode time threshold set to: ");
  Serial.print(threshold);
  Serial.println(" ms");
}

// Configuration getters
int ReadBarcode::getIRSensorChannel() {
  return irSensorChannel;
}

int ReadBarcode::getIRThreshold() {
  return irThreshold;
}

unsigned long ReadBarcode::getTimeThreshold() {
  return timeThreshold;
}

// Read color from IR sensor
bool ReadBarcode::readColorFromIR() {
  // Read IR sensor value
  int irValue = readIRSensor(irSensorChannel);
  
  // Return true if white (above threshold), false if black (below threshold)
  return (irValue > irThreshold);
}

// Print results to serial
void ReadBarcode::printResults() {
  Serial.println("\n========================================");
  Serial.println("BARCODE READING RESULTS");
  Serial.println("========================================");
  Serial.print("Total color changes: ");
  Serial.println(colorChangeCount);
  Serial.print("White strips detected: ");
  Serial.println(stripCount);
  Serial.println();
  
  // Print duration for each strip
  Serial.println("Strip Durations:");
  for (int i = 0; i < stripCount && i < 4; i++) {
    Serial.print("  Strip ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(stripDurations[i]);
    Serial.print(" ms");
    if (stripDurations[i] < timeThreshold) {
      Serial.println(" (Binary 0)");
    } else {
      Serial.println(" (Binary 1)");
    }
  }
  Serial.println();
  
  Serial.println("BINARY RESULT:");
  Serial.println(binaryResult);
  Serial.print("DECIMAL VALUE: ");
  Serial.println(getDecimalValue());
  Serial.println("========================================\n");
}
