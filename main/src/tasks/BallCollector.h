// -------------------------------------------------------------------------
// Ball Collector - Handles ball collection mechanism
// -------------------------------------------------------------------------

#ifndef BALLCOLLECTOR_H
#define BALLCOLLECTOR_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../ColorSensors.h"
// Servo pin definitions
#define ARM_SERVO_PIN 35
#define GRIPPER_SERVO_PIN 36
#define SORTING_SERVO_PIN 37

class BallCollector {
private:
  bool isCollecting;
  bool collectionComplete;
  unsigned long collectionStartTime;
  
  // Sensor thresholds (to be defined later based on hardware)
  int irThreshold;
  int tofThreshold;
  int colorThreshold;
  
  // Servo objects
  Servo armServo;
  Servo gripperServo;
  Servo sortingServo;
  
  // Servo position variables (to be calibrated)
  int armPos0;        // Arm home position
  int armPos1;        // Arm pickup position
  int gripperPos0;    // Gripper open
  int gripperPos1;    // Gripper closed
  int gripperInitialPos;  // Gripper initial position (before arm moves)
  int gripperDropPos;     // Gripper drop position (after arm returns)
  int sortingPos0;    // Sorting yellow position
  int sortingPos1;    // Sorting initial/home position
  int sortingPos2;    // Sorting white position
  
  // Timing delay variables (in milliseconds, to be calibrated)
  unsigned long servoMoveDelay;       // Delay after servo movement
  unsigned long colorDetectDelay;     // Delay before color detection
  unsigned long sortingDelay;         // Delay at sorting position
  unsigned long completionDelay;      // Delay after sorting before DONE

  // --- Added for Task5: last detected color ---
  DetectedColor lastDetectedColor;
  
public:
  BallCollector();
  
  // Initialize the ball collector
  void init();
  
  // Main ball collecting function
  // Returns true when collection is complete
  bool collectingBall();
  
  // Check if currently collecting
  bool isActive();
  
  // Reset collector state
  void reset();
  
  // Stop collection process
  void stop();
  
  // Get collection status
  bool isComplete();
  
  // Set sensor thresholds (for future calibration)
  void setIRThreshold(int threshold);
  void setTOFThreshold(int threshold);
  void setColorThreshold(int threshold);
  
  // Set servo positions (for calibration)
  void setArmPositions(int pos0, int pos1);
  void setGripperPositions(int pos0, int pos1, int initialPos, int dropPos);
  void setSortingPositions(int pos0, int pos1, int pos2);
  
  // Servo control methods
  void moveArmTo(int position);
  void moveGripperTo(int position);
  void moveSortingTo(int position);
  
  // Get current servo positions (for display)
  int getArmPos0() { return armPos0; }
  int getArmPos1() { return armPos1; }
  int getGripperPos0() { return gripperPos0; }
  int getGripperPos1() { return gripperPos1; }
  int getGripperInitialPos() { return gripperInitialPos; }
  int getGripperDropPos() { return gripperDropPos; }
  int getSortingPos0() { return sortingPos0; }
  int getSortingPos1() { return sortingPos1; }
  int getSortingPos2() { return sortingPos2; }
  
  // Set timing delays (in milliseconds)
  void setServoMoveDelay(unsigned long delay) { servoMoveDelay = delay; }
  void setColorDetectDelay(unsigned long delay) { colorDetectDelay = delay; }
  void setSortingDelay(unsigned long delay) { sortingDelay = delay; }
  void setCompletionDelay(unsigned long delay) { completionDelay = delay; }
  
  // Get timing delays (for display)
  unsigned long getServoMoveDelay() { return servoMoveDelay; }
  unsigned long getColorDetectDelay() { return colorDetectDelay; }
  unsigned long getSortingDelay() { return sortingDelay; }
  unsigned long getCompletionDelay() { return completionDelay; }
  
  // Individual servo test/configuration method
  void testServo(const String& servoName, int angle);

  // Get the last color detected during collection (useful for Task5 unloading)
  DetectedColor getLastDetectedColor() { return lastDetectedColor; }

};

// Global instance
extern BallCollector ballCollector;

#endif
