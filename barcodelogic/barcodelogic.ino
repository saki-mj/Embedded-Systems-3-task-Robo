#include "IRReading.h"
#include "Motors.h"

// -------------------------------------------------------------------------
// Binary Strip Detection Configuration
// -------------------------------------------------------------------------

const int IR_SENSOR_CHANNEL = 6;        // 7th IR sensor (0-indexed as channel 6)
const int IR_THRESHOLD = 2684;          // Threshold for sensor 6 from IRReading.cpp
const unsigned long TIME_THRESHOLD = 505; // 505ms threshold (mean of 360ms and 650ms)
const int DEFAULT_MOTOR_SPEED = 210;    // Default PWM speed

// -------------------------------------------------------------------------
// State Variables
// -------------------------------------------------------------------------

enum RobotState {
  IDLE,
  RUNNING,
  TASK_COMPLETE
};

RobotState currentState = IDLE;

bool previousColor = false;             // false = black, true = white
bool currentColor = false;
int colorChangeCount = 0;
unsigned long whiteStartTime = 0;
bool onWhiteStrip = false;

String binaryResult = "";               // Store the binary sequence
int stripCount = 0;                     // Count white strips detected
unsigned long stripDurations[4];        // Store duration of each strip (max 4 strips)

int motorSpeed = DEFAULT_MOTOR_SPEED;   // Current motor speed

// -------------------------------------------------------------------------
// Function Prototypes
// -------------------------------------------------------------------------

void startBinaryDetection();
void processBinaryDetection();
void completeTask();
void resetTask();
void handleSerialCommands();
bool readColorFromIR();

// -------------------------------------------------------------------------
// Setup
// -------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n========================================");
  Serial.println("Binary Strip Detection Robot");
  Serial.println("========================================\n");
  
  // Initialize subsystems
  initIRSensors();
  initMotors();
  
  // Set initial motor speed
  setCurrentSpeed(motorSpeed);
  
  Serial.println("\nCommands:");
  Serial.println("  START     - Start binary detection task");
  Serial.println("  STOP      - Stop and reset task");
  Serial.println("  SPEED XXX - Set motor speed (0-1023), e.g., SPEED 200");
  Serial.println("\nRobot ready. Send START command to begin.\n");
}

// -------------------------------------------------------------------------
// Main Loop
// -------------------------------------------------------------------------

void loop() {
  // Handle serial commands
  handleSerialCommands();
  
  // Process binary detection if running
  if (currentState == RUNNING) {
    processBinaryDetection();
  }
  
  delay(10); // Small delay for stability
}

// -------------------------------------------------------------------------
// Binary Detection Functions
// -------------------------------------------------------------------------

void startBinaryDetection() {
  if (currentState == RUNNING) {
    Serial.println("Task already running!");
    return;
  }
  
  // Reset all variables
  resetTask();
  
  // Start motors
  setCurrentSpeed(motorSpeed);
  robotForward();
  
  // Initialize color state
  currentColor = readColorFromIR();
  previousColor = currentColor;
  
  currentState = RUNNING;
  
  Serial.println("\n========================================");
  Serial.println("Binary Detection Task STARTED");
  Serial.println("========================================");
  Serial.println("Robot moving forward...");
  Serial.println("Detecting white strips...\n");
}

void processBinaryDetection() {
  // Read current color from IR sensor
  currentColor = readColorFromIR();
  
  // Detect color change
  if (currentColor != previousColor) {
    colorChangeCount++;
    
    // Black to White transition
    if (currentColor == true && previousColor == false) {
      whiteStartTime = millis();
      onWhiteStrip = true;
      Serial.print("Strip detected (");
      Serial.print(stripCount + 1);
      Serial.println(") - Timer started");
    }
    
    // White to Black transition
    if (currentColor == false && previousColor == true && onWhiteStrip) {
      unsigned long whiteStripDuration = millis() - whiteStartTime;
      onWhiteStrip = false;
      
      // Store duration
      if (stripCount < 4) {
        stripDurations[stripCount] = whiteStripDuration;
      }
      stripCount++;
      
      // Determine binary value based on duration
      if (whiteStripDuration < TIME_THRESHOLD) {
        binaryResult += "0";
        Serial.print("Binary 0 detected (Duration: ");
        Serial.print(whiteStripDuration);
        Serial.println(" ms)");
      } else {
        binaryResult += "1";
        Serial.print("Binary 1 detected (Duration: ");
        Serial.print(whiteStripDuration);
        Serial.println(" ms)");
      }
      
      Serial.print("Current binary sequence: ");
      Serial.println(binaryResult);
      Serial.print("Color changes: ");
      Serial.println(colorChangeCount);
      Serial.println();
    }
    
    previousColor = currentColor;
  }
  
  // Check if task is complete (8 color changes detected)
  if (colorChangeCount >= 8) {
    completeTask();
  }
}

void completeTask() {
  // Stop motors
  stopAllMotors();
  
  currentState = TASK_COMPLETE;
  
  Serial.println("\n========================================");
  Serial.println("TASK COMPLETE!");
  Serial.println("========================================");
  Serial.print("Total color changes detected: ");
  Serial.println(colorChangeCount);
  Serial.print("White strips detected: ");
  Serial.println(stripCount);
  Serial.println();
  
  // Print duration for each strip
  Serial.println("Strip Durations:");
  for (int i = 0; i < stripCount && i < 4; i++) {
    Serial.print("  Strip ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(stripDurations[i]);
    Serial.print(" ms");
    if (stripDurations[i] < TIME_THRESHOLD) {
      Serial.println(" (Binary 0)");
    } else {
      Serial.println(" (Binary 1)");
    }
  }
  Serial.println();
  
  Serial.println("BINARY RESULT:");
  Serial.println(binaryResult);
  Serial.println("========================================\n");
  
  Serial.println("Send START to run again, or STOP to reset.");
}

void resetTask() {
  previousColor = false;
  currentColor = false;
  colorChangeCount = 0;
  whiteStartTime = 0;
  onWhiteStrip = false;
  binaryResult = "";
  stripCount = 0;
  
  // Clear duration array
  for (int i = 0; i < 4; i++) {
    stripDurations[i] = 0;
  }
  
  currentState = IDLE;
}

bool readColorFromIR() {
  // Read IR sensor channel 6 (7th sensor)
  int irValue = readIRSensor(IR_SENSOR_CHANNEL);
  
  // Return true if white (above threshold), false if black (below threshold)
  return (irValue > IR_THRESHOLD);
}

// -------------------------------------------------------------------------
// Serial Command Handler
// -------------------------------------------------------------------------

void handleSerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();
    
    if (command == "START") {
      startBinaryDetection();
    }
    else if (command == "STOP") {
      stopAllMotors();
      resetTask();
      Serial.println("Task stopped and reset.");
    }
    else if (command.startsWith("SPEED ")) {
      // Extract speed value
      int newSpeed = command.substring(6).toInt();
      
      if (newSpeed >= 0 && newSpeed <= 1023) {
        motorSpeed = newSpeed;
        setCurrentSpeed(motorSpeed);
        
        Serial.print("Motor speed set to: ");
        Serial.println(motorSpeed);
        
        // If robot is running, update speed immediately
        if (currentState == RUNNING) {
          robotForward();
        }
      } else {
        Serial.println("Invalid speed! Use value between 0 and 1023.");
      }
    }
    else if (command == "HELP") {
      Serial.println("\nAvailable Commands:");
      Serial.println("  START     - Start binary detection task");
      Serial.println("  STOP      - Stop and reset task");
      Serial.println("  SPEED XXX - Set motor speed (0-1023)");
      Serial.println("  HELP      - Show this help message\n");
    }
    else {
      Serial.println("Unknown command. Type HELP for available commands.");
    }
  }
}