#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

const int stepsPerRevolution = 200;


class Motor {
private:
    int stepPin;
public:
    int dirPin;
    int stepDelay;
    float gearRatio;
    int currentAngle;  // Tracks current motor position

    Motor(int stepPin, int dirPin, int stepDelay, float gearRatio);  // Constructor

    void moveMotor();  // Moves joint to commanded position
    void Motor::updatePosition(int stepCounter);
    int calculateSteps(int targetAngle);
};

#endif
