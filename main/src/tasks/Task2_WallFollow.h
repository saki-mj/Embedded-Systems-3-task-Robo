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
};

extern Task2WallFollow task2WallFollow;

#endif
