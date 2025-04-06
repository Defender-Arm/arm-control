#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>

enum RobotState {
    OFF = 0,
    STANDBY = 1,
    CALIBRATING = 2,
    READY = 3,
    ACTIVE = 4
};

class StateManager {
private:
    RobotState currentState;

public:
    StateManager();  // Constructor

    RobotState getState();
    void standby();
    bool calibrate();
    bool ready();
    bool active();
    void stop();
    bool error(const String &msg);
    void finish();
};

#endif
