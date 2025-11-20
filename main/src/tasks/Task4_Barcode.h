/*********************************************************************
 * Task 4: Read Barcode
 * Description: Navigate walls and read 4-bar barcode using IR sensors
 *********************************************************************/

#ifndef TASK4_BARCODE_H
#define TASK4_BARCODE_H

#include <Arduino.h>

enum Task4SubState {
  T4_INIT,
  T4_SEARCHING_WALL1,      // Following left wall to detect first wall
  T4_TURNING_RIGHT1,       // Turn right 90° after first wall
  T4_SEARCHING_WALL2,      // Following left wall to detect second wall
  T4_TURNING_LEFT2,        // Turn left 90° after second wall
  T4_MOVING_REVERSE,       // Move reverse for configured time
  T4_ALIGNING,             // Align with barcode
  T4_READING,              // Reading barcode while crossing
  T4_PROCESSING,           // Process barcode data
  T4_COMPLETED
};

class Task4Barcode {
  private:
    Task4SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    String barcodeData;
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t wallDetectionDistance;  // Distance to detect wall (mm)
    uint16_t wallFollowDistance;     // Target distance for wall following (mm)
    unsigned long turnRightDuration;  // Time for right turn 90° (ms)
    unsigned long turnLeftDuration;   // Time for left turn 90° (ms)
    unsigned long straightDuration;   // Time for straight movement (ms)
    unsigned long reverseDuration;    // Time for reverse movement (ms)
    uint16_t irWhiteThreshold;        // IR threshold: above=white(1), below=black(0)
    
    // Barcode reading variables
    int barcodeBits[4];           // Barcode bits read by sensors 6,7,8,9
    int barcodeCounts[16];        // Count occurrences of each barcode value (0-15)
    bool barcodeReadComplete;
    
    // Timing tracking
    unsigned long turnStartTime;
    unsigned long straightStartTime;
    unsigned long reverseStartTime;
    
    // Helper functions
    void detectWallAndTurn();
    void performTurn90();
    void readBarcodeBar();
    void processBarcodeData();
    bool isTurnComplete();
    
  public:
    Task4Barcode();
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task4SubState newSubState);
    Task4SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    String getBarcodeData();
    
    // Configuration setters
    void setWallDetectionDistance(uint16_t distance);
    void setWallFollowDistance(uint16_t distance);
    void setTurnRightDuration(unsigned long timeMs);
    void setTurnLeftDuration(unsigned long timeMs);
    void setStraightDuration(unsigned long timeMs);
    void setReverseDuration(unsigned long timeMs);
    void setIRWhiteThreshold(uint16_t threshold);
    
    // Getters for current values
    uint16_t getWallDetectionDistance();
    uint16_t getWallFollowDistance();
    unsigned long getTurnRightDuration();
    unsigned long getTurnLeftDuration();
    unsigned long getStraightDuration();
    unsigned long getReverseDuration();
    uint16_t getIRWhiteThreshold();
};

extern Task4Barcode task4Barcode;

#endif
/*********************************************************************
 * Task 4: Read Barcode
 * Description: Navigate walls and read 4-bar barcode using IR sensors
 *********************************************************************/