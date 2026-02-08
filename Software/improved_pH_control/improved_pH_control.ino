// Arduino sketch: pH-based control of two water-tanks via relays on pins 11 and 12.
// Sensor input: analog pH sensor on A2, calibrated so 2.5V = pH7:contentReference[oaicite:5]{index=5}.
// Digital outputs: pin 11 = Primary tank relay, pin 12 = Secondary tank relay.

const int primaryRelayPin = 12;   // Relay control pin for Primary tank
const int secondaryRelayPin = 11; // Relay control pin for Secondary tank
const int pHAnalogPin = A0;       // Analog input pin for pH sensor

void setup() {
  Serial.begin(9600);                   // Initialize Serial Monitor at 9600 baud
  pinMode(primaryRelayPin, OUTPUT);
  pinMode(secondaryRelayPin, OUTPUT);
  
  // Default states: Primary ON, Secondary OFF
  digitalWrite(primaryRelayPin, HIGH);   // Turn ON primary tank
  digitalWrite(secondaryRelayPin, LOW);  // Turn OFF secondary tank
}

void loop() {
  float sumPH = 0.0;

  // Take 50 samples at 100ms intervals (about 5 seconds total)
  for (int i = 0; i < 50; i++) {
    int raw = analogRead(pHAnalogPin);             
    float voltage = raw * (5.0 / 1023.0);         // Convert ADC reading to voltage:contentReference[oaicite:6]{index=6}
    // Convert voltage to pH using calibrated formula
    float pHValue = 7.0 - (7.0/2.5)*(voltage - 2.5);
    sumPH += pHValue;
    delay(100);  // Wait 100 ms before next sample
  }
  
  float avgPH = sumPH / 50.0;   // Compute average pH
  
  // Check if average pH is out of safe range [6.0, 8.0]
  if (avgPH < 6.0 || avgPH > 8.0) {
    // pH out of range: switch to Secondary tank
    digitalWrite(primaryRelayPin, LOW);    // Turn OFF primary tank
    digitalWrite(secondaryRelayPin, HIGH); // Turn ON secondary tank
  } else {
    // pH back in range: switch to Primary tank
    digitalWrite(secondaryRelayPin, LOW);  // Turn OFF secondary tank
    digitalWrite(primaryRelayPin, HIGH);   // Turn ON primary tank
  }
  
  // Output average pH to Serial Monitor
  Serial.print("Average pH: ");
  Serial.println(avgPH);
  
  // (The sampling loop takes ~5s, so repeating loop ~every 5s)
}
