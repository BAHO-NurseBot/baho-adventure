#include <Wire.h>

// Temperature library
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// internet connection libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
WiFiClientSecure wifiClientSec;
const char* host = "innovahyperbackend.onrender.com";

unsigned long t = 0;

bool uploaded = false;

void setup() {
  Serial.begin(9600);
  connectToWiFi("iPhone", "esther12345");
  // connectToWiFi("Innovahyper_technologies", "i_t_2024");
  if (!mlx.begin()) {
    Serial.println("Error connecting to MLX sensor. Check wiring.");
    while (1)
      ;
  };
}

void loop() {
  uploaded = false;
  getTemperature();
  delay(1000);
}


void getTemperature() {
  Serial.print("Object temperature = ");
  Serial.print(mlx.readObjectTempC());
  Serial.println("°C");
  float temperature = mlx.readObjectTempC();  // Read temperature from the sensor

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  if (temperature <= 40 && temperature >= 30) {
    String data = "{\"dataType\": \"temperature\", \"dataValue\": \"" + (String)temperature + "\"}";
    Serial.println(data);
    uploaded = uploadData(data, "/medicalRecords/record-medical-data");
  }

  delay(1000);
}


bool uploadData(String data, const char* filepath) {
  Serial.println("Sending data ... ");
  wifiClientSec.println("POST " + (String)filepath + " HTTP/1.1");
  wifiClientSec.println("Host: " + (String)host);
  wifiClientSec.println("User-Agent: ESP8266/1.0");
  wifiClientSec.println("Content-Type: application/json");
  wifiClientSec.println("Content-Length: " + (String)data.length());
  wifiClientSec.println();
  wifiClientSec.print(data);

  while (wifiClientSec.connected()) {
    String line = wifiClientSec.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    Serial.println(line);
    return true;
  }

  return false;
}


void connectToWiFi(const char* ssid, const char* passwd) {
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  connectToHost("8B 8E 8B 6C B9 94 FC 33 E9 3E F9 71 7C 9C CF AF 10 F1 DA 7A", 443);
  Serial.println();
}


void connectToHost(const char sha1_fingerprint[], const int httpsPort) {
  int retry_counter = 0;  //To be used while retrying to get connected
  //SHA-1 fingerprint
  wifiClientSec.setFingerprint(sha1_fingerprint);
  wifiClientSec.setTimeout(15000);  // 15 Seconds
  delay(1000);
  Serial.printf("Connecting to \"%s\"\n", host);
  while ((!wifiClientSec.connect(host, httpsPort)) && (retry_counter <= 30)) {
    delay(100);
    Serial.print(".");
    retry_counter++;
  }
  if (retry_counter == 61) {
    Serial.println("\nConnection failed.");
    return;
  } else {
    Serial.printf("Connected to \"%s\"\n", host);
  }
}
