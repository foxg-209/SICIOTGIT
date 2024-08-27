#include <OneWire.h>// DS18B20 Đo nhiệt độ dưới nước 
#include <DallasTemperature.h>
#include <DHT.h> // Thư viện cảm biến đo nhiệt độ không khí và độ ẩm

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN 4
#define DHTTYPE DHT21
#define THRESHOLD_VALUE 500 // Giá trị ngưỡng để xác định tiếp xúc với nước, cần điều chỉnh dựa trên cảm biến pH cụ thể
DHT dht(DHTPIN, DHTTYPE);


float pH_value; // Biến để lưu giá trị pH


void setup(void) {
  Serial.begin(9600);// Tốc độ Baud 

  // Ensure that the SIM800L module is in text mode (AT+CMGF=1)
  delay(1000);

  sensors.begin();
  dht.begin();

}





void loop(void) {
  sensors.requestTemperatures();
  float tempDS18B20 = sensors.getTempCByIndex(0);
  float tempDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Đọc giá trị từ cảm biến pH
  float pH_value = readpHValue(); // Hàm readpHValue sẽ cần được cài đặt dựa trên thư viện bạn đang sử dụng

    // G?i d? li?u theo d?nh d?ng CSV
  Serial.print(tempDS18B20); // Nhi?t d? nu?c
  Serial.print(",");
  Serial.print(tempDHT); // Nhi?t d? kh�ng kh�
  Serial.print(",");
  Serial.print(humidity); // �? ?m
  Serial.print(",");
  Serial.println(pH_value); // Gi� tr? pH
  delay(15000);
 
}

// Hàm đọc giá trị từ cảm biến pH (cần cài đặt dựa trên thư viện bạn đang sử dụng)
float readpHValue() {
  int analogValue = analogRead(A0); // Đọc giá trị từ cổng analog A0
  float voltage = analogValue * (5.0 / 1023.0); // Chuyển đổi giá trị analog sang điện áp
  float pH_value = 7 - (voltage - 2.5) * 3; // Ứng dụng công thức calibation tùy thuộc vào cảm biến pH cụ thể

  return pH_value;
}
