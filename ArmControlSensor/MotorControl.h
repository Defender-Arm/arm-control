#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <Wire.h>
#include <AS5600.h>

#define PCA9548A_ADDRESS 0x70 

const int stepsPerRevolution = 200;

class Motor {
private:
    int stepPin;
public:
    int dirPin;
    int stepDelay;
    float gearRatio;
    int sensorIndex; //Mux: Base = 0, Elbow = 1, Wrist = 2
    int sensorOffset; // Base = ?, Elbow = 100, Wrist = 45
    int currentAngle;  // Tracks current motor position

    Motor(int stepPin, int dirPin, int stepDelay, float gearRatio, int sensorIndex, int sensorOffset);  // Constructor

    void moveMotor();  // Moves joint to commanded position
    void Motor::updatePosition(int stepCounter);
    int calculateSteps(int targetAngle);
    int Motor::getAngle();
};

#endif
