#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <MAX30100_PulseOximeter.h>
#include <ESP8266WebServer.h>

#define REPORTING_PERIOD_MS 5000
#define UPLOAD_TIMEOUT_MS 10000

const char* ssid = "Innovahyper_technologies";
const char* password = "i_t_2024"; 
const char* host = "innovahyperbackend.onrender.com";
const int httpsPort = 443;
const char* fingerprint = "8B 8E 8B 6C B9 94 FC 33 E9 3E F9 71 7C 9C CF AF 10 F1 DA 7A";

float BPM, SpO2;
PulseOximeter pox;
uint32_t tsLastReport = 0;
ESP8266WebServer server(80);
WiFiClientSecure wifiClientSec;

unsigned long lastUploadTime = 0;
float lastUploadedBPM = 0;
bool uploaded = false;

void setup() {
  Serial.begin(115200);
  delay(100);

  connectToWiFi(ssid, password);
  setupWebServer();

  Serial.print("Initializing pulse oximeter..");
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;)
      ;
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
}

// Correct
void loop() {
  server.handleClient();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {

    // if (uploaded) {

    // }
    pox.update();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    Serial.print("BPM: ");
    Serial.println(BPM);
    Serial.print("SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");


    if (BPM > 50) {
      String heartRateStr = String(BPM, 2);
      String data = "{\"dataType\": \"bloodPressure\", \"dataValue\": \"" + heartRateStr + "\"}";
      if (!uploadData(data, "/medicalRecords/record-medical-data")) {
        Serial.println("Uploading failed after timeout.");
      } else {
        Serial.println("Uploading heart beat successful.");

        String spO2Str = String(SpO2, 2);
        String spO2Data = "{\"dataType\": \"oxygen\", \"dataValue\": \"" + spO2Str + "\"}";
        if (!uploadData(spO2Data, "/medicalRecords/record-medical-data")) {
          Serial.println("Uploading failed after timeout.");
        } else {
          Serial.println("Uploading oxygen successful.");
        }

        BPM = 0;
        SpO2 = 0;
        if (!pox.begin()) {
          Serial.println("FAILED");
        } else {
          Serial.println("SUCCESS");
          pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
        }
      }
    }
    tsLastReport = millis();
  }
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(BPM, SpO2));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float BPM, float SpO2) {
  String ptr = "<!DOCTYPE html><html><head><title>PULSE OXIMETER</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css'>";
  ptr += "<style>body { background-color: #fff; font-family: sans-serif; color: #333; font: 14px Helvetica, sans-serif; box-sizing: border-box;}";
  ptr += "#page { margin: 20px; background-color: #fff;}";
  ptr += ".header { padding: 20px; text-align: center;}";
  ptr += ".header h1 { color: #9a49bf; font-size: 45px; font-weight: bold;}";
  ptr += ".box-full { padding: 20px; border: 1px solid #ddd; border-radius: 1em; box-shadow: 1px 7px 7px rgba(0,0,0,0.4); background: #fff; margin: 20px; width: 300px;}";
  ptr += ".sensor { margin: 12px 0; font-size: 2.5rem;}";
  ptr += ".sensor-labels { font-size: 1rem; font-weight: bold; padding-bottom: 15px;}";
  ptr += ".units { font-size: 1.2rem;}";
  ptr += "hr { height: 1px; color: #eee; background-color: #eee; border: none;}";
  ptr += "</style>";
  ptr += "<script>setInterval(loadDoc,1000);function loadDoc() {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {document.body.innerHTML =this.responseText;}};xhttp.open('GET', '/', true);xhttp.send();}</script>";
  ptr += "</head><body><div id='page'><div class='header'><h1>WEB SERVER BASED PULSE OXIMETER</h1></div>";
  ptr += "<div class='box-full' align='left'><h2><center>Sensor Data</center></h2><div class='sensors-container'>";
  ptr += "<p class='sensor'><i class='fas fa-heartbeat' style='color:#ed0000'></i><span class='sensor-labels'> Heart Rate </span>" + String((int)BPM) + "<span class='sensor-labels'> BPM</span></p><hr>";
  ptr += "<p class='sensor'><i class='fas fa-burn' style='color:#0d05ff'></i><span class='sensor-labels'> SpO2 </span>" + String((int)SpO2) + "<span class='sensor-labels'> %</span></p><hr>";
  ptr += "</div></div></div></body></html>";
  return ptr;
}

bool uploadData(String data, const char* filepath) {
  // unsigned long startTime = millis();

  wifiClientSec.setFingerprint(fingerprint);
  // // wifiClientSec.setTimeout(UPLOAD_TIMEOUT_MS / 1000);

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
    // if (millis() - startTime > UPLOAD_TIMEOUT_MS) {
    //   Serial.println("Timeout waiting for response");
    //   return false;
    // }
    String line = wifiClientSec.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
    Serial.println(line);
    uploaded = true;
    return true;
  }

  // BPM = 0;
  // SpO2 = 0;
  return false;
}
