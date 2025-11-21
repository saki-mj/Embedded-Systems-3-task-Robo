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
  T3_AT_TOP,
  T3_DESCENDING,
  T3_AFTER_RAMP,
  T3_TURN_RIGHT_90,
  T3_WALL_FOLLOW_LEFT,
  T3_COMPLETED
};

class Task3Ramp {
  private:
    Task3SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    bool stateMessagePrinted;
    
    // Configuration parameters (can be changed via serial commands)
    unsigned long climbDuration;
    unsigned long descendDuration;
    uint16_t rampDetectionDistance;
    uint16_t topDetectionThreshold;
    unsigned long turnDuration;  // Duration for 90° turn

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
    void setClimbDuration(unsigned long timeMs);
    void setDescendDuration(unsigned long timeMs);
    void setRampDetectionDistance(uint16_t distance);
    void setTopDetectionThreshold(uint16_t threshold);
    void setTurnDuration(unsigned long timeMs);
    
    // Configuration getters
    unsigned long getClimbDuration();
    unsigned long getDescendDuration();
    uint16_t getRampDetectionDistance();
    uint16_t getTopDetectionThreshold();
    unsigned long getTurnDuration();
};

extern Task3Ramp task3Ramp;

#endif
