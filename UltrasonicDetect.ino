#define PACKET_SIZE 4  // # followed by 3 characters
char packetBuffer[PACKET_SIZE];
int packetIndex = 0;

void setup() {
  Serial.begin(9600);      
  Serial1.begin(600);     
}

void loop() {
  if (Serial1.available()) {
    byte data = Serial1.read();

    // Build the string
    if (packetIndex == 0 && data == '#') {
      packetBuffer[packetIndex++] = data;
    } 
    else if (packetIndex > 0 && packetIndex < PACKET_SIZE) {
      // Only store printable ASCII characters
      if (data >= 32 && data <= 126) {
        packetBuffer[packetIndex++] = data;
      } else {

        packetIndex = 0;
      }
    }

    if (packetIndex == PACKET_SIZE) {
      // Print the 4-character ASCII string
      Serial.print("Received: ");
      for (int i = 0; i < PACKET_SIZE; i++) {
        Serial.write(packetBuffer[i]);
      }
      Serial.println();
      
      // Reset for next string
      packetIndex = 0;
    }
  }
}
