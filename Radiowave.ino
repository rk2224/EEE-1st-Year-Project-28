// Improved Radio-Sensor Frequency Reader + Averaging @115200 baud
// Only measures on RISING edges, LED is flashed in loop, no delay in loop.

const uint8_t RADIO_PIN       = 3;        // D3: interrupt-capable
const unsigned long BAUD_RATE = 115200;
const uint8_t AVERAGE_SAMPLES = 5;        // average 5 measurements

volatile unsigned long lastRiseTime  = 0;
volatile unsigned long period        = 0;
volatile bool          newMeasurement = false;

unsigned long sumPeriods = 0;
uint8_t      sampleCount = 0;
bool ledFlash = false;

void handleEdge() {
  unsigned long now = micros();
  if (lastRiseTime) {
    period = now - lastRiseTime;
    newMeasurement = true;
  }
  lastRiseTime = now;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(BAUD_RATE);
  while (!Serial) delay(10);

  pinMode(RADIO_PIN, INPUT_PULLUP);  // Pull-up in case comparator is open-drain
  attachInterrupt(digitalPinToInterrupt(RADIO_PIN), handleEdge, RISING);

  Serial.println(F(">> Radio Averaging Reader Started"));
}

void loop() {
  if (newMeasurement) {
    noInterrupts();
    unsigned long p = period;
    newMeasurement = false;
    interrupts();

    if (p > 1000 && p < 50000) {
      sumPeriods += p;
      sampleCount++;
    }

    if (sampleCount >= AVERAGE_SAMPLES) {
      unsigned long avgPeriod = sumPeriods / AVERAGE_SAMPLES;
      float freq = 1e6f / (float)avgPeriod;
      Serial.print(F("Avg over "));
      Serial.print(AVERAGE_SAMPLES);
      Serial.print(F(" samples: "));
      Serial.print(freq, 2);
      Serial.println(F(" Hz"));
      sumPeriods = 0;
      sampleCount = 0;

      // LED flash for visual feedback
      ledFlash = true;
    }
  }

  // LED feedback: flash quickly when a new batch is printed
  static unsigned long lastFlash = 0;
  if (ledFlash) {
    digitalWrite(LED_BUILTIN, HIGH);
    lastFlash = millis();
    ledFlash = false;
  }
  if (millis() - lastFlash > 50) {
    digitalWrite(LED_BUILTIN, LOW);
  }
}
