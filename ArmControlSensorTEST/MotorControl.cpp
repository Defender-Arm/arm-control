#include "MotorControl.h"

Motor::Motor(int stepPin, int dirPin, int stepDelay, float gearRatio, int sensorIndex, int sensorOffset) {
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->stepDelay = stepDelay;
    this->gearRatio = gearRatio;
    this->sensorIndex = sensorIndex;
    this->sensorOffset = sensorOffset;

    this->currentAngle = 0;

    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
}

void Motor::init(){
    this->currentAngle = getAngle();
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
  // float angleDelta = targetAngle - getAngle();
  currentAngle = getAngle();
  float angleDelta = targetAngle - currentAngle;
  int motorSteps = round((float)stepsPerRevolution * gearRatio * (float)abs(angleDelta) / 360.0);
  // Serial.println(String(currentAngle) + " " + String(motorSteps));
  // Set direction
  digitalWrite(dirPin, (angleDelta >= 0) ? HIGH : LOW);
  delay(2);  // Allow time for direction to stabilize
  return motorSteps;
}

void selectMuxChannel(uint8_t channel) {
  static uint8_t lastChannel = 255;  // Store last used channel
  if (channel == lastChannel) return;  // Skip if already selected

  lastChannel = channel;
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
}


int Motor::getAngle(){
  static AS5600 encoder;
  selectMuxChannel(sensorIndex);

  if (!encoder.begin()) {
    return currentAngle;

  }
    int raw = encoder.readAngle();
    int angle = round(raw * (360.0 / 4096.0)) -  sensorOffset;  // Convert to degrees
    if (angle > 180) {
      angle -= 360;
    }
    // Serial.println(String(sensorIndex) + ": "+ String(angle* (sensorIndex == 1 ? -1 : 1)));
    return angle* (sensorIndex == 1 ? -1 : 1);
  }