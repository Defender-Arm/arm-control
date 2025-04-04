#include "MotorControl.h"

Motor::Motor(int stepPin, int dirPin, int stepDelay, float gearRatio, int sensorIndex, int sensorOffset) {
    this->stepPin = stepPin;
    this->dirPin = dirPin;
    this->stepDelay = stepDelay;
    this->gearRatio = gearRatio;
    this->sensorIndex = sensorIndex;
    this->sensorOffset = sensorOffset;

    this->currentAngle = getAngle() - sensorOffset;

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
    // include error?
  float angleDelta = targetAngle - currentAngle;
  int motorSteps = round((float)stepsPerRevolution * gearRatio * (float)abs(angleDelta) / 360.0);
  // Serial.println(String(currentAngle) + " " + String(motorSteps));
  // Set direction
  digitalWrite(dirPin, (angleDelta >= 0) ? HIGH : LOW);
  delay(2);  // Allow time for direction to stabilize
  return motorSteps;
}

void selectMuxChannel(uint8_t channel) {
  if (channel > 7) return;
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

int Motor::getAngle(){
  AS5600 encoder;
  selectMuxChannel(sensorIndex);
  delay(5);

  if (!encoder.begin()) {
    // String errorCode = "000";
    // Serial.println((errorCode.replace(sensorIndex, "1")));
    return -1;
  } else {
    int raw = encoder.readAngle();
    return round(raw * (360.0 / 4096.0));  // Convert to degrees
  }
}

