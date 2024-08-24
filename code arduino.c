#include <OneWire.h>// DS18B20 Äo nhiá»‡t Ä‘á»™ dÆ°á»›i nÆ°á»›c 
#include <DallasTemperature.h>
#include <DHT.h> // ThÆ° viá»‡n cáº£m biáº¿n Ä‘o nhiá»‡t Ä‘á»™ khÃ´ng khÃ­ vÃ  Ä‘á»™ áº©m
#include <SoftwareSerial.h>// Sim800L

SoftwareSerial sim800L(12, 11);// Khai bÃ¡o chÃ¢n Sim800L
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define DHTPIN 4
#define DHTTYPE DHT21
#define THRESHOLD_VALUE 500 // GiÃ¡ trá»‹ ngÆ°á»¡ng Ä‘á»ƒ xÃ¡c Ä‘á»‹nh tiáº¿p xÃºc vá»›i nÆ°á»›c, cáº§n Ä‘iá»u chá»‰nh dá»±a trÃªn cáº£m biáº¿n pH cá»¥ thá»ƒ
DHT dht(DHTPIN, DHTTYPE);

unsigned long Time_truoc = 0;
const long interval = 2000; 
const String PHONE = "+84332248203"; 
unsigned long callStartTime = 0; // LÆ°u giá»¯ thá»i Ä‘iá»ƒm bÃ¡t Ä‘áº§u cuá»™c gá»i
bool callInProgress = false;
unsigned long callDuration = 20000;// Thá»i lÆ°á»£ng cho cuá»™c gá»i lÃ  20 giÃ¢y
char Received_SMS;
short Bao_OK = -1;
bool hasSentBaoMessage = false;// XÃ¡c nháº­n Ä‘Ã£ gá»­i thÃ´ng bÃ¡o hay chÆ°a
float pH_value; // Biáº¿n Ä‘á»ƒ lÆ°u giÃ¡ trá»‹ pH


void setup(void) {
  Serial.begin(9600);// Tá»‘c Ä‘á»™ Baud 
  sim800L.begin(9600);
  delay(1000);

  // Ensure that the SIM800L module is in text mode (AT+CMGF=1)
  delay(1000);

  sensors.begin();
  dht.begin();

  ReceiveMode();
}

void send_sms(const String &message) {
  Serial.println("Sending SMS....");
  delay(50);
  sim800L.print("AT+CMGF=1\r");
  delay(1000);
  sim800L.print("AT+CMGS=\"" + PHONE + "\"\r");
  delay(1000);
  sim800L.print(message + "\n"); // ThÃªm kÃ½ tá»± xuá»‘ng dÃ²ng (\n) vÃ o thÃ´ng Ä‘iá»‡p
  sim800L.write(26); 
  delay(5000);
}

void make_call() {
  Serial.println("SOS !!!");
  Serial.println("Calling....");
  sim800L.println("ATD" + PHONE + ";");
  callStartTime = millis();
  callInProgress = true;
}

void loop(void) {
  sensors.requestTemperatures();
  float tempDS18B20 = sensors.getTempCByIndex(0);
  float tempDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Äá»c giÃ¡ trá»‹ tá»« cáº£m biáº¿n pH
  float pH_value = readpHValue(); // HÃ m readpHValue sáº½ cáº§n Ä‘Æ°á»£c cÃ i Ä‘áº·t dá»±a trÃªn thÆ° viá»‡n báº¡n Ä‘ang sá»­ dá»¥ng

    // G?i d? li?u theo d?nh d?ng CSV
  Serial.print(tempDS18B20); // Nhi?t d? nu?c
  Serial.print(",");
  Serial.print(tempDHT); // Nhi?t d? khï¿½ng khï¿½
  Serial.print(",");
  Serial.print(humidity); // ï¿½? ?m
  Serial.print(",");
  Serial.println(pH_value); // Giï¿½ tr? pH
  delay(1000);
//Äiá»u kiá»‡n Ä‘á»ƒ gá»­i thÃ´ng tin cáº£nh bÃ¡o 
  if (tempDS18B20 > 35) {
    String alertMessage = "Water temperature exceeds 36 Celsius Degree: " + String(tempDS18B20) + "Celsius Degree";
    send_sms(alertMessage);
  }
  if (tempDS18B20 > 35) {
    String alertMessage = "Water temperature is below 40 Celsius Degree: " + String(tempDS18B20) + "Celsius Degree";
    make_call();  // Gá»i cáº£nh bÃ¡o khi nhiá»‡t Ä‘á»™ dÆ°á»›i nÆ°á»›c tren 35 Ä‘á»™
  }
  if (pH_value < 5.0) {
    String alertMessage = "pH value is below 5: " + String(pH_value);
    make_call();  // Gá»i cáº£nh bÃ¡o khi Ä‘á»™ pH dÆ°á»›i 5
  }
  
 
  String RSMS;
  while (sim800L.available() > 0) {
    char Received_SMS = sim800L.read();
    Serial.print(Received_SMS);
    RSMS += Received_SMS;
  }

  if (RSMS.indexOf("Bao") != -1 && !hasSentBaoMessage) {
    Serial.println("found Bao");
    String response = "Temperature of water: " + String(tempDS18B20) + " Celsius Degree\nTemperature of air: " + String(tempDHT) + " Celsius Degree\nHumidity: " + String(humidity) + "%" + "\npH Value: " + String(pH_value) ;
    send_sms(response);
    hasSentBaoMessage = true;
  }
  
  // Äáº·t láº¡i biáº¿n hasSentBaoMessage sau khi gá»­i thÃ´ng bÃ¡o
  if (RSMS.indexOf("Bao") == -1) {
    hasSentBaoMessage = false;
  }
}

// HÃ m Ä‘á»c giÃ¡ trá»‹ tá»« cáº£m biáº¿n pH (cáº§n cÃ i Ä‘áº·t dá»±a trÃªn thÆ° viá»‡n báº¡n Ä‘ang sá»­ dá»¥ng)
float readpHValue() {
  int analogValue = analogRead(A0); // Äá»c giÃ¡ trá»‹ tá»« cá»•ng analog A0
  float voltage = analogValue * (5.0 / 1023.0); // Chuyá»ƒn Ä‘á»•i giÃ¡ trá»‹ analog sang Ä‘iá»‡n Ã¡p
  float pH_value = 7 - (voltage - 2.5) * 3; // á»¨ng dá»¥ng cÃ´ng thá»©c calibation tÃ¹y thuá»™c vÃ o cáº£m biáº¿n pH cá»¥ thá»ƒ

  return pH_value;
}


// CÃ¡c hÃ m khÃ¡c giá»¯ nguyÃªn
void Serialcom() {
  delay(500);
  while (Serial.available()) {
    sim800L.write(Serial.read());
  }
  while (sim800L.available()) {
    Serial.write(sim800L.read());
  }
}

void ReceiveMode() {
  sim800L.println("AT");
  Serialcom();
  sim800L.println("AT+CMGF=1");
  Serialcom();
  sim800L.println("AT+CNMI=2,2,0,0,0");
  Serialcom();
}