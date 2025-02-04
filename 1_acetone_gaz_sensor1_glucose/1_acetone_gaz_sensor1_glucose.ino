#include <DHT.h>              // Include library for temperature and humidity sensor

#define MQ138_PIN A0         // Pin connected to MQ-138 sensor
#define DHT_PIN 2            // Pin connected to DHT sensor
#define DHT_TYPE DHT22       // Specify DHT sensor type

DHT dht(DHT_PIN, DHT_TYPE);

// Sensor Calibration
float R0 = 10.0;  // Baseline resistance for MQ-138 (calibrated in clean air)
float tempComp = 1.0; // Temperature compensation factor
float humComp = 1.0;  // Humidity compensation factor

// Pre-calculated coefficients for the simplified linear model
float coeff_RsR0 = 100.0;     // Coefficient for Rs/R0 ratio
float coeff_temp = 1.5;       // Coefficient for temperature
float coeff_humidity = 0.5;   // Coefficient for humidity
float intercept = 50.0;       // Intercept for glucose prediction

// Measurement thresholds
float rsR0Threshold = 0.1; // Change in Rs/R0 ratio that indicates start or end
unsigned long measurementTimeout = 5000; // Timeout in milliseconds for stable reading

// Variables to track the state of measurement
bool isMeasuring = false;
unsigned long measurementStartTime = 0;
float lastRsR0 = 0.0;
unsigned long lastTime = 0;

// Function Prototypes
float calibrateSensor();
float calculateResistance(int rawValue);

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("Calibrating MQ-138 sensor...");
  R0 = calibrateSensor();
  Serial.print("Calibrated R0: ");
  Serial.println(R0);

  Serial.println("System ready.");
}

void loop() {
  // Step 1: Read raw sensor value
  int rawValue = analogRead(MQ138_PIN);

  // Step 2: Calculate Rs and Rs/R0 ratio
  float Rs = calculateResistance(rawValue);
  float ratio = Rs / R0;

  // Step 3: Read environmental conditions
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error reading temperature or humidity!");
    return;
  }

  // Step 4: Apply compensation factors
  ratio *= (1 + tempComp * (temperature - 25.0) / 100.0); // Adjust for temperature
  ratio *= (1 - humComp * (humidity - 50.0) / 100.0);     // Adjust for humidity

  // Step 5: Check for start or end of measurement
  if (!isMeasuring && abs(ratio - lastRsR0) > rsR0Threshold) {
    // Start measurement
    isMeasuring = true;
    measurementStartTime = millis();
    Serial.println("Measurement started.");
  } else if (isMeasuring && abs(ratio - lastRsR0) < rsR0Threshold) {
    // Check if measurement is stable for timeout period (indicating end)
    if (millis() - measurementStartTime > measurementTimeout) {
      isMeasuring = false;
      Serial.println("Measurement finished.");
      
      // Reset the system for next measurement
      resetSystem();
    }
  }

  // Step 6: Run the AI model to predict glucose level if measuring
  if (isMeasuring) {
    float predictedGlucoseLevel = coeff_RsR0 * ratio + coeff_temp * temperature + coeff_humidity * humidity + intercept;

    // Step 7: Display results
    Serial.println("-------------------------");
    Serial.print("Rs/R0 Ratio: ");
    Serial.println(ratio);
    Serial.print("Temperature (°C): ");
    Serial.println(temperature);
    Serial.print("Humidity (%): ");
    Serial.println(humidity);
    Serial.print("Predicted Glucose Level (mg/dL): ");
    Serial.println(predictedGlucoseLevel);
    Serial.println("-------------------------");
  }

  // Save last Rs/R0 ratio and time for next loop
  lastRsR0 = ratio;
  lastTime = millis();

  delay(500); // Delay for stability and to prevent overwhelming the serial output
}

// Calibrate MQ-138 sensor in clean air
float calibrateSensor() {
  float sum = 0;
  for (int i = 0; i < 100; i++) {
    sum += calculateResistance(analogRead(MQ138_PIN));
    delay(50);
  }
  return sum / 100;
}

// Calculate sensor resistance Rs
float calculateResistance(int rawValue) {
  float voltage = (rawValue / 1023.0) * 5.0;
  return (5.0 - voltage) * 10 / voltage;
}

// Reset the system for the next measurement
void resetSystem() {
  Serial.println("Preparing for next measurement...");
  delay(2000);  // Short delay to prepare for the next measurement
  lastRsR0 = 0.0;  // Reset the last Rs/R0 ratio
  isMeasuring = false;  // Reset the measurement flag
}
