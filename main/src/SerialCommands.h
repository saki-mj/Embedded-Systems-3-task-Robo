/*********************************************************************
 * Serial Commands Library
 * Handles all serial command processing for the robot
 *********************************************************************/

#ifndef SERIALCOMMANDS_H
#define SERIALCOMMANDS_H

#include <Arduino.h>

// Process serial command string
void processSerialCommand(String command);

// Print available serial commands
void printSerialCommands();

#endif
