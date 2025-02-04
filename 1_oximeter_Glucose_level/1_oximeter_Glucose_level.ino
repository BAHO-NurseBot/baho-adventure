#include <Wire.h>
#include <EEPROM.h>
#include "MAX30100_PulseOximeter.h"
#include "MAX30100.h"

// Enable MAX30100 functionality
#define ENABLE_MAX30100 1

#if ENABLE_MAX30100
    #define REPORTING_PERIOD_MS 5000  // Reporting period in milliseconds

    // Define LED currents for IR and Red LEDs
    #define IR_LED_CURRENT MAX30100_LED_CURR_7_6MA
    #define RED_LED_CURRENT MAX30100_LED_CURR_7_6MA

    PulseOximeter pox;   // Pulse Oximeter instance
    MAX30100 sensor;     // MAX30100 sensor instance for advanced settings
#endif

uint32_t tsLastReport = 0;  // Last report time
uint32_t tsLastSave = 0;    // Last save time

float glucose_records[3] = {0.0, 0.0, 0.0};  // Glucose records

// Function called when a heartbeat is detected
void onBeatDetected() {
    Serial.println("Beat detected!");
}

// Setup function
void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");

#if ENABLE_MAX30100
    // Initialize Pulse Oximeter
    Serial.print("Initializing pulse oximeter...");
    if (!pox.begin()) {
        Serial.println("FAILED");
        for (;;);  // Halt if initialization fails
    } else {
        Serial.println("SUCCESS");
    }
    pox.setIRLedCurrent(IR_LED_CURRENT);  // Set IR LED current for Pulse Oximeter
    pox.setOnBeatDetectedCallback(onBeatDetected);

    // Initialize MAX30100
    Serial.print("Initializing MAX30100 sensor...");
    if (!sensor.begin()) {
        Serial.println("FAILED");
        for (;;);  // Halt if initialization fails
    } else {
        Serial.println("SUCCESS");
    }
    sensor.setMode(MAX30100_MODE_SPO2_HR);  // Set mode for SpO2 and heart rate
    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);  // Set LED currents

    // Load previous glucose records from EEPROM
    load_glucose_records();
    print_glucose_records();
#endif
}

// Main loop
void loop() {
#if ENABLE_MAX30100
    pox.update();  // Update Pulse Oximeter data

    int bpm = 0;
    int spo2 = 0;
    float glucose_level = 0.0;

    // Check if there is human touch (valid heart rate or SpO2 values)
    if (detectHumanTouch()) {
        // Update the data every REPORTING_PERIOD_MS milliseconds
        if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
            bpm = pox.getHeartRate();  // Get heart rate
            spo2 = pox.getSpO2();     // Get SpO2 level

            if (bpm > 0 && spo2 > 0) {
                // Example formula for glucose estimation (ensure this is scientifically valid)
                glucose_level = 16714.61 + 0.47 * bpm - 351.045 * spo2 + 1.85 * (spo2 * spo2);
            }

            // Print the values
            Serial.print("Heart rate: ");
            Serial.println(bpm);
            Serial.print("SpO2: ");
            Serial.println(spo2);
            Serial.print("Glucose Level: ");
            Serial.println(glucose_level);

            tsLastReport = millis();  // Update the last report time
        }

        // Save glucose level if it is within a valid range and has passed 10 seconds
        if (glucose_level < 500.0 && glucose_level > 0 && millis() - tsLastSave > 10000) {
            save_glucose_level(glucose_level);
            tsLastSave = millis();  // Update the last save time
        }
    } else {
        Serial.println("Waiting for human contact...");
    }
#endif
}

// Detect human touch using valid heart rate or SpO2 values
bool detectHumanTouch() {
    // Human touch is assumed if valid SpO2 or heart rate values are detected
    int bpm = pox.getHeartRate();
    int spo2 = pox.getSpO2();
    return (bpm > 0 && spo2 > 0);  // Check if readings are valid
}

// Save glucose level to EEPROM
void save_glucose_level(float glucose_level) {
    // Shift the old records and store the new glucose level
    glucose_records[2] = glucose_records[1];
    glucose_records[1] = glucose_records[0];
    glucose_records[0] = glucose_level;

    // Write glucose records to EEPROM
    for (int i = 0; i < 3; i++) {
        EEPROM.put(i * sizeof(float), glucose_records[i]);
    }

    print_glucose_records();  // Print updated records
}

// Load glucose records from EEPROM
void load_glucose_records() {
    for (int i = 0; i < 3; i++) {
        EEPROM.get(i * sizeof(float), glucose_records[i]);
    }
}

// Print the stored glucose records
void print_glucose_records() {
    Serial.println("Glucose Records:");
    for (int i = 0; i < 3; i++) {
        Serial.print(i);
        Serial.print(": ");
        Serial.println(glucose_records[i]);
    }
}
