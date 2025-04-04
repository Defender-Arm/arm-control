#include "MotorControl.h"

Motor::Motor(int stepPin, int dirPin, int stepDelay, float gearRatio) {
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->stepDelay = stepDelay;
    this->gearRatio = gearRatio;
    this->currentAngle = 0;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
}

void Motor::moveMotor() {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(stepPin, LOW);
}

void Motor::updatePosition(int stepCounter){
    int direction = digitalRead(dirPin) == HIGH ? 1 : -1;
    currentAngle += direction * (stepCounter * 360.0) / (stepsPerRevolution * gearRatio);
}

int Motor::calculateSteps(int targetAngle){
    // Calculate shortest path
  float angleDelta = targetAngle - currentAngle;
  int motorSteps = round((float)stepsPerRevolution * gearRatio * (float)abs(angleDelta) / 360.0);
  // Serial.println(String(currentAngle) + " " + String(motorSteps));
  // Set direction
  digitalWrite(dirPin, (angleDelta >= 0) ? HIGH : LOW);
  delay(2);  // Allow time for direction to stabilize
  return motorSteps;
}

