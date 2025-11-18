/*********************************************************************
 * Task 2: Wall Following
 * Description: [Add your task description here]
 *********************************************************************/

#ifndef TASK2_WALLFOLLOW_H
#define TASK2_WALLFOLLOW_H

#include <Arduino.h>

// Task 2 Sub-states (customize as needed)
enum Task2SubState {
  T2_INIT,
  T2_FIND_WALL,
  T2_ALIGN,
  T2_FOLLOWING,
  T2_CORNER_DETECTED,
  T2_COMPLETED
};

class Task2WallFollow {
  private:
    Task2SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    
    // Configuration parameters (can be changed via serial commands)
    uint16_t approachSpeed;
    uint16_t followSpeed;
    uint16_t turnSpeed;
    uint16_t wallDetectionDistance;
    uint16_t targetWallDistance;
    unsigned long turnDuration;
    unsigned long alignDuration;

  public:
    Task2WallFollow();
    void init();
    void execute();
    void updateDisplay();
    void setSubState(Task2SubState newSubState);
    Task2SubState getSubState();
    String getSubStateName();
    void start();
    void stop();
    bool isActive();
    bool isCompleted();
    void reset();
    
    // Configuration setters
    void setApproachSpeed(uint16_t speed);
    void setFollowSpeed(uint16_t speed);
    void setTurnSpeed(uint16_t speed);
    void setWallDetectionDistance(uint16_t distance);
    void setTargetWallDistance(uint16_t distance);
    void setTurnDuration(unsigned long timeMs);
    void setAlignDuration(unsigned long timeMs);
    
    // Configuration getters
    uint16_t getApproachSpeed();
    uint16_t getFollowSpeed();
    uint16_t getTurnSpeed();
    uint16_t getWallDetectionDistance();
    uint16_t getTargetWallDistance();
    unsigned long getTurnDuration();
    unsigned long getAlignDuration();
};

extern Task2WallFollow task2WallFollow;

#endif
