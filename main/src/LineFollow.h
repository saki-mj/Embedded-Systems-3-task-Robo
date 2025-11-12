// -------------------------------------------------------------------------
// LineFollow.h - PD Controller Line Following Library for ESP32-S3
// -------------------------------------------------------------------------

#ifndef LINE_FOLLOW_H
#define LINE_FOLLOW_H

#include <Arduino.h>

// -------------------------------------------------------------------------
// Line Following Configuration
// -------------------------------------------------------------------------

const int CENTER_SENSORS[4] = {6, 7, 8, 9};  // Middle sensors for line centering
const int CROSS_START = 3;   // Start of cross detection range
const int CROSS_END = 12;    // End of cross detection range
const int CROSS_SENSORS = 10; // Number of sensors for cross detection (3-12)

// -------------------------------------------------------------------------
// Global Variables
// -------------------------------------------------------------------------

extern float Kp;              // Proportional gain
extern float Kd;              // Derivative gain
extern bool lineFollowActive; // Flag for line following mode
extern bool crossDetected;    // Flag for cross detection
extern bool invertLine;       // Flag for line color inversion (false = white on black, true = black on white)

// -------------------------------------------------------------------------
// Line Following Functions
// -------------------------------------------------------------------------

/**
 * @brief Initialize line following system
 */
void initLineFollow();

/**
 * @brief Calculate weighted position of line (-7 to +7)
 * Negative = line is to the left, Positive = line is to the right
 * 0 = line is centered on sensors 6,7,8,9
 * @return Position error value
 */
float calculateLinePosition();

/**
 * @brief Detect if robot is at a cross (sensors 3-12 all detect white)
 * @return true if cross detected, false otherwise
 */
bool detectCross();

/**
 * @brief PD controller to calculate steering correction
 * @param position Current line position error
 * @return Steering correction value
 */
int calculatePD(float position);

/**
 * @brief Execute line following with PD control
 * Called continuously in loop when line following is active
 */
void executeLineFollow();

/**
 * @brief Toggle line following mode on/off
 */
void toggleLineFollow();

/**
 * @brief Set proportional gain (Kp)
 * @param value New Kp value
 */
void setKp(float value);

/**
 * @brief Set derivative gain (Kd)
 * @param value New Kd value
 */
void setKd(float value);

/**
 * @brief Get current Kp value
 * @return Current Kp
 */
float getKp();

/**
 * @brief Get current Kd value
 * @return Current Kd
 */
float getKd();

/**
 * @brief Check if line following is active
 * @return true if active, false otherwise
 */
bool isLineFollowActive();

/**
 * @brief Reset cross detection flag
 */
void resetCrossDetection();

/**
 * @brief Toggle line color mode (white on black / black on white)
 */
void toggleLineColor();

/**
 * @brief Get current line color mode
 * @return true if inverted (black on white), false if normal (white on black)
 */
bool isLineInverted();

#endif // LINE_FOLLOW_H
