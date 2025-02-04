#include <Wire.h>
#include <NewPing.h>
#include <VL53L0X.h>
#include <TensorFlowLite.h>
#include "model.h"  // Fine-tuned AI model for human detection & navigation

// Ultrasonic Sensor Pins
const int triggerPins[4] = {5, 6, 7, 8};
const int echoPins[4] = {9, 10, 11, 12};

// PIR Sensors
#define PIR_LEFT 34
#define PIR_RIGHT 35

// Motor Driver Pins
#define FORWARD_PIN 2
#define BACKWARD_PIN 3
#define LEFT_PIN 4
#define RIGHT_PIN 13

NewPing sensors[4] = {
  NewPing(triggerPins[0], echoPins[0], 400),
  NewPing(triggerPins[1], echoPins[1], 400),
  NewPing(triggerPins[2], echoPins[2], 400),
  NewPing(triggerPins[3], echoPins[3], 400)
};

VL53L0X vl53l0x;
tflite::MicroInterpreter* interpreter;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  pinMode(FORWARD_PIN, OUTPUT);
  pinMode(BACKWARD_PIN, OUTPUT);
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);
  
  pinMode(PIR_LEFT, INPUT);
  pinMode(PIR_RIGHT, INPUT);

  // Init VL53L0X
  if (!vl53l0x.init()) {
    Serial.println("Failed to initialize VL53L0X!");
    while (1);
  }
  vl53l0x.startContinuous();

  // Load Fine-Tuned AI Model
  tflite::Initialize();
  interpreter = tflite::GetInterpreter();
}

void loop() {
  float ai_input[2] = {static_cast<float>(digitalRead(PIR_LEFT)), static_cast<float>(digitalRead(PIR_RIGHT))};
  memcpy(interpreter->input(0)->data.f, ai_input, sizeof(ai_input));
  interpreter->Invoke();
  float human_detected = interpreter->output(0)->data.f[0];

  float obstacle_features[5] = {
    static_cast<float>(sensors[0].ping_cm()),
    static_cast<float>(sensors[1].ping_cm()),
    static_cast<float>(sensors[2].ping_cm()),
    static_cast<float>(sensors[3].ping_cm()),
    static_cast<float>(vl53l0x.readRangeContinuousMillimeters() / 10)
  };

  memcpy(interpreter->input(1)->data.f, obstacle_features, sizeof(obstacle_features));
  interpreter->Invoke();
  int move_direction = interpreter->output(1)->data.i32[0];

  if (human_detected > 0.7) {
    stopRobot();
  } else {
    switch (move_direction) {
      case 0: stopRobot(); break;
      case 1: moveForward(); break;
      case 2: turnLeft(); break;
      case 3: turnRight(); break;
    }
  }

  delay(50);
}

void moveForward() {
  digitalWrite(FORWARD_PIN, HIGH);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
}

void stopRobot() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);
}

void turnLeft() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, HIGH);
  digitalWrite(RIGHT_PIN, LOW);
}

void turnRight() {
  digitalWrite(FORWARD_PIN, LOW);
  digitalWrite(BACKWARD_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, HIGH);
}

