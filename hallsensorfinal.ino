const int hallPin = A0;
const int sampleCount = 10;      // Number of samples for oversampling
const float threshold = 50.0;    // Threshold for detecting field (~10 G for 5 mV/G)
const float referenceVoltage = 5.0; // Use 3.3 if on a 3.3V board

// EMA filtering
const float alpha = 0.2;         // EMA smoothing factor (0.1 to 0.3 works well)
float filteredValue = 0;

// Baseline calibration
float baseline = 0;

// Software gain factor
const float gain = 2.0;          // Amplify signal delta (e.g., 2x for weak fields)

void calibrateBaseline() {
  long total = 0;
  for (int i = 0; i < 100; i++) {
    total += analogRead(hallPin);
    delay(5);
  }
  baseline = total / 100.0;
  filteredValue = baseline;  // Initialize EMA with baseline
  Serial.print("Calibrated Baseline: ");
  Serial.println(baseline);
}

void showMagneticReading() {
  long sum = 0;
  for (int i = 0; i < sampleCount; i++) {
    sum += analogRead(hallPin);
    delay(2);  // Small delay between samples
  }
  float avgSensorValue = sum / (float)sampleCount;

  // Apply EMA
  filteredValue = alpha * avgSensorValue + (1 - alpha) * filteredValue;

  // Calculate delta and apply software gain
  float delta = (filteredValue - baseline) * gain;
  float voltage = filteredValue * (referenceVoltage / 1023.0);

  Serial.print("Sensor: ");
  Serial.print(filteredValue, 1);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V | Delta: ");
  Serial.print(delta, 1);
  Serial.print(" | Status: ");

  if (delta > threshold) {
    Serial.println("South Pole Detected");
  } else if (delta < -threshold) {
    Serial.println("North Pole Detected");
  } else {
    Serial.println("No Magnetic Event Detected");
  }
}

void setup() {
  Serial.begin(9600);
  calibrateBaseline();
}

void loop() {
  showMagneticReading();
  delay(200);  // Update every 200 ms
}
