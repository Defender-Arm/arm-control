#include "StateManager.h"

StateManager::StateManager() {
    currentState = OFF;  // Initialize state to OFF

}

RobotState StateManager::getState() {
    return currentState;
}

void StateManager::standby() {
    currentState = STANDBY;
}

bool StateManager::calibrate() {
  if (currentState == STANDBY || currentState == READY){
        currentState = CALIBRATING;
        return true;
  }
  else{
    return false;
  }
}

bool StateManager::ready() {
    if (currentState == CALIBRATING || currentState == READY || currentState == ACTIVE){
        currentState = READY;
        return true;
    }
    else{
      return false;
    }
}

bool StateManager::active() {
  if (currentState == READY || currentState == ACTIVE){
        currentState = ACTIVE;
        return true;
  }
  else{
    return false;
  }
}

void StateManager::stop() {
    currentState = OFF;
}

bool StateManager::error(const String &msg) {
    Serial.println(msg);
    currentState = STANDBY;
    return false;
}

void StateManager::finish() {
        currentState = READY;
}
