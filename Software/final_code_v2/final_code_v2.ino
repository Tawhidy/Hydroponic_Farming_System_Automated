#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------------- OLED CONFIG ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- SENSOR CONFIG ----------------
#define DHTPIN 6
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int fanPin = 3;
int lightPin = 4;
int humidifierPin = 5;

int buzzerPin = 14;  
float pHValue = 7.0;    // simulated, replace with sensor reading
float pHMin = 6.0;
float pHMax = 8.0;

float temperature = 0;
float humidity = 0;

// ---------------- BUZZER CONTROL ----------------
unsigned long lastBeep = 0;
bool buzzerState = false;
const unsigned long beepInterval = 5000;   // beep every 5s
const unsigned long beepDuration = 200;    // beep length

// ---------------- ICONS (small bitmaps 8x8) ----------------
static const unsigned char PROGMEM fanIcon[] = {
  B00011000,
  B00111100,
  B01111110,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000
};

static const unsigned char PROGMEM bulbIcon[] = {
  B00111100,
  B01111110,
  B01111110,
  B00111100,
  B00011000,
  B00011000,
  B00011000,
  B00000000
};

static const unsigned char PROGMEM dropIcon[] = {
  B00010000,
  B00111000,
  B01111100,
  B11111110,
  B11111110,
  B01111100,
  B00111000,
  B00010000
};

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(fanPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(humidifierPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.display();
}

// ---------------- LOOP ----------------
void loop() {
  // --- Read sensors ---
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // (Replace with your actual pH sensor function)
  pHValue = getPHReading();

  // --- Control devices ---
  digitalWrite(fanPin, temperature > 30 ? HIGH : LOW);
  digitalWrite(lightPin, LOW);   // example, manual
  digitalWrite(humidifierPin, humidity < 40 ? HIGH : LOW);

  // --- Handle buzzer ---
  handleBuzzer();

  // --- Update display ---
  drawDashboard();

  delay(1000);
}

// ---------------- FUNCTIONS ----------------
float getPHReading() {
  // TODO: replace with actual analog read + conversion
  return 6.5 + (random(-10, 10) * 0.01);  
}

void handleBuzzer() {
  unsigned long now = millis();
  if (pHValue < pHMin || pHValue > pHMax) {
    if (!buzzerState && (now - lastBeep >= beepInterval)) {
      tone(buzzerPin, 1000);
      buzzerState = true;
      lastBeep = now;
    }
    if (buzzerState && (now - lastBeep >= beepDuration)) {
      noTone(buzzerPin);
      buzzerState = false;
    }
  } else {
    noTone(buzzerPin);
    buzzerState = false;
  }
}

void drawDashboard() {
  display.clearDisplay();

  // --- Big readings ---
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print("pH:");
  display.print(pHValue, 1);

  display.setCursor(0, 20);
  display.print("T:");
  display.print(temperature, 0);
  display.print((char)247); // degree symbol
  display.print("C");

  display.setCursor(0, 40);
  display.print("H:");
  display.print(humidity, 0);
  display.print("%");

  // --- Small device states with icons ---
  display.setTextSize(1);
  int x = 90, y = 0;

  display.drawBitmap(x, y, fanIcon, 8, 8, SSD1306_WHITE);
  display.setCursor(x + 10, y);
  display.print(digitalRead(fanPin) ? "ON" : "OFF");

  display.drawBitmap(x, y + 12, bulbIcon, 8, 8, SSD1306_WHITE);
  display.setCursor(x + 10, y + 12);
  display.print(digitalRead(lightPin) ? "ON" : "OFF");

  display.drawBitmap(x, y + 24, dropIcon, 8, 8, SSD1306_WHITE);
  display.setCursor(x + 10, y + 24);
  display.print(digitalRead(humidifierPin) ? "ON" : "OFF");

  display.display();
}
