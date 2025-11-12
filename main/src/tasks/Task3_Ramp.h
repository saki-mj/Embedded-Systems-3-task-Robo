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
};

extern Task3Ramp task3Ramp;

#endif
