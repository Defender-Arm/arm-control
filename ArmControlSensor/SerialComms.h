#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#include <Arduino.h>

class SerialComms {
public:
    SerialComms();  // Constructor

    bool readSerial(int &state, int &angBase, int &angElbow, int &angWrist);
    void writeSerial(const String &errorCode);
};

#endif
