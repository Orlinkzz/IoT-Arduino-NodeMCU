#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include <SoftwareSerial.h>

#define I2C_ADDR 0x27
#define LCD_COLUMNS 20
#define LCD_LINES 4
#define DHTPIN 2     
#define DHTTYPE DHT22 // DHT 22 (AM2302), AM2321
#define SPEAKER_PIN 8
#define LED_PIN 7

// Jika tidak mengunakan modul sim hapus ini
SoftwareSerial sim(12, 13); // TX RX
String apn = "internet"; // Nama APN
String user = ""; // Nama pengguna APN (kosongkan jika tidak ada)
String password = ""; // Kata sandi APN (kosongkan jika tidak ada)

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_LINES);
DHT dht(DHTPIN, DHTTYPE);
const int relayPin = 12;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  sim.begin(9600);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  dht.begin();

  // Init
  lcd.init();
  lcd.backlight();

  // Jika tidak mengunakan modul sim hapus ini
  //AT Command untuk inisialisasi SIM800L
  sim.println("AT");
  delay(1000);
  sim.println("AT+CMGF=1"); // Set mode teks SMS
  delay(1000);
  sim.println("AT+CNMI=1,2,0,0,0"); // Aktifkan pemberitahuan SMS
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Node MCU Serial Comunication
  String minta = "";
  while(Serial.available() > 0) {
    minta += char(Serial.read());
  }
  minta.trim();
  if (minta == "Ya") {
    send();
  }
  minta = "";

  // Print something
  lcd.setCursor(4, 0);
  lcd.print("Suhu Ruangan");
  lcd.setCursor(2, 1);
  lcd.print("Humidity: ");
  lcd.print(humidity);
  lcd.print("%");
  lcd.setCursor(0, 2);
  lcd.print("Temperature: ");
  lcd.print(temperature);
  lcd.print((char)223);
  lcd.print("C");
  if (temperature >= 26) {
    lcd.setCursor(4, 3);
    lcd.print("PANAS KAWAN!");
    //Mengirim pesan SMS
    // sendSMS("+6281234567890", "Halo, ini contoh pesan SMS dari Arduino!");
    String data = String(temperature) + "#" + String(humidity); // Node MCU
    Serial.println(data);
    digitalWrite(relayPin, HIGH);
    delay(5000);
    digitalWrite(relayPin, LOW);
    tone(SPEAKER_PIN, 262, 10000);
  } else if (temperature >= 24){
    lcd.setCursor(5, 3);
    lcd.print("MASIH AMAN!");
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(relayPin, HIGH);
  }else{
    lcd.setCursor(7, 3);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(relayPin, HIGH);
    lcd.print("Enjoy!");
  }
  delay(10000); // 10 detik
}

void send() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
}

// Jika tidak mengunakan modul sim hapus ini
void sendSMS(String number, String message) {
  sim.println("AT+CGATT=1"); //Aktifkan attach ke GPRS
  delay(1000);
  sim.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\""); //Set APN
  delay(1000);
  sim.print("AT+SAPBR=3,1,\"APN\",\"");
  sim.print(apn);
  sim.println("\"");
  delay(1000);
  sim.print("AT+SAPBR=3,1,\"USER\",\"");
  sim.print(user);
  sim.println("\"");
  delay(1000);
  sim.print("AT+SAPBR=3,1,\"PWD\",\"");
  sim.print(password);
  sim.println("\"");
  delay(1000);
  sim.println("AT+SAPBR=1,1"); //Buka koneksi GPRS
  delay(3000);
  sim.println("AT+CMGF=1"); //Set mode teks SMS
  delay(1000);
  sim.print("AT+CMGS=\"");
  sim.print(number);
  sim.println("\""); //Kirim nomor tujuan SMS
  delay(1000);
  sim.print(message); //Kirim isi pesan SMS
  delay(100);
  sim.write(0x1A); //Kirim karakter Ctrl+Z untuk mengirim SMS
  delay(1000);
  sim.println();
  delay(1000);
  sim.println("AT+SAPBR=0,1"); //Tutup koneksi GPRS
  delay(1000);
  sim.println("AT+CGATT=0"); //Nonaktifkan attach ke GPRS
}
