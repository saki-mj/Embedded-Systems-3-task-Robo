/*********************************************************************
 * Task 5: Unloading Balls
 * Description: [Add your task description here]
 *********************************************************************/

#ifndef TASK5_UNLOADING_H
#define TASK5_UNLOADING_H

#include <Arduino.h>

enum Task5SubState {
  T5_INIT,
  T5_NAVIGATE_TO_ZONE,
  T5_ALIGN,
  T5_UNLOADING,
  T5_VERIFY,
  T5_COMPLETED
};

class Task5Unloading {
  private:
    Task5SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    int ballsUnloaded;
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t navigateSpeed;
    uint16_t alignSpeed;
    uint16_t unloadSpeed;
    unsigned long unloadDuration;
    unsigned long alignDuration;
    uint16_t zoneDetectionDistance;
    uint16_t targetBallCount;

  public:
    Task5Unloading();
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task5SubState newSubState);
    Task5SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    int getBallsUnloaded();
    
    // Configuration setters
    void setNavigateSpeed(uint16_t speed);
    void setAlignSpeed(uint16_t speed);
    void setUnloadSpeed(uint16_t speed);
    void setUnloadDuration(unsigned long timeMs);
    void setAlignDuration(unsigned long timeMs);
    void setZoneDetectionDistance(uint16_t distance);
    void setTargetBallCount(uint16_t count);
    
    // Configuration getters
    uint16_t getNavigateSpeed();
    uint16_t getAlignSpeed();
    uint16_t getUnloadSpeed();
    unsigned long getUnloadDuration();
    unsigned long getAlignDuration();
    uint16_t getZoneDetectionDistance();
    uint16_t getTargetBallCount();
};

extern Task5Unloading task5Unloading;

#endif
