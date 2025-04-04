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

SerialComms serialComms;
StateManager stateManager;

// Build 3 motors with pins, speed, and gear ratios
Motor baseMotor(2, 3, 4000, 16.0/3.0);  
Motor elbowMotor(4, 5, 1700, 5.18 * 93.0/25.0);
Motor wristMotor(6, 7, 20000, 1.0);



void moveToPosition(float targetBase, float targetElbow, float targetWrist) {
  // Convert degrees to steps
  float overshootFactor = 1.1;
  int baseSteps = baseMotor.calculateSteps(targetBase)*overshootFactor;
  int elbowSteps = elbowMotor.calculateSteps(targetElbow)*overshootFactor;
  int wristSteps = wristMotor.calculateSteps(targetWrist)*overshootFactor;

  // Movement loop
  int baseCounter = 0, elbowCounter = 0, wristCounter = 0;
  unsigned long lastBaseStep = micros();
  unsigned long lastElbowStep = micros();
  unsigned long lastWristStep = micros();

  // unsigned long startTime = millis();
  while (baseCounter < baseSteps || elbowCounter < elbowSteps || wristCounter < wristSteps) {
    // && (millis() - startTime < 150)) {  // Run for max 0.5 seconds
      unsigned long now = micros();

      if (baseCounter < baseSteps && (now - lastBaseStep >= baseMotor.stepDelay)) {
          baseMotor.moveMotor();
          lastBaseStep = now;
          baseCounter++;
      }

      if (elbowCounter < elbowSteps && (now - lastElbowStep >= elbowMotor.stepDelay)) {
          elbowMotor.moveMotor();
          lastElbowStep = now;
          elbowCounter++;
      }

      if (wristCounter < wristSteps && (now - lastWristStep >= wristMotor.stepDelay)) {
          wristMotor.moveMotor();
          lastWristStep = now;
          wristCounter++;
      }
    }
    // baseMotor.updatePosition(baseCounter);
    // elbowMotor.updatePosition(elbowCounter);
    // wristMotor.updatePosition(wristCounter);
    baseMotor.currentAngle = targetBase;
    elbowMotor.currentAngle = targetElbow;
    wristMotor.currentAngle = targetWrist;


  }


bool calibration(){
  // moveToPosition(60, 45, 180);
  // delay(1000);

  // moveToPosition(-60, -45, 270);
  // delay(1000);

  // moveToPosition(0, 0, 0);

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
                serialComms.writeSerial("000");
                break;
            case 1:
                  stateManager.standby();
                  serialComms.writeSerial("000");
                break;
            case 2:
                if(stateManager.calibrate()){
                  if (calibration()) {
                    serialComms.writeSerial("000");
                  } else {
                    serialComms.writeSerial("001");
                    stateManager.standby();
                  }
                  }
                else{
                    serialComms.writeSerial("010");

                }
                break;
            case 3:
                if(stateManager.ready()){
                  serialComms.writeSerial("000");
                }
                else{
                  serialComms.writeSerial("011");
                }
                break;
            case 4:
                if(stateManager.active()){
                  serialComms.writeSerial("000");
                  moveToPosition(baseAngle, elbowAngle, wristAngle);
                }
                else{
                  serialComms.writeSerial("100");

                }
                  // if (!(success1 && success2 && success3)) {
                  //     serialComms.writeSerial("106");  // Error moving motors
                    // stateManager.standby();
                  // }
                break;
            default:
                serialComms.writeSerial("101");  // Invalid state code
                stateManager.standby();
                break;
        }
      }
    }