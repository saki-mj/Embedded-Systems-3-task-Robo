/*********************************************************************
 * Task 3: Ramp Navigation
 * Description: [Add your task description here]
 *********************************************************************/

#ifndef TASK3_RAMP_H
#define TASK3_RAMP_H

#include <Arduino.h>

enum Task3SubState {
  T3_INIT,
  T3_APPROACH,
  T3_CLIMBING,
  T3_DESCENDING,
  T3_COMPLETED
};

class Task3Ramp {
  private:
    Task3SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t approachSpeed;
    uint16_t climbSpeed;
    uint16_t descendSpeed;
    unsigned long climbDuration;
    unsigned long descendDuration;
    uint16_t rampDetectionDistance;
    uint16_t topDetectionThreshold;

  public:
    Task3Ramp();
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task3SubState newSubState);
    Task3SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    
    // Configuration setters
    void setApproachSpeed(uint16_t speed);
    void setClimbSpeed(uint16_t speed);
    void setDescendSpeed(uint16_t speed);
    void setClimbDuration(unsigned long timeMs);
    void setDescendDuration(unsigned long timeMs);
    void setRampDetectionDistance(uint16_t distance);
    void setTopDetectionThreshold(uint16_t threshold);
    
    // Configuration getters
    uint16_t getApproachSpeed();
    uint16_t getClimbSpeed();
    uint16_t getDescendSpeed();
    unsigned long getClimbDuration();
    unsigned long getDescendDuration();
    uint16_t getRampDetectionDistance();
    uint16_t getTopDetectionThreshold();
};

extern Task3Ramp task3Ramp;

#endif
