/*********************************************************************
 * Wall Following Library - PD Controller for Left Wall Following
 *********************************************************************/

#ifndef WALL_FOLLOW_H
#define WALL_FOLLOW_H

#include <Arduino.h>

class WallFollow {
  private:
    // PD Controller parameters
    float kp;                    // Proportional gain
    float kd;                    // Derivative gain
    uint16_t targetDistance;     // Target distance from wall in mm
    
    // Error tracking
    float lastError;
    unsigned long lastUpdateTime;
    
    // Configuration
    int maxCorrection;           // Maximum correction value
    
    bool active;
    
  public:
    WallFollow();
    void init();
    void setKp(float newKp);
    void setKd(float newKd);
    void setTargetDistance(uint16_t distance);
    void setMaxCorrection(int correction);
    
    float getKp();
    float getKd();
    uint16_t getTargetDistance();
    
    void executeWallFollow(uint16_t leftDistance);
    void start();
    void stop();
    bool isActive();
    
    float getLastError();
};

extern WallFollow wallFollow;

#endif // WALL_FOLLOW_H
