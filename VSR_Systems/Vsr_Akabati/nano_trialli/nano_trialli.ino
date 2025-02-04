// Define the Ultrasonic sensor pins
const int trigPin = 5;
const int echoPin = 4;

// Define the relay pins
const int relayPin1 = 8;  // Pin 1 of the relay module (activated when obstacle detected)
const int relayPin2 = 9;  // Pin 2 of the relay module (activated after a delay)

// Variables to store the duration and distance
long duration;
int distance;

void setup() {
  // Initialize the Ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Initialize the relay pins as output
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);

  // Ensure both relays are initially off
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
  
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

  // Check if obstacle is detected
  if (distance < 15) {
    // Obstacle detected, activate relay pin 1 for 2 seconds
    digitalWrite(relayPin1, HIGH);
    Serial.println("Obstacle detected! Relay Pin 1 activated.");
    delay(2000);  // Keep relay 1 in this state for 2 seconds
    digitalWrite(relayPin1, LOW);  // Deactivate relay pin 1 after 2 seconds
    Serial.println("Relay Pin 1 deactivated.");
    
    // Wait for 7 seconds before activating relay pin 2
    delay(7000);
    
    // Activate relay pin 2 for 2 seconds
    digitalWrite(relayPin2, HIGH);
    Serial.println("Relay Pin 2 activated.");
    delay(2000);  // Keep relay 2 in this state for 2 seconds
    digitalWrite(relayPin2, LOW);  // Deactivate relay pin 2 after 2 seconds
    Serial.println("Relay Pin 2 deactivated.");
  }

  // Wait before checking again
  delay(100);
}
