#define BLYNK_TEMPLATE_ID "TMPL2rGTzYMQj"
#define BLYNK_TEMPLATE_NAME "VSR Navigation System"
#define BLYNK_AUTH_TOKEN "UTadoJyh7VOwIkkvhI4UEiTnWUoB1tLU"

#include <BlynkSimpleEsp8266.h>

// WiFi credentials
char ssid[] = "Elijah32";
char pass[] = "thankyou12";

// Relay pins
int relay1 = D3;
int relay2 = D5;
int relay3 = D6;
int relay4 = D7;

void setup() {
  // Initialize serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Print "Hello, World!" to the serial monitor
  Serial.println("Hello, World!");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Wait until connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set relay pins as outputs
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Ensure relays are off (HIGH level means off for most relay modules)
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  Serial.println("Relays are initialized and set to off.");
}

// BLYNK_WRITE functions handle the button press/release from the Blynk app
BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  digitalWrite(relay1, pinValue ? LOW : HIGH); // LOW to turn on the relay, HIGH to turn off
  Serial.print("Relay 1 ");
  Serial.println(pinValue ? "ON" : "OFF");
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt();
  digitalWrite(relay2, pinValue ? LOW : HIGH);
  Serial.print("Relay 2 ");
  Serial.println(pinValue ? "ON" : "OFF");
}

BLYNK_WRITE(V2) {
  int pinValue = param.asInt();
  digitalWrite(relay3, pinValue ? LOW : HIGH);
  Serial.print("Relay 3 ");
  Serial.println(pinValue ? "ON" : "OFF");
}

BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  digitalWrite(relay4, pinValue ? LOW : HIGH);
  Serial.print("Relay 4 ");
  Serial.println(pinValue ? "ON" : "OFF");
}

void loop() {
  Blynk.run(); // Run the Blynk library
}
