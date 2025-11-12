/*********************************************************************
 * OLED Display Test Program
 * 
 * This program demonstrates all features of the OLEDDisplay library
 * Connect SSD1306 OLED (128x64) to I2C pins (SDA=8, SCL=9 for ESP32-S3)
 * 
 * Serial Commands:
 * - TEXT - Show simple text demo
 * - STATUS - Show robot status demo
 * - TOF - Show TOF sensor demo
 * - IR - Show IR sensor demo
 * - POS - Show position demo
 * - MSG - Show custom message demo
 * - CLEAR - Clear display
 * - CYCLE - Cycle through all demos
 *********************************************************************/

#include "OLEDDisplay.h"

// Demo mode
int currentDemo = 0;
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 3000; // 3 seconds between demos

void setup() {
  Serial.begin(115200);
  
  Serial.println("\n=== OLED Display Test ===");
  
  // Initialize OLED
  if (!oledDisplay.begin()) {
    Serial.println("Failed to initialize OLED!");
    while (1) delay(10);
  }
  
  Serial.println("\nOLED Display Test Ready!");
  printHelp();
  
  // Show welcome message
  displayMessage("OLED Ready!", "Type HELP for commands");
  delay(2000);
}

void loop() {
  // Check for serial commands
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    processCommand(command);
  }
  
  // Auto-cycle demo mode
  if (currentDemo > 0 && millis() - lastUpdate >= UPDATE_INTERVAL) {
    lastUpdate = millis();
    showNextDemo();
  }
}

void processCommand(String command) {
  if (command == "TEXT") {
    demoSimpleText();
  }
  else if (command == "STATUS") {
    demoRobotStatus();
  }
  else if (command == "TOF") {
    demoTOFSensors();
  }
  else if (command == "IR") {
    demoIRSensors();
  }
  else if (command == "POS") {
    demoPosition();
  }
  else if (command == "MSG") {
    demoMessage();
  }
  else if (command == "CLEAR") {
    oledDisplay.clear();
    Serial.println("Display cleared");
  }
  else if (command == "CYCLE") {
    currentDemo = 1;
    lastUpdate = millis();
    Serial.println("Starting auto-cycle mode (3s intervals)");
    showNextDemo();
  }
  else if (command == "STOP") {
    currentDemo = 0;
    Serial.println("Auto-cycle stopped");
  }
  else if (command == "HELP" || command == "?") {
    printHelp();
  }
  else {
    Serial.println("Unknown command. Type HELP for available commands.");
  }
}

void showNextDemo() {
  switch (currentDemo) {
    case 1:
      demoSimpleText();
      break;
    case 2:
      demoRobotStatus();
      break;
    case 3:
      demoTOFSensors();
      break;
    case 4:
      demoIRSensors();
      break;
    case 5:
      demoPosition();
      break;
    case 6:
      demoMessage();
      currentDemo = 0; // Reset after last demo
      break;
  }
  currentDemo++;
}

void demoSimpleText() {
  Serial.println("Demo: Simple Text");
  oledDisplay.showText(
    "Line Following",
    "Robot v1.0",
    "ESP32-S3",
    "Ready!"
  );
}

void demoRobotStatus() {
  Serial.println("Demo: Robot Status");
  oledDisplay.showRobotStatus("Line Follow", 512, "Active");
}

void demoTOFSensors() {
  Serial.println("Demo: TOF Sensors");
  // Simulate TOF readings
  uint16_t left = random(50, 500);
  uint16_t front = random(50, 500);
  uint16_t right = random(50, 500);
  oledDisplay.showTOFReadings(left, front, right);
  
  Serial.print("Simulated TOF: L=");
  Serial.print(left);
  Serial.print(" F=");
  Serial.print(front);
  Serial.print(" R=");
  Serial.println(right);
}

void demoIRSensors() {
  Serial.println("Demo: IR Sensors");
  // Simulate IR readings
  int irValues[8];
  for (int i = 0; i < 8; i++) {
    irValues[i] = random(0, 4095);
  }
  oledDisplay.showIRReadings(irValues, 8);
}

void demoPosition() {
  Serial.println("Demo: Position");
  // Simulate position
  int col = random(0, 10);
  int row = random(0, 10);
  int facing = random(0, 4);
  int obstacles = random(0, 5);
  int trackingMode = random(0, 3);
  oledDisplay.showPosition(col, row, facing, obstacles, trackingMode);
}

void demoMessage() {
  Serial.println("Demo: Custom Message");
  oledDisplay.showMessage("Test Complete!", "All demos shown");
}

void printHelp() {
  Serial.println("\n========================================");
  Serial.println("OLED Display Test Commands:");
  Serial.println("========================================");
  Serial.println("TEXT    - Show simple text demo");
  Serial.println("STATUS  - Show robot status demo");
  Serial.println("TOF     - Show TOF sensor demo");
  Serial.println("IR      - Show IR sensor demo");
  Serial.println("POS     - Show position demo");
  Serial.println("MSG     - Show custom message demo");
  Serial.println("CLEAR   - Clear display");
  Serial.println("CYCLE   - Auto-cycle through all demos");
  Serial.println("STOP    - Stop auto-cycle");
  Serial.println("HELP/?  - Show this help");
  Serial.println("========================================\n");
}
