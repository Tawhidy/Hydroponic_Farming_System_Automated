#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ===== OLED Setup =====
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ===== pH Sensor Variables =====
float pH_Value;
int pH_analog;
float Voltage;

unsigned long lastUpdate = 0;
float sumPH = 0;
int countPH = 0;

// ===== DHT11 Setup =====
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

void setup() 
{ 
  Serial.begin(9600);

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Sensor Init");
  display.display();

  // DHT init
  dht.begin();
  delay(2000);
} 
 
void loop() 
{ 
  // ===== Read pH =====
  pH_analog = analogRead(A0); 
  Voltage = pH_analog * (5.0 / 1023.0); 
  pH_Value = 7 - (7.0/2.5)*(Voltage-2.5);

  sumPH += pH_Value;
  countPH++;

  // ===== Every 2 seconds =====
  if (millis() - lastUpdate >= 2000) {
    float avgPH = sumPH / countPH;

    // Read DHT
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      // Serial output
      Serial.print("Avg pH: ");
      Serial.println(avgPH, 2);
      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" °C  |  Hum: ");
      Serial.print(humidity);
      Serial.println(" %");
    }

    // ===== Update OLED =====
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print("pH: ");
    display.println(avgPH, 2);

    display.setTextSize(1);
    display.setCursor(0,30);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.setCursor(0,45);
    display.print("Hum: ");
    display.print(humidity);
    display.println(" %");

    display.display();

    // Reset accumulators
    sumPH = 0;
    countPH = 0;
    lastUpdate = millis();
  }
}
