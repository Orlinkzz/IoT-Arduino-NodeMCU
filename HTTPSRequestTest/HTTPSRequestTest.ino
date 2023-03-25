#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SoftwareSerial.h>

// SET YOUR NETWORK CREDENTIALS
const char* ssid = "Wifi Name";
const char* password = "Password Wifi";

String phoneNumbers[] = {"62XXXXXXXXX", "62XXXXXXXXX", "62XXXXXXXXX"};
const char* apiKey = "API Key";

SoftwareSerial mySerial(12, 13); // D6 = RX, D7 = TX
unsigned long prevMillis = 0;
const long interval = 3000; // 3 detik

unsigned long lastSentTime = 0;
const unsigned long sendInterval = 10 * 60 * 1000; // 10 menit dalam milidetik
String arrData[1];

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Wi-Fi connected");
}

void loop() {
  unsigned long currentMill = millis();
  Serial.println(currentMill - prevMillis >= interval);
  Serial1.setDebugOutput(true);
  if (currentMill - prevMillis >= interval) {
    prevMillis = currentMill;

    String data = "";
    
    if (mySerial.available()) {
      String receivedData = mySerial.readStringUntil('\n');
      int delimiterIndex = receivedData.indexOf('#');

      if (delimiterIndex != -1) {
        float receivedTemperature = receivedData.substring(0, delimiterIndex).toFloat();
        float receivedHumidity = receivedData.substring(delimiterIndex + 1).toFloat();

        // lakukan apa yang perlu dilakukan dengan nilai suhu dan kelembaban yang diterima
        Serial.println(receivedTemperature);
        Serial.println(receivedHumidity);
        
        // Mengirim pesan setiap 10 mnt sekali
        if (millis() - lastSentTime >= sendInterval) {
          for (int i = 0; i < sizeof(phoneNumbers) / sizeof(phoneNumbers[0]); i++) {
            sendapi(phoneNumbers[i],receivedTemperature, receivedHumidity);
            delay(1000);
          }
          lastSentTime = millis(); // update waktu terakhir pengiriman
        }
      }
    }
  }
  delay(9000);
}

  void sendapi(String phoneNumber,float data, float data1) {
    char buffer[8];
    char buffer1[8];
    dtostrf(data, 6, 2, buffer);
    dtostrf(data1, 6, 2, buffer1);
    String myString = String(buffer);
    String myString1 = String(buffer1);
    myString.trim();
    myString1.trim();
    if (WiFi.status() == WL_CONNECTED) {
      if (myString != "" && myString1 != "") {
        // Membuat objek WiFiClient
        WiFiClient client;

        // IP address dan port dari server API
        const char* server = "192.168.10.1";
        const int port = 80;
        
        // Membuat data JSON untuk dikirim sebagai body
        String jsonData = "{\"phone_no\":\"" + phoneNumber + "\", \"key\":\"" + apiKey + "\", \"message\": \"Peringatan Ruangan Panas .Temperature: " + String(myString) +" Humidity: "+ String(myString1) +" Pesan ini akan di ulang 10 menit sekali \", \"skip_link\": true}";
        // Membuat request POST
        String request = "POST /api/send_message HTTP/1.1\r\n";
        request += "Host: 116.203.191.58\r\n";
        request += "Content-Type: application/json\r\n";
        request += "Content-Length: " + String(jsonData.length()) + "\r\n";
        request += "Connection: close\r\n\r\n";
        request += jsonData;

        // Mengirim request ke server
        if (client.connect(server, port)) {
          client.print(request);
          Serial.println("Request sent");
        }
        else {
          Serial.println("Connection failed");
        }

        // Membaca response dari server
        while (client.connected()) {
          if (client.available()) {
            String line = client.readStringUntil('\n');
            Serial.println(line);
          }
        }

        Serial.println();
        Serial.println("GET request done");
        Serial.println();
        Serial.println("Closing connection");
        client.stop();
      }
    }
  }
