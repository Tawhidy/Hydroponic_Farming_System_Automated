#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Relay pins
#define FAN_RELAY_PIN        5
#define LIGHT_RELAY_PIN      4
#define HUMIDIFIER_RELAY_PIN 3
#define FAN_ACTIVE_LEVEL        HIGH
#define FAN_INACTIVE_LEVEL      LOW
#define LIGHT_ACTIVE_LEVEL      HIGH
#define LIGHT_INACTIVE_LEVEL    LOW
#define HUMIDIFIER_ACTIVE_LEVEL   LOW
#define HUMIDIFIER_INACTIVE_LEVEL HIGH

float FAN_TEMP_THRESHOLD    = 35.0;
float LIGHT_TEMP_THRESHOLD  = 28.0;
float HUM_HUMID_THRESHOLD   = 70.0;

const int pHAnalogPin      = A0;
const int primaryRelayPin   = 12;
const int secondaryRelayPin = 11;

const int buzzerPin = 14;
const unsigned long TRANSITION_BEEP_MS     = 2000UL;
const unsigned long PERIODIC_BEEP_INTERVAL = 5000UL;
const unsigned long PERIODIC_BEEP_DURATION = 200UL;
const unsigned int  BUZZER_FREQ = 1000;

bool secondaryActivePrev = false;
bool inTransitionBeep = false;
unsigned long transitionStartMillis = 0;
bool periodicActive = false;
unsigned long periodicLastBeepMillis = 0;
bool periodicBeepOn = false;
unsigned long periodicBeepStartMillis = 0;

// ---- pH averaging variables ----
unsigned long lastUpdate = 0;
float sumPH = 0;
int countPH = 0;

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  dht.begin();

  pinMode(FAN_RELAY_PIN, OUTPUT);
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  pinMode(HUMIDIFIER_RELAY_PIN, OUTPUT);
  pinMode(primaryRelayPin, OUTPUT);
  pinMode(secondaryRelayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  digitalWrite(FAN_RELAY_PIN, FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN, LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, HUMIDIFIER_INACTIVE_LEVEL);
  digitalWrite(primaryRelayPin, HIGH);
  digitalWrite(secondaryRelayPin, LOW);

  Serial.println("System start");
}

bool controlTanks(float avgPH) {
  bool secondaryActive = false;
  if (avgPH < 6.0 || avgPH > 8.0) {
    digitalWrite(primaryRelayPin, LOW);
    digitalWrite(secondaryRelayPin, HIGH);
    secondaryActive = true;
  } else {
    digitalWrite(secondaryRelayPin, LOW);
    digitalWrite(primaryRelayPin, HIGH);
    secondaryActive = false;
  }
  return secondaryActive;
}

void controlEnvironment(float temp, float hum) {
  bool fanOn = false, lightOn = false;
  if (temp >= FAN_TEMP_THRESHOLD) {
    fanOn = true; lightOn = false;
  } else if (temp <= LIGHT_TEMP_THRESHOLD) {
    fanOn = false; lightOn = true;
  }
  bool humidifierOn = (hum < HUM_HUMID_THRESHOLD);

  digitalWrite(FAN_RELAY_PIN, fanOn ? FAN_ACTIVE_LEVEL : FAN_INACTIVE_LEVEL);
  digitalWrite(LIGHT_RELAY_PIN, lightOn ? LIGHT_ACTIVE_LEVEL : LIGHT_INACTIVE_LEVEL);
  digitalWrite(HUMIDIFIER_RELAY_PIN, humidifierOn ? HUMIDIFIER_ACTIVE_LEVEL : HUMIDIFIER_INACTIVE_LEVEL);
}

void updateDisplay(float temp, float hum, float avgPH) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("T:");
  if (!isnan(temp)) display.print(temp, 1);
  else display.print("--");
  display.print("C");

  display.setCursor(0, 18);
  display.print("H:");
  if (!isnan(hum)) display.print(hum, 0);
  else display.print("--");
  display.print("%");

  display.setCursor(0, 36);
  display.print("pH:");
  display.print(avgPH, 2);

  display.setTextSize(1);
  display.setCursor(0, 54);
  bool fanOn = (digitalRead(FAN_RELAY_PIN) == FAN_ACTIVE_LEVEL);
  bool lightOn = (digitalRead(LIGHT_RELAY_PIN) == LIGHT_ACTIVE_LEVEL);
  bool humidOn = (digitalRead(HUMIDIFIER_RELAY_PIN) == HUMIDIFIER_ACTIVE_LEVEL);
  display.print("Fan:");   display.print(fanOn  ? "ON " : "OFF ");
  display.print("Light:"); display.print(lightOn? "ON " : "OFF ");
  display.print("Hum:");   display.print(humidOn? "ON"  : "OFF");

  display.display();
}

void handleBuzzerState(bool secondaryActive) {
  unsigned long now = millis();

  if (secondaryActive != secondaryActivePrev) {
    inTransitionBeep = true;
    transitionStartMillis = now;
    periodicActive = false;
    periodicBeepOn = false;
    tone(buzzerPin, BUZZER_FREQ);
  }

  if (inTransitionBeep) {
    if (now - transitionStartMillis >= TRANSITION_BEEP_MS) {
      noTone(buzzerPin);
      inTransitionBeep = false;
      if (secondaryActive) {
        periodicActive = true;
        periodicLastBeepMillis = now;
      }
    }
    secondaryActivePrev = secondaryActive;
    return;
  }

  if (periodicActive && secondaryActive) {
    if (!periodicBeepOn && (now - periodicLastBeepMillis >= PERIODIC_BEEP_INTERVAL)) {
      tone(buzzerPin, BUZZER_FREQ);
      periodicBeepOn = true;
      periodicBeepStartMillis = now;
    }
    if (periodicBeepOn && (now - periodicBeepStartMillis >= PERIODIC_BEEP_DURATION)) {
      noTone(buzzerPin);
      periodicBeepOn = false;
      periodicLastBeepMillis = now;
    }
  } else {
    periodicActive = false;
    if (periodicBeepOn) {
      noTone(buzzerPin);
      periodicBeepOn = false;
    }
  }

  if (!inTransitionBeep) {
    secondaryActivePrev = secondaryActive;
  }
}

void loop() {
  // accumulate pH samples quickly (no big delays)
  int raw = analogRead(pHAnalogPin);
  float voltage = raw * (5.0 / 1023.0);
  float pHValue = 7.0 - (7.0/2.5) * (voltage - 2.5);
  sumPH += pHValue;
  countPH++;

  if (millis() - lastUpdate >= 2000) {
    float avgPH = sumPH / countPH;

    bool secondaryActive = controlTanks(avgPH);
    handleBuzzerState(secondaryActive);

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if (!isnan(temperature) && !isnan(humidity)) {
      controlEnvironment(temperature, humidity);
    }

    updateDisplay(temperature, humidity, avgPH);

    sumPH = 0;
    countPH = 0;
    lastUpdate = millis();
  }
}
