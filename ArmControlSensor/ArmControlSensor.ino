#include "SerialComms.h"
#include "StateManager.h"
#include "MotorControl.h"

//BASE: Positive = left
//ELBOW: Positive = up
//Wrist: Positive = CW
/*
0 = off
1 = standby
  - if ever go to 1, stop in place
2 = calibration
  send 0 if good, else send 1,2,3 for each motor

3 = ready (send constantly)
  - from 4-->3, go to 000
4 = Active (recieve 4, base, elbow, wrist)
  Send error 1, 2, 3 (binary 111)

if ever get 1, 3: exit and remove queue

*/
String ACCEPTED = "000";

SerialComms serialComms;
StateManager stateManager;

// Build 3 motors with pins, speed, and gear ratios
Motor baseMotor(2, 3, 4000, 16.0/3.0, 0, 0);  // adjust with actual sensor value
Motor elbowMotor(4, 5, 1700, 5.18 * 93.0/25.0, 1, 100); // 
Motor wristMotor(6, 7, 20000, 1.0, 2, 45);



int moveToPosition(int targetBase, int targetElbow, int targetWrist) {
  // Convert degrees to steps
  float overshootFactor = 1.1;
  int baseEstimateSteps = baseMotor.calculateSteps(targetBase)*overshootFactor;
  int elbowEstimateSteps = elbowMotor.calculateSteps(targetElbow)*overshootFactor;
  int wristEstimateSteps = wristMotor.calculateSteps(targetWrist)*overshootFactor;

  // Movement loop
  int baseCounter = 0, elbowCounter = 0, wristCounter = 0;
  unsigned long lastBaseStep = micros();
  unsigned long lastElbowStep = micros();
  unsigned long lastWristStep = micros();

  // unsigned long startTime = millis();
  while (baseMotor.getAngle() != targetBase|| 
        elbowMotor.getAngle() != targetElbow || 
        wristMotor.getAngle() != targetWrist) {
    // && (millis() - startTime < 150)) {  // Run for max 0.5 seconds
      unsigned long now = micros();

      if (baseMotor.getAngle() != targetBase && (now - lastBaseStep >= baseMotor.stepDelay)) {
          baseMotor.moveMotor();
          lastBaseStep = now;
          baseCounter++;
      }

      if (elbowMotor.getAngle() != targetElbow && (now - lastElbowStep >= elbowMotor.stepDelay)) {
          elbowMotor.moveMotor();
          lastElbowStep = now;
          elbowCounter++;
      }

      if (wristMotor.getAngle() != targetWrist && (now - lastWristStep >= wristMotor.stepDelay)) {
          wristMotor.moveMotor();
          lastWristStep = now;
          wristCounter++;
      }
      if (baseCounter > baseEstimateSteps ){
        return 1;
      }
      if (elbowCounter > elbowEstimateSteps){
        return 2;
      }
      if(wristCounter > wristEstimateSteps){
        return 3;
      }
    }
    return 0;
    // baseMotor.updatePosition(baseCounter);
    // elbowMotor.updatePosition(elbowCounter);
    // wristMotor.updatePosition(wristCounter);
    // baseMotor.currentAngle = targetBase;
    // elbowMotor.currentAngle = targetElbow;
    // wristMotor.currentAngle = targetWrist;
  }

void checkMotorStatus(int motorCode){
  if(!motorCode){ //All motors moved with error range
    serialComms.writeSerial(ACCEPTED);
  }
  else{
    String errorCode = ACCEPTED;
    errorCode[motorCode - 1] = "1";
    stateManager.error(errorCode);
  }
}

bool calibration() {
    int positions[][3] = { {0, 0, 0}, {60, 45, 180}, {-60, -45, 270}, {0, 0, 0} };

    for (int i = 0; i < 4; i++) {
        int motorCode = moveToPosition(positions[i][0], positions[i][1], positions[i][2]);
        checkMotorStatus(motorCode);
        if (motorCode != 0) {
            return false;
        }
        delay(1000);
    }
    return true;
}

void setup() {
      Serial.begin(9600);  // Initialize serial communication
      delay(500);
}

void loop() {
    int state, baseAngle, elbowAngle, wristAngle;

    if (serialComms.readSerial(state, baseAngle, elbowAngle, wristAngle)) {
        // serialComms.writeSerial("State: " + String(state) + "   Base Angle: " + String(baseAngle) + 
        //                     "     Elbow Angle: " + String(elbowAngle) + "     Wrist Angle: " + String(wristAngle));
        switch (state) {
            case 0:
                stateManager.stop();
                serialComms.writeSerial(ACCEPTED);
                break;
            case 1:
                  stateManager.standby();
                  serialComms.writeSerial(ACCEPTED);
                break;
            case 2:
                if(stateManager.calibrate()){
                  if (calibration()) {
                    serialComms.writeSerial(ACCEPTED);
                  } 
                  // else {
                  //   stateManager.error("001");
                  // }
                  }
                else{
                    serialComms.writeSerial("010");

                }
                break;
            case 3:
                if(stateManager.ready()){
                  serialComms.writeSerial(ACCEPTED);
                }
                else{
                  stateManager.error("011");
                }
                break;
            case 4:
                if(stateManager.active()){
                  int motorCode = moveToPosition(baseAngle, elbowAngle, wristAngle);
                  checkMotorStatus(motorCode);
                }
                else{
                  serialComms.writeSerial("100");

                }
                break;
            default:
                serialComms.writeSerial("101");  // Invalid state code
                stateManager.standby();
                break;
        }
      }
    }