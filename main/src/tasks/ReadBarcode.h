/*********************************************************************
 * ReadBarcode - Binary Strip Detection Library
 * Detects and decodes 4-bit binary barcode using IR sensor
 *********************************************************************/

#ifndef READBARCODE_H
#define READBARCODE_H

#include <Arduino.h>

class ReadBarcode {
  private:
    // Configuration
    int irSensorChannel;          // IR sensor channel to use
    int irThreshold;              // Threshold for white/black detection
    unsigned long timeThreshold;  // Time threshold for binary 0/1 (ms)
    
    // State variables
    bool previousColor;           // false = black, true = white
    bool currentColor;
    int colorChangeCount;
    unsigned long whiteStartTime;
    bool onWhiteStrip;
    bool readingActive;
    bool readingComplete;
    
    // Results
    String binaryResult;          // Store the binary sequence
    int stripCount;               // Count white strips detected
    unsigned long stripDurations[4]; // Store duration of each strip
    
    // Helper functions
    bool readColorFromIR();
    
  public:
    // Constructor
    ReadBarcode();
    
    // Initialize with custom parameters
    void init(int sensorChannel = 6, int threshold = 2684, unsigned long timeThresh = 505);
    
    // Start barcode reading
    void startReading();
    
    // Process barcode reading (call in loop)
    void processReading();
    
    // Stop reading
    void stopReading();
    
    // Reset for new reading
    void reset();
    
    // Check if reading is complete
    bool isComplete();
    
    // Check if currently reading
    bool isActive();
    
    // Get results
    String getBinaryResult();
    int getStripCount();
    unsigned long getStripDuration(int index);
    int getColorChangeCount();
    
    // Get decimal value from binary
    int getDecimalValue();
    
    // Configuration setters
    void setIRSensorChannel(int channel);
    void setIRThreshold(int threshold);
    void setTimeThreshold(unsigned long threshold);
    
    // Configuration getters
    int getIRSensorChannel();
    int getIRThreshold();
    unsigned long getTimeThreshold();
    
    // Print results to serial
    void printResults();
};

// Global barcode reader object
extern ReadBarcode barcodeReader;

#endif
