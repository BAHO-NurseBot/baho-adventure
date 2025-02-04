// Define the Ultrasonic sensor pins for Relay control
const int trigPin = 5;
const int echoPin = 4;

// Define the relay pin
const int relayPin = 8;

// Variables to store the duration and distance
long duration;
int distance;

void setup() {
  // Initialize the Ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Initialize the relay pin as output
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Ensure relay is off initially
  
  // Start the serial communication for debugging purposes
  Serial.begin(9600);
}

void loop() {
  // Send a 10us pulse to the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echoPin, which returns the time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in cm
  distance = duration * 0.034 / 2;
  
  // Print the distance to the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Control the relay based on the distance measured by the ultrasonic sensor
  if (distance < 5) {
    // Obstacle detected, activate the relay
    digitalWrite(relayPin, HIGH);
    Serial.println("Obstacle detected! Relay activated.");
    
    // Wait until the obstacle is no longer detected
    while (distance < 5) {
      // Repeat the distance measurement
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      
      duration = pulseIn(echoPin, HIGH);
      distance = duration * 0.034 / 2;

      delay(100); // Brief delay to avoid continuous checking
    }

    // Once the obstacle is no longer detected, deactivate the relay
    Serial.println("Obstacle no longer detected. Relay deactivated.");
    digitalWrite(relayPin, LOW);
  }

  // Wait for a short period before checking again
  delay(100);
}
