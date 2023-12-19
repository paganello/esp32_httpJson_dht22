#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>

#define DHTPIN 26
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Paganelli_2.4Ghz";
const char* password = "a3*A!@4##5u8VP";  

WiFiServer server(80);

DynamicJsonDocument doc(1024);

void wifiKeepAlive() 
{
  if(WiFi.RSSI() < -127){
      Serial.println("WiFi Disconnected");
      WiFi.disconnect();
      WiFi.reconnect();
  }
}

float* readDHT()
{
  float* r = new float[2];

  r[0] = dht.readTemperature();
  r[1] = dht.readHumidity();
  
  if (isnan(r[0]) || isnan(r[1])) {
    r[0] = 0.0;
    r[1] = 0.0;
  }
  
  return r;
}

DynamicJsonDocument createJson(float data[]) 
{
  DynamicJsonDocument doc(1024);
  doc["device"] = "ESP32_DHT22_01";
  doc["humidity"] = data[0];
  doc["temperature"] = data[1];
  return doc;
}

void sendToServer() 
{
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println(createJson(readDHT()).as<String>());  // refresh the page automatically every 5 sec
            client.println("</html>");
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
}


void setup() {
  Serial.begin(115200);

  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to WiFi..");
    delay(1000);
  }
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() 
{  
  wifiKeepAlive();

  //float *data = readDHT();
  //doc = createJson(data);

  sendToServer();
}
