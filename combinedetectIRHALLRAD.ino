// --------- Analog threshold frequency measurement on A3 ----------
const int analogPin = A3;                   // Analog input pin for voltage threshold
const float thresholdVoltage = 2.5;         // Voltage threshold in volts
const unsigned long sampleInterval = 1;     // Sampling interval in ms
const unsigned long debounceMsAnalog = 2;   // Debounce time for analog crossing in ms

unsigned long lastSampleTimeAnalog = 0;
unsigned long lastPulseTimeAnalog = 0;
unsigned long pulseCountAnalog = 0;

bool lastAboveThreshold = false;

// --------- Digital interrupt pulse frequency measurement on A0 ---------
const int digitalPin = A0;
volatile unsigned long pulseCountDigital = 0;
const unsigned long debounceUsDigital = 100;  // debounce for digital ISR in microseconds
volatile unsigned long lastPulseTimeDigital = 0;

// --------- Hall effect sensor on A1 ----------
const int hallPin = A1;               // Analog pin for Hall sensor, changed from A0
const int sampleCount = 10;           // Number of samples for oversampling
const float hallThreshold = 50.0;    // Threshold for magnetic event detection
const float referenceVoltage = 5.0;  // Adjust to 3.3 if board uses 3.3V ADC reference

// EMA filtering
const float alpha = 0.2;              // EMA smoothing factor
float filteredValue = 0;
float baseline = 0;
const float gain = 2.0;               // Software gain factor

// --------- Display timing ----------
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 1000;  // Display frequencies every 1 second
unsigned long lastMagReadTime = 0;
const unsigned long magReadInterval = 200;   // Read Hall sensor every 200 ms

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Setup pins
  pinMode(analogPin, INPUT);
  pinMode(digitalPin, INPUT_PULLUP);
  pinMode(hallPin, INPUT);

  // Attach interrupt for digital pulse counting on A0
  attachInterrupt(digitalPinToInterrupt(digitalPin), countPulseDigital, FALLING);

  Serial.println("Starting combined frequency & magnetic sensor measurement:");
  Serial.println("Format: IR_FREQ | RADIO_FREQ | MAG_STATUS");
  Serial.println("----------------------------------------");

  calibrateBaseline();
}

void loop() {
  unsigned long currentMillis = millis();

  // --- Analog pulse detection by voltage threshold crossing ---
  if (currentMillis - lastSampleTimeAnalog >= sampleInterval) {
    lastSampleTimeAnalog = currentMillis;

    int rawValue = analogRead(analogPin);
    float voltage = rawValue * (3.3 / 1023.0);

    bool currentlyAbove = (voltage > thresholdVoltage);

    if (lastAboveThreshold && !currentlyAbove) {
      if (currentMillis - lastPulseTimeAnalog > debounceMsAnalog) {
        pulseCountAnalog++;
        lastPulseTimeAnalog = currentMillis;
      }
    }

    lastAboveThreshold = currentlyAbove;
  }

  // --- Display frequencies every second ---
  if (currentMillis - lastDisplayTime >= displayInterval) {
    lastDisplayTime = currentMillis;

    // Analog frequency (IR)
    float frequencyAnalog = (float)pulseCountAnalog / (displayInterval / 1000.0);
    pulseCountAnalog = 0;

    // Digital frequency (Radio)
    noInterrupts();
    unsigned long pulsesDigital = pulseCountDigital;
    pulseCountDigital = 0;
    interrupts();
    float frequencyDigital = (float)pulsesDigital / (displayInterval / 1000.0);

    // Get magnetic status
    String magStatus = getMagneticStatus();

    // Print all in one line
    Serial.print("IR: ");
    Serial.print(frequencyDigital, 1);
    Serial.print(" Hz | RADIO: ");
    Serial.print(frequencyAnalog, 1);
    Serial.print(" Hz | MAG: ");
    Serial.println(magStatus);
  }

  // --- Hall sensor reading every 200ms ---
  if (currentMillis - lastMagReadTime >= magReadInterval) {
    lastMagReadTime = currentMillis;
    updateMagneticReading();
  }
}

// --------- ISR for digital pulse counting ---------
void countPulseDigital() {
  unsigned long now = micros();
  if (now - lastPulseTimeDigital >= debounceUsDigital) {
    pulseCountDigital++;
    lastPulseTimeDigital = now;
  }
}

// --------- Hall sensor baseline calibration ---------
void calibrateBaseline() {
  long total = 0;
  for (int i = 0; i < 100; i++) {
    total += analogRead(hallPin);
    delay(5);
  }
  baseline = total / 100.0;
  filteredValue = baseline;
}

// --------- Update Hall sensor reading ---------
void updateMagneticReading() {
  long sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    sum += analogRead(hallPin);
    delay(2);
  }
  float avgSensorValue = sum / (float)sampleCount;

  // Apply EMA filter
  filteredValue = alpha * avgSensorValue + (1 - alpha) * filteredValue;
}

// --------- Get magnetic status as string ---------
String getMagneticStatus() {
  float delta = (filteredValue - baseline) * gain;
  
  if (delta > hallThreshold) {
    return "SOUTH";
  } else if (delta < -hallThreshold) {
    return "NORTH";
  } else {
    return "NONE";
  }
}