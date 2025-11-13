/*
 * SG90 9g Servo Controller - Example Usage
 * 
 * This example demonstrates the ServoController library
 * Control servo via Serial Monitor commands
 * 
 * Connections:
 * - Signal (Orange/Yellow): GPIO 39
 * - VCC (Red): 5V
 * - GND (Brown): GND
 * 
 * Serial Commands:
 * p90    - Move to 90 degrees (smooth)
 * i45    - Set immediate to 45 degrees
 * s120   - Set speed to 120°/s
 * m500,2400 - Set pulse width range
 * a0,180 - Set angle range
 * ?      - Show status
 * h      - Show help
 */

#include "ServoController.h"

const int SERVO_PIN = 37;
ServoController servo(SERVO_PIN);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== SG90 Servo Controller ===");
  Serial.println("Initializing servo...");
  
  // Initialize servo with default SG90 pulse width (500-2400μs)
  servo.begin(500, 2400);
  
  // Optional: Set custom speed (default is 60°/s)
  servo.setSpeed(90);  // 90 degrees per second
  
  // Optional: Set angle limits if needed
  // servo.setMinMaxAngles(0, 180);
  
  Serial.println("Servo ready!");
  servo.printHelp();
  
  // Demo: Move to center position
  servo.setPositionImmediate(90);
  Serial.println("\nServo centered at 90°");
  Serial.println("Send commands via Serial Monitor:");
}

void loop() {
  // Handle serial commands
  servo.handleSerialCommand();
  
  // Update servo position (required for smooth movement)
  servo.update();
  
  // Optional: Add your own code here
  // You can check servo.isMoving() to see if it's still moving
  // You can call servo.getCurrentPosition() to get current angle
}
