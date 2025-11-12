/*********************************************************************
 * Task 4: Read Barcode
 * Description: [Add your task description here]
 *********************************************************************/

#ifndef TASK4_BARCODE_H
#define TASK4_BARCODE_H

#include <Arduino.h>

enum Task4SubState {
  T4_INIT,
  T4_SEARCHING,
  T4_ALIGNING,
  T4_READING,
  T4_PROCESSING,
  T4_COMPLETED
};

class Task4Barcode {
  private:
    Task4SubState currentSubState;
    unsigned long subStateStartTime;
    bool taskActive;
    String barcodeData;

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
};

extern Task4Barcode task4Barcode;

#endif
