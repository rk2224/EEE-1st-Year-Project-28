void setup() {
  Serial.begin(9600);      
  Serial1.begin(600);     
}

void loop() {
  if (Serial1.available()) {
    byte data = Serial1.read();

    Serial.print("HEX: 0x");
    if (data < 16) Serial.print("0");
    Serial.print(data, HEX);

    Serial.print("  ASCII: ");
    if (data >= 32 && data <= 126) {
      Serial.write(data);  
    } else {
      Serial.print("<non-printable>");
    }

    Serial.println();
  }
}
