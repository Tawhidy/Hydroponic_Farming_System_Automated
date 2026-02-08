#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT sensor setup (digital pin 2)
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins for environmental control
#define FAN_RELAY_PIN        5   // Relay for fan
#define LIGHT_RELAY_PIN      4   // Relay for light
#define HUMIDIFIER_RELAY_PIN 3   // Relay for humidifier

// Relay logic levels (some relays may be active HIGH or active LOW)
const int FAN_ACTIVE_LEVEL        = HIGH;
const int FAN_INACTIVE_LEVEL      = LOW;
const int LIGHT_ACTIVE_LEVEL      = HIGH;
const int LIGHT_INACTIVE_LEVEL    = LOW;
const int HUMIDIFIER_ACTIVE_LEVEL   = LOW;   // Humidifier is active LOW
const int HUMIDIFIER_INACTIVE_LEVEL = HIGH;

// Thresholds for control
float FAN_TEMP_THRESHOLD    = 35.0;  // °C, if ≥ this, turn on fan
float LIGHT_TEMP_THRESHOLD  = 30.0;  // °C, if ≤ this, turn on light
float HUM_HUMID_THRESHOLD   = 60.0;  // %, if < this, turn on humidifier

// pH sensor and tank relay pins
const int pHAnalogPin      = A0;  // pH sensor analog input (A0)
const int primaryRelayPin   = 12; // Relay for Primary tank (digital pin 12)
const int secondaryRelayPin = 11; // Relay for Secondary tank (digital pin 11)

void setup() {
  Serial.begin(9600); 
  // Initialize DHT sensor
  dht.begin();
  // Initialize relay output pins
  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  pinMode(HUMIDIFIER_RELAY_PIN, OUTPUT);
  pinMode(primaryRelayPin, OUTPUT);
  pinMode(secondaryRelayPin, OUTPUT);
  // Set relays to safe default states: all OFF except Primary ON
  digitalWrite(FAN_RELAY_PIN,        FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN,      LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, HUMIDIFIER_INACTIVE_LEVEL);
  digitalWrite(primaryRelayPin, HIGH);  // Primary tank ON
  digitalWrite(secondaryRelayPin, LOW); // Secondary tank OFF

  // Initialize I2C and OLED display
  Wire.begin(); // SDA/SCL on Mega (20/21):contentReference[oaicite:3]{index=3}
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); // Hang if display init fails
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  Serial.println("Combined environment and pH control starting...");
}

// Reads DHT sensor (with a 2s delay) and returns values via reference parameters.
// (Uses dht.readHumidity() and dht.readTemperature() as documented:contentReference[oaicite:4]{index=4}.)
void readTempHumidity(float &temp, float &hum) {
  delay(2000); // DHT11 timing requirement (2000 ms) 
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  // If reading failed, keep temp/hum at previous values (or NaN); print error
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" C, Hum: "); Serial.print(hum); Serial.println(" %");
}

// Applies control logic for fan/light/humidifier based on temp/hum.
// Exclusive fan/light: if hot, fan ON (light off); if cold, light ON (fan off); else both off.
// Humidifier: ON if humidity is below threshold.
// (Implements logic from original temp_humidity_control sketch.)
void controlEnvironment(float temp, float hum) {
  bool fanOn = false, lightOn = false;
  if (temp >= FAN_TEMP_THRESHOLD) {
    fanOn = true; 
    lightOn = false;
  }
  else if (temp <= LIGHT_TEMP_THRESHOLD) {
    lightOn = true;
    fanOn = false;
  }
  else {
    fanOn = false;
    lightOn = false;
  }
  bool humidifierOn = (hum < HUM_HUMID_THRESHOLD);

  digitalWrite(FAN_RELAY_PIN,        fanOn        ? FAN_ACTIVE_LEVEL        : FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN,      lightOn      ? LIGHT_ACTIVE_LEVEL      : LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, humidifierOn ? HUMIDIFIER_ACTIVE_LEVEL : HUMIDIFIER_INACTIVE_LEVEL);

  Serial.print("Fan: "); Serial.print(fanOn ? "ON" : "OFF");
  Serial.print(" | Light: "); Serial.print(lightOn ? "ON" : "OFF");
  Serial.print(" | Humidifier: "); Serial.println(humidifierOn ? "ON" : "OFF");
}

// Samples the pH sensor 50 times (100 ms apart) to compute an average pH.
// Converts the analog reading to voltage and then to pH using the calibration formula.
float readAveragePH() {
  float sumPH = 0.0;
  for (int i = 0; i < 50; i++) {
    int raw = analogRead(pHAnalogPin);
    float voltage = raw * (5.0 / 1023.0); // ADC 10-bit (0–1023) over 0–5V
    float pHValue = 7.0 - (7.0/2.5) * (voltage - 2.5); // calibration: 2.5V→pH7
    sumPH += pHValue;
    delay(100);
  }
  float avgPH = sumPH / 50.0;
  Serial.print("Average pH: ");
  Serial.println(avgPH);
  return avgPH;
}

// Controls tank relays based on averaged pH.
// If pH is outside [6.0, 8.0], switch to Secondary tank; otherwise use Primary.
void controlTanks(float avgPH) {
  if (avgPH < 6.0 || avgPH > 8.0) {
    // pH out of safe range: Secondary ON
    digitalWrite(primaryRelayPin, LOW);   // Primary OFF
    digitalWrite(secondaryRelayPin, HIGH); // Secondary ON
  } else {
    // pH in range: Primary ON
    digitalWrite(secondaryRelayPin, LOW);   // Secondary OFF
    digitalWrite(primaryRelayPin, HIGH);   // Primary ON
  }
}

// Updates the OLED display with temperature, humidity, and pH.
void updateDisplay(float temp, float hum, float avgPH) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temp); display.println(" C");
  display.print("Hum:  "); display.print(hum);  display.println(" %");
  display.print("pH:   "); display.print(avgPH);
  display.display();
}

void loop() {
  // 1. Read and control pH system
  float avgPH = readAveragePH();
  controlTanks(avgPH);

  // 2. Read and control temperature/humidity system
  float temperature = NAN, humidity = NAN;
  readTempHumidity(temperature, humidity);
  if (!isnan(temperature) && !isnan(humidity)) {
    controlEnvironment(temperature, humidity);
  }

  // 3. Update OLED display
  updateDisplay(temperature, humidity, avgPH);

  // The loop naturally repeats (~7 seconds due to DHT & pH delays).
}
