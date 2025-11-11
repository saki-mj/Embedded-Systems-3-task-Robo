// Define the ESP32-S3 GPIO pins connected to the MUX
const int SIG_PIN = 1;  // Connects to the MUX's SIG (Common) pin - Analog Capable Pin
const int S0 = 40;      // MUX Select Pin S0
const int S1 = 41;      // MUX Select Pin S1
const int S2 = 42;      // MUX Select Pin S2
const int S3 = 2;       // MUX Select Pin S3

// Array to hold the select pin numbers
const int selectPins[4] = {S0, S1, S2, S3};

// --- Function to set the MUX channel ---
void setMuxChannel(int channel) {
  if (channel < 0 || channel > 15) {
    return;
  }
  
  // Set the four select pins (S0-S3) based on the binary representation of the channel number
  for (int i = 0; i < 4; i++) {
    digitalWrite(selectPins[i], bitRead(channel, i));
  }
}

// --- Setup Function ---
void setup() {
  // Use a fast baud rate for quicker data transfer
  Serial.begin(115200); 
  
  // Set the MUX select pins as OUTPUTs
  for (int i = 0; i < 4; i++) {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], LOW); // Initialize to channel 0
  }
  
  // Set the MUX SIG pin as an INPUT
  pinMode(SIG_PIN, INPUT); 
  
  Serial.println("Starting Continuous Mux Scan (Values 0-15)");
}

// --- Main Loop (The continuous reading happens here) ---
void loop() {
  
  // Loop through all 16 channels (0 to 15)
  for (int channel = 0; channel < 16; channel++) {
    setMuxChannel(channel); // Select the current channel
    
    // Tiny delay for the MUX to settle and the signal to stabilize (Crucial for fast reading)
    // 5 microseconds is a good starting point.
    delayMicroseconds(5); 

    // Read the analog value from the common SIG pin
    int ir_value = analogRead(SIG_PIN); 

    // Print the value on the same line
    Serial.print(ir_value);
    
    // Print a separator, but not after the last sensor
    if (channel < 15) {
      Serial.print(" | ");
    }
  }
  
  // Move to the next line after all 16 sensors have been read
  Serial.println(); 
  
  // You can add a small delay here if the data is scrolling too fast, 
  // e.g., delay(10); for a 10ms wait between full scans.
  // For maximum speed, leave it out:
  // delay(100); 
}