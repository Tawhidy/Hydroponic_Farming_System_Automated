// EnvPhControl.ino

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT sensor setup (digital pin 6)
#define DHTPIN 6
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins for environmental control
#define FAN_RELAY_PIN         5   // Relay for fan
#define LIGHT_RELAY_PIN       4   // Relay for light
#define HUMIDIFIER_RELAY_PIN  3   // Relay for humidifier

// Relay logic levels (some relays may be active HIGH or active LOW)
const int FAN_ACTIVE_LEVEL           = HIGH;
const int FAN_INACTIVE_LEVEL         = LOW;
const int LIGHT_ACTIVE_LEVEL         = HIGH;
const int LIGHT_INACTIVE_LEVEL       = LOW;
const int HUMIDIFIER_ACTIVE_LEVEL    = LOW;   // Humidifier is active LOW
const int HUMIDIFIER_INACTIVE_LEVEL  = HIGH;

// Thresholds for control
float FAN_TEMP_THRESHOLD     = 35.0;  // °C, if ≥ this, turn on fan
float LIGHT_TEMP_THRESHOLD   = 30.0;  // °C, if ≤ this, turn on light
float HUM_HUMID_THRESHOLD    = 70.0;  // %, if < this, turn on humidifier

// pH sensor and tank relay pins
const int pHAnalogPin       = A0;  // pH sensor analog input (A0)
const int primaryRelayPin   = 12;  // Relay for Primary tank (digital pin 12)
const int secondaryRelayPin = 11;  // Relay for Secondary tank (digital pin 11)

// Persist last known DHT readings and humidifier state for display robustness
float lastTemp = NAN;
float lastHum  = NAN;
bool  lastDhtValid = false;
bool  lastHumidifierOn = false;

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
  digitalWrite(FAN_RELAY_PIN,         FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN,       LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN,  HUMIDIFIER_INACTIVE_LEVEL);
  digitalWrite(primaryRelayPin, HIGH);   // Primary tank ON
  digitalWrite(secondaryRelayPin, LOW);  // Secondary tank OFF

  // Initialize I2C and OLED display
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  Serial.println("Combined environment and pH control starting...");
}

// Reads DHT sensor (with a 2s delay) and returns values via reference parameters.
void readTempHumidity(float &temp, float &hum) {
  delay(2000); // DHT11 timing requirement (2000 ms)
  hum = dht.readHumidity();
  temp = dht.readTemperature();

  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!");
    // Mark invalid this cycle; display will fall back to last valid values
    lastDhtValid = false;
    return;
  }

  // Successful read: update persisted last-known values for display robustness
  lastTemp = temp;
  lastHum  = hum;
  lastDhtValid = true;

  Serial.print("Temp: "); Serial.print(temp);
  Serial.print(" C, Hum: "); Serial.print(hum); Serial.println(" %");
}

// Applies control logic for fan/light/humidifier based on temp/hum.
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

  digitalWrite(FAN_RELAY_PIN,         fanOn        ? FAN_ACTIVE_LEVEL        : FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN,       lightOn      ? LIGHT_ACTIVE_LEVEL      : LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN,  humidifierOn ? HUMIDIFIER_ACTIVE_LEVEL : HUMIDIFIER_INACTIVE_LEVEL);

  // Persist humidifier state for display (no change to control algorithm)
  lastHumidifierOn = humidifierOn;

  Serial.print("Fan: "); Serial.print(fanOn ? "ON" : "OFF");
  Serial.print(" | Light: "); Serial.print(lightOn ? "ON" : "OFF");
  Serial.print(" | Humidifier: "); Serial.println(humidifierOn ? "ON" : "OFF");
}

// Samples the pH sensor 50 times (100 ms apart) to compute an average pH.
float readAveragePH() {
  float sumPH = 0.0;
  for (int i = 0; i < 50; i++) {
    int raw = analogRead(pHAnalogPin);
    float voltage = raw * (5.0 / 1023.0); // ADC 10-bit (0–1023) over 0–5V
    float pHValue = 7.0 - (7.0/2.5) * (voltage - 1.5); // calibration: 2.5V→pH7
    sumPH += pHValue;
    delay(100);
  }
  float avgPH = sumPH / 50.0;
  Serial.print("Average pH: ");
  Serial.println(avgPH);
  return avgPH;
}

// Controls tank relays based on averaged pH.
void controlTanks(float avgPH) {
  if (avgPH < 6.0 || avgPH > 8.0) {
    // pH out of safe range: Secondary ON
    digitalWrite(primaryRelayPin, LOW);    // Primary OFF
    digitalWrite(secondaryRelayPin, HIGH); // Secondary ON
  } else {
    // pH in range: Primary ON
    digitalWrite(secondaryRelayPin, LOW);  // Secondary OFF
    digitalWrite(primaryRelayPin, HIGH);   // Primary ON
  }
}

// Updates the OLED display with temperature, humidity, and pH.
void updateDisplay(float temp, float hum, float avgPH) {
  // Choose values to show: if current read failed, keep last good values.
  float tShow = isnan(temp) ? lastTemp : temp;
  float hShow = isnan(hum)  ? lastHum  : hum;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // Temp line
  display.print("Temp: ");
  if (isnan(tShow)) {
    display.println("--");
  } else {
    display.print(tShow, 1);  // 1 decimal to keep line compact
    display.println(" C");
  }

  // Humidity line + inline humidifier state in small font ("H:ON/OFF")
  display.print("Hum:  ");
  if (isnan(hShow)) {
    display.print("--");
  } else {
    display.print(hShow, 0); // integer % to keep room for state tag
  }
  display.print(" % ");
  display.print("H:");
  display.println(lastHumidifierOn ? "ON" : "OFF");

  // pH line
  display.print("pH:   ");
  display.print(avgPH, 2);

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