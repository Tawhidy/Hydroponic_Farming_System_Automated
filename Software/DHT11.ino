#include <Wire.h>
#include <DHT.h>

#define DHTPIN 2       
#define DHTTYPE DHT11  

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println("DHT11 Weather Station Initialized");
}

void loop() {
  delay(2000);  

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); 


  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}