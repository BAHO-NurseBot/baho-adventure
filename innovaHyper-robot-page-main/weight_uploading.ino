#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WebServer.h>

#define REPORTING_PERIOD_MS 5000
#define UPLOAD_TIMEOUT_MS 10000

const char* ssid = "Hanga hub 1";
const char* password = "Musanze@1";
const char* host = "innovahyperbackend.onrender.com";
const int httpsPort = 443;
const char* fingerprint = "B7 65 A0 75 AB ED 1F 46 38 65 09 F8 7D 73 8E 39 DD A0 ED 50";

uint32_t tsLastReport = 0;
WiFiClientSecure wifiClientSec;

unsigned long lastUploadTime = 0;
float lastUploadedBPM = 0;
bool uploaded = false;


#include <HX711_ADC.h>
#if defined(ESP8266) || defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = 13;  //mcu > HX711 dout pin
const int HX711_sck = 12;   //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(9600);
  connectToWiFi(ssid, password);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  float calibrationValue;    // calibration value
  calibrationValue = 21.08;  // uncomment this if you want to set this value in the sketch
#if defined(ESP8266) || defined(ESP32)
  //EEPROM.begin(512); // uncomment this if you use ESP8266 and want to fetch this value from eeprom
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch this value from eeprom

  LoadCell.begin();
  LoadCell.setReverseOutput();
  unsigned long stabilizingtime = 2000;  // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true;                  //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(calibrationValue);  // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update())
    ;
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }
}


void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 1000;  // Increase value to slow down serial print activity

  static float previousWeightKg = 0.0;
  static int stableCount = 0;
  const int stableThreshold = 3;  // Number of consistent readings needed
  const float weightTolerance = 1;  // Tolerance for weight stability in kg

  // Check for new data/start next conversion
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weightGrams = LoadCell.getData();
      float weightKg = weightGrams / 1000.0;  // Convert grams to kilograms
      weightKg = round(weightKg * 100) / 100.0;  // Round to two decimal places

      Serial.print("Load cell output val (kg): ");
      Serial.println(weightKg);
      Serial.println(stableCount);

      // Check if the weight is stable
      if (abs(weightKg - previousWeightKg) < weightTolerance) {
        stableCount++;
      } else {
        stableCount = 0;
      }

      previousWeightKg = weightKg;
      

      // If the weight is stable for the threshold count and greater than 5 kg, upload data
      if (stableCount >= stableThreshold) {
        Serial.print("Stable weight (kg): ");
        Serial.println(weightKg);
        if (weightKg > 5.0) {
          Serial.println("Warning: Weight is greater than 5 kg!");
          String data = "{\"dataType\": \"weight\", \"dataValue\": \"" + String(weightKg, 2) + "\"}";
          Serial.println(data);
          if (!uploadData(data, "/medicalRecords/record-medical-data")) {
            Serial.println("Uploading failed after timeout.");
          } else {
            Serial.println("Uploading weight successful.");
          }
        }
        stableCount = 0;  // Reset stable count after upload
      }

      newDataReady = 0;
      t = millis();
    }
  }

  // Receive command from serial terminal, send 't' to initiate tare operation
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // Check if last tare operation is complete
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}

// void loop() {
//   static boolean newDataReady = 0;
//   const int serialPrintInterval = 1000;  //increase value to slow down serial print activity

//   // check for new data/start next conversion:
//   if (LoadCell.update()) newDataReady = true;

//   // get smoothed value from the dataset:
//   if (newDataReady) {
//     if (millis() > t + serialPrintInterval) {
//       float i = LoadCell.getData();
//       // Serial.print("Load_cell output val: ");
//       // Serial.println(i);
//       float weightKg = i / 1000.0;
//       Serial.print("Load cell output val (kg): ");
//       Serial.println(weightKg);

//       if (weightKg > 5.0) {
//         Serial.println("Warning: Weight is greater than 5 kg!");
//         String data = "{\"dataType\": \"weight\", \"dataValue\": \"" + (String)weightKg + "\"}";
//         Serial.print(data);
//         if (!uploadData(data, "/medicalRecords/record-medical-data")) {
//           Serial.println("Uploading failed after timeout.");
//         } else {
//           Serial.println("Uploading weight successful.");
//         }
//       }

//       newDataReady = 0;
//       t = millis();
//     }
//   }

//   // receive command from serial terminal, send 't' to initiate tare operation:
//   if (Serial.available() > 0) {
//     char inByte = Serial.read();
//     if (inByte == 't') LoadCell.tareNoDelay();
//   }

//   // check if last tare operation is complete:
//   if (LoadCell.getTareStatus() == true) {
//     Serial.println("Tare complete");
//   }
// }



bool uploadData(String data, const char* filepath) {
  wifiClientSec.setFingerprint(fingerprint);
  if (!wifiClientSec.connect(host, httpsPort)) {
    Serial.println("Connection failed");
    return false;
  }

  Serial.println("Sending data ... ");
  Serial.println(data);
  wifiClientSec.println("POST " + String(filepath) + " HTTP/1.1");
  wifiClientSec.println("Host: " + String(host));
  wifiClientSec.println("User-Agent: ESP8266/1.0");
  wifiClientSec.println("Content-Type: application/json");
  wifiClientSec.println("Content-Length: " + String(data.length()));
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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");
}
