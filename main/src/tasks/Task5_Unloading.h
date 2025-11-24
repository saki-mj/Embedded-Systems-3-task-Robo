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
};

extern Task5Unloading task5Unloading;

#endif
