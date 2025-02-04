#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>

// Define GPIO pins for modules
#define RST_PIN 0     // RFID Reset pin (D3 on NodeMCU)
#define SS_PIN 2      // RFID Slave Select pin (D4 on NodeMCU)
#define GSM_RX 5      // GSM RX (D1 on NodeMCU)
#define GSM_TX 4      // GSM TX (D2 on NodeMCU)
#define GPS_RX 14     // GPS RX (D5 on NodeMCU)
#define GPS_TX 12     // GPS TX (D6 on NodeMCU)

// Initialize modules
MFRC522 mfrc522(SS_PIN, RST_PIN);  // RFID instance
SoftwareSerial gsmSerial(GSM_RX, GSM_TX); // GSM Software Serial
SoftwareSerial gpsSerial(GPS_RX, GPS_TX); // GPS Software Serial
TinyGPSPlus gps;

// WiFi and Server Configuration
const char* ssid = "Hanga hub 1";
const char* password = "Musanze@1";
const char* serverURL = "https://innovahyperbackend.onrender.com";

String cardId;
String lastIdSent = ""; // Track last sent ID to prevent duplicates

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init();
  gsmSerial.begin(9600);
  gpsSerial.begin(9600);

  connectToWiFi();

  // GSM Init
  delay(2000);
  sendGSMCommand("AT+CSMP=49,167,0,0", "CSMP: ");
  sendGSMCommand("AT+CMGF=1", "SMS Mode: ");
  Serial.println("Setup complete. Ready to scan RFID and fetch/send data.");
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    getCardID();
    uploadData("{\"cardId\": \"" + cardId + "\"}", "/medicalRecords/activate-card");
  }

  // Detect card removal and handle deactivation
  if (!mfrc522.PICC_IsNewCardPresent()) {
    uploadData("", "/medicalRecords/deactivate-card");
    fetchDataAndSendMessage(); // Send health data via SMS after card removal
    delay(5000);  // Delay to avoid multiple quick triggers
  }

  // Process GPS data
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  delay(1000);  // General loop delay
}

// Function to get card ID
void getCardID() {
  cardId = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    cardId += String(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) {
      cardId += ":";
    }
  }
  cardId.toUpperCase();
  Serial.println("Card ID: " + cardId);
}

// Upload data function
void uploadData(String data, const char* endpoint) {
  WiFiClient wifiClient;
  HTTPClient http;

  String fullURL = String(serverURL) + String(endpoint);
  http.begin(wifiClient, fullURL);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.PUT(data); // Use HTTP PUT for updating data
  if (httpCode > 0) {
    Serial.printf("HTTP PUT Response: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      Serial.println("Response: " + response);
    }
  } else {
    Serial.printf("HTTP PUT Error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

// Fetch data and send SMS function
void fetchDataAndSendMessage() {
  WiFiClient wifiClient;
  HTTPClient http;

  String endpoint = "/api/health-records";
  String fullURL = String(serverURL) + endpoint;
  http.begin(wifiClient, fullURL);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    if (doc.containsKey("id") && doc.containsKey("content") && doc.containsKey("phoneNumber")) {
      String id = doc["id"].as<String>();
      String content = doc["content"].as<String>();
      String phoneNumber = doc["phoneNumber"].as<String>();

      if (id != lastIdSent) {
        String coordinates = gps.location.isUpdated() ? String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) : "GPS loading";
        String message = "Health Record for ID: " + id + "\n" + content + "\nGPS: " + coordinates;
        sendSMS(message, phoneNumber);
        lastIdSent = id;
      }
    }
  } else {
    Serial.printf("HTTP Error: %d\n", httpCode);
  }
  http.end();
}

// GSM Commands function
void sendGSMCommand(const char* command, const char* responseLabel) {
  gsmSerial.println(command);
  delay(1000);
  while (gsmSerial.available()) {
    String response = gsmSerial.readString();
    Serial.print(responseLabel);
    Serial.println(response);
  }
}

// Send SMS function
void sendSMS(String message, String phoneNumber) {
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(1000);

  if (gsmSerial.find(">")) {
    gsmSerial.print(message);
    gsmSerial.write(26);  // End message with CTRL+Z
    Serial.print("Message sent: ");
    Serial.println(message);
  }
  delay(3000);
}

// WiFi connection function
void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}
