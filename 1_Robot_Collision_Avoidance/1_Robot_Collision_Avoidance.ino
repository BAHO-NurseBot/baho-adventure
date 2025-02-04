#include <Wire.h>      // Include the Wire library for I2C communication
#include <NewPing.h>   // Include the NewPing library for ultrasonic sensors
#include <VL53L0X.h>   // Include the VL53L0X library for the time-of-flight sensor

// VL53L0X sensor configuration
VL53L0X vl53l0x;
#define COLLISION_DISTANCE 30 // Distance threshold to avoid obstacles (in cm)
#define DROP_OFF_DISTANCE 15  // Distance threshold to detect stairs or ravines (in cm)

// Ultrasonic sensor configuration
#define NUM_SENSORS 4          // Number of ultrasonic sensors
#define MAX_DISTANCE 400       // Maximum measurable distance (in cm)
#define TRIGGER_DISTANCE 30    // Distance threshold to activate obstacle avoidance (in cm)
const int triggerPins[NUM_SENSORS] = {5, 6, 7, 8}; // GPIO trigger pins
const int echoPins[NUM_SENSORS] = {9, 10, 11, 12}; // GPIO echo pins
NewPing sensors[NUM_SENSORS] = {
  NewPing(triggerPins[0], echoPins[0], MAX_DISTANCE),
  NewPing(triggerPins[1], echoPins[1], MAX_DISTANCE),
  NewPing(triggerPins[2], echoPins[2], MAX_DISTANCE),
  NewPing(triggerPins[3], echoPins[3], MAX_DISTANCE)
};

// Motor control pins
#define FORWARD_PIN 2
#define BACKWARD_PIN 3
#define LEFT_PIN 4
#define RIGHT_PIN 13  // Changed this from 5 to 13

// Motor control functions
void moveForward() {
  digitalWrite(FORWARD_PIN, HIGH);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  Serial.println("Moving forward.");
}

void moveBackward() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, HIGH);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  Serial.println("Moving backward.");
}

void turnLeft() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, HIGH);  // Turning the left motor
  digitalWrite(RIGHT_PIN, LOW);  // Stopping the right motor
  Serial.println("Turning left.");
}

void turnRight() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);   // Stopping the left motor
  digitalWrite(RIGHT_PIN, HIGH); // Turning the right motor
  Serial.println("Turning right.");
}

void stopRobot() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
  Serial.println("Stopping robot.");
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  Wire.begin();          // Initialize I2C communication

  // Setup motor control pins as outputs
  pinMode(FORWARD_PIN, OUTPUT);
  pinMode(BACKWARD_PIN, OUTPUT);
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);

  // Initialize VL53L0X sensor
  if (!vl53l0x.init()) {
    Serial.println("Failed to initialize VL53L0X!");
    while (1);
  }
  vl53l0x.setTimeout(500);
  vl53l0x.startContinuous();

  Serial.println("Sensors and motors initialized.");
}

void loop() {
  bool obstacleDetected = false;

  // Ultrasonic sensors: check for walls, people, or devices
  for (int i = 0; i < NUM_SENSORS; i++) {
    int distance = sensors[i].ping_cm();
    if (distance > 0 && distance <= TRIGGER_DISTANCE) {
      obstacleDetected = true;
      Serial.print("Ultrasonic Sensor ");
      Serial.print(i + 1);
      Serial.print(": Obstacle detected at ");
      Serial.print(distance);
      Serial.println(" cm");
    }
  }

  // VL53L0X: check for stairs or ravines
  int lidarDistance = vl53l0x.readRangeContinuousMillimeters() / 10; // Convert mm to cm
  if (vl53l0x.timeoutOccurred()) {
    Serial.println("VL53L0X timeout!");
  } else {
    Serial.print("VL53L0X Distance: ");
    Serial.print(lidarDistance);
    Serial.println(" cm");

    if (lidarDistance > 0 && lidarDistance < DROP_OFF_DISTANCE) {
      obstacleDetected = true;
      Serial.println("VL53L0X: Drop-off or stairs detected!");
    }
  }

  // Obstacle avoidance logic
  if (obstacleDetected) {
    // Stop and avoid obstacle
    stopRobot();
    delay(500);

    // Check left and right directions
    turnLeft();
    delay(500);
    int leftDistance = vl53l0x.readRangeContinuousMillimeters() / 10;
    Serial.print("Checking left direction: Distance = ");
    Serial.print(leftDistance);
    Serial.println(" cm");

    turnRight();
    delay(1000);
    int rightDistance = vl53l0x.readRangeContinuousMillimeters() / 10;
    Serial.print("Checking right direction: Distance = ");
    Serial.print(rightDistance);
    Serial.println(" cm");

    if (leftDistance > COLLISION_DISTANCE) {
      Serial.println("Clear path on the left. Turning left and moving forward.");
      turnLeft();
      delay(1000);
      moveForward();
    } else if (rightDistance > COLLISION_DISTANCE) {
      Serial.println("Clear path on the right. Turning right and moving forward.");
      turnRight();
      delay(1000);
      moveForward();
    } else {
      Serial.println("No clear path detected. Moving backward.");
      moveBackward();
      delay(1000);
    }
  } else {
    // No obstacle detected, move forward
    moveForward();
  }

  delay(100); // Delay between each loop iteration
}
