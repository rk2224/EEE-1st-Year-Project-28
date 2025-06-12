char windowBuf[4];         
uint8_t winIdx = 0;
bool matched = false;
unsigned long lastBeat = 0; 

void setup() {
  Serial.begin(9600);        
  while (!Serial);           
  Serial1.begin(600);       
  Serial.println(F("Ready")); 
}

void loop() {

  if (millis() - lastBeat >= 1000) {
    Serial.println(F("...waiting for Serial1 data..."));
    lastBeat = millis();
  }
  while (!matched && Serial1.available()) {
    char c = Serial1.read();

    Serial.print(F("Rcv: 0x"));
    if ((uint8_t)c < 16) Serial.print('0');
    Serial.print((uint8_t)c, HEX);
    Serial.print(F("  ASCII: "));
    if (c >= 32 && c <= 126) Serial.write(c); 
    else                     Serial.print(F("<np>"));
    Serial.println();

    if (winIdx < 3) windowBuf[winIdx++] = c;
    if (winIdx == 3) {
      windowBuf[3] = '\0';          
      if (strcmp(windowBuf, "Sam") == 0) {
        Serial.println(F("✅ Matched \"Sam\" — entering stop mode."));
        matched = true;
        break;                       
      }

      windowBuf[0] = windowBuf[1];
      windowBuf[1] = windowBuf[2];
      winIdx = 2;
    }
  }

  if (matched) {
    while (1) {
    }
  }
}

