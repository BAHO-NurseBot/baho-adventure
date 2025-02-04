#define BLYNK_TEMPLATE_ID "TMPL2uGf1nKIA"
#define BLYNK_TEMPLATE_NAME "VSR Switching Portal"
#define BLYNK_AUTH_TOKEN "z9ie6Km1Fn7fFREWYna2HVwVE17ZVzMm"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Servo.h>

// Blynk authentication token
char auth[] = "z9ie6Km1Fn7fFREWYna2HVwVE17ZVzMm";

// Your WiFi credentials
char ssid[] = "Elijah32";
char pass[] = "thankyou12";

// Define relay pins
#define RELAY1 D7
#define RELAY2 D2
#define RELAY3 D3
#define RELAY4 D1
#define RELAY5 D5 // Additional relay for V0 action
#define RELAY6 D4 // New relay 6

// Servo motor control pin D6
#define SERVO_PIN D8

// Create a Servo object
Servo myServo;

// Variable to track the antenna state
bool antennaClosed = true; // Initially, the antenna is assumed to be closed

void setup() {
    // Start serial monitor for debugging
    Serial.begin(115200);

    // Setup Blynk
    Blynk.begin(auth, ssid, pass);

    // Set relay pins as outputs
    pinMode(RELAY1, OUTPUT);
    pinMode(RELAY2, OUTPUT);
    pinMode(RELAY3, OUTPUT);
    pinMode(RELAY4, OUTPUT);
    pinMode(RELAY5, OUTPUT);
    pinMode(RELAY6, OUTPUT); // Setup relay 6

    // Initially turn off all relays
    digitalWrite(RELAY1, HIGH); // Relay off (assuming LOW triggers the relay)
    digitalWrite(RELAY2, HIGH);
    digitalWrite(RELAY3, HIGH);
    digitalWrite(RELAY4, HIGH);
    digitalWrite(RELAY5, HIGH);
    digitalWrite(RELAY6, HIGH); // Relay 6 off initially

    // Attach servo to its pin
    myServo.attach(SERVO_PIN);

    // Log initial state to serial
    Serial.println("System Initialized:");
    Serial.println("All relays are OFF, servo is attached.");
}

// Function to open the antenna
void openAntenna() {
    if (!antennaClosed) {
        Serial.println("Antenna is already open.");
        return;
    }

    Serial.println("Opening Antenna...");

    // Activate Relay 1 for VCC
    digitalWrite(RELAY1, LOW);
    delay(1000);  // Small delay for VCC stabilization

    // Activate Relay 5 (Switch) to open the antenna
    digitalWrite(RELAY5, LOW);

    // Move servo to 180 degrees
    myServo.write(180);
    Serial.println("Servo moved to 180 degrees (antenna opened).");

    antennaClosed = false; // Update the state
}

// Function to close the antenna
void closeAntenna() {
    if (antennaClosed) {
        Serial.println("Antenna is already closed.");
        return;
    }

    Serial.println("Closing Antenna...");

    // Move the servo to -180 degrees
    myServo.write(-180);
    delay(500); // Small delay for the servo to move

    // Activate Relay 1 (VCC) and Relay 5 (Switch) to close
    digitalWrite(RELAY1, LOW);  // Activate VCC
    delay(1000);  // Small delay for VCC stabilization
    digitalWrite(RELAY5, LOW);  // Activate the switch to close the antenna
    delay(1000); // Keep switch active for 1 seconds
    digitalWrite(RELAY1, HIGH); // Deactivate the switch

    Serial.println("Servo moved to -180 degrees (antenna closed).");

    antennaClosed = true; // Update the state
}

// Blynk function to control Relay 1 and Servo via V0 (Antenna control)
BLYNK_WRITE(V0) {
    if (param.asInt() == 1) { // Button is pressed
        if (antennaClosed) {
            Serial.println("V0 pressed: Opening Antenna...");
            openAntenna();
        } else {
            Serial.println("V0 pressed: Closing Antenna...");
            closeAntenna();
        }
    } else { // Button is released
        // Stop the antenna operation by turning off relays
        digitalWrite(RELAY1, HIGH); // Deactivate VCC
        digitalWrite(RELAY5, HIGH); // Deactivate the switch
        Serial.println("Antenna operation stopped.");
    }
}

// Blynk function to control Relay 2 via V1 (normal on/off control)
BLYNK_WRITE(V1) {
    int relay2State = param.asInt(); // Get value from Blynk app (0 or 1)
    digitalWrite(RELAY2, relay2State == 1 ? LOW : HIGH); // Turn on/off relay 2
    Serial.print("Navigation1 ");
    Serial.println(relay2State == 1 ? "ON" : "OFF");
}

// Blynk function to control Relay 3 via V2 (normal on/off control)
BLYNK_WRITE(V2) {
    int relay3State = param.asInt(); // Get value from Blynk app (0 or 1)
    digitalWrite(RELAY3, relay3State == 1 ? LOW : HIGH); // Turn on/off relay 3
    Serial.print("Navigation2 ");
    Serial.println(relay3State == 1 ? "ON" : "OFF");
}

// Blynk function to control Relay 4 via V3 (normal on/off control)
BLYNK_WRITE(V3) {
    int relay4State = param.asInt(); // Get value from Blynk app (0 or 1)
    digitalWrite(RELAY4, relay4State == 1 ? LOW : HIGH); // Turn on/off relay 4
    Serial.print("Scale Opening ");
    Serial.println(relay4State == 1 ? "ON" : "OFF");
}

// Blynk function to control Relay 6 via V4 (normal on/off control)
BLYNK_WRITE(V4) {
    int relay6State = param.asInt(); // Get value from Blynk app (0 or 1)
    digitalWrite(RELAY6, relay6State == 1 ? LOW : HIGH); // Turn on/off relay 6
    Serial.print("Scale Closing ");
    Serial.println(relay6State == 1 ? "ON" : "OFF");
}

void loop() {
    // Blynk loop to keep connection alive
    Blynk.run();
}
