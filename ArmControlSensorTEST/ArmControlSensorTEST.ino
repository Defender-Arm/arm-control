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
Motor baseMotor(2, 3, 3500, 16.0/3.0, 0, 193);  
Motor elbowMotor(4, 5, 1500, 5.18 * 93.0/25.0, 1, 100); 
Motor wristMotor(6, 7, 17000, 1.0, 2, 45);

int moveToPosition(float targetBase, float targetElbow, float targetWrist) {
  // Convert degrees to steps
  float overshootFactor = 1;
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
    return 0;
  }

void checkMotorStatus(int motorCode){
  if(motorCode){ // 1 motor moved outside error range
    String errorCode = "000";
    errorCode[motorCode - 1] = '1';
    stateManager.error(errorCode);
  }
}

bool calibration(){
  int positions[][3] = { {0, 0, 0}, {60, 55, 179}, {-60, -25, 90}, {0, 0, 0}, {0, 0, 0} };

    // for (int i = 0; i < 4; i++) {
    //     int motorCode = moveToPosition(positions[i][0], positions[i][1], positions[i][2]);
    //     checkMotorStatus(motorCode);
    //     if (motorCode != 0) {
    //         return false;
    //     }
    //     delay(1000);
    // }
  return true;
}

void setup() {
      Serial.begin(115200);  // Initialize serial communication
      delay(500);

      Wire.begin();

      baseMotor.init();
      elbowMotor.init();
      wristMotor.init();
      // Serial.println("Setup");
}

void loop() {
    int state, baseAngle, elbowAngle, wristAngle;
    // Serial.println("Loop");
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
                    stateManager.standby();
                  }
                  }
                else{
                    serialComms.writeSerial("110");

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
                  // serialComms.writeSerial("000");
                  // if (!motorCode){
                  //   serialComms.writeSerial("000");
                  // }
                }
                else{
                  serialComms.writeSerial("101");

                }

                break;
            default:
                serialComms.writeSerial("111");  // Invalid state code
                stateManager.standby();
                break;
        }
      }
    }