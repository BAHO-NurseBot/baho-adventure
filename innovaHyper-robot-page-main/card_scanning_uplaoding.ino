#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN = D3;
#define SS_PIN = D4;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key; 

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
WiFiClientSecure wifiClientSec;
const char* host = "innovahyperbackend.onrender.com";

String cardId;

String res = "";

void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  connectToWiFi("Tk", "Esther12345");
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init();
}

uint8_t buf[10] = {};
MFRC522::Uid id;
MFRC522::Uid id2;
bool is_card_present = false;
uint8_t control = 0x00;

void PrintHex(uint8_t* data, uint8_t length) {
  cardId = "";
  char tmp[4];
  for (int i = 0; i < length; i++) {
    sprintf(tmp, "%.2X", data[i]);
    cardId += tmp;
    if (i < length - 1) {
      cardId += " ";
    }
  }
}


// Send message

void cpid(MFRC522::Uid* id) {
  memset(id, 0, sizeof(MFRC522::Uid));
  memcpy(id->uidByte, mfrc522.uid.uidByte, mfrc522.uid.size);
  id->size = mfrc522.uid.size;
  id->sak = mfrc522.uid.sak;
}

void loop() {
  Serial.println("Tap your card to continue");
  delay(1000);

  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status;

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  bool result = true;
  uint8_t buf_len = 4;
  cpid(&id);
  Serial.println("New Card was found!");
  PrintHex(id.uidByte, id.size);
  Serial.println("Read Card:");
  Serial.println(cardId);  // Print the card ID
  String data = "{\"cardId\": \"" + cardId + "\"}";
  uploadData(data, "/medicalRecords/activate-card");

  Serial.println("");

  while (true) {
    control = 0;
    for (int i = 0; i < 3; i++) {
      if (!mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          //Serial.print('a');
          control |= 0x16;
        }
        if (mfrc522.PICC_ReadCardSerial()) {
          //Serial.print('b');
          control |= 0x16;
        }
        //Serial.print('c');
        control += 0x1;
      }
      //Serial.print('d');
      control += 0x4;
    }

    //Serial.println(control);
    if (control == 13 || control == 14) {
      //card is still there
    } else {
      break;
    }
  }

  Serial.println("CardRemoved");
  uploadData("", "/medicalRecords/deactivate-card");

  // Send a message since we have finished
  
  delay(1000);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}


void uploadData(String data, const char* filepath) {
  Serial.println("Sending data ... ");
  wifiClientSec.println("PUT " + (String)filepath + " HTTP/1.1");
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
    res = line;
  }
}


void connectToWiFi(const char* ssid, const char* passwd) {
  WiFi.mode(WIFI_OFF);
  // delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  connectToHost("B7 65 A0 75 AB ED 1F 46 38 65 09 F8 7D 73 8E 39 DD A0 ED 50", 443);
  Serial.println();
}


void connectToHost(const char sha1_fingerprint[], const int httpsPort) {
  int retry_counter = 0;  //To be used while retrying to get connected
  //SHA-1 fingerprint
  wifiClientSec.setFingerprint(sha1_fingerprint);
  wifiClientSec.setTimeout(15000);  // 15 Seconds
  delay(1000);
  Serial.printf("Connecting to \"%s\"\n", host);
  //Serial.printf("Fingerprint \"%s\"\n", fingerprint);
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
