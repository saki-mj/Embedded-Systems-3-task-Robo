// -------------------------------------------------------------------------
// Ball Collector - Handles ball collection mechanism
// -------------------------------------------------------------------------

#ifndef BALLCOLLECTOR_H
#define BALLCOLLECTOR_H

#include <Arduino.h>

class BallCollector {
private:
  bool isCollecting;
  bool collectionComplete;
  unsigned long collectionStartTime;
  
  // Sensor thresholds (to be defined later based on hardware)
  int irThreshold;
  int tofThreshold;
  int colorThreshold;
  
  // Servo positions (to be defined later based on hardware)
  int servoOpenPosition;
  int servoClosePosition;
  
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
  
  // Set servo positions (for future hardware implementation)
  void setServoPositions(int openPos, int closePos);
};

// Global instance
extern BallCollector ballCollector;

#endif
