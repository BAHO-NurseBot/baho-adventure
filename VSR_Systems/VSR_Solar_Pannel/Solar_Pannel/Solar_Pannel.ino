#include <DHT.h>  // Include the library for the DHT sensor

#define RELAY1_PIN  10    // Define the first relay pin
#define RELAY2_PIN  11    // Define the second relay pin
#define RELAY3_PIN  12    // Define the third relay pin
#define RELAY4_PIN  9    // Define the fourth relay pin
#define TEMP_PIN    2     // Define the temperature sensor pin (for DHT11/DHT22)
#define TEMP_THRESHOLD 30  // Set the temperature threshold (in degrees Celsius)
#define RELAY_OPEN_TIME 4000  // Relay open time in milliseconds (e.g., 4000 ms = 4 second)

DHT dht(TEMP_PIN, DHT11);  // Initialize DHT sensor (use DHT22 for a DHT22 sensor)

bool relay1State = HIGH;  // Current state of the first relay (initially HIGH)
bool relay2State = HIGH;  // Current state of the second relay (initially HIGH)
bool relay3State = HIGH;  // Current state of the third relay (initially HIGH)
bool relay4State = HIGH;  // Current state of the fourth relay (initially HIGH)

unsigned long relay1Timer = 0;  // Timer for relay1
unsigned long relay2Timer = 0;  // Timer for relay2
unsigned long relay3Timer = 0;  // Timer for relay3
unsigned long relay4Timer = 0;  // Timer for relay4

bool tempAboveThreshold = false;  // Flag to indicate temperature above threshold

void setup() {
  pinMode(RELAY1_PIN, OUTPUT);  // Set relay1 pin as output
  pinMode(RELAY2_PIN, OUTPUT);  // Set relay2 pin as output
  pinMode(RELAY3_PIN, OUTPUT);  // Set relay3 pin as output
  pinMode(RELAY4_PIN, OUTPUT);  // Set relay4 pin as output

  digitalWrite(RELAY1_PIN, HIGH);  // Ensure relay1 is initially high (on)
  digitalWrite(RELAY2_PIN, HIGH);  // Ensure relay2 is initially high (on)
  digitalWrite(RELAY3_PIN, HIGH);  // Ensure relay3 is initially high (on)
  digitalWrite(RELAY4_PIN, HIGH);  // Ensure relay4 is initially high (on)

  dht.begin();  // Start the DHT sensor
  Serial.begin(9600);  // Start the serial monitor for debugging
}

void loop() {
  float temperature = dht.readTemperature();  // Read temperature in Celsius

  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  if (temperature > TEMP_THRESHOLD && !tempAboveThreshold) {
    // Temperature has risen above the threshold
    tempAboveThreshold = true;

    // Turn off Relay 1 and Relay 3 (set to LOW)
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY3_PIN, LOW);

    relay1Timer = millis();
    relay3Timer = millis();

    relay1State = LOW;
    relay3State = LOW;
  }

  if (temperature <= TEMP_THRESHOLD && tempAboveThreshold) {
    // Temperature has fallen below the threshold
    tempAboveThreshold = false;

    // Turn off Relay 2 and Relay 4 (set to LOW)
    digitalWrite(RELAY2_PIN, LOW);
    digitalWrite(RELAY4_PIN, LOW);

    relay2Timer = millis();
    relay4Timer = millis();

    relay2State = LOW;
    relay4State = LOW;
  }

  // Turn on Relay 1 after the delay (set to HIGH)
  if (!relay1State && (millis() - relay1Timer >= RELAY_OPEN_TIME)) {
    digitalWrite(RELAY1_PIN, HIGH);
    relay1State = HIGH;
  }

  // Turn on Relay 3 after the delay (set to HIGH)
  if (!relay3State && (millis() - relay3Timer >= RELAY_OPEN_TIME)) {
    digitalWrite(RELAY3_PIN, HIGH);
    relay3State = HIGH;
  }

  // Turn on Relay 2 after the delay (set to HIGH)
  if (!relay2State && (millis() - relay2Timer >= RELAY_OPEN_TIME)) {
    digitalWrite(RELAY2_PIN, HIGH);
    relay2State = HIGH;
  }

  // Turn on Relay 4 after the delay (set to HIGH)
  if (!relay4State && (millis() - relay4Timer >= RELAY_OPEN_TIME)) {
    digitalWrite(RELAY4_PIN, HIGH);
    relay4State = HIGH;
  }

  delay(1000);  // Wait before checking the temperature again (adjust as needed)
}
