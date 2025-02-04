#include <Servo.h>

// Define the Ultrasonic sensor pins
const int trigPin = 7;
const int echoPin = 6;
  
// Create Servo objects
Servo servo1;
Servo servo2;

// Define the servo pins
const int servo1Pin = 9;
const int servo2Pin = 10;

// Variables to store the duration and distance
long duration;
int distance;

void setup() {
  // Initialize the Ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  // Attach the Servo objects to the appropriate pins
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  
  // Set the initial position of the servos
  servo1.write(0);
  servo2.write(180); // Opposite direction to servo1
  
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
  
  // Check if the distance is less than a threshold (e.g., 50 cm)
  if (distance < 50) {
    // Obstacle detected, move both servos to 120 degrees in opposite directions
    servo1.write(120);  // Rotate servo1 to 120 degrees
    servo2.write(60);   // Rotate servo2 to 60 degrees (opposite direction)
    Serial.println("Obstacle detected! Moving servos to 120 and 60 degrees.");
    
    // Wait for 30 seconds
    delay(30000);
    
    // Check if the obstacle is still there
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;

    if (distance >= 0) {
      // Obstacle no longer detected, close the servos
      Serial.println("Obstacle no longer detected. Moving servos back to 0 and 180 degrees.");
      servo1.write(0);     // Move servo1 back to 0 degrees
      servo2.write(180);   // Move servo2 back to 180 degrees (opposite direction)
    }
  }

  // Wait for a short period before checking again
  delay(100);
}
