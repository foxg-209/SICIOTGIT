#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN 4
#define DHTTYPE DHT21
#define THRESHOLD_VALUE 500
DHT dht(DHTPIN, DHTTYPE);


float pH_value;

void setup(void) {
  Serial.begin(9600);
  delay(1000);
  sensors.begin();
  dht.begin();

}

void loop(void) {
  sensors.requestTemperatures();
  float tempDS18B20 = sensors.getTempCByIndex(0);
  float tempDHT = dht.readTemperature();
  float humidity = dht.readHumidity();
  float pH_value = readpHValue();
  Serial.print(tempDS18B20);
  Serial.print(",");
  Serial.print(tempDHT);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
  Serial.println(pH_value);
  delay(15000);
 
}

float readpHValue() {
  int analogValue = analogRead(A0); 
  float voltage = analogValue * (5.0 / 1023.0); 
  float pH_value = 7 - (voltage - 2.5) * 3;
  return pH_value;
}
