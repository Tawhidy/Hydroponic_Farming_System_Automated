#include <DHT.h>

// -------- Pins --------
#define DHTPIN 2
#define DHTTYPE DHT11

#define FAN_RELAY_PIN        5
#define LIGHT_RELAY_PIN      4
#define HUMIDIFIER_RELAY_PIN 3

// -------- Relay logic levels --------
const int FAN_ACTIVE_LEVEL        = HIGH;
const int FAN_INACTIVE_LEVEL      = LOW;

const int LIGHT_ACTIVE_LEVEL      = HIGH;
const int LIGHT_INACTIVE_LEVEL    = LOW;

const int HUMIDIFIER_ACTIVE_LEVEL   = LOW;   // Active LOW
const int HUMIDIFIER_INACTIVE_LEVEL = HIGH;

// -------- Thresholds --------
float FAN_TEMP_THRESHOLD   = 35.0;  // Too hot → Fan ON
float LIGHT_TEMP_THRESHOLD = 30.0;  // Too cold → Light ON
float HUM_HUMID_THRESHOLD  = 60.0;  // Too dry → Humidifier ON

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  pinMode(HUMIDIFIER_RELAY_PIN, OUTPUT);

  digitalWrite(FAN_RELAY_PIN, FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN, LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, HUMIDIFIER_INACTIVE_LEVEL);

  Serial.println("Env controller (exclusive fan/light) starting...");
}

void loop() {
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT11, retrying...");
    return;
  }

  // Exclusive fan/light control
  bool fanOn = false;
  bool lightOn = false;

  if (t >= FAN_TEMP_THRESHOLD) {
    fanOn = true;      // too hot → fan ON
    lightOn = false;   // ensure light OFF
  } 
  else if (t <= LIGHT_TEMP_THRESHOLD) {
    lightOn = true;    // too cold → light ON
    fanOn = false;     // ensure fan OFF
  } 
  else {
    // Comfortable zone → both OFF
    fanOn = false;
    lightOn = false;
  }

  // Humidifier control
  bool humidifierOn = (h < HUM_HUMID_THRESHOLD);

  // Drive relays
  digitalWrite(FAN_RELAY_PIN,        fanOn        ? FAN_ACTIVE_LEVEL        : FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN,      lightOn      ? LIGHT_ACTIVE_LEVEL      : LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, humidifierOn ? HUMIDIFIER_ACTIVE_LEVEL : HUMIDIFIER_INACTIVE_LEVEL);

  // Debug status
  Serial.print("Temp: "); Serial.print(t); Serial.print(" C | ");
  Serial.print("Humidity: "); Serial.print(h); Serial.print(" % | ");
  Serial.print("Fan: "); Serial.print(fanOn ? "ON" : "OFF"); Serial.print(" | ");
  Serial.print("Light: "); Serial.print(lightOn ? "ON" : "OFF"); Serial.print(" | ");
  Serial.print("Humidifier: "); Serial.println(humidifierOn ? "ON" : "OFF");
}
