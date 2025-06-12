const int inputPin = A0;
volatile unsigned long pulseCount = 0;
unsigned long lastDisplayTime = 0;
float frequency = 0;
const unsigned long displayInterval = 1000; // Update frequency every 1 second, could be reduced, just for readability purpose

// Debouncing variables
unsigned long lastPulseTime = 0;
const unsigned long debounceTime = 100; // Minimum 100µs between pulses

void setup() {
  Serial.begin(115200);
  pinMode(inputPin, INPUT_PULLUP);
  // Trigger interrupt on falling edge
  attachInterrupt(digitalPinToInterrupt(inputPin), countPulse, FALLING);
  Serial.println("IR Pulse Frequency Measurement Started");
  Serial.println("Detecting FALLING edges (pulse starts)");
}

void loop() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastDisplayTime >= displayInterval) {
    noInterrupts();
    frequency = (float)pulseCount / ((currentTime - lastDisplayTime) / 1000.0);
    pulseCount = 0;
    interrupts();
    
    Serial.print("Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");
    
    lastDisplayTime = currentTime;
  }
}

// ISR with debouncing enabled
void countPulse() {
  unsigned long currentMicros = micros();
  if (currentMicros - lastPulseTime >= debounceTime) {
    pulseCount++;
    lastPulseTime = currentMicros;
  }
}