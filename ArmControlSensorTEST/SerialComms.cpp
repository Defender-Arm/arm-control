#include "SerialComms.h"

SerialComms::SerialComms() {

}

bool SerialComms::readSerial(int &state, int &angBase, int &angElbow, int &angWrist) {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');  // Read until newline
        input.trim();  // Remove whitespace

        int numValues = sscanf(input.c_str(), "%d %d %d %d", &state, &angBase, &angElbow, &angWrist);
        // Serial.println(String(state));
        if (numValues == 4) {  // Ensure valid input (state + 3 angles)
            return true;
        } else {
            writeSerial("110");  // Example: Error code "101" for invalid format
            return false;
        }
    }
    return false;  // No data available
}

void SerialComms::writeSerial(const String &errorCode) {
    Serial.println(errorCode);  // Send error code
}
