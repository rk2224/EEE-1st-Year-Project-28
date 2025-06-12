// UART serial reading
char buffer[5];  // 4 characters + null
int bufferIndex = 0;
bool collecting = false;

// A3 analogue threshold 
const int analogPin = A3;
const float thresholdVoltage = 2.5;
const unsigned long sampleInterval = 1;
const unsigned long debounceMsAnalog = 5;

unsigned long lastSampleTimeAnalog = 0;
unsigned long lastPulseTimeAnalog = 0;
unsigned long pulseCountAnalog = 0;

bool lastAboveThreshold = false;

// digital interrupt on A0
const int digitalPin = A0;
volatile unsigned long pulseCountDigital = 0;
const unsigned long debounceUsDigital = 100;
volatile unsigned long lastPulseTimeDigital = 0;

// A1: Hall sensor
const int hallPin = A1;
const int sampleCount = 10;
const float hallThreshold = 50.0;
const float referenceVoltage = 5.0;

const float alpha = 0.2;
float filteredValue = 0;
float baseline = 0;
const float gain = 2.0;

// Display timings
unsigned long lastDisplayTime = 0;
const unsigned long displayInterval = 1000;
unsigned long lastMagReadTime = 0;
const unsigned long magReadInterval = 200;

String lastTag = "----";

void setup() {
  Serial.begin(115200);
  Serial1.begin(600);
  while (!Serial);

  pinMode(analogPin, INPUT);
  pinMode(digitalPin, INPUT_PULLUP);
  pinMode(hallPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(digitalPin), countPulseDigital, FALLING);

  Serial.println("Starting combined system:");
  Serial.println("Format: IR_FREQ | RADIO_FREQ | MAG_STATUS | TAG");
  Serial.println("------------------------------------------------");

  calibrateBaseline();
}

void loop() {
  unsigned long currentMillis = millis();

  // Tag read from serial1
  if (Serial1.available()) {
    char data = Serial1.read();

    if (!collecting) {
      if (data == '#') {
        collecting = true;
        bufferIndex = 0;
        buffer[bufferIndex++] = data;
      }
    } else {
      if (bufferIndex < 4) {
        buffer[bufferIndex++] = data;
      }
      if (bufferIndex == 4) {
        buffer[4] = '\0';
        lastTag = String(buffer);
        collecting = false;
        bufferIndex = 0;
      }
    }
  }

  // IR sensing
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

  // Mag sensor update
  if (currentMillis - lastMagReadTime >= magReadInterval) {
    lastMagReadTime = currentMillis;
    updateMagneticReading();
  }

  // repeat display output
  if (currentMillis - lastDisplayTime >= displayInterval) {
    lastDisplayTime = currentMillis;

    // Analog (IR)
    float frequencyAnalog = (float)pulseCountAnalog / (displayInterval / 1000.0);
    pulseCountAnalog = 0;

    // Digital (Radio)
    noInterrupts();
    unsigned long pulsesDigital = pulseCountDigital;
    pulseCountDigital = 0;
    interrupts();
    float frequencyDigital = (float)pulsesDigital / (displayInterval / 1000.0);

    // Magnetic status
    String magStatus = getMagneticStatus();

    // Print combined output
    Serial.print("IR: ");
    Serial.print(frequencyAnalog, 1);
    Serial.print(" Hz | RADIO: ");
    Serial.print(frequencyDigital, 1);
    Serial.print(" Hz | MAG: ");
    Serial.print(magStatus);
    Serial.print(" | TAG: ");
    Serial.println(lastTag);
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
  filteredValue = alpha * avgSensorValue + (1 - alpha) * filteredValue;
}

// --------- Get magnetic status as string ---------
String getMagneticStatus() {
  float delta = (filteredValue - baseline) * gain;
  if (delta > hallThreshold) return "SOUTH";
  if (delta < -hallThreshold) return "NORTH";
  return "NONE";
}
